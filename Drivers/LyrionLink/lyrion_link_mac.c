/**
 * @file  lyrion_link_mac.c
 * @brief CSMA/CA implementation: RSSI sense, LFSR backoff, retry.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#include "lyrion_link_mac.h"
#include "cc1101.h"

/* The radio instance — provided by main.c */
extern cc1101_t *cc1101_radio1;

/* Port-layer delay — provided by cc1101_port.c */
void cc1101_delay_ms(uint32_t ms);

/* ------------------------------------------------------------------ */
/*  16-bit LFSR (x^16 + x^14 + x^13 + x^11 + 1)                      */
/* ------------------------------------------------------------------ */
static uint16_t lfsr_state;

void ll_mac_init(uint32_t seed)
{
    /* Ensure the LFSR state is never 0 (would lock up). */
    lfsr_state = (seed & 0xFFFF) ? (seed & 0xFFFF) : 0xAAAA;
}

static uint16_t lfsr_next(void)
{
    uint16_t bit = ((lfsr_state >> 0) ^ (lfsr_state >> 2) ^
                    (lfsr_state >> 3) ^ (lfsr_state >> 5)) & 1;
    lfsr_state = (lfsr_state >> 1) | (bit << 15);
    return lfsr_state;
}

/* ------------------------------------------------------------------ */
/*  Channel-clear check via CC1101 RSSI status register                 */
/* ------------------------------------------------------------------ */
/* The radio must be in RX (or at least IDLE with receiver on) to
 * read a meaningful RSSI. cc1101_get_rssi() returns the cached value
 * from the last packet read, so we read the live RSSI register directly. */
static bool channel_clear(void)
{
    cc1101_t *r = cc1101_radio1;
    if (!r) return false;

    uint8_t raw = cc1101_read_reg(r, CC1101_REG_RSSI);
    int8_t rssi = ((int8_t)raw / 2) - 74;

    return (rssi < LL_MAC_RSSI_THRESHOLD);
}

/* ------------------------------------------------------------------ */
/*  CSMA/CA transmit                                                    */
/* ------------------------------------------------------------------ */
ll_status_t ll_mac_send(const uint8_t *data, size_t len)
{
    cc1101_t *r = cc1101_radio1;
    if (!r) return LL_ERR_NO_ROUTE;

    for (uint8_t attempt = 0; attempt <= LL_RETRY_COUNT; attempt++)
    {
        if (channel_clear())
        {
            cc1101_status_t st = cc1101_transmit(r, data, len, 0);
            return (st == CC1101_STATUS_OK) ? LL_OK : LL_ERR_BUSY;
        }

        /* Channel busy — random backoff */
        if (attempt < LL_RETRY_COUNT)
        {
            uint8_t slots = (uint8_t)(lfsr_next() % (uint16_t)LL_BACKOFF_SLOTS);
            for (uint8_t s = 0; s < slots; s++)
                cc1101_delay_ms(LL_BACKOFF_SLOT_MS);
        }
    }

    return LL_ERR_BUSY;
}