/**
 *  Copyright (C) 2026 - Rbel12b
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

#include "Lpf2/Battery.hpp"
#include "Lpf2/log/log.h"

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_err.h>

namespace Lpf2::Battery
{
    namespace
    {
        constexpr uint16_t DEFAULT_MAX_MV = 9000;
        constexpr uint16_t DEFAULT_MIN_MV = 6000;

        uint16_t g_maxVoltage = DEFAULT_MAX_MV;
        uint16_t g_minVoltage = DEFAULT_MIN_MV;
        uint16_t g_curVoltage = DEFAULT_MAX_MV;

        uint8_t defaultPercent(uint16_t mV, uint16_t vmin, uint16_t vmax)
        {
            if (vmax <= vmin)
                return 0;
            if (mV >= vmax)
                return 100;
            if (mV <= vmin)
                return 0;
            return (uint8_t)((uint32_t)(mV - vmin) * 100 / (vmax - vmin));
        }

        PercentFunc g_percentFunc = defaultPercent;

        // ADC state.
        adc_oneshot_unit_handle_t g_adc_handle = nullptr;
        adc_cali_handle_t         g_cali_handle = nullptr;
        adc_channel_t             g_adc_channel = (adc_channel_t)0;
        float                     g_divider_ratio = 1.0f; // (Rt+Rb)/Rb
        uint8_t                   g_adc_samples = 8;
    } // namespace

    void setMaxVoltage(uint16_t mV)     { g_maxVoltage = mV; }
    void setMinVoltage(uint16_t mV)     { g_minVoltage = mV; }
    uint16_t getMaxVoltage()            { return g_maxVoltage; }
    uint16_t getMinVoltage()            { return g_minVoltage; }

    void setCurrentVoltage(uint16_t mV) { g_curVoltage = mV; }
    uint16_t getCurrentVoltage()        { return g_curVoltage; }

    void setPercentFunc(PercentFunc fn)
    {
        g_percentFunc = fn ? fn : defaultPercent;
    }

    uint8_t getPercent()
    {
        return g_percentFunc(g_curVoltage, g_minVoltage, g_maxVoltage);
    }

    bool setupAdcDivider(const AdcConfig &cfg)
    {
        if (cfg.r_bottom_ohms <= 0.0f)
            return false;

        // Tear down any prior init (idempotent reconfigure).
        if (g_cali_handle)
        {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            adc_cali_delete_scheme_curve_fitting(g_cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
            adc_cali_delete_scheme_line_fitting(g_cali_handle);
#endif
            g_cali_handle = nullptr;
        }
        if (g_adc_handle)
        {
            adc_oneshot_del_unit(g_adc_handle);
            g_adc_handle = nullptr;
        }

        adc_unit_t unit = (cfg.adc_unit == 2) ? ADC_UNIT_2 : ADC_UNIT_1;
        g_adc_channel = (adc_channel_t)cfg.adc_channel;
        g_adc_samples = cfg.samples ? cfg.samples : 1;
        g_divider_ratio =
            (cfg.r_top_ohms + cfg.r_bottom_ohms) / cfg.r_bottom_ohms;

        adc_oneshot_unit_init_cfg_t init_cfg = {};
        init_cfg.unit_id = unit;
        init_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
        if (adc_oneshot_new_unit(&init_cfg, &g_adc_handle) != ESP_OK)
        {
            g_adc_handle = nullptr;
            return false;
        }

        adc_oneshot_chan_cfg_t ch_cfg = {};
        ch_cfg.atten = ADC_ATTEN_DB_12; // full ~0-3.3V range
        ch_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
        if (adc_oneshot_config_channel(g_adc_handle, g_adc_channel, &ch_cfg)
            != ESP_OK)
        {
            adc_oneshot_del_unit(g_adc_handle);
            g_adc_handle = nullptr;
            return false;
        }

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_curve_fitting_config_t cali_cfg = {};
        cali_cfg.unit_id = unit;
        cali_cfg.chan = g_adc_channel;
        cali_cfg.atten = ADC_ATTEN_DB_12;
        cali_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
        if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &g_cali_handle)
            != ESP_OK)
        {
            g_cali_handle = nullptr;
        }
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_line_fitting_config_t cali_cfg = {};
        cali_cfg.unit_id = unit;
        cali_cfg.atten = ADC_ATTEN_DB_12;
        cali_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
        if (adc_cali_create_scheme_line_fitting(&cali_cfg, &g_cali_handle)
            != ESP_OK)
        {
            g_cali_handle = nullptr;
        }
#endif

        (void)cfg.vref_mv;
        return true;
    }

    uint16_t readBatteryVoltage()
    {
        if (!g_adc_handle)
            return 0;

        int32_t acc_raw = 0;
        int valid = 0;
        for (uint8_t i = 0; i < g_adc_samples; ++i)
        {
            int raw = 0;
            if (adc_oneshot_read(g_adc_handle, g_adc_channel, &raw) == ESP_OK)
            {
                acc_raw += raw;
                ++valid;
            }
        }
        if (valid == 0)
            return 0;

        int avg_raw = (int)(acc_raw / valid);
        int v_tap_mv = 0;
        if (g_cali_handle)
        {
            if (adc_cali_raw_to_voltage(g_cali_handle, avg_raw, &v_tap_mv)
                != ESP_OK)
            {
                v_tap_mv = 0;
            }
        }
        else
        {
            // Uncalibrated fallback: assume 12-bit, 3300 mV full-scale.
            v_tap_mv = avg_raw * 3300 / 4095;
        }

        float v_batt_f = (float)v_tap_mv * g_divider_ratio;
        if (v_batt_f < 0.0f) v_batt_f = 0.0f;
        if (v_batt_f > 65535.0f) v_batt_f = 65535.0f;
        uint16_t v_batt = (uint16_t)v_batt_f;
        setCurrentVoltage(v_batt);
        return v_batt;
    }
}; // namespace Lpf2::Battery
