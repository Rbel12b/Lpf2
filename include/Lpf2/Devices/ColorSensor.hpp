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

#include "Lpf2/config.hpp"
#include "Lpf2/DeviceFactory.hpp"

namespace Lpf2::Devices
{
    class TechnicColorSensorControl
    {
    public:
        virtual ~TechnicColorSensorControl() = default;

        /**
         * @brief Get the color index from the sensor (mode 0 = COLOR).
         */
        virtual ColorIDX getColorIdx() = 0;

        /**
         * @brief Get reflected light percentage (mode 1 = REFLT, 0-100 PCT).
         */
        virtual float getReflectivity() = 0;

        /**
         * @brief Get raw RGB channels and intensity (mode 5 = RGB I).
         */
        virtual void getRGB(uint16_t &r, uint16_t &g, uint16_t &b) = 0;

        /**
         * @brief Get HSV readings (mode 6 = HSV; H 0-360, S 0-100, V 0-360).
         */
        virtual void getHSV(uint16_t &h, uint16_t &s, uint16_t &v)
        {
            return;
        }

        /**
         * @brief Get ambient light percentage (mode 2 = AMBI, 0-100 PCT).
         */
        virtual float getAmbientLight() = 0;

        /**
         * @brief Set the on-board light channels (mode 3 = LIGHT; 0-100 PCT each).
         */
        virtual void setLight(uint8_t l1, uint8_t l2, uint8_t l3)
        {
            return;
        }

        /**
         * @brief Switch the sensor to a specific mode.
         *
         * Modes 0, 1, 5, 6 are configured through a mode combo internally so
         * multiple values (colour, reflectivity, RGB, HSV) can be read back
         * without repeated mode switches.
         *
         * @param modeNum One of MODE_COLOR, MODE_REFLT, MODE_AMBI, MODE_LIGHT,
         *                MODE_RGB, MODE_HSV.
         * @param delta   Per-mode delta threshold. 0 = report every update.
         */
        virtual int setMode(uint8_t modeNum, float delta = 1.0f) = 0;
    };

    class TechnicColorSensor : public PortDevice, public TechnicColorSensorControl
    {
    public:
        TechnicColorSensor(Port &port) : PortDevice(port) {}

        bool init() override;
        void update() override;

        const char *name() const override
        {
            return "Technic Color Sensor";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("technic_color_sensor");

        static void registerFactory(DeviceRegistry &reg);

        inline static const int MODE_COLOR = 0;
        inline static const int MODE_REFLT = 1;
        inline static const int MODE_AMBI = 2;
        inline static const int MODE_LIGHT = 3;
        inline static const int MODE_RGB = 5;
        inline static const int MODE_HSV = 6;

        ColorIDX getColorIdx() override;
        float getReflectivity() override;
        float getAmbientLight() override;
        void getRGB(uint16_t &r, uint16_t &g, uint16_t &b) override;
        void getHSV(uint16_t &h, uint16_t &s, uint16_t &v) override;
        void setLight(uint8_t l1, uint8_t l2, uint8_t l3) override;
        int setMode(uint8_t modeNum, float delta = 1.0f) override;

    private:
        bool m_comboActive = false;
        uint8_t m_modeActive = 0;
    };

    class TechnicColorSensorFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new TechnicColorSensor(port);
        }

        const char *name() const
        {
            return "Technic Color Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices