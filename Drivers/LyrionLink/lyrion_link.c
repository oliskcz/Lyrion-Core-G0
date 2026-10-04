/**
 * @file  lyrion_link.c
 * @brief Lyrion Link — core: init, security, send, receive, task.
 *
 * Phase 2: AES-CCM with AAD, 15-byte clear header (counter + PacketID),
 * per-sender replay protection with large-jump detection.
 *
 * Phase 5: ACK/retransmit — sender requests ACK per-message, retransmits
 * on timeout. Runtime-configurable ack_timeout_ms and retry_count.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#include "lyrion_link.h"
#include "lyrion_link_packet.h"
#include "lyrion_link_security.h"
#include "lyrion_link_network.h"
#include "lyrion_link_mac.h"
#include "aes128.h"
#include "ccm.h"
#include "key.h"
#include "cc1101.h"
#include "cc1101_port.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/*  Debug hex dump (temporary — remove after diagnosis)                  */
/* ------------------------------------------------------------------ */
extern UART_HandleTypeDef huart1;

static void debug_hex(const char *tag, const uint8_t *data, size_t len)
{
    char buf[4];
    HAL_UART_Transmit(&huart1, (const uint8_t *)tag, strlen(tag), 100);
    HAL_UART_Transmit(&huart1, (const uint8_t *)": ", 2, 100);
    for (size_t i = 0; i < len; i++) {
        uint8_t h = data[i] >> 4, l = data[i] & 0x0F;
        buf[0] = (h < 10) ? '0'+h : 'A'+h-10;
        buf[1] = (l < 10) ? '0'+l : 'A'+l-10;
        buf[2] = ' '; buf[3] = 0;
        HAL_UART_Transmit(&huart1, (const uint8_t *)buf, 3, 100);
    }
    HAL_UART_Transmit(&huart1, (const uint8_t *)"\r\n", 2, 100);
}
#define DBG_HEX(t,d,l) debug_hex(t,d,l)

/* ------------------------------------------------------------------ */
/*  Internal state                                                      */
/* ------------------------------------------------------------------ */
static struct {
    ll_config_t   config;
    ll_callbacks_t cb;
    aes128_ctx_t  aes_ctx;
    uint32_t      tx_counter;     /* next outgoing counter */
    uint16_t      tx_packet_id;   /* next outgoing packet_id */
    bool          inited;
} g_state;

/* Exported for lyrion_link_network.c */
ll_config_t g_ll_network_config;

/* ------------------------------------------------------------------ */
/*  TX pending entry — tracks a packet waiting for ACK                  */
/* ------------------------------------------------------------------ */

static struct {
    bool     active;
    uint16_t dest;
    uint8_t  type;
    uint32_t counter;
    uint16_t packet_id;
    uint8_t  raw_packet[LL_HEADER_SIZE + LL_MAX_PAYLOAD + LL_MAC_SIZE];
    size_t   raw_len;
    uint32_t last_tx_ms;
    uint8_t  attempts;
    uint8_t  max_attempts;
    uint16_t timeout_ms;
} g_pending;

/* ------------------------------------------------------------------ */
/*  Init / callbacks                                                    */
/* ------------------------------------------------------------------ */
ll_status_t ll_init(const ll_config_t *config)
{
    if (!config) return LL_ERR_INVALID_PARAM;

    g_state.config = *config;
    g_ll_network_config = *config;
    g_state.tx_counter   = 0;
    g_state.tx_packet_id = 0;
    g_state.inited       = false;

    memset(&g_pending, 0, sizeof(g_pending));

    aes128_init(&g_state.aes_ctx, AES_KEY);
    ll_security_init();
    ll_mac_init(g_state.tx_counter);
    g_state.inited = true;
    return LL_OK;
}

void ll_set_callbacks(const ll_callbacks_t *callbacks)
{
    if (callbacks) g_state.cb = *callbacks;
}

/* ------------------------------------------------------------------ */
/*  Internal: build nonce from prefix + net_id + counter                */
/* ------------------------------------------------------------------ */
static void build_nonce(uint32_t counter, uint8_t nonce[LL_NONCE_SIZE])
{
    memcpy(nonce, AES_NONCE_PREFIX, LL_PREFIX_SIZE);
    nonce[7]  = (uint8_t)(g_state.config.network_id >> 8);
    nonce[8]  = (uint8_t)(g_state.config.network_id & 0xFF);
    nonce[9]  = (uint8_t)(counter >> 24);
    nonce[10] = (uint8_t)(counter >> 16);
    nonce[11] = (uint8_t)(counter >> 8);
    nonce[12] = (uint8_t)(counter & 0xFF);
}

