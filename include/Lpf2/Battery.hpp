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

#pragma once

#include <cstdint>

namespace Lpf2::Battery
{
    // Defaults: max 9000 mV, min 6000 mV, current = max.
    void     setMaxVoltage(uint16_t mV);
    void     setMinVoltage(uint16_t mV);
    uint16_t getMaxVoltage();
    uint16_t getMinVoltage();

    // Manual update path (no ADC required).
    void     setCurrentVoltage(uint16_t mV);
    uint16_t getCurrentVoltage();

    // Voltage -> percent mapping. Default: linear with V_min cutoff.
    // Pass nullptr to restore default.
    using PercentFunc = uint8_t (*)(uint16_t mV, uint16_t vmin, uint16_t vmax);
    void    setPercentFunc(PercentFunc fn);
    uint8_t getPercent();

    // Optional ADC reader. Uses ESP-IDF adc_oneshot + adc_cali_curve_fitting.
    // User calls readBatteryVoltage() periodically from their loop; the
    // freshly-measured mV is stored via setCurrentVoltage().
    struct AdcConfig
    {
        int      adc_channel;     // adc_channel_t value (e.g. ADC_CHANNEL_0)
        int      adc_unit;        // 1 or 2 (ADC_UNIT_1 / ADC_UNIT_2)
        float    r_top_ohms;      // divider top (battery -> tap)
        float    r_bottom_ohms;   // divider bottom (tap -> GND)
        uint16_t vref_mv = 3300;  // informational; cali handles actual ref
        uint8_t  samples = 8;     // averaged per read
    };
    bool     setupAdcDivider(const AdcConfig &cfg);
    uint16_t readBatteryVoltage(); // 0 if ADC not configured
}; // namespace Lpf2::Battery
