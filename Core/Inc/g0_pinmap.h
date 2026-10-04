/*
 * g0_pinmap.h
 *
 * Lyrion Core G0 — MCU pin map (STM32G071CBU6, UFQFPN-48).
 *
 * This header is the single source of truth for how the MCU pins are used.
 * It is mirrored by:
 *   - PINOUT.md            (documentation)
 *   - STM32_Lyrion_Core_G0.ioc (CubeMX configuration)
 *
 * NOTE: STM32G071 has a single I2S peripheral (I2S1 on SPI1). The mic capture
 * and the speaker playback share the same I2S instance in half-duplex (PTT)
 * mode; the codec's DACIN and ADCOUT are both wired to PB5 (I2S1_SD).
 */
#ifndef G0_PINMAP_H
#define G0_PINMAP_H

/* ---- Displays / sensors -------------------------------------------------- */
/* I2C1: PB6 = SCL, PB7 = SDA
 *   SSD1306 OLED @ 0x3C, TMP102 @ 0x49, NAU88C22 @ 0x1A */

/* ---- Radios (Lyrion Link modules) ---------------------------------------- */
/* SPI2: PA0 = SCK, PB14 = MISO, PB15 = MOSI (shared by J1 + J3)
 *   J1 (radio 1): CS1 = PA8,  GDO0_1 = PA11, GDO2_1 = PA12
 *   J3 (radio 2): CS2 = PC6,  GDO0_2 = PB2,  GDO2_2 = PB10
 *   M1 module port: CS3 = PC2, IRQ = PC3, UART (USART3) = PC4/PC5
 *   (no spare GPIO on the M1 port in Rev A; PC0/PC1 are LED1/LED2)
 */

/* ---- Audio (NAU88C22YG codec) -------------------------------------------- */
/* I2S1: PB3 = CK, PB0 = WS, PB5 = SD, PB4 = MCLK (12.288 MHz for 48 kHz)
 *   codec control on I2C1 @ 0x1A; speaker on BTL LSPKOUT/RSPKOUT */

/* ---- User interface ------------------------------------------------------ */
/* PTTButton = PA1 (EXTI line 1), BTN2 = PA5 (EXTI line 5)
 *   LED1 = PC0, LED2 = PC1
 *   WS2812B data = PB1 (TIM3_CH4 + DMA1_Ch1 update request) */

/* ---- Console / debug ----------------------------------------------------- */
/* USART1: PA9 = TX, PA10 = RX (CH340 + ROM bootloader)
 *   SWD: PA13 = SWDIO, PA14 = SWCLK */

#endif /* G0_PINMAP_H */
