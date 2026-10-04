/**
 * @file  audio.h
 * @brief I2S audio engine for Lyrion Core G0 (STM32G071 + NAU88C22 codec).
 *
 * STM32G071 exposes a single I2S (I2S1 on SPI1) and the G0 HAL only supports
 * half-duplex modes (SLAVE_TX / SLAVE_RX / MASTER_TX / MASTER_RX). This driver
 * therefore implements PTT-style half-duplex audio:
 *   - playback: I2S MASTER_TX at 48 kHz, DMA double-buffered
 *   - capture:  I2S SLAVE_RX  at 48 kHz (the codec drives BCLK/FS)
 *
 * The active direction is switched with audio_set_mode(), which also tells the
 * codec (see nau88c22_select_path) to power down the unused path so the shared
 * I2S1_SD net is driven by exactly one side at a time.
 *
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef AUDIO_H
#define AUDIO_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_PCM_BUF_SAMPLES   1024U   /* per half-buffer, mono frames */
#define AUDIO_SILENCE_FRAMES    480U    /* 10 ms of silence at 48 kHz   */
#define AUDIO_TONE_FRAMES       2400U   /* 50 ms 440 Hz beep            */

typedef enum
{
    AUDIO_MODE_OFF = 0,
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_CAPTURE
} audio_mode_t;

/**
 * @brief Initialise the codec and the I2S1 peripheral (master TX idle).
 * @return HAL_OK on success, HAL_ERROR if the codec is missing.
 */
HAL_StatusTypeDef audio_init(void);

/** @brief Switch audio direction (half-duplex). */
HAL_StatusTypeDef audio_set_mode(audio_mode_t mode);

/** @brief Current audio mode. */
audio_mode_t audio_get_mode(void);

/** @brief Play a 16-bit mono PCM buffer (blocking, DMA-assisted). */
HAL_StatusTypeDef audio_play_pcm(const int16_t *samples, uint32_t count);

/** @brief Start continuous capture into the internal DMA buffer.
 *  @param cb  called from the DMA half/full transfer ISR with a buffer half. */
HAL_StatusTypeDef audio_capture_start(void (*cb)(const int16_t *buf, uint32_t len));

/** @brief Stop continuous capture. */
HAL_StatusTypeDef audio_capture_stop(void);

/** @brief Self-test: play a short 440 Hz beep on the speaker. */
HAL_StatusTypeDef audio_beep(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H */
