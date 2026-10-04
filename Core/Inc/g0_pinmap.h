/*
 * g0_pinmap.h
 *
 * Lyrion Core G0 — MCU pin map (STM32G071CBU6, UFQFPN-48).
 *
 * This header is the single source of truth for how the MCU pins are used.
 * It is mirrored by:
 *   - PINOUT.md            (documentation)
 *   - CONNECTIONS.txt      (net list)
 *   - STM32_Lyrion_Core_G0.ioc (CubeMX configuration)
 *
 * NOTE: STM32G071 has a single I2S peripheral (I2S1 on SPI1). The mic capture
 * and the speaker playback share the same I2S instance in half-duplex (PTT)
 * mode; the codec's DACIN and ADCOUT are both wired to PB5 (I2S1_SD).
 */
#ifndef G0_PINMAP_H
#define G0_PINMAP_H

/* ---- Audio (NAU88C22YG codec) -------------------------------------------- */
/* I2S1: PB3 = CK, PB0 = WS, PB5 = SD, PB4 = MCLK (12.288 MHz for 48 kHz)
 *   Codec control on I2C1 @ 0x1A; speaker on BTL LSPKOUT/RSPKOUT
 *   Switched 3.5 mm Jack In detect: PC7 (GPIO IN with pull-up, low when plugged)
 *   Switched 3.5 mm Jack Out detect: PC13 (GPIO IN with pull-up, low when plugged)
 */

/* ---- Primary Displays / sensors ------------------------------------------ */
/* I2C1: PB6 = SCL, PB7 = SDA
 *   SSD1306 OLED (4-pin header) @ 0x3C
 *   TMP102 temperature sensor @ 0x49 (or 0x48)
 *   NAU88C22YG codec control @ 0x1A
 */

/* ---- Radios (Lyrion Link modules) ---------------------------------------- */
/* Shared SPI2: PA0 = SCK, PB14 = MISO, PB15 = MOSI
 *   LL1 (radio 1): CS1 = PA8,  GDO0_1 = PA11 (EXTI11), GDO2_1 = PA12
 *   LL2 (radio 2): CS2 = PC6,  GDO0_2 = PB2 (EXTI2),   GDO2_2 = PB10
 */

/* ---- LMS (Lyrion Smart Port standard) ------------------------------------ */
/* 1x7 2.54 mm header:
 *   Pin 1: 5V
 *   Pin 2: 3.3V
 *   Pin 3: GND
 *   Pin 4: TX  (PC4, USART3_TX)
 *   Pin 5: RX  (PC5, USART3_RX)
 *   Pin 6: CS  (PC2, GPIO OUT)
 *   Pin 7: IRQ (PC3, EXTI line 3)
 */

/* ---- User interface ------------------------------------------------------ */
/* Push-to-Talk (PTT): PA1 (EXTI line 1)
 * 4-Button OLED navigation:
 *   UP:   PB8  (EXTI line 8)
 *   DOWN: PB9  (EXTI line 9)
 *   OK:   PA15 (EXTI line 15)
 *   BACK: PC14 (EXTI line 14)
 * Status indicators:
 *   LED1 = PC0, LED2 = PC1
 *   WS2812B RGB LED = PB1 (TIM3_CH4 PWM + DMA1)
 */

/* ---- Expansion Headers --------------------------------------------------- */
/* Spare UART (USART2): PA2 = TX, PA3 = RX (4-pin header with 3V3 + GND)
 * Spare I2C (I2C2):    PB13 = SCL, PB11 = SDA (4-pin header with 3V3 + GND)
 * Spare SPI (SPI2):    PA0 = SCK, PB14 = MISO, PB15 = MOSI, PC15 = CS_SPARE
 * ADC - DAC Header:    PA4 = DAC1_OUT1, PA5 = DAC1_OUT2, PA6 = ADC1_IN6, PA7 = ADC1_IN7
 * Battery fuel gauge:  PB12 (ADC1_IN16, 2:1 resistive divider from VBAT)
 */

/* ---- Console / debug / clock --------------------------------------------- */
/* USART1: PA9 = TX, PA10 = RX (CH340C USB-UART bridge)
 * SWD:    PA13 = SWDIO, PA14 = SWCLK (5-pin 2.54 mm header with 3V3, GND, NRST)
 * Clocks: PF0 = HSE_IN, PF1 = HSE_OUT (12.288 MHz audio crystal), PF2 = NRST
 */

#endif /* G0_PINMAP_H */
