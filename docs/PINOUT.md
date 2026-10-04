# Lyrion Core G0 — Pinout

**Board:** Lyrion Core G0 Rev A (planning)
**MCU:** STM32G071CBU6 (UFQFPN-48)
**Source of truth:** [`Core/Inc/g0_pinmap.h`](../Core/Inc/g0_pinmap.h) — this document mirrors it; update both together.

> Physical package pin numbers are intentionally omitted (UFQFPN-48 numbering to be taken from the STM32G071 datasheet during schematic capture).

---

## 1. MCU pin map

### Port A

| Pin | Net | Peripheral / function | Direction | Notes |
|-----|-----|----------------------|-----------|-------|
| PA0 | SPI2_SCK | SPI2 clock | I/O | Shared by J1 + J3 |
| PA1 | PTTButton | GPIO EXTI (line 1) | Input | Push-to-talk, `EXTI0_1_IRQn` |
| PA2 | — | Free | — | Freed when BTN2 moved to PA5 (line 2 is shared with GDO0_2/PB2) |
| PA3 | — | Free | — | Do not use for EXTI — line 3 is used by M1_IRQ (PC3) |
| PA4 | ADC1_IN4 | ADC (spare) | Analog | Spare analog input, also DAC_OUT1; not used by the application |
| PA5 | BTN2 | GPIO EXTI (line 5) | Input | Second user button, `EXTI4_15_IRQn` |
| PA6–PA7 | — | Free | — | SPI1/I2S1 alternates unused |
| PA8 | CS1 | GPIO output | Output | J1 chip select (active low) |
| PA9 | USART1_TX | USART1 TX | Output | CH340 RX |
| PA10 | USART1_RX | USART1 RX | Input | CH340 TX |
| PA11 | GDO0_1 | GPIO EXTI (line 11) | Input | J1 CC1101 RX-done / IRQ |
| PA12 | GDO2_1 | GPIO EXTI (line 12) | Input | J1 CC1101 sync / TX-done |
| PA13 | SWDIO | SWD | I/O | Debug |
| PA14 | SWCLK / BOOT0 | SWD / boot strap | Input | Debug; BOOT0 is sampled at reset on this shared pin |
| PA15 | — | Free | — | JTDI (SWD-only ok) |

### Port B

| Pin | Net | Peripheral / function | Direction | Notes |
|-----|-----|----------------------|-----------|-------|
| PB0 | I2S1_WS | I2S1 word select | I/O | Codec FS/LRC |
| PB1 | WS2812B | TIM3_CH4 + DMA1 | Output | RGB status LED |
| PB2 | GDO0_2 | GPIO EXTI (line 2) | Input | J3 CC1101 RX-done — **conflicts with BTN2 (PA2)** |
| PB3 | I2S1_CK | I2S1 bit clock | I/O | Also SWO — SWO trace unavailable |
| PB4 | I2S1_MCK | I2S1 master clock | Output | 12.288 MHz to codec |
| PB5 | I2S1_SD | I2S1 serial data | I/O | Shared DACIN (TX) / ADCOUT (RX), half-duplex |
| PB6 | I2C1_SCL | I²C1 clock | I/O | OLED, TMP102, NAU88C22 |
| PB7 | I2C1_SDA | I²C1 data | I/O | OLED, TMP102, NAU88C22 |
| PB8 | — | Free | — | |
| PB9 | — | Free | — | |
| PB10 | GDO2_2 | GPIO EXTI (line 10) | Input | J3 CC1101 sync / TX-done |
| PB11 | — | Free | — | Alternative GDO0_2 home |
| PB12 | — | Free | — | |
| PB13 | — | Free | — | |
| PB14 | SPI2_MISO | SPI2 data in | Input | CC1101 SO / CHIP_RDYn poll |
| PB15 | SPI2_MOSI | SPI2 data out | Output | CC1101 SI |

### Port C / F

| Pin | Net | Peripheral / function | Direction | Notes |
|-----|-----|----------------------|-----------|-------|
| PC0 | LED1 | GPIO output | Output | Status LED |
| PC1 | LED2 | GPIO output | Output | Status LED |
| PC2 | CS3 | GPIO output | Output | M1 port chip select |
| PC3 | M1_IRQ | GPIO EXTI (line 3) | Input | M1 module interrupt — **shares `EXTI2_3_IRQn` with BTN2/GDO0_2 lines but is its own line 3** |
| PC4 | USART3_TX | USART3 TX | Output | M1 module RX |
| PC5 | USART3_RX | USART3 RX | Input | M1 module TX |
| PC6 | CS2 | GPIO output | Output | J3 chip select |
| PC7 | — | Free | — | |
| PC13–PC15 | — | Free | — | |
| PF0 / PF1 | HSE_IN / HSE_OUT | HSE crystal | — | 12.288 MHz audio crystal (UNVERIFIED) |
| PF2 | NRST | Reset | Input | PF2 = NRST on STM32G071; CH340 auto-download drives NRST + PA14/BOOT0 (UNVERIFIED) |

