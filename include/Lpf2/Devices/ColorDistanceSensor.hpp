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
#include "Lpf2/Devices/ColorSensor.hpp"
#include "Lpf2/Devices/DistanceSensor.hpp"

namespace Lpf2::Devices
{
    class ColorDistanceSensorControl
    {
    public:
        virtual ~ColorDistanceSensorControl() = default;

        /**
         * @brief Get the detected colour index (mode 0 = COLOR).
         */
        virtual ColorIDX getColorIdx() = 0;

        /**
         * @brief Get measured distance in centimetres (mode 1 = DIST).
         */
        virtual float getDistance() = 0;

        /**
         * @brief Get reflected light percentage (mode 3 = REFLT, 0-100 PCT).
         */
        virtual float getReflectivity() = 0;

        /**
         * @brief Get ambient light percentage (mode 4 = AMBI, 0-100 PCT).
         */
        virtual float getAmbientLight() = 0;

        /**
         * @brief Get raw RGB channels (mode 6 = RGB I).
         */
        virtual void getRGB(uint16_t &r, uint16_t &g, uint16_t &b) = 0;

        /**
         * @brief Emit an IR value on the LEGO Power Functions RC channel
         *        (mode 7 = IR TX).
         */
        virtual void setIrTx(uint16_t value) = 0;

        /**
         * @brief Set the on-board LED colour (mode 5 = LED).
         */
        virtual void setLedColor(ColorIDX color) = 0;

        /**
         * @brief Switch the sensor to a specific mode.
         *
         * @param modeNum One of MODE_COLOR, MODE_DIST, MODE_REFLT, MODE_AMBI,
         *                MODE_LED, MODE_RGB, MODE_IR.
         * @param delta   Per-mode delta threshold. 0 = report every update.
         */
        virtual int setMode(uint8_t modeNum, float delta = 1.0f) = 0;
    };

    class ColorDistanceSensor :
        public PortDevice,
        public ColorDistanceSensorControl,
        public TechnicColorSensorControl,
        public TechnicDistanceSensorControl
    {
    public:
        ColorDistanceSensor(Port &port) : PortDevice(port) {}

        bool init() override;
        void update() override;

        const char *name() const override
        {
            return "Color Distance Sensor";
        }

        bool hasCapability(DeviceCapabilityId id) const override;
        void *getCapability(DeviceCapabilityId id) override;

        inline static const DeviceCapabilityId CAP =
            Lpf2CapabilityRegistry::registerCapability("color_distance_sensor");

        static void registerFactory(DeviceRegistry &reg);

        ColorIDX getColorIdx() override;
        float getDistance() override;
        float getReflectivity() override;
        float getAmbientLight() override;
        void getRGB(uint16_t &r, uint16_t &g, uint16_t &b) override;
        void setIrTx(uint16_t value) override;
        void setLedColor(ColorIDX color) override;
        int setMode(uint8_t modeNum, float delta = 1.0f) override;

        inline static const uint8_t MODE_COLOR = 0;
        inline static const uint8_t MODE_DIST = 1;
        inline static const uint8_t MODE_REFLT = 3;
        inline static const uint8_t MODE_AMBI = 4;
        inline static const uint8_t MODE_LED = 5;
        inline static const uint8_t MODE_RGB = 6;
        inline static const uint8_t MODE_IR = 7;

    private:
        bool m_comboActive = false;
        uint8_t m_modeActive = 0;
    };

    class ColorDistanceSensorFactory : public DeviceFactory
    {
    public:
        bool matches(const Port &port) const override;

        PortDevice *create(Port &port) const override
        {
            return new ColorDistanceSensor(port);
        }

        const char *name() const override
        {
            return "Color Distance Sensor Factory";
        }
    };
}; // namespace Lpf2::Devices
