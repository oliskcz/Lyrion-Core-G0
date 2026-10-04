/**
 * @file  lyrion_link_security.c
 * @brief Per-sender replay map with Option A large-jump detection.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#include "lyrion_link_security.h"
#include <string.h>

static ll_sender_state_t g_senders[LL_MAX_NODES];
static uint8_t           g_sender_count;

void ll_security_init(void)
{
    memset(g_senders, 0, sizeof(g_senders));
    g_sender_count = 0;
}

static ll_sender_state_t *find_or_alloc(uint16_t src)
{
    /* Linear scan — LL_MAX_NODES is small (32). */
    for (uint8_t i = 0; i < g_sender_count; i++) {
        if (g_senders[i].node_id == src)
            return &g_senders[i];
    }

    /* Not found — allocate a new slot. */
    if (g_sender_count < LL_MAX_NODES) {
        ll_sender_state_t *s = &g_senders[g_sender_count++];
        memset(s, 0, sizeof(*s));
        s->node_id = src;
        return s;
    }

    /* Map full — evict the oldest entry by last_counter. */
    uint8_t oldest = 0;
    for (uint8_t i = 1; i < LL_MAX_NODES; i++) {
        if (g_senders[i].last_counter < g_senders[oldest].last_counter)
            oldest = i;
    }
    memset(&g_senders[oldest], 0, sizeof(g_senders[oldest]));
    g_senders[oldest].node_id = src;
    return &g_senders[oldest];
}

static bool pkt_id_in_ring(const ll_sender_state_t *s, uint16_t pkt_id)
{
    for (uint8_t i = 0; i < LL_PKTID_RING_SIZE; i++) {
        if (s->pkt_id_ring[i] == pkt_id)
            return true;
    }
    return false;
}

/* ------------------------------------------------------------------ */

ll_status_t ll_security_check(uint16_t src, uint32_t counter, uint16_t packet_id)
{
    ll_sender_state_t *s = NULL;

    /* Find this sender in the map. */
    for (uint8_t i = 0; i < g_sender_count; i++) {
        if (g_senders[i].node_id == src) {
            s = &g_senders[i];
            break;
        }
    }

    /* New sender — always accept. */
    if (!s || !s->seen)
        return LL_OK;

    if (counter > s->last_counter)
        return LL_OK;                       /* normal new packet */

    if (counter == s->last_counter) {
        if (pkt_id_in_ring(s, packet_id))
            return LL_ERR_DUPLICATE;        /* retransmit of same packet */
        return LL_OK;                       /* rare: counter same, new PacketID */
    }

    /* counter < last_counter */
    if ((s->last_counter - counter) > LL_LARGE_JUMP_THRESHOLD)
        return LL_OK;                       /* sender rebooted */

    return LL_ERR_REPLAY;                   /* small gap = replay attack */
}

void ll_security_record(uint16_t src, uint32_t counter, uint16_t packet_id)
{
    ll_sender_state_t *s = find_or_alloc(src);

    s->last_counter = counter;
    s->seen = true;

    /* Push packet_id into the ring (overwrite oldest). */
    s->pkt_id_ring[s->pkt_id_ring_head] = packet_id;
    s->pkt_id_ring_head = (s->pkt_id_ring_head + 1) % LL_PKTID_RING_SIZE;
}

void ll_security_reset(uint16_t src)
{
    for (uint8_t i = 0; i < g_sender_count; i++) {
        if (g_senders[i].node_id == src) {
            memset(&g_senders[i], 0, sizeof(g_senders[i]));
            /* Compact the array by shifting if this wasn't the last entry. */
            uint8_t last = g_sender_count - 1;
            if (i < last) {
                g_senders[i] = g_senders[last];
                memset(&g_senders[last], 0, sizeof(g_senders[last]));
            }
            g_sender_count--;
            return;
        }
    }
}