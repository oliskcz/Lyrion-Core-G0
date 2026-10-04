# Lyrion Core G0 — Specifications

**Board:** Lyrion Core G0, Rev A (planning)
**Date:** 2026-10-04
**Status:** Planning — schematic not captured yet
**MCU:** STM32G071CBU6 (UFQFPN-48)
**Related:** [README](../README.md) · [PINOUT](PINOUT.md) · [PLAN](PLAN.md) · [CONNECTIONS](CONNECTIONS.txt)

> Items marked **UNVERIFIED** are engineering assumptions that must be confirmed during Phase 1 (schematic capture) before the PCB is ordered.

---

## 1. Overview

Lyrion Core G0 is the Pro-tier Lyrion Core board: an STM32G071 voice/audio node with a Nuvoton NAU88C22 codec, analog microphone, case-mounted speaker, OLED UI, and three Lyrion Link module ports (two raw CC1101 sockets plus one M1 smart-module port). It is the audio-capable counterpart to the [Lyrion Core C0](https://github.com/oliskcz/Lyrion-Core-C0).

## 2. MCU — STM32G071CBU6

| Parameter | Value |
|-----------|-------|
| Core | Arm Cortex-M0+ @ 64 MHz |
| Flash / RAM | 128 KB / 36 KB (32 KB with parity) |
| Package | UFQFPN-48, 7 × 7 mm, 44 GPIO |
| Supply | 1.7 – 3.6 V |
| I²S | **1 instance** (I2S1, on SPI1) — half-duplex only via HAL |
| SPI | 2 (SPI1 = I2S1 audio, SPI2 = CC1101 radios J1/J3) |
| I²C | 2 (FM+ up to 1 Mbit/s) |
| USART / LPUART | 4 + 1 |
| Timers | 14 (TIM1 advanced; TIM3 used for WS2812B) |
| ADC | 17 × 12-bit channels |
| DAC | 2 × 12-bit (unused on this board) |
| USB | **None** (UCPD PHY only) — no USB FS device |
| Oscillators | HSE 4–48 MHz, HSI16, LSI32 |
| Debug | SWD (PA13/PA14) |

LCSC part: **C529343** (~$2.5–3.9 @ 1, in stock — UNVERIFIED pricing).

## 3. Audio

### 3.1 Codec — NAU88C22YG (Rev A)

| Parameter | Value |
|-----------|-------|
| ADC / DAC | 2 / 2, 24-bit, 8–192 kHz |
| ADC SNR / DAC SNR | ~90 dB / ~94 dB |
| Mic path | 2 differential inputs, PGA −12…+35.25 dB, MICBIAS |
| Line input | Stereo `LLIN`/`RLIN` (pins 3/6, shared with alternate `MICP`/`RMICP`) |
| Speaker driver | BTL, 1 W into 8 Ω @ 5 V (≈0.4 W @ 3.3 V) |
| Headphone/line driver | 40 mW into 16 Ω @ 3.3 V (`LHP`/`RHP`, pins 29/30) |
| Control | I²C @ 0x1A (7-bit, CSB/GPIO1 = 0) |
| Digital audio | I²S/PCM, slave or master |
| MCLK | 12.288 MHz for 48 kHz (256 × fs) |
| Package | QFN-32, 5 × 5 mm |
| Supplies | AVDD 2.5–3.6 V, DVDD 1.65–3.6 V, VDDSPK up to 5 V |
| LCSC | C914209 |

**Alternative (cost-down, mono):** NAU88C10YG — QFN-20 4 × 4 mm, 1 ADC + 1 DAC, same BTL driver and I²S/I²C interface, 8–48 kHz, LCSC C914208. Not footprint-compatible; would be a Rev B variant.

### 3.2 Digital audio interface (half-duplex)

| Signal | MCU pin | Codec pin | Direction |
|--------|---------|-----------|-----------|
| MCLK | PB4 (I2S1_MCK) | MCLK | MCU → codec |
| BCLK | PB3 (I2S1_CK) | BCLK | playback: MCU → codec; capture: codec → MCU |
| FS / WS | PB0 (I2S1_WS) | LRC/FS | playback: MCU → codec; capture: codec → MCU |
| SD | PB5 (I2S1_SD) | DACIN (playback) / ADCOUT (capture) | bidirectional, half-duplex |

- Playback: MCU = I²S **master TX**, 48 kHz, DMA double-buffered.
- Capture: MCU = I²S **slave RX**, codec drives BCLK/FS.
- `I2S1_SD` is shared between `DACIN` and `ADCOUT`; the unused path is powered down by `nau88c22_select_path()` so exactly one side drives the net.

### 3.3 Microphone

| Parameter | Value |
|-----------|-------|
| Part | INGHAi GMI9745P-30dB |
| Type | Electret condenser (ECM), through-hole |
| Size | Ø 9.7 mm × 4.5 mm |
| Sensitivity | −30 dB (0 dB = 1 V/Pa) |
| Bias | Codec MICBIAS through 2.2 kΩ |
| Input | Differential MIC1± via AC coupling |
| Gain | Codec PGA, 0.5 dB/step |

### 3.4 Speaker

| Parameter | Value |
|-----------|-------|
| Part | XHXDZ 40 mm full-range |
| Impedance / power | 8 Ω / 2 W rated |
| Drive | Codec BTL output (1 W @ 5 V) |
| Mounting | Case-mounted, 2-pin connector |
| Enclosure | Small sealed/lightly vented chamber; gasket recommended (UNVERIFIED) |

### 3.5 Audio jacks (J_IN / J_OUT)

| Jack | Wiring | Codec pins | Purpose |
|------|--------|-----------|---------|
| **J_OUT** (output) | Tip = L, Ring = R, Sleeve = GND | `LHP` (30) / `RHP` (29) | Headphone / line-out, 40 mW into 16 Ω |
| **J_IN** (input) | Tip = L, Ring = R, Sleeve = GND | `LLIN` (3) / `RLIN` (6) via AC coupling | Line-in ~1 V<sub>RMS</sub> FS (phone/PC/radio) |

- Input selection (mic vs line) and output selection (speaker vs jack, or both) are codec register settings — no analog switch.
- Optional jack-detect on a spare GPIO (PA6/PA7) — **not wired in Rev A**.
- Mono TS plug on J_IN grounds the ring; capture falls back to the left channel (or summed mono).

## 4. Lyrion Link ports

| Port | Signals | Pins |
|------|---------|------|
| J1 (raw CC1101) | SCK/MISO/MOSI (SPI2, shared), CS1, GDO0_1, GDO2_1 | PA8, PA11, PA12 |
| J3 (raw CC1101) | SCK/MISO/MOSI (SPI2, shared), CS2, GDO0_2, GDO2_2 | PC6, PB2, PB10 |
| M1 (smart module) | USART3 TX/RX, CS3, IRQ | PC4, PC5, PC2, PC3 |

- SPI2 bus: PA0 = SCK, PB14 = MISO, PB15 = MOSI (shared by J1 + J3).
- No reset line on J1/J3 — CC1101 reset via SRES strobe (same as C0).
- Connector type: 2.54 mm through-hole headers/sockets (UNVERIFIED pin order — copy from Core C0 J1/J3).
- M1 physical pinout: confirm against the Lyrion M1 module archive before layout (**UNVERIFIED**).

## 5. Display, sensors, UI

| Item | Interface | Address / pin |
|------|-----------|---------------|
| SSD1306 OLED 0.96" 128×64 | I²C1 (PB6/PB7) | 0x3C |
| TMP102 temperature | I²C1 | 0x49 |
| PTT button | EXTI | PA1 |
| BTN2 | EXTI | PA2 (**EXTI conflict — see §8**) |
| LED1 / LED2 | GPIO | PC0 / PC1 |
| WS2812B | TIM3_CH4 + DMA1 | PB1 |
| Audio routing menu (planned) | OLED UI | — |

## 6. Power

| Rail | Source | Notes |
|------|--------|-------|
| VBUS 5 V | USB-C (5.1 kΩ CC pulldowns) | Power + CH340 USB 2.0 FS data |
| 3V3 | AP2112K-3.3, 600 mA LDO | Digital + analog rail |
| VDDSPK | Filtered VBUS + bulk 470 µF | Codec BTL only |

Estimated 3V3 current: ~60 mA idle, ~200 mA with radios TX and WS2812B active. Speaker peaks are drawn from VBUS.

## 7. Clocking (UNVERIFIED)

- Implemented in the skeleton: I2S1 kernel clock from the **PLL** (`RCC_I2S1CLKSOURCE_PLL` in `stm32g0xx_hal_msp.c`), targeting MCLK = 12.288 MHz at 48 kHz.
- Alternative to evaluate in Phase 1: **12.288 MHz audio-grade HSE crystal** as the I2S1 kernel clock source (MCU core from HSI16 PLL at 64 MHz) for exact MCLK accuracy.
- Codec PLL expects MCLK = 256 × fs (12.288 MHz at 48 kHz).
- The PLL divider chain must be validated so playback is exactly 48.000 kHz.

## 8. Known electrical issues to resolve before layout

1. **EXTI line 2 conflict (resolved in skeleton):** EXTI lines follow the pin number, so PA2 (BTN2) and PB2 (GDO0_2) originally shared line 2. BTN2 is now on **PA5** (EXTI line 5, `EXTI4_15_IRQn`) in `main.h`/`g0_pinmap.h` — note PA3 (line 3) is used by M1_IRQ and PB11 (line 11) by GDO0_1, so neither was a valid fix. Mirror this in the CubeMX `.ioc` when it is created.
2. **PB3 = I2S1_CK = SWO:** SWD works; SWO trace is unavailable.
3. **VDDSPK source:** confirm filtered-VBUS approach vs dedicated 5 V boost (only matters for loud playback).
4. **BOOT0 / auto-download circuit:** CH340 DTR/RTS → NRST/BOOT0 wiring to be drawn (UNVERIFIED).
5. **Line-in level:** reserve a pad/attenuator footprint and verify PGA range for typical line sources; decide mono-plug handling (left-only vs summed) in firmware.
6. **Jack detection:** optional spare-GPIO detect for J_IN/J_OUT — decide before layout.

## 9. Bill of materials (Rev A draft)

| # | Part | Purpose | LCSC / source | Notes |
|---|------|---------|---------------|-------|
| 1 | STM32G071CBU6 | MCU | C529343 | |
| 2 | NAU88C22YG | Audio codec | C914209 | C10YG = C914208 (alt) |
| 3 | GMI9745P-30dB | Electret mic | AliExpress / INGHAi | |
| 4 | XHXDZ 40 mm 8 Ω 2 W | Speaker | AliExpress | Case-mounted |
| 5 | SSD1306 0.96" module | Display | common module | 1.3" SH1106 option |
| 6 | TMP102 | Temperature | TBD | I²C 0x49 |
| 7 | CC1101 433 MHz module | Radio ×2 (J1/J3) | AliExpress | 26 MHz crystal |
| 8 | CH340C | USB-UART | TBD | Auto-download support |
| 9 | AP2112K-3.3 | 3.3 V LDO | TBD | ≥ 600 mA |
| 10 | WS2812B | RGB LED | TBD | |
| 11 | 12.288 MHz crystal | Audio clock | TBD | UNVERIFIED |
| 12 | USB-C receptacle | Power/data | TBD | 5.1 kΩ CC pulldowns |
| 13 | 3.5 mm stereo jack ×2 | J_OUT output + J_IN line input | TBD | TRS; AC coupling caps on J_IN |
| 14 | Tactile buttons ×2 | PTT + BTN2 | TBD | |
| 15 | 2.54 mm headers/sockets | J1, J3, M1, SWD, UART | TBD | |
| 16 | 2-pin speaker connector | Speaker | TBD | JST-XH class |
| 17 | Passives | R/C/L, bulk caps | TBD | 470 µF VBUS bulk |

LCSC codes marked TBD must be selected at order time; the STM32/NAU88 codes were confirmed during component research (Oct 2026).

## 10. Open questions summary

| # | Question | Impact |
|---|----------|--------|
| 1 | HSE vs PLL for exact 12.288 MHz MCLK | Audio sample-rate accuracy |
| 2 | EXTI line 2 conflict — resolved: BTN2 moved PA2 → PA5 | Button + radio IRQ |
| 3 | M1 module connector pinout | Footprint |
| 4 | J1/J3 socket pin order vs C0 | Module compatibility |
| 5 | VDDSPK rail (VBUS vs boost) | Speaker loudness |
| 6 | Final board size / enclosure | PCB + case |
| 7 | License header alignment (MIT vs GPL-3.0) | Release hygiene |
| 8 | Line-in attenuation / mono-plug handling | Input level + capture |
| 9 | Audio routing menu design (input/output source) | Firmware UI |
