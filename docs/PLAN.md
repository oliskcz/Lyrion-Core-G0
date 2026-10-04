# Lyrion Core G0 — Build Plan

**Board:** Lyrion Core G0 Rev A
**Created:** 2026-10-04
**Status:** Phase 0 complete — planning repo and firmware skeleton exist

This plan takes the G0 from repo to a working PTT voice node. Each phase has a deliverable, a task list, and a verification step. Hardware and firmware phases are interleaved: nothing is ordered until the pin map is proven on a Nucleo/blue-pill class board, and no PCB is ordered until the codec works on a breakout.

---

## Phase 0 — Repo & documentation ✅

**Deliverable:** this repository.

- [x] Firmware skeleton (Core, Drivers, linker script, startup)
- [x] Pin map (`Core/Inc/g0_pinmap.h`) and config toggles (`Core/Inc/config.h`)
- [x] Drivers: NAU88C22, Audio, CC1101, LyrionLink, Crypto, OLED, TMP102, WS2812B
- [x] Docs: README, SPECS, PINOUT, CONNECTIONS, PLAN, REMAINING
- [x] GPL-3.0 license, .gitignore, label workflow

**Verification:** repo builds a consistent story — pin map, docs and code agree.

---

## Phase 1 — Schematic capture & design freeze

**Deliverable:** Altium (or EasyEDA) schematic for Rev A + resolved open questions.

- [x] Resolve **EXTI line 2 conflict**: BTN2 moved PA2 → PA5 (EXTI line 5) in `main.h`/`g0_pinmap.h`/docs; mirror in `.ioc` when created
- [ ] Validate the **clock tree**: PLL I2S (implemented in the skeleton) vs 12.288 MHz HSE for the I2S1 kernel clock (bring up on a dev board first)
- [ ] Confirm **M1 module connector** pinout from the Lyrion M1 module archive
- [ ] Confirm **J1/J3 socket pin order** against the Core C0 schematic
- [ ] Codec circuit: MICBIAS + 2.2 kΩ bias, AC coupling, differential MIC1±, BTL output filter, headphone jack
- [ ] Power: USB-C (5.1 kΩ CC), CH340 auto-download (DTR/RTS → NRST/BOOT0), AP2112K-3.3, VDDSPK filter + bulk
- [ ] Decide VDDSPK source (filtered VBUS vs 5 V boost)
- [ ] BOM freeze with LCSC part numbers (STM32 C529343, NAU88C22YG C914209, …)
- [ ] Sheet-level review: pin map vs schematic net-by-net

**Verification:** ERC clean; every `g0_pinmap.h` net appears in the schematic; conflict items closed.

---

## Phase 2 — PCB layout

**Deliverable:** 2-layer PCB, Rev A, fabrication files.

- [ ] Placement: MCU center, codec close to MCU with short I²S traces, mic away from speaker, speaker connector at board edge
- [ ] Solid GND plane; analog (mic/codec) star-connected to digital ground at the codec
- [ ] BTL speaker traces wide (≥ 0.5 mm), kept away from mic input
- [ ] Decoupling per ST/Nuvoton reference (100 nF + bulk on every rail)
- [ ] J1/J3/M1/SWD/OLED/UART/speaker connectors placed for case access
- [ ] Silkscreen: port names, pin 1 markers, warning on 5 V VDDSPK
- [ ] DRC + Gerber review; order 5 boards

**Verification:** visual Gerber check against CONNECTIONS.txt; continuity on first article.

---

## Phase 3 — Firmware bring-up (no audio)

**Deliverable:** `main.c` + `STM32_Lyrion_Core_G0.ioc` + working peripherals on the first article.

