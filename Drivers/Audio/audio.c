/**
 * @file  audio.c
 * @brief I2S audio engine for Lyrion Core G0.
 *
 * @copyright SPDX-License-Identifier: MIT
 */

#include "audio.h"
#include "config.h"
#include "nau88c22.h"

#include <string.h>

extern I2S_HandleTypeDef hi2s1;

static audio_mode_t s_mode = AUDIO_MODE_OFF;
static void (*s_capture_cb)(const int16_t *, uint32_t);

static int16_t s_tx_buf[AUDIO_PCM_BUF_SAMPLES];
static int16_t s_rx_buf[AUDIO_PCM_BUF_SAMPLES];

/* -------------------------------------------------------------------------- */
/*  I2S configuration helpers                                                 */
/* -------------------------------------------------------------------------- */

static HAL_StatusTypeDef i2s_config_playback(void)
{
    hi2s1.Instance = SPI1;
    hi2s1.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_48K;
    hi2s1.Init.CPOL = I2S_CPOL_LOW;
    return HAL_I2S_Init(&hi2s1);
}

static HAL_StatusTypeDef i2s_config_capture(void)
{
    /* In slave RX the codec supplies BCLK/FS; MCLK is still driven by the MCU
     * (I2S1_MCK), which is what the codec PLL locks to, so the codec must run
     * as the bit-clock master once powered. */
    hi2s1.Instance = SPI1;
    hi2s1.Init.Mode = I2S_MODE_SLAVE_RX;
    hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_48K;
    hi2s1.Init.CPOL = I2S_CPOL_LOW;
    return HAL_I2S_Init(&hi2s1);
}

/* -------------------------------------------------------------------------- */
/*  Public API                                                                */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef audio_init(void)
{
    if (nau88c22_init() != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Idle in playback configuration with the output muted. */
    if (i2s_config_playback() != HAL_OK)
    {
        return HAL_ERROR;
    }
    (void)nau88c22_set_volume(0);
    (void)nau88c22_select_path(NAU88C22_PATH_NONE);
    s_mode = AUDIO_MODE_OFF;
    return HAL_OK;
}

HAL_StatusTypeDef audio_set_mode(audio_mode_t mode)
{
    if (mode == s_mode)
    {
        return HAL_OK;
    }

    (void)audio_capture_stop();

    switch (mode)
    {
    case AUDIO_MODE_PLAYBACK:
        if (i2s_config_playback() != HAL_OK)      return HAL_ERROR;
        if (nau88c22_select_path(NAU88C22_PATH_PLAYBACK) != HAL_OK) return HAL_ERROR;
        (void)nau88c22_set_volume(NAU88C22_DAC_VOL_MAX);
        break;

    case AUDIO_MODE_CAPTURE:
        if (nau88c22_select_path(NAU88C22_PATH_CAPTURE) != HAL_OK) return HAL_ERROR;
        if (i2s_config_capture() != HAL_OK)       return HAL_ERROR;
        break;

    case AUDIO_MODE_OFF:
    default:
        (void)nau88c22_select_path(NAU88C22_PATH_NONE);
        (void)nau88c22_set_volume(0);
        break;
    }

    s_mode = mode;
    return HAL_OK;
}

audio_mode_t audio_get_mode(void)
{
    return s_mode;
}

HAL_StatusTypeDef audio_play_pcm(const int16_t *samples, uint32_t count)
{
    if (samples == NULL || count == 0U)
    {
        return HAL_ERROR;
    }
    if (audio_set_mode(AUDIO_MODE_PLAYBACK) != HAL_OK)
    {
        return HAL_ERROR;
    }

    uint32_t offset = 0;
    while (offset < count)
    {
        uint32_t chunk = count - offset;
        if (chunk > AUDIO_PCM_BUF_SAMPLES)
        {
            chunk = AUDIO_PCM_BUF_SAMPLES;
        }
        memcpy(s_tx_buf, &samples[offset], chunk * sizeof(int16_t));
        if (HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)s_tx_buf,
                                 (uint16_t)chunk) != HAL_OK)
        {
            return HAL_ERROR;
        }
        /* Wait for the DMA completion callback flag. */
        while (HAL_I2S_GetState(&hi2s1) == HAL_I2S_STATE_BUSY_TX)
        {
        }
        offset += chunk;
    }

    /* Trailing silence so the codec FIFO drains cleanly. */
    memset(s_tx_buf, 0, AUDIO_SILENCE_FRAMES * sizeof(int16_t));
    (void)HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)s_tx_buf,
                               (uint16_t)AUDIO_SILENCE_FRAMES);
    while (HAL_I2S_GetState(&hi2s1) == HAL_I2S_STATE_BUSY_TX)
    {
    }
    return HAL_OK;
}

HAL_StatusTypeDef audio_capture_start(void (*cb)(const int16_t *, uint32_t))
{
    if (audio_set_mode(AUDIO_MODE_CAPTURE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    s_capture_cb = cb;
    return HAL_I2S_Receive_DMA(&hi2s1, (uint16_t *)s_rx_buf,
                               AUDIO_PCM_BUF_SAMPLES / 2U);
}

HAL_StatusTypeDef audio_capture_stop(void)
{
    if (s_mode == AUDIO_MODE_CAPTURE)
    {
        (void)HAL_I2S_DMAStop(&hi2s1);
    }
    return HAL_OK;
}

HAL_StatusTypeDef audio_beep(void)
{
    static int16_t tone[AUDIO_TONE_FRAMES];
    const uint32_t half = AUDIO_SAMPLE_RATE_HZ / (2U * 440U); /* 440 Hz */

    for (uint32_t i = 0; i < AUDIO_TONE_FRAMES; i++)
    {
        tone[i] = ((i / half) & 1U) ? 6000 : -6000;
    }
    return audio_play_pcm(tone, AUDIO_TONE_FRAMES);
}

/* -------------------------------------------------------------------------- */
/*  HAL I2S callbacks                                                         */
/* -------------------------------------------------------------------------- */

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
}
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
    if (s_capture_cb != NULL)
    {
        s_capture_cb(&s_rx_buf[0], AUDIO_PCM_BUF_SAMPLES / 2U);
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    (void)hi2s;
    if (s_capture_cb != NULL)
    {
        s_capture_cb(&s_rx_buf[AUDIO_PCM_BUF_SAMPLES / 2U],
                     AUDIO_PCM_BUF_SAMPLES / 2U);
    }
}
