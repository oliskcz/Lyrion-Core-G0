/**
 * @file  lyrion_link_packet.h
 * @brief Lyrion Link — on-the-air packet structure and pack/unpack API.
 *
 * Wire layout (15 bytes clear + N bytes payload + 4 bytes MAC = 63 max):
 *
 *   Offset Size  Field
 *   ------ ----- ---------------------------------------------------
 *   0      1     Version (LL_VERSION)
 *   1      1     Type (ll_type_t)
 *   2      1     HopCnt (Pro only)
 *   3      1     TTL (Pro only)
 *   4      1     Flags (LL_FLAG_*)
 *   5-6    2     Dest address (big-endian)
 *   7-8    2     Src address (big-endian)
 *   9-12   4     Counter (big-endian; 32-bit, monotonically increasing)
 *   13-14  2     PacketID (big-endian; per-message identifier)
 *   ----  ----  (encrypted section follows)
 *   15..   N     ciphertext (N = 0..44)
 *   +N..   4     MAC (encrypted; 4 bytes)
 *
 * The 15-byte clear header is also the AAD for AES-CCM (authenticated
 * but not encrypted).
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef LYRION_LINK_PACKET_H
#define LYRION_LINK_PACKET_H

#include "lyrion_link_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t  version;
    uint8_t  type;
    uint8_t  hop_cnt;
    uint8_t  ttl;
    uint8_t  flags;
    uint16_t dest;
    uint16_t src;
    uint32_t counter;         /* 32-bit monotonically increasing counter */
    uint16_t packet_id;       /* 16-bit per-message identifier */
    const uint8_t *encrypted;     /* ciphertext || MAC (total length = encrypted_len) */
    uint8_t  encrypted_len;       /* total encrypted section (ciphertext + LL_MAC_SIZE) */
} ll_packet_t;

size_t ll_packet_pack(const ll_packet_t *pkt, uint8_t *out);
int    ll_packet_unpack(const uint8_t *in, size_t in_len, ll_packet_t *pkt);
void   ll_packet_get_aad(const ll_packet_t *pkt, uint8_t aad[LL_HEADER_SIZE]);

#ifdef __cplusplus
}
#endif
#endif /* LYRION_LINK_PACKET_H */
