/**
 * @file  lyrion_link_security.h
 * @brief Lyrion Link — per-sender replay protection + duplicate detection.
 *
 * Tracks up to LL_MAX_NODES senders. Each sender has a 32-bit counter
 * and a PacketID ring buffer. The receiver accepts a packet if:
 *   - sender is new (not yet tracked), or
 *   - counter > last_counter (normal new packet), or
 *   - counter == last_counter and packet_id NOT in ring (rare: fresh
 *     PacketID with same counter after a long delay), or
 *   - counter < last_counter but the gap exceeds LARGE_JUMP_THRESHOLD
 *     (treats as sender reboot).
 * Rejects as replay if counter < last_counter with small gap.
 * Rejects as duplicate if counter == last_counter and packet_id in ring.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef LYRION_LINK_SECURITY_H
#define LYRION_LINK_SECURITY_H

#include "lyrion_link_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Per-sender security state.
 */
typedef struct {
    uint16_t node_id;                        /* sender address */
    uint32_t last_counter;                   /* highest counter seen */
    uint16_t pkt_id_ring[LL_PKTID_RING_SIZE];/* last N packet IDs */
    uint8_t  pkt_id_ring_head;               /* next slot to overwrite */
    bool     seen;                           /* false until first packet */
} ll_sender_state_t;

/**
 * @brief  Initialise the security module (resets all sender states).
 */
void ll_security_init(void);

/**
 * @brief  Check whether a packet passes replay / duplicate detection.
 * @param  src       Sender address.
 * @param  counter   32-bit message counter from the packet.
 * @param  packet_id 16-bit per-message identifier.
 * @return LL_OK, LL_ERR_DUPLICATE, or LL_ERR_REPLAY.
 *
 * Does NOT modify state. Call ll_security_record() after accepting
 * (LL_OK or LL_ERR_DUPLICATE). Do NOT call record for LL_ERR_REPLAY.
 */
ll_status_t ll_security_check(uint16_t src, uint32_t counter, uint16_t packet_id);

/**
 * @brief  Record an accepted packet (update counter + push PacketID into ring).
 * @param  src       Sender address.
 * @param  counter   Accepted message counter.
 * @param  packet_id Accepted packet identifier.
 */
void ll_security_record(uint16_t src, uint32_t counter, uint16_t packet_id);

/**
 * @brief  Reset a sender's security state (call when LL_FLAG_RESET is set).
 *         Clears the sender entry so the next packet is accepted as new.
 * @param  src  Sender address.
 */
void ll_security_reset(uint16_t src);

#ifdef __cplusplus
}
#endif
#endif /* LYRION_LINK_SECURITY_H */