/* ------------------------------------------------------------------ */
/*  Internal: encrypt and pack a full packet into raw bytes              */
/*  Returns the raw packet length (or 0 on error).                      */
/* ------------------------------------------------------------------ */
extern cc1101_t *cc1101_radio1;
extern volatile bool cc1101_rx_ready;
static size_t build_packet(uint16_t dest, uint8_t type,
                           const uint8_t *payload, size_t len,
                           bool ack, bool stream,
                           uint32_t counter, uint16_t packet_id,
                           uint8_t *out_buf, size_t out_cap)
{
    if (len > LL_MAX_PAYLOAD) return 0;
    size_t needed = LL_HEADER_SIZE + len + LL_MAC_SIZE;
    if (needed > out_cap) return 0;

    uint8_t flags = 0;
    if (ack)    flags |= LL_FLAG_ACK_REQUIRED;
    if (stream) flags |= LL_FLAG_STREAM;

    uint8_t aad[LL_HEADER_SIZE];
    aad[0]  = LL_VERSION;
    aad[1]  = type;
    aad[2]  = 0;
    aad[3]  = 8;
    aad[4]  = flags;
    aad[5]  = (uint8_t)(dest >> 8);
    aad[6]  = (uint8_t)(dest & 0xFF);
    aad[7]  = (uint8_t)(g_state.config.address >> 8);
    aad[8]  = (uint8_t)(g_state.config.address & 0xFF);
    aad[9]  = (uint8_t)(counter >> 24);
    aad[10] = (uint8_t)(counter >> 16);
    aad[11] = (uint8_t)(counter >> 8);
    aad[12] = (uint8_t)(counter & 0xFF);
    aad[13] = (uint8_t)(packet_id >> 8);
    aad[14] = (uint8_t)(packet_id & 0xFF);

    uint8_t nonce[LL_NONCE_SIZE];
    build_nonce(counter, nonce);

    uint8_t encrypted[LL_MAX_PAYLOAD + LL_MAC_SIZE];
    size_t ct_len = ccm_encode(&g_state.aes_ctx, nonce,
                               aad, LL_HEADER_SIZE,
                               payload, len,
                               encrypted);
    if (ct_len == 0) return 0;

    uint8_t *p = out_buf;
    memcpy(p, aad, LL_HEADER_SIZE); p += LL_HEADER_SIZE;
    memcpy(p, encrypted, ct_len);   p += ct_len;
    return (size_t)(p - out_buf);
}

/* ------------------------------------------------------------------ */
/*  Internal: send a raw packet (blocking CSMA/CA)                       */
/* ------------------------------------------------------------------ */
static ll_status_t do_send(uint16_t dest, uint8_t type, const uint8_t *payload,
                          size_t len, bool ack, bool stream)
{
    if (!g_state.inited) return LL_ERR_INVALID_PARAM;
    if (len > LL_MAX_PAYLOAD) return LL_ERR_INVALID_PARAM;

    uint32_t counter   = g_state.tx_counter;
    uint16_t packet_id = g_state.tx_packet_id;

    uint8_t raw[LL_HEADER_SIZE + LL_MAX_PAYLOAD + LL_MAC_SIZE];
    size_t raw_len = build_packet(dest, type, payload, len,
                                  ack, stream,
                                  counter, packet_id,
                                  raw, sizeof(raw));
    if (raw_len == 0) return LL_ERR_INVALID_PARAM;

    /* Advance state — must happen BEFORE any retransmit logic uses counter */
    g_state.tx_counter++;
    g_state.tx_packet_id++;

    /* Transmit with CSMA/CA */
    ll_status_t st = ll_mac_send(raw, raw_len);

    /* Re-enter RX mode */
    cc1101_start_receive(cc1101_radio1, 0);

    if (st != LL_OK) {
        /* Transmit failed — immediate failure regardless of ack */
        return st;
    }

    if (ack) {
        /* Save to pending queue for ACK tracking */
        g_pending.active       = true;
        g_pending.dest         = dest;
        g_pending.type         = type;
        g_pending.counter      = counter;
        g_pending.packet_id    = packet_id;
        memcpy(g_pending.raw_packet, raw, raw_len);
        g_pending.raw_len      = raw_len;
        g_pending.last_tx_ms   = cc1101_millis();
        g_pending.attempts     = 1;
        g_pending.max_attempts = (g_state.config.retry_count > 0)
                                 ? g_state.config.retry_count : LL_RETRY_COUNT;
        g_pending.timeout_ms   = (g_state.config.ack_timeout_ms > 0)
                                 ? g_state.config.ack_timeout_ms : LL_ACK_TIMEOUT_MS;
    }

    return st;
}

