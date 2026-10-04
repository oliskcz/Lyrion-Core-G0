# Lyrion Core G0 — Remaining Work

Checklist of everything not yet in the repo or not yet decided.
Legend: `[x]` done · `[ ]` open · `[!]` blocker for the next phase

## Repo & docs

- [x] README, SPECS, PINOUT, CONNECTIONS, PLAN, REMAINING
- [x] Firmware skeleton (drivers, startup, linker script)
- [ ] Align license headers (drivers say MIT, repo is GPL-3.0) `[!]`
- [ ] Add `.github/labeler.yml` referenced by the label workflow
- [ ] PCB files folder once the schematic exists (`PCB_Files/`)

## Firmware

- [ ] `Core/Src/main.c` — does not exist yet `[!]`
- [ ] `STM32_Lyrion_Core_G0.ioc` — CubeMX project does not exist yet `[!]`
- [x] Fix EXTI line 2 conflict — BTN2 moved PA2 → PA5 in `main.h`/`g0_pinmap.h`; mirror in `.ioc` when created
- [x] Add PB0 (I2S1_WS) to the I2S1 GPIO mux in `stm32g0xx_hal_msp.c`
- [x] Keep MCLK output enabled in capture mode (`audio.c`)
- [ ] Add CS3 / M1_IRQ / USART3 pin defines to `main.h` (currently only in `g0_pinmap.h` comments)
- [ ] Validate I2S/audio driver against real codec (only compile-level so far)
- [ ] Wire `audio.c` / `nau88c22.c` / `lyrion_link.c` into `main.c` application logic
- [ ] Audio routing: input source (mic ↔ line-in) + output source (speaker ↔ 3.5 mm ↔ both) selection
- [ ] OLED audio routing menu (`ENABLE_AUDIO_ROUTING`)
- [ ] M1 module UART protocol implementation (currently no M1 code)

## Hardware decisions

- [ ] Clocking: PLL I2S (implemented) vs 12.288 MHz HSE `[!]`
- [ ] VDDSPK: filtered VBUS vs 5 V boost
- [ ] M1 connector pinout (verify against `LyrionMIVTwo.zip`) `[!]`
- [ ] J1/J3 socket pin order (verify against Core C0 Altium schematic) `[!]`
- [ ] Board size and mounting holes
- [ ] CH340 auto-download circuit (DTR/RTS → NRST/BOOT0)
- [ ] Speaker connector type (JST-XH vs screw terminal)
- [ ] External microphone header (optional)
- [ ] J_IN line-in circuit: AC coupling value, pad/attenuator footprint, mono-plug handling
- [ ] Jack detection (optional spare GPIO PA6/PA7) — decide for Rev A

## BOM

- [ ] Freeze LCSC part numbers for TBD items (TMP102, CH340C, AP2112K, WS2812B, crystal, USB-C, 3.5 mm jacks ×2, buttons, connectors)
- [ ] Verify STM32G071CBU6 (C529343) and NAU88C22YG (C914209) stock before ordering
- [ ] Decide speaker/mic sourcing (AliExpress samples vs stocked parts)

## Documentation follow-ups

- [ ] Add schematic PDF + renders when available
- [ ] Add measured audio/power numbers to SPECS after Phase 4
- [ ] Add field-test report after Phase 7
- [ ] Update README status badges as phases complete
