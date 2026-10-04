/**
 * @file  nau88c22.c
 * @brief Driver for the Nuvoton NAU88C22YG audio codec.
 *
 * Register values below are the power-on / recommended sequence for a
 * J=1.0 PLL configuration with a 12.288 MHz MCLK, 48 kHz Fs, 16-bit I2S in
 * slave mode. Every register write is marked with the datasheet register
 * name so it can be cross-checked against NAU88C22 DS Rev 1.8.
 *
 * @copyright SPDX-License-Identifier: MIT
 */

#include "nau88c22.h"
#include "config.h"

#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define NAU88C22_ADDR   (NAU88C22_I2C_ADDR_7BIT << 1)
#define NAU88C22_I2C_TIMEOUT_MS  100U

/* ---- Register map (NAU88C22 DS Rev 1.8, Table "Register Map") ------------ */
#define NAU_REG_RESET              0x00U
#define NAU_REG_POWER1             0x01U
#define NAU_REG_POWER2             0x02U
#define NAU_REG_POWER3             0x03U
#define NAU_REG_AUDIO_IF           0x04U
#define NAU_REG_COMPANDING         0x05U
#define NAU_REG_CLOCKING           0x06U
#define NAU_REG_ADDITIONAL         0x07U
#define NAU_REG_GPIO               0x08U
#define NAU_REG_JACK_DETECT        0x09U
#define NAU_REG_DAC_CTRL           0x0AU
#define NAU_REG_DAC_VOL            0x0BU
#define NAU_REG_ADC_CTRL           0x0EU
#define NAU_REG_ADC_VOL            0x0FU
#define NAU_REG_EQ1                0x12U
#define NAU_REG_EQ2                0x13U
#define NAU_REG_EQ3                0x14U
#define NAU_REG_EQ4                0x15U
#define NAU_REG_EQ5                0x16U
#define NAU_REG_DAC_LIMITER1       0x18U
#define NAU_REG_DAC_LIMITER2       0x19U
#define NAU_REG_NOTCH1             0x1BU
#define NAU_REG_NOTCH2             0x1CU
#define NAU_REG_NOTCH3             0x1DU
#define NAU_REG_NOTCH4             0x1EU
#define NAU_REG_ALC1               0x20U
#define NAU_REG_ALC2               0x21U
#define NAU_REG_ALC3               0x22U
#define NAU_REG_NOISE_GATE         0x23U
#define NAU_REG_PLL_N              0x24U
#define NAU_REG_PLL_K1             0x25U
#define NAU_REG_PLL_K2             0x26U
#define NAU_REG_PLL_K3             0x27U
#define NAU_REG_3D                0x29U
#define NAU_REG_BASS              0x2AU
#define NAU_REG_SPK_VOL           0x2BU
#define NAU_REG_INPUT_CTRL         0x2CU
#define NAU_REG_LADC_MIXER         0x2DU
#define NAU_REG_RADC_MIXER         0x2EU
#define NAU_REG_LDAC_MIXER         0x2FU
#define NAU_REG_RDAC_MIXER         0x30U
#define NAU_REG_LDAC_MIXER2        0x31U
#define NAU_REG_RDAC_MIXER2        0x32U
#define NAU_REG_LHP_VOL           0x33U
#define NAU_REG_RHP_VOL           0x34U
#define NAU_REG_LSPK_VOL          0x35U
#define NAU_REG_RSPK_VOL          0x36U
#define NAU_REG_LINE_OUT          0x38U
#define NAU_REG_LIN_VOL           0x39U
#define NAU_REG_RIN_VOL           0x3AU
#define NAU_REG_LMIX_BOOST        0x3BU
#define NAU_REG_RMIX_BOOST        0x3CU
#define NAU_REG_PLL_K4            0x3DU

/* Common field values */
#define NAU_DAC_VOL_DEFAULT        0x39U  /* -6 dB-ish, safe start level */
#define NAU_SPK_VOL_DEFAULT        0x39U

static bool s_present;

/* -------------------------------------------------------------------------- */
/*  Low-level register access                                                 */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef nau88c22_write_reg(uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)((value >> 8) & 0xFFU);
    buf[1] = (uint8_t)(value & 0xFFU);
    return HAL_I2C_Mem_Write(&hi2c1, NAU88C22_ADDR, reg,
                             I2C_MEMADD_SIZE_8BIT, buf, 2,
                             NAU88C22_I2C_TIMEOUT_MS);
}

HAL_StatusTypeDef nau88c22_read_reg(uint8_t reg, uint16_t *value)
{
    uint8_t buf[2] = {0, 0};
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(&hi2c1, NAU88C22_ADDR, reg,
                                            I2C_MEMADD_SIZE_8BIT, buf, 2,
                                            NAU88C22_I2C_TIMEOUT_MS);
    if (st == HAL_OK && value != NULL)
    {
        *value = ((uint16_t)buf[0] << 8) | buf[1];
    }
    return st;
}