## 2. Peripheral map

| Peripheral | Pins | Purpose |
|-----------|------|---------|
| I2S1 | PB0 (WS), PB3 (CK), PB4 (MCK), PB5 (SD) | NAU88C22 codec, half-duplex |
| I2C1 | PB6 (SCL), PB7 (SDA) | OLED 0x3C, TMP102 0x49, codec 0x1A |
| SPI2 | PA0 (SCK), PB14 (MISO), PB15 (MOSI) | J1 + J3 CC1101 sockets |
| USART1 | PA9 (TX), PA10 (RX) | CH340 console + ROM bootloader |
| USART3 | PC4 (TX), PC5 (RX) | M1 module port |
| TIM3_CH4 + DMA1 | PB1 | WS2812B |
| EXTI | PA1, PA2, PC3, PA11, PA12, PB2, PB10 | Buttons + radio GDO + M1 IRQ |

## 3. Connectors

### J1 — raw CC1101 socket (J1 module)

| Pin | Signal | MCU |
|-----|--------|-----|
| 1 | VCC 3V3 | 3V3 rail |
| 2 | GND | GND |
| 3 | SCK | PA0 |
| 4 | MISO (SO) | PB14 |
| 5 | MOSI (SI) | PB15 |
| 6 | CSN | PA8 |
| 7 | GDO0 | PA11 |
| 8 | GDO2 | PA12 |

### J3 — raw CC1101 socket (J3 module)

| Pin | Signal | MCU |
|-----|--------|-----|
| 1 | VCC 3V3 | 3V3 rail |
| 2 | GND | GND |
| 3 | SCK | PA0 (shared) |
| 4 | MISO (SO) | PB14 (shared) |
| 5 | MOSI (SI) | PB15 (shared) |
| 6 | CSN | PC6 |
| 7 | GDO0 | PB2 |
| 8 | GDO2 | PB10 |

> **UNVERIFIED:** physical pin order must match the Core C0 J1/J3 sockets (copy from the C0 Altium schematic) so existing modules plug in unchanged.

### M1 — smart module port

| Pin | Signal | MCU |
|-----|--------|-----|
| 1 | VCC 3V3 | 3V3 rail |
| 2 | GND | GND |
| 3 | CS | PC2 |
| 4 | IRQ | PC3 |
| 5 | UART TX (host → module) | PC4 |
| 6 | UART RX (module → host) | PC5 |

> **UNVERIFIED:** confirm against the Lyrion M1 module archive (`LyrionMIVTwo.zip`) before layout.

### Other headers

| Connector | Pins | Notes |
|-----------|------|-------|
| OLED | VCC, GND, SCL, SDA | 4-pin I²C module header |
| SWD | 3V3, SWDIO, SWCLK, NRST, GND | 5-pin debug header |
| UART (optional) | 3V3, TX, RX, GND | Mirrors CH340 / USART1 |
| Speaker | SPK+, SPK− | BTL output, 2-pin |
| USB-C | VBUS, GND, CC1/CC2 (5.1 kΩ), D+/D− → CH340 | Power + serial |
| Headphone | Tip/Ring/Sleeve | 3.5 mm stereo jack |

## 4. Conflicts and constraints

1. **EXTI line 2 (resolved):** EXTI lines follow the pin number, so PA2 (BTN2) and PB2 (GDO0_2) originally shared line 2. BTN2 is now on **PA5** (line 5) in `main.h`/`g0_pinmap.h`; PA3 (line 3) is taken by M1_IRQ and PB11 (line 11) by GDO0_1, so neither was a valid alternative. Keep the CubeMX `.ioc` in sync.
2. **PB3 (I2S1_CK)** is the SWO pin — SWD debugging works, SWO tracing is unavailable.
3. **I2S1_SD (PB5)** is driven by either the MCU or the codec depending on `audio_set_mode()` — never both.
4. **SPI2 is shared** by J1 and J3; firmware must raise only one CS at a time.
5. **PB14 must be readable as GPIO** for the CC1101 CHIP_RDYn poll before the first SPI byte.

## 5. I²C address map

| Device | Address (7-bit) |
|--------|-----------------|
| SSD1306 OLED | 0x3C |
| TMP102 | 0x49 |
| NAU88C22 codec | 0x1A |
