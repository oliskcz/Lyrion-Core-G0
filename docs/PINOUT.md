# Lyrion Core G0 — Pinout

Board: Lyrion Core G0 Rev A (planning)
MCU: STM32G071CBU6 (UFQFPN-48)
Source of truth: [Core/Inc/g0_pinmap.h](../Core/Inc/g0_pinmap.h) — this document mirrors it; update both together.

Physical package pin numbers are intentionally omitted (UFQFPN-48 numbering to be taken from the STM32G071 datasheet during schematic capture).

---

## 1. MCU pin map

### Port A

| Pin | Net | Peripheral / function | Direction | Notes |
|---|---|---|---|---|
| PA0 | SPI2_SCK | SPI2 clock | I/O | Shared by LL1, LL2, Spare SPI |
| PA1 | PTTButton | GPIO EXTI (line 1) | Input | Push-to-talk button, EXTI0_1_IRQn |
| PA2 | USART2_TX | USART2 TX | Output | Spare UART header |
| PA3 | USART2_RX | USART2 RX | Input | Spare UART header |
| PA4 | DAC1_OUT1 | DAC1 output 1 | Analog | ADC / DAC header pin 1 |
| PA5 | DAC1_OUT2 | DAC1 output 2 | Analog | ADC / DAC header pin 2 |
| PA6 | ADC1_IN6 | ADC1 input channel 6 | Analog | ADC / DAC header pin 3 |
| PA7 | ADC1_IN7 | ADC1 input channel 7 | Analog | ADC / DAC header pin 4 |
| PA8 | CS1 | GPIO output | Output | LL1 chip select (active low) |
| PA9 | USART1_TX | USART1 TX | Output | CH340C USB-UART bridge RX |
| PA10 | USART1_RX | USART1 RX | Input | CH340C USB-UART bridge TX |
| PA11 | GDO0_1 | GPIO EXTI (line 11) | Input | LL1 CC1101 RX-done / IRQ |
| PA12 | GDO2_1 | GPIO input | Input | LL1 CC1101 sync / channel sense |
| PA13 | SWDIO | SWD data | I/O | SWD programming header |
| PA14 | SWCLK / BOOT0 | SWD clock / boot strap | Input | SWD programming header; BOOT0 sampled at reset |
| PA15 | BTN_OK | GPIO EXTI (line 15) | Input | Menu OK / Select button |

### Port B

| Pin | Net | Peripheral / function | Direction | Notes |
|---|---|---|---|---|
| PB0 | I2S1_WS | I2S1 word select | I/O | NAU88C22YG FS/LRC |
| PB1 | WS2812B | TIM3_CH4 + DMA1 | Output | Addressable RGB status LED |
| PB2 | GDO0_2 | GPIO EXTI (line 2) | Input | LL2 CC1101 RX-done / IRQ |
| PB3 | I2S1_CK | I2S1 bit clock | I/O | NAU88C22YG BCLK |
| PB4 | I2S1_MCK | I2S1 master clock | Output | 12.288 MHz clock to NAU88C22YG |
| PB5 | I2S1_SD | I2S1 serial data | I/O | Shared DACIN (TX) / ADCOUT (RX), half-duplex |
| PB6 | I2C1_SCL | I2C1 clock | I/O | SSD1306 OLED, TMP102, NAU88C22YG |
| PB7 | I2C1_SDA | I2C1 data | I/O | SSD1306 OLED, TMP102, NAU88C22YG |
| PB8 | BTN_UP | GPIO EXTI (line 8) | Input | Menu Up button |
| PB9 | BTN_DOWN | GPIO EXTI (line 9) | Input | Menu Down button |
| PB10 | GDO2_2 | GPIO input | Input | LL2 CC1101 sync / channel sense |
| PB11 | I2C2_SDA | I2C2 data | I/O | Spare I2C header |
| PB12 | VBAT_SENSE | ADC1_IN16 | Analog | 2:1 resistive divider from battery |
| PB13 | I2C2_SCL | I2C2 clock | I/O | Spare I2C header |
| PB14 | SPI2_MISO | SPI2 data in | Input | Shared by LL1, LL2, Spare SPI |
| PB15 | SPI2_MOSI | SPI2 data out | Output | Shared by LL1, LL2, Spare SPI |

