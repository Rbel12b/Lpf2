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

#include "Lpf2/DeviceDescLib.hpp"

namespace Lpf2
{
    void DeviceDescRegistry::registerDefault()
    {
    #define REGISTER_DEVICE_DESC(dev) \
        DeviceDescRegistry::instance().registerDesc( \
            DeviceType::dev, &DeviceDescriptors:: dev)

        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_GEST_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_TILT_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_GYRO_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_ACCELEROMETER);
        // REGISTER_DEVICE_DESC(HUB_LED);
        // REGISTER_DEVICE_DESC(TRAIN_MOTOR);
        // REGISTER_DEVICE_DESC(CURRENT_SENSOR);
        // REGISTER_DEVICE_DESC(VOLTAGE_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_LARGE_ANGULAR_MOTOR_GREY);
        // REGISTER_DEVICE_DESC(TECHNIC_DISTANCE_SENSOR);
        // REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY);
        REGISTER_DEVICE_DESC(TECHNIC_COLOR_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_LARGE_LINEAR_MOTOR);

    #undef REGISTER_DEVICE_DESC
    }
};

namespace Lpf2::DeviceDescriptors
{
    // Device 0x3D
    const DeviceDescriptor TECHNIC_COLOR_SENSOR =
    {
        .type = DeviceType::TECHNIC_COLOR_SENSOR,
        .inModesMask = 0x0000,
        .outModesMask = 0x0000,
        .caps = 0x00,
        .combos = {0x0063, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        .fwVersion = Version({
            .Major = 1,
            .Minor = 0,
            .Bugfix = 0,
            .Build = 0,
        }),
        .hwVersion = Version({
            .Major = 1,
            .Minor = 0,
            .Bugfix = 0,
            .Build = 0,
        }),
        .modes =
        {
            {
                "COLOR",
                0.0f, 10.0f,
                0.0f, 100.0f,
                0.0f, 10.0f,
                "IDX",
                0xE4, 0x00,
                1, DATA8, 2, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x04, 0x84 }}
            },
            {
                "REFLT",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "PCT",
                0x30, 0x00,
                1, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x04, 0x84 }}
            },
            {
                "AMBI",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "PCT",
                0x30, 0x00,
                1, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x40, 0x00, 0x00, 0x00, 0x04 }}
            },
            {
                "LIGHT",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "PCT",
                0x00, 0x10,
                3, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x05, 0x04 }}
            },
            {
                "RREFL",
                0.0f, 1024.0f,
                0.0f, 100.0f,
                0.0f, 1024.0f,
                "RAW",
                0x10, 0x00,
                2, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x04, 0x84 }}
            },
            {
                "RGB I",
                0.0f, 1024.0f,
                0.0f, 100.0f,
                0.0f, 1024.0f,
                "RAW",
                0x10, 0x00,
                4, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x04, 0x84 }}
            },
            {
                "HSV",
                0.0f, 360.0f,
                0.0f, 100.0f,
                0.0f, 360.0f,
                "RAW",
                0x10, 0x00,
                3, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 }}
            },
            {
                "SHSV",
                0.0f, 360.0f,
                0.0f, 100.0f,
                0.0f, 360.0f,
                "RAW",
                0x10, 0x00,
                4, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x40, 0x00, 0x00, 0x00, 0x04 }}
            },
            {
                "DEBUG",
                0.0f, 65535.0f,
                0.0f, 100.0f,
                0.0f, 65535.0f,
                "RAW",
                0x10, 0x00,
                4, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x00, 0x00, 0x00, 0x04, 0x84 }}
            },
            {
                "CALIB",
                0.0f, 65535.0f,
                0.0f, 100.0f,
                0.0f, 65535.0f,
                "",
                0x00, 0x00,
                7, DATA16, 5, 0,
                {},
                0x00,
                Mode::Flags{{ 0x40, 0x40, 0x00, 0x00, 0x04, 0x84 }}
            },
        }
    };

    const DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_LINEAR_MOTOR =
    {
        .type = DeviceType::TECHNIC_LARGE_LINEAR_MOTOR,
        .inModesMask = 0x0000,
        .outModesMask = 0x0000,
        .caps = 0x00,
        .combos = {0x000E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        .fwVersion = Version({
            .Major = 0,
            .Minor = 0,
            .Bugfix = 0,
            .Build = 4,
        }),
        .hwVersion = Version({
            .Major = 1,
            .Minor = 0,
            .Bugfix = 0,
            .Build = 0,
        }),
        .modes =
        {
            {
                "POWER",
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                "PCT",
                0x00, 0x50,
                1, DATA8, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x30, 0x00, 0x00, 0x00, 0x05, 0x04 }}
            },
            {
                "SPEED",
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                "PCT",
                0x30, 0x70,
                1, DATA8, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x21, 0x00, 0x00, 0x00, 0x05, 0x04 }}
            },
            {
                "POS",
                -360.0f, 360.0f,
                -100.0f, 100.0f,
                -360.0f, 360.0f,
                "DEG",
                0x28, 0x68,
                1, DATA32, 11, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x00, 0x24, 0x00, 0x00, 0x00 }}
            },
            {
                "APOS",
                -180.0f, 179.0f,
                -200.0f, 200.0f,
                -180.0f, 179.0f,
                "DEG",
                0x32, 0x32,
                1, DATA16, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x22, 0x00, 0x00, 0x00, 0x05 }}
            },
            {
                "CALIB",
                0.0f, 3600.0f,
                0.0f, 100.0f,
                0.0f, 3600.0f,
                "CAL",
                0x00, 0x00,
                2, DATA16, 5, 0,
                {},
                0x00,
                Mode::Flags{{ 0x22, 0x40, 0x00, 0x00, 0x05, 0x04 }}
            },
            {
                "STATS",
                0.0f, 65535.0f,
                0.0f, 100.0f,
                0.0f, 65535.0f,
                "MIN",
                0x00, 0x00,
                14, DATA16, 5, 0,
                {},
                0x00,
                Mode::Flags{{ 0x00, 0x00, 0x00, 0x00, 0x05, 0x04 }}
            },
        }
    };
}; // namespace Lpf2::DeviceDescriptors