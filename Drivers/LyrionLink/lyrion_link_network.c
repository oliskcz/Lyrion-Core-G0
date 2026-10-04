/**
 * @file  lyrion_link_network.c
 * @brief Addressing / broadcast / TTL implementation.
 *
 * On Lite this compiles to just the address-matching logic; the relay
 * path is gated behind `#if !LL_LITE_BUILD`.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#include "lyrion_link_network.h"

/* The application provides our config via ll_init(). We read the
 * current address from this extern (set in lyrion_link.c). */
extern ll_config_t g_ll_network_config;

bool ll_network_is_for_me(uint16_t dest)
{
    return (dest == g_ll_network_config.address || dest == 0xFFFF);
}

bool ll_network_should_relay(uint16_t dest, uint8_t ttl)
{
#if !LL_LITE_BUILD
    return (g_ll_network_config.mesh_enabled &&
            ttl > 0 &&
            dest != g_ll_network_config.address &&
            dest != 0xFFFF);
#else
    (void)dest;
    (void)ttl;
    return false;
#endif
}