- [ ] Create `STM32_Lyrion_Core_G0.ioc` matching `g0_pinmap.h` (clocks, I²S1, SPI2, I²C1, USART1/3, TIM3+DMA, EXTI)
- [ ] `main.c` skeleton in CubeMX style with `USER CODE` markers; init order: clocks → GPIO → I²C → OLED → TMP102 → SPI2 → CC1101 → WS2812B → UART
- [ ] Verify 64 MHz core clock and 12.288 MHz I2S clock (scope/ MCO)
- [ ] UART console over CH340 + ROM bootloader flash test
- [ ] I²C scan finds 0x3C (OLED), 0x49 (TMP102), 0x1A (codec)
- [ ] OLED shows status screen; TMP102 reads plausible temperature
- [ ] WS2812B test pattern
- [ ] CC1101: J1 and J3 self-test (partnum/version read, loopback TX→RX)

**Verification:** all peripherals on one board without resets or I²C errors; radio link between J1 and J3 at 1 m.

---

## Phase 4 — Audio bring-up

**Deliverable:** working half-duplex voice path.

- [ ] Codec probe + init (`nau88c22_init`) returns OK
- [ ] Playback: `audio_beep()` audible on speaker and headphone
- [ ] Capture: `audio_capture_start()` streams mic samples; verify level with PGA sweep
- [ ] `audio_set_mode()` switching: no bus contention on PB5, no pop (mute before path switch)
- [ ] Measure output power into the 8 Ω speaker at 5 V VDDSPK (target ≈ 1 W)
- [ ] Record/playback loop test at 48 kHz; check sample-rate accuracy (MCLK validation)
- [ ] Enclosure mock-up: speaker in case, mic placement, feedback/acoustic test

**Verification:** intelligible voice recording and playback; no audible clock drift over 60 s.

---

## Phase 5 — Lyrion Link Pro integration

**Deliverable:** two G0 boards exchanging encrypted PTT voice.

- [ ] CC1101 settings for Lyrion Link (band, 26 MHz crystal, sync word, whitening)
- [ ] Lyrion Link packet/MAC/security stack over the Pro build (`LL_PRO_BUILD 1`)
- [ ] AES-128-CCM with per-node keys; key provisioning flow
- [ ] Voice framing: capture → compress/stream → radio TX (PTT), RX → playback
- [ ] UI: OLED shows link state, RSSI, node address; PTT button drives mode
- [ ] Range test at 433 MHz, indoor/outdoor

**Verification:** two boards, PTT voice, AES on, 100+ packets without drop; range logged.

---

## Phase 6 — Enclosure

**Deliverable:** case with mounted 40 mm speaker and mic opening.

- [ ] Case design (3D print) around the PCB, speaker chamber, mic port, connector cutouts
- [ ] Speaker gasket; sealed chamber tuning for voice
- [ ] Button caps / light pipes for LEDs
- [ ] USB-C, SWD, module ports accessible

**Verification:** assembled unit survives handling; voice intelligibility in the case matches bench test.

---

## Phase 7 — Field test & Rev B backlog

**Deliverable:** test report + Rev B change list.

- [ ] 24 h soak with radios + audio active
- [ ] Power measurements (idle/TX/audio)
- [ ] List issues → Rev B backlog (e.g., NAU88C10YG mono variant, 5 V boost, 4-layer option)

**Verification:** documented results committed to the repo.

---

## Dependencies

| Phase | Needs |
|-------|-------|
| 1 | Datasheets: STM32G071, NAU88C22, CC1101, AP2112K, CH340 |
| 1 | Core C0 Altium schematic (J1/J3 pin order), Lyrion M1 module archive |
| 3 | STM32CubeIDE, ST-LINK, first PCB article |
| 4 | Speaker + mic samples, scope, audio source |
| 5 | Two G0 boards, two CC1101 modules, AES key provisioning |

## Risk register

| Risk | Mitigation |
|------|------------|
| Single I²S cannot do full duplex | PTT half-duplex (already designed in) |
| 12.288 MHz clock not exact | Validate HSE option in Phase 1; codec tolerates small error |
| Codec BTL too quiet at 3.3 V | VDDSPK from 5 V VBUS; optional boost in Rev B |
| M1 module pinout mismatch | Freeze connector after checking module archive |
| 2-layer audio noise | Short analog traces, star ground, shielded mic wiring |