ll_status_t ll_send_raw(uint16_t dest, uint8_t type,
                        const uint8_t *payload, size_t len, bool ack)
{
    if (!g_state.inited) return LL_ERR_INVALID_PARAM;
    if (len > LL_MAX_PAYLOAD) return LL_ERR_INVALID_PARAM;

    ll_status_t st = do_send(dest, type, payload, len, ack, false);

    if (!ack && g_state.cb.on_send_complete)
        g_state.cb.on_send_complete(dest, type, st);

    return st;
}

ll_status_t ll_send_text(uint16_t dest, const char *text, size_t len, bool ack)
{
    if (!g_state.inited) return LL_ERR_INVALID_PARAM;
    if (len > LL_MAX_PAYLOAD) return LL_ERR_INVALID_PARAM;

    ll_status_t st = ll_send_raw(dest, LL_TYPE_TEXT,
                                 (const uint8_t *)text, len, ack);
    return st;
}

ll_status_t ll_send_command(uint16_t dest, const char *json_cmd,
                            const char *req_id)
{
    (void)req_id;
    if (!g_state.inited) return LL_ERR_INVALID_PARAM;
    if (req_id && strlen(req_id) > 16) return LL_ERR_INVALID_PARAM;

    ll_status_t st = do_send(dest, LL_TYPE_COMMAND,
                             (const uint8_t *)json_cmd, strlen(json_cmd),
                             true, false);
    return st;
}

ll_status_t ll_send_response(uint16_t dest, const char *json_resp,
                             const char *req_id)
{
    (void)req_id;
    if (!g_state.inited) return LL_ERR_INVALID_PARAM;
    if (req_id && strlen(req_id) > 16) return LL_ERR_INVALID_PARAM;

    ll_status_t st = do_send(dest, LL_TYPE_RESPONSE,
                             (const uint8_t *)json_resp, strlen(json_resp),
                             true, false);
    return st;
}

ll_status_t ll_send_beacon(void)
{
    const char *beacon = "{\"name\":\"node\",\"version\":1}";
    ll_status_t st = do_send(0xFFFF, LL_TYPE_BEACON,
                             (const uint8_t *)beacon, strlen(beacon),
                             false, false);
    return st;
}

/* ------------------------------------------------------------------ */
/*  Internal: send an ACK packet to acknowledge receipt                  */
/* ------------------------------------------------------------------ */
static void send_ack(uint16_t dest, uint32_t counter, uint16_t packet_id)
{
    uint8_t payload[6];
    payload[0] = (uint8_t)(counter >> 24);
    payload[1] = (uint8_t)(counter >> 16);
    payload[2] = (uint8_t)(counter >> 8);
    payload[3] = (uint8_t)(counter & 0xFF);
    payload[4] = (uint8_t)(packet_id >> 8);
    payload[5] = (uint8_t)(packet_id & 0xFF);

    do_send(dest, LL_TYPE_ACK, payload, sizeof(payload), false, false);
}

/* ------------------------------------------------------------------ */
/*  Internal: retransmit the saved packet (same raw bytes)               */
/* ------------------------------------------------------------------ */
static void retransmit_pending(void)
{
    cc1101_t *r = cc1101_radio1;
    if (!r) return;

    cc1101_status_t st = cc1101_transmit(r, g_pending.raw_packet,
                                          g_pending.raw_len, 0);
    if (st != CC1101_STATUS_OK) return;

    cc1101_start_receive(r, 0);
    g_pending.last_tx_ms = cc1101_millis();
    g_pending.attempts++;
}

