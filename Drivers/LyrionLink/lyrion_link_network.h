/**
 * @file  lyrion_link_network.h
 * @brief Lyrion Link — addressing, broadcast filtering, TTL.
 *
 * Lite build (`LL_LITE_BUILD=1`): unicast/broadcast check only, no relay.
 * Pro build  (`LL_LITE_BUILD=0`): mesh relay with TTL decrement.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef LYRION_LINK_NETWORK_H
#define LYRION_LINK_NETWORK_H

#include "lyrion_link_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Check whether a packet is addressed to this node.
 * @param  dest  Destination address from the packet header.
 * @return true if the packet should be processed locally
 *         (dest == config.address || dest == 0xFFFF).
 */
bool ll_network_is_for_me(uint16_t dest);

/**
 * @brief  Check whether this node should relay the packet (Pro only).
 *         Returns false on Lite builds.
 * @param  dest  Destination address.
 * @param  ttl   Time-to-live from the packet header.
 * @return true if mesh is enabled, dest is not us, not broadcast, and TTL > 0.
 */
bool ll_network_should_relay(uint16_t dest, uint8_t ttl);

#ifdef __cplusplus
}
#endif
#endif /* LYRION_LINK_NETWORK_H */