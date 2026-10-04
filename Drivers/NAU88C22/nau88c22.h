/**
 * @file  nau88c22.h
 * @brief Driver for the Nuvoton NAU88C22YG audio codec (I2S + I2C control).
 *
 * The NAU88C22 is a 24-bit stereo audio codec with:
 *   - 2x ADC (stereo differential / single-ended microphone preamps + MICBIAS)
 *   - 2x DAC
 *   - integrated BTL loudspeaker driver (1 W into 8 ohm)
 *   - integrated stereo headphone driver (40 mW into 16 ohm)
 *   - stereo line input / line output
 *   - I2S / PCM digital audio interface (slave or master)
 *   - 2-wire (I2C) or 3-wire MPU control interface
 *
 * On Lyrion Core G0 the codec shares the single STM32 I2S1 (half-duplex):
 * it is an I2S slave during playback (MCU master TX) and the bit-clock
 * master during capture (MCU slave RX, codec drives BCLK/FS):
 *   MCLK  <- PB4 (12.288 MHz, generated on the I2S1_MCK pin)
 *   BCLK  <- PB3 (I2S1_CK)
 *   FS    <- PB0 (I2S1_WS)
 *   DACIN <- PB5 (I2S1_SD, MCU -> codec)
 *   ADCOUT -> PB5 (I2S1_SD, codec -> MCU)  [half-duplex, PTT]
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef NAU88C22_H
#define NAU88C22_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 7-bit I2C control address (CSB/GPIO1 = 0 -> 0x1A). */
#define NAU88C22_I2C_ADDR_7BIT   0x1A

/* Audio path selection (half-duplex: only one at a time). */
typedef enum
{
    NAU88C22_PATH_NONE = 0,
    NAU88C22_PATH_PLAYBACK,   /* I2S DAC -> speaker / headphone */
    NAU88C22_PATH_CAPTURE     /* microphone -> I2S ADC -> MCU    */
} nau88c22_path_t;

/**
 * @brief Probe the codec on I2C and run the full register init sequence.
 * @return HAL_OK on success, HAL_ERROR if the codec does not respond.
 */
HAL_StatusTypeDef nau88c22_init(void);

/** @brief True once nau88c22_init() has completed successfully. */
bool nau88c22_is_present(void);

/**
 * @brief Select the active audio path (playback or capture).
 *        Powers down the unused path so the shared I2S1_SD net is driven by
 *        exactly one side at a time.
 */
HAL_StatusTypeDef nau88c22_select_path(nau88c22_path_t path);

/**
 * @brief Set the DAC (speaker / headphone) output volume.
 * @param vol 0..63 (0 = mute, 63 = 0 dB).  See NAU88C22_DAC_VOL_MAX.
 */
HAL_StatusTypeDef nau88c22_set_volume(uint8_t vol);

/**
 * @brief Set the microphone PGA gain.
 * @param gain 0..63, 0.5 dB per step (NAU88C22_MIC_GAIN_MAX).
 */
HAL_StatusTypeDef nau88c22_set_mic_gain(uint8_t gain);

/** @brief Enable/disable the BTL speaker driver. */
HAL_StatusTypeDef nau88c22_speaker_enable(bool enable);

/** @brief Enable/disable the stereo headphone driver. */
HAL_StatusTypeDef nau88c22_headphone_enable(bool enable);

/** @brief Mute (true) or unmute (false) the DAC path. */
HAL_StatusTypeDef nau88c22_mute(bool mute);

/** @brief Read one 7-bit control register (16-bit value). */
HAL_StatusTypeDef nau88c22_read_reg(uint8_t reg, uint16_t *value);

/** @brief Write one 7-bit control register (16-bit value). */
HAL_StatusTypeDef nau88c22_write_reg(uint8_t reg, uint16_t value);

#define NAU88C22_DAC_VOL_MAX   63
#define NAU88C22_MIC_GAIN_MAX  63

#ifdef __cplusplus
}
#endif

#endif /* NAU88C22_H */