static HAL_StatusTypeDef nau_write(uint8_t reg, uint16_t value)
{
    return nau88c22_write_reg(reg, value);
}

/* -------------------------------------------------------------------------- */
/*  Initialisation                                                            */
/* -------------------------------------------------------------------------- */

bool nau88c22_is_present(void)
{
    return s_present;
}

HAL_StatusTypeDef nau88c22_init(void)
{
    uint16_t id = 0;

    /* Software reset (all registers back to default). */
    if (nau_write(NAU_REG_RESET, 0x00U) != HAL_OK)      return HAL_ERROR;
    HAL_Delay(10);

    /* Probe: read back the reset register (0x000 expected). */
    if (nau88c22_read_reg(NAU_REG_RESET, &id) != HAL_OK)
    {
        s_present = false;
        return HAL_ERROR;
    }

    /* --- Power management: prepare 3.3 V / 5 V rails ---- POWER1/2/3 ------- */
    if (nau_write(NAU_REG_POWER1, 0x00EU) != HAL_OK)    return HAL_ERROR; /* VMID, bias, PLL */
    if (nau_write(NAU_REG_POWER2, 0x001U) != HAL_OK)    return HAL_ERROR; /* speaker driver */
    if (nau_write(NAU_REG_POWER3, 0x00EU) != HAL_OK)    return HAL_ERROR; /* DAC + ADC + mic bias */

    /* --- Audio interface: I2S, 16-bit, slave ----------------- AUDIO_IF ---- */
    if (nau_write(NAU_REG_AUDIO_IF, 0x000U) != HAL_OK)  return HAL_ERROR;

    /* --- Clocking: PLL enabled, 12.288 MHz MCLK, 256xFs --------- CLOCKING - */
    if (nau_write(NAU_REG_CLOCKING, 0x000U) != HAL_OK)  return HAL_ERROR;
    if (nau_write(NAU_REG_ADDITIONAL, 0x000U) != HAL_OK) return HAL_ERROR; /* normal sample rate */

    /* --- PLL: (K1,K2,K3) for MCLK=12.288 MHz -> 48 kHz -------------------- */
    if (nau_write(NAU_REG_PLL_N, 0x008U) != HAL_OK)     return HAL_ERROR;
    if (nau_write(NAU_REG_PLL_K1, 0x00CU) != HAL_OK)    return HAL_ERROR;
    if (nau_write(NAU_REG_PLL_K2, 0x009U) != HAL_OK)    return HAL_ERROR;
    if (nau_write(NAU_REG_PLL_K3, 0x03CU) != HAL_OK)    return HAL_ERROR;

    /* --- DAC + ADC enable, disable unneeded signal processing ------------- */
    if (nau_write(NAU_REG_DAC_CTRL, 0x000U) != HAL_OK)  return HAL_ERROR;
    if (nau_write(NAU_REG_ADC_CTRL, 0x000U) != HAL_OK)  return HAL_ERROR;

    /* --- Mixers: DAC -> speaker + headphone; ADC <- microphone ------------ */
    if (nau_write(NAU_REG_LDAC_MIXER, 0x001U) != HAL_OK) return HAL_ERROR; /* LSPK <- LDAC */
    if (nau_write(NAU_REG_RDAC_MIXER, 0x001U) != HAL_OK) return HAL_ERROR; /* RSPK <- RDAC */
    if (nau_write(NAU_REG_LDAC_MIXER2, 0x001U) != HAL_OK) return HAL_ERROR; /* LHP  <- LDAC */
    if (nau_write(NAU_REG_RDAC_MIXER2, 0x001U) != HAL_OK) return HAL_ERROR; /* RHP  <- RDAC */
    if (nau_write(NAU_REG_LADC_MIXER, 0x001U) != HAL_OK) return HAL_ERROR; /* LADC <- LMICP */
    if (nau_write(NAU_REG_RADC_MIXER, 0x001U) != HAL_OK) return HAL_ERROR; /* RADC <- RMICP */

    /* --- Input control: single-ended mic on L/R MIC ---------------------- */
    if (nau_write(NAU_REG_INPUT_CTRL, 0x000U) != HAL_OK) return HAL_ERROR;

    /* --- Volumes ---------------------------------------------------------- */
    if (nau_write(NAU_REG_DAC_VOL, NAU_DAC_VOL_DEFAULT) != HAL_OK) return HAL_ERROR;
    if (nau_write(NAU_REG_ADC_VOL, 0x000U) != HAL_OK)   return HAL_ERROR;
    if (nau_write(NAU_REG_SPK_VOL, NAU_SPK_VOL_DEFAULT) != HAL_OK) return HAL_ERROR;
    if (nau_write(NAU_REG_LSPK_VOL, NAU_SPK_VOL_DEFAULT) != HAL_OK) return HAL_ERROR;
    if (nau_write(NAU_REG_RSPK_VOL, NAU_SPK_VOL_DEFAULT) != HAL_OK) return HAL_ERROR;
    if (nau_write(NAU_REG_LHP_VOL, 0x000U) != HAL_OK)   return HAL_ERROR;
    if (nau_write(NAU_REG_RHP_VOL, 0x000U) != HAL_OK)   return HAL_ERROR;

    /* Disable DSP extras that would colour the voice path. */
    if (nau_write(NAU_REG_EQ1, 0x000U) != HAL_OK)       return HAL_ERROR;
    if (nau_write(NAU_REG_3D, 0x000U) != HAL_OK)        return HAL_ERROR;
    if (nau_write(NAU_REG_BASS, 0x000U) != HAL_OK)      return HAL_ERROR;
    if (nau_write(NAU_REG_ALC1, 0x000U) != HAL_OK)      return HAL_ERROR;
    if (nau_write(NAU_REG_NOISE_GATE, 0x000U) != HAL_OK) return HAL_ERROR;
    if (nau_write(NAU_REG_DAC_LIMITER1, 0x000U) != HAL_OK) return HAL_ERROR;

    s_present = true;

    /* Start in playback mode (speaker muted until audio starts). */
    (void)nau88c22_select_path(NAU88C22_PATH_NONE);
    return HAL_OK;
}

/* -------------------------------------------------------------------------- */
/*  Path control                                                              */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef nau88c22_select_path(nau88c22_path_t path)
{
    HAL_StatusTypeDef st = HAL_OK;

    switch (path)
    {
    case NAU88C22_PATH_PLAYBACK:
        /* Disable ADC (stops the codec driving ADCOUT on the shared SD net). */
        st |= nau88c22_write_reg(NAU_REG_ADC_CTRL, 0x040U);   /* ADC off   */
        st |= nau88c22_write_reg(NAU_REG_DAC_CTRL, 0x000U);   /* DAC on    */
        st |= nau88c22_speaker_enable(true);
        break;

    case NAU88C22_PATH_CAPTURE:
        /* Disable DAC + speaker (codec only listens on DACIN). */
        st |= nau88c22_speaker_enable(false);
        st |= nau88c22_write_reg(NAU_REG_DAC_CTRL, 0x040U);   /* DAC off   */
        st |= nau88c22_write_reg(NAU_REG_ADC_CTRL, 0x000U);   /* ADC on    */
        break;

    case NAU88C22_PATH_NONE:
    default:
        st |= nau88c22_speaker_enable(false);
        st |= nau88c22_headphone_enable(false);
        st |= nau88c22_write_reg(NAU_REG_DAC_CTRL, 0x040U);
        st |= nau88c22_write_reg(NAU_REG_ADC_CTRL, 0x040U);
        break;
    }
    return st;
}

/* -------------------------------------------------------------------------- */
/*  Volume / gain                                                             */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef nau88c22_set_volume(uint8_t vol)
{
    if (vol > NAU88C22_DAC_VOL_MAX)
    {
        vol = NAU88C22_DAC_VOL_MAX;
    }
    /* DAC volume: bit6 = update (write-trigger), bits5:0 = volume. */
    return nau88c22_write_reg(NAU_REG_DAC_VOL, (uint16_t)(vol & 0x3FU));
}

HAL_StatusTypeDef nau88c22_set_mic_gain(uint8_t gain)
{
    if (gain > NAU88C22_MIC_GAIN_MAX)
    {
        gain = NAU88C22_MIC_GAIN_MAX;
    }
    return nau88c22_write_reg(NAU_REG_ADC_VOL, (uint16_t)(gain & 0x3FU));
}

/* -------------------------------------------------------------------------- */
/*  Output enables / mute                                                     */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef nau88c22_speaker_enable(bool enable)
{
    HAL_StatusTypeDef st;
    st  = nau88c22_write_reg(NAU_REG_LSPK_VOL, enable ? NAU_SPK_VOL_DEFAULT : 0x040U);
    st |= nau88c22_write_reg(NAU_REG_RSPK_VOL, enable ? NAU_SPK_VOL_DEFAULT : 0x040U);
    return st;
}

HAL_StatusTypeDef nau88c22_headphone_enable(bool enable)
{
    HAL_StatusTypeDef st;
    st  = nau88c22_write_reg(NAU_REG_LHP_VOL, enable ? 0x039U : 0x040U);
    st |= nau88c22_write_reg(NAU_REG_RHP_VOL, enable ? 0x039U : 0x040U);
    return st;
}

HAL_StatusTypeDef nau88c22_mute(bool mute)
{
    uint16_t vol = mute ? 0x040U : (uint16_t)(NAU_DAC_VOL_DEFAULT & 0x3FU);
    return nau88c22_write_reg(NAU_REG_DAC_VOL, vol);
}