### Port C / F

| Pin | Net | Peripheral / function | Direction | Notes |
|---|---|---|---|---|
| PC0 | LED1 | GPIO output | Output | Discrete status LED 1 |
| PC1 | LED2 | GPIO output | Output | Discrete status LED 2 |
| PC2 | LMS_CS | GPIO output | Output | LMS port chip select (active low) |
| PC3 | LMS_IRQ | GPIO EXTI (line 3) | Input | LMS port interrupt |
| PC4 | USART3_TX | USART3 TX | Output | LMS port TX (host to module) |
| PC5 | USART3_RX | USART3 RX | Input | LMS port RX (module to host) |
| PC6 | CS2 | GPIO output | Output | LL2 chip select (active low) |
| PC7 | JACK_IN_DET | GPIO input (pull-up) | Input | Switched line-in 3.5 mm jack detect (low = plugged) |
| PC13 | JACK_OUT_DET | GPIO input (pull-up) | Input | Switched line-out 3.5 mm jack detect (low = plugged) |
| PC14 | BTN_BACK | GPIO EXTI (line 14) | Input | Menu Back / Cancel button |
| PC15 | CS_SPARE | GPIO output | Output | Spare SPI header chip select |
| PF0 | HSE_IN | Clock input | Input | 12.288 MHz audio crystal oscillator |
| PF1 | HSE_OUT | Clock output | Output | 12.288 MHz audio crystal oscillator |
| PF2 | NRST | Hardware reset | Input | Reset pin to SWD header |

---

## 2. Connectors & Headers

### LL1 — Lyrion Link Socket 1 (Radio 1)
* 1: 3V3
* 2: GND
* 3: SCK (PA0)
* 4: MISO (PB14)
* 5: MOSI (PB15)
* 6: CS1 (PA8)
* 7: GDO0_1 (PA11)
* 8: GDO2_1 (PA12)

### LL2 — Lyrion Link Socket 2 (Radio 2)
* 1: 3V3
* 2: GND
* 3: SCK (PA0, shared)
* 4: MISO (PB14, shared)
* 5: MOSI (PB15, shared)
* 6: CS2 (PC6)
* 7: GDO0_2 (PB2)
* 8: GDO2_2 (PB10)

### LMS — Lyrion Smart Port (1x7 2.54 mm)
* 1: 5V (USB VBUS or boosted 5V rail)
* 2: 3.3V (system logic rail)
* 3: GND
* 4: TX (USART3_TX, PC4)
* 5: RX (USART3_RX, PC5)
* 6: CS / GPIO (PC2)
* 7: IRQ (PC3, EXTI line 3)

### OLED Display Header (1x4 2.54 mm)
* 1: VCC (+3.3V)
* 2: GND
* 3: SCL (I2C1_SCL, PB6)
* 4: SDA (I2C1_SDA, PB7)

### SWD Programming Header (1x5 2.54 mm)
* 1: +3.3V
* 2: SWDIO (PA13)
* 3: SWCLK (PA14)
* 4: NRST (PF2)
* 5: GND

### Spare Expansion Headers
* Spare UART (1x4): 1=+3V3, 2=TX (PA2, USART2), 3=RX (PA3, USART2), 4=GND
* Spare I2C (1x4): 1=+3V3, 2=SCL (PB13, I2C2), 3=SDA (PB11, I2C2), 4=GND
* Spare SPI (1x6): 1=+3V3, 2=GND, 3=SCK (PA0), 4=MISO (PB14), 5=MOSI (PB15), 6=CS (PC15)
* ADC / DAC (1x4): 1=DAC1 (PA4), 2=DAC2 (PA5), 3=ADC1 (PA6), 4=ADC2 (PA7)
