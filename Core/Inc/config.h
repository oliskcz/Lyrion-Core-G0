/*
 * config.h
 *
 *  Created on: 4. 10. 2026
 *      Author: Oliver Zoller
 */
#pragma once

// ===== SETTINGS =====
#define ENABLE_UART1   1
#define ENABLE_WS2812  1
#define ENABLE_OLED    1
#define ENABLE_I2C     1
#define ENABLE_TMP102  1
#define ENABLE_SPI     1   /* SPI2 for the CC1101 radios (SPI1 is used by I2S) */
#define ENABLE_I2C_SCAN 0
#define ENABLE_CC1101  1
#define ENABLE_AES     1   /* 1 = AES-128-CCM encrypt/decrypt, 0 = plaintext */
#define UART_DEBUG      1

/* ===== Audio (NAU88C22YG codec + I2S1) ===== */
#define ENABLE_AUDIO   1   /* audio codec + I2S driver */
#define ENABLE_CODEC   1   /* NAU88C22 codec driver */
#define ENABLE_HP_JACK 1   /* headphone / line output on the 3.5 mm jack */
#define AUDIO_SAMPLE_RATE_HZ 48000
#define AUDIO_MCLK_HZ       12288000  /* 256 x 48000, generated from PLL I2S */

/* ===== CC1101 driver (ported from mfurga Arduino library) =====
 * Each CC1101 module can be enabled/disabled independently:
 *   CC1101_ENABLE_RADIO1 -> J1 module (CS1=PA8,  GDO0_1=PA11, GDO2_1=PA12)
 *   CC1101_ENABLE_RADIO2 -> J3 module (CS2=PC6,  GDO0_2=PB2,  GDO2_2=PB10)
 * Both share SPI2. When both are enabled the examples self-test via loopback.
 */
#define CC1101_ENABLE_RADIO1  1   /* 1 = use J1 module, 0 = disabled */
#define CC1101_ENABLE_RADIO2  1   /* 1 = use J3 module, 0 = disabled */

/* CC1101 reference crystal frequency in MHz (verify on your module). */
#ifndef CC1101_CRYSTAL_FREQ
#define CC1101_CRYSTAL_FREQ   26
#endif

/* Oled Display Height */
//#define SSD1306_HEIGHT 32

/* ===== OLED FONT SETTINGS ===== */
#define SSD1306_INCLUDE_FONT_6x8	1
#define SSD1306_INCLUDE_FONT_7x10	0
#define SSD1306_INCLUDE_FONT_11x18	0
#define SSD1306_INCLUDE_FONT_16x26	0
#define SSD1306_INCLUDE_FONT_16x24	0
#define SSD1306_INCLUDE_FONT_16x15	0

/* Network identity (Lyrion Link protocol). Must match on every node. */
#define LL_NETWORK_ID   0x0001

/* Lyrion Link node address (0x0001-0xFFFE; 0xFFFF is broadcast). */
#define LL_NODE_ADDRESS     0x0001

/* Remote node address (destination for button/UART messages). */
#ifndef LL_REMOTE_ADDRESS
#define LL_REMOTE_ADDRESS   0x0000
#endif

/* Lyrion Link band (0=315, 1=433, 2=868, 3=915 MHz). */
#define LL_BAND 1

/* Lyrion Link build tier: this board is the G071 Pro node. */
#define LL_LITE_BUILD  0
#define LL_PRO_BUILD   1
