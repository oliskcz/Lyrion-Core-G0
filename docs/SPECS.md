# Lyrion Core G0 — Specifications

Board: Lyrion Core G0, Rev A (design freeze)
Date: 2026-10-04
Status: Specification frozen — schematic capture ready
MCU: STM32G071CBU6 (UFQFPN-48)
Related: [README](../README.md) · [PINOUT](PINOUT.md) · [PLAN](PLAN.md) · [CONNECTIONS](CONNECTIONS.txt)

---

## 1. Overview

Lyrion Core G0 is the Pro-tier Lyrion Core board: an STM32G071 voice/audio node with a Nuvoton NAU88C22YG codec, analog differential microphone, case-mounted speaker, OLED UI with 4-button navigation, dual Lyrion Link CC1101 module sockets, LMS smart expansion port, expansion headers (UART, I2C, SPI, ADC/DAC), and integrated LiPo battery management with dynamic power-path.

## 2. MCU — STM32G071CBU6

| Parameter | Value |
|---|---|
| Core | Arm Cortex-M0+ @ 64 MHz |
| Flash / RAM | 128 KB / 36 KB (32 KB with parity) |
| Package | UFQFPN-48, 7 × 7 mm, 44 GPIO |
| Supply | 1.7 – 3.6 V |
| I2S | 1 instance (I2S1, on SPI1) — half-duplex PTT |
| SPI | 2 (SPI1 = I2S1 audio, SPI2 = LL1, LL2, Spare SPI) |
| I2C | 2 (I2C1 = OLED, TMP102, Codec; I2C2 = Spare I2C) |
| USART / LPUART | 4 + 1 (USART1 = CH340C, USART2 = Spare UART, USART3 = LMS port) |
| Timers | 14 (TIM3_CH4 for WS2812B RGB LED) |
| ADC | 12-bit (ADC1_IN6/IN7 on header, ADC1_IN16 for battery gauge) |
| DAC | 2 × 12-bit (DAC1_OUT1, DAC1_OUT2 on header) |
| Debug | SWD (PA13/PA14) + NRST (PF2) |

## 3. Audio Subsystem

### 3.1 Codec — NAU88C22YG
* Digital audio: I2S1 (PB0=WS, PB3=CK, PB4=MCK, PB5=SD). Half-duplex PTT.
* Control: I2C1 @ 0x1A.
* MCLK: 12.288 MHz generated from HSE crystal.
* Playback: Codec BTL amplifier driving XHXDZ 40 mm 8-ohm 2W speaker (1W into 8 ohms at 5V VDDSPK).
* Headphone Output: 3.5 mm switched TRS jack (J_OUT) with mechanical jack detect into PC13.
* Microphone Input: INGHAi GMI9745P-30dB electret condenser mic wired differentially to MIC1P/MIC1N with 2.2k pull-up to MICBIAS.
* Line Input: 3.5 mm switched TRS jack (J_IN) with mechanical jack detect into PC7 and AC-coupling caps into LLIN/RLIN.

## 4. RF & Wireless Ports

* LL1 (Lyrion Link Socket 1): CC1101 module socket on SPI2 (PA0, PB14, PB15) + CS1 (PA8) + GDO0_1 (PA11, EXTI11) + GDO2_1 (PA12).
* LL2 (Lyrion Link Socket 2): CC1101 module socket on SPI2 + CS2 (PC6) + GDO0_2 (PB2, EXTI2) + GDO2_2 (PB10).
* LMS (Lyrion Smart Port standard): 1x7 2.54 mm header:
  * Pin 1: 5V
  * Pin 2: 3.3V
  * Pin 3: GND
  * Pin 4: TX (USART3_TX, PC4)
  * Pin 5: RX (USART3_RX, PC5)
  * Pin 6: CS / GPIO (PC2)
  * Pin 7: IRQ (PC3, EXTI3)

## 5. UI & Expansion

* 4-Button OLED Navigation: UP (PB8), DOWN (PB9), OK (PA15), BACK (PC14).
* PTT Button: Tactile switch on PA1 (EXTI1).
* Indicators: LED1 (PC0), LED2 (PC1), WS2812B RGB LED (PB1).
* Display: 4-pin I2C header for SSD1306 0.96 inch or 1.3 inch OLED.
* Sensor: Texas Instruments TMP102 temperature sensor on I2C1.
* Spare UART: 4-pin header (3V3, TX=PA2, RX=PA3, GND).
* Spare I2C: 4-pin header (3V3, SCL=PB13, SDA=PB11, GND).
* Spare SPI: 6-pin header (3V3, GND, SCK=PA0, MISO=PB14, MOSI=PB15, CS=PC15).
* ADC / DAC: 4-pin header (DAC1=PA4, DAC2=PA5, ADC1=PA6, ADC2=PA7).

## 6. Power & Battery Management

* USB-C: 5V input, 5.1k pulldown resistors on CC1/CC2.
* Charger IC: IP2312-4V35 synchronous buck charger with solder bridges for 1A, 1.5A, and 3A charge current, and NTC temperature protection.
* Battery Protection: XB5352A monolithic 1S LiPo protection IC (2.8V cutoff, 4.28V overcharge clamp, short-circuit protection).
* Boost Converter: MT3608 1.2 MHz boost converter boosting battery voltage up to 5.0V.
* Dynamic Power-Path (Load Sharing):
  * USB 5V turns on an NPN transistor that pulls MT3608 EN pin low when USB is connected (zero battery drain).
  * System 5V rail (5V_SYS) is powered directly by USB 5V (via Schottky diode) when plugged in, and MT3608 when running on battery.
* Logic LDO: TLV75733PDBVR powered from 5V_SYS, guaranteeing steady 3.30V logic rail without dropout.
* Speaker Supply: VDDSPK powered by 5V_SYS, delivering full 1W speaker power in both USB and battery modes.
* Battery Fuel Gauge: 2x 100k precision resistive divider from VBAT into PB12 (ADC1_IN16).
