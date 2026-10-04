/**
 * @file  lyrion_link_mac.h
 * @brief Lyrion Link — CSMA/CA MAC layer.
 *
 * Before each transmission the radio checks whether the channel is clear
 * by sampling the CC1101 RSSI. If the channel is busy it backs off for a
 * random number of slots and retries, up to LL_RETRY_COUNT attempts.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef LYRION_LINK_MAC_H
#define LYRION_LINK_MAC_H

#include "lyrion_link_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialise the MAC layer (seed the LFSR random generator).
 * @param  seed  Initial seed (use tx_counter from the link layer).
 */
void ll_mac_init(uint32_t seed);

/**
 * @brief  Attempt to send a packet with CSMA/CA.
 *         Listens on the channel before each attempt. If busy, waits a
 *         random backoff (LL_BACKOFF_SLOTS × LL_BACKOFF_SLOT_MS) and
 *         retries. Returns LL_ERR_BUSY after LL_RETRY_COUNT failures.
 * @param  data  Raw packet (clear header + ciphertext + MAC) to transmit.
 * @param  len   Total packet length.
 * @return LL_OK on success, LL_ERR_BUSY if channel stays busy.
 */
ll_status_t ll_mac_send(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* LYRION_LINK_MAC_H */