/* ------------------------------------------------------------------ */
/*  Task: check for received packet + retransmit timeout                 */
/* ------------------------------------------------------------------ */
void ll_task(void)
{
    if (!g_state.inited) return;

    /* ---- RX path ---- */
    if (cc1101_rx_ready) {
        cc1101_rx_ready = false;

        cc1101_t *r = cc1101_radio1;
        if (!r) return;

        uint8_t raw[64];
        size_t  raw_len = 0;
        cc1101_status_t st = cc1101_read_data(r, raw, sizeof(raw), &raw_len);
        if (st != CC1101_STATUS_OK || raw_len < LL_HEADER_SIZE + LL_MAC_SIZE) {
            goto rx_done;
        }

        ll_packet_t pkt;
        if (ll_packet_unpack(raw, raw_len, &pkt) != 0)
            goto rx_done;

        /* ---- Reboot flag: reset sender state and accept unconditionally ---- */
        if (pkt.flags & LL_FLAG_RESET) {
            ll_security_reset(pkt.src);
        } else {
            ll_status_t sec = ll_security_check(pkt.src, pkt.counter, pkt.packet_id);
            if (sec == LL_ERR_REPLAY) {
                goto rx_done;
            }
            if (sec == LL_ERR_DUPLICATE) {
                /* This is a retransmit of a packet we already received.
                 * Re-send the ACK if the sender asks for one, then drop. */
                if ((pkt.flags & LL_FLAG_ACK_REQUIRED) &&
                    pkt.dest != 0xFFFF &&
                    ll_network_is_for_me(pkt.dest)) {
                    send_ack(pkt.src, pkt.counter, pkt.packet_id);
                }
                ll_security_record(pkt.src, pkt.counter, pkt.packet_id);
                goto rx_done;
            }
        }

        /* Build AAD and nonce */
        uint8_t aad[LL_HEADER_SIZE];
        ll_packet_get_aad(&pkt, aad);

        uint8_t nonce[LL_NONCE_SIZE];
        build_nonce(pkt.counter, nonce);

        /* Decrypt */
        uint8_t plaintext[LL_MAX_PAYLOAD + 1];
        int pt_len = ccm_decode(&g_state.aes_ctx, nonce,
                                aad, LL_HEADER_SIZE,
                                pkt.encrypted, pkt.encrypted_len,
                                plaintext);
        if (pt_len < 0) {
            DBG_HEX("RAW", raw, raw_len < 20 ? raw_len : 20);
            DBG_HEX("NONCE", nonce, LL_NONCE_SIZE);
            DBG_HEX("AAD", aad, LL_HEADER_SIZE);
            if (g_state.cb.on_text_received)
                g_state.cb.on_text_received(pkt.src, "MAC FAIL", 9,
                                            cc1101_get_rssi(r), cc1101_get_lqi(r));
            goto rx_done;
        }
        plaintext[pt_len] = '\0';

        /* Record this packet as accepted */
        ll_security_record(pkt.src, pkt.counter, pkt.packet_id);

        /* Network: drop if not addressed to us */
        if (!ll_network_is_for_me(pkt.dest))
            goto rx_done;

        /* ---- Auto-ACK: send ACK before dispatch (fastest response) ---- */
        if ((pkt.flags & LL_FLAG_ACK_REQUIRED) &&
            pkt.dest != 0xFFFF &&
            pkt.type != LL_TYPE_ACK &&
            pkt.type != LL_TYPE_NACK) {
            send_ack(pkt.src, pkt.counter, pkt.packet_id);
        }

        int8_t  rssi = cc1101_get_rssi(r);
        uint8_t lqi  = cc1101_get_lqi(r);

        switch (pkt.type) {
        case LL_TYPE_TEXT:
            if (g_state.cb.on_text_received)
                g_state.cb.on_text_received(pkt.src, (const char *)plaintext,
                                            (size_t)pt_len, rssi, lqi);
            break;
        case LL_TYPE_BEACON:
            if (g_state.cb.on_beacon_received)
                g_state.cb.on_beacon_received(pkt.src, (const char *)plaintext,
                                              (size_t)pt_len, rssi);
            break;
        case LL_TYPE_COMMAND:
            if (g_state.cb.on_command_received)
                g_state.cb.on_command_received(pkt.src, (const char *)plaintext,
                                               (size_t)pt_len, NULL);
            break;
        case LL_TYPE_RESPONSE:
            if (g_state.cb.on_response_received)
                g_state.cb.on_response_received(pkt.src, (const char *)plaintext,
                                                (size_t)pt_len);
            break;
        case LL_TYPE_ACK:
            /* Match ACK against pending outgoing packet */
            if (pt_len >= 6 && g_pending.active) {
                uint32_t ack_counter = ((uint32_t)plaintext[0] << 24) |
                                       ((uint32_t)plaintext[1] << 16) |
                                       ((uint32_t)plaintext[2] << 8) |
                                        (uint32_t)plaintext[3];
                uint16_t ack_pkt_id  = ((uint16_t)plaintext[4] << 8) |
                                        (uint16_t)plaintext[5];
                if (g_pending.counter == ack_counter &&
                    g_pending.packet_id == ack_pkt_id) {
                    if (g_state.cb.on_send_complete)
                        g_state.cb.on_send_complete(g_pending.dest,
                                                     g_pending.type, LL_OK);
                    g_pending.active = false;
                }
            }
            break;
        case LL_TYPE_NACK:
            /* NACK — could trigger immediate retransmit, but for now
             * treat same as ACK timeout: let the timeout handle it. */
            break;
        default:
            break;
        }

rx_done:
        cc1101_start_receive(r, 0);
    }

    /* ---- Retransmit / timeout check ---- */
    if (g_pending.active) {
        uint32_t now = cc1101_millis();
        if (now - g_pending.last_tx_ms >= g_pending.timeout_ms) {
            if (g_pending.attempts < g_pending.max_attempts) {
                retransmit_pending();
            } else {
                if (g_state.cb.on_send_complete)
                    g_state.cb.on_send_complete(g_pending.dest,
                                                 g_pending.type, LL_ERR_TIMEOUT);
                g_pending.active = false;
            }
        }
    }
}
