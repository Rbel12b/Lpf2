#include "Lpf2DeviceDescLib.h"

namespace Lpf2DeviceDescriptors
{
    // Device 0x36
    const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GEST_SENSOR =
    {
        .type = Lpf2DeviceType::TECHNIC_MEDIUM_HUB_GEST_SENSOR,
        .inModes = 0x0001,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "GEST",
                0.0f, 4.0f,
                0.0f, 100.0f,
                0.0f, 4.0f,
                "",
                0x44, 0x00,
                1, DATA8, 1, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3B
    const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TILT_SENSOR =
    {
        .type = Lpf2DeviceType::TECHNIC_MEDIUM_HUB_TILT_SENSOR,
        .inModes = 0x0003,
        .outModes = 0x0004,
        .caps = 0x03,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "POS",
                -180.0f, 180.0f,
                -100.0f, 100.0f,
                -180.0f, 180.0f,
                "DEG",
                0x50, 0x00,
                3, DATA16, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "CNT",
                0x08, 0x00,
                1, DATA32, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                0.0f, 255.0f,
                0.0f, 100.0f,
                0.0f, 255.0f,
                "",
                0x00, 0x10,
                2, DATA8, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3A
    const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GYRO_SENSOR =
    {
        .type = Lpf2DeviceType::TECHNIC_MEDIUM_HUB_GYRO_SENSOR,
        .inModes = 0x0001,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "ROT",
                -28571.419922f, 28571.419922f,
                -100.0f, 100.0f,
                -2000.0f, 2000.0f,
                "DPS",
                0x50, 0x00,
                3, DATA16, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x39
    const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_ACCELEROMETER =
    {
        .type = Lpf2DeviceType::TECHNIC_MEDIUM_HUB_ACCELEROMETER,
        .inModes = 0x0003,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "GRV",
                -32768.0f, 32768.0f,
                -100.0f, 100.0f,
                -8000.0f, 8000.0f,
                "mG",
                0x50, 0x00,
                3, DATA16, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                1.0f, 1.0f,
                -100.0f, 100.0f,
                1.0f, 1.0f,
                "",
                0x50, 0x00,
                1, DATA8, 0, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x17
    const Lpf2DeviceDescriptor LPF2_DEVICE_HUB_LED =
    {
        .type = Lpf2DeviceType::HUB_LED,
        .inModes = 0x0000,
        .outModes = 0x0003,
        .caps = 0x01,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "COL O",
                0.0f, 10.0f,
                0.0f, 100.0f,
                0.0f, 10.0f,
                "",
                0x00, 0x44,
                1, DATA8, 1, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                0.0f, 255.0f,
                0.0f, 100.0f,
                0.0f, 255.0f,
                "",
                0x00, 0x10,
                3, DATA8, 3, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x02
    const Lpf2DeviceDescriptor LPF2_DEVICE_TRAIN_MOTOR =
    {
        .type = Lpf2DeviceType::TRAIN_MOTOR,
        .inModes = 0x0000,
        .outModes = 0x0001,
        .caps = 0x01,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "LPF2-TRAIN",
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                -100.0f, 100.0f,
                "",
                0x00, 0x18,
                1, DATA8, 4, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x15
    const Lpf2DeviceDescriptor LPF2_DEVICE_CURRENT_SENSOR =
    {
        .type = Lpf2DeviceType::CURRENT_SENSOR,
        .inModes = 0x0003,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "CUR L",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 4175.0f,
                "mA",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 4175.0f,
                "mA",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x14
    const Lpf2DeviceDescriptor LPF2_DEVICE_VOLTAGE_SENSOR =
    {
        .type = Lpf2DeviceType::VOLTAGE_SENSOR,
        .inModes = 0x0003,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "VLT L",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 9615.0f,
                "mV",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
            {
                "",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 9615.0f,
                "mV",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3C
    const Lpf2DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR =
    {
        .type = Lpf2DeviceType::TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR,
        .inModes = 0x0001,
        .outModes = 0x0000,
        .caps = 0x02,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "TEMP",
                -900.0f, 900.0f,
                -100.0f, 100.0f,
                -90.0f, 90.0f,
                "DEG",
                0x50, 0x00,
                1, DATA16, 5, 1,
                {},
                0x00,
                Lpf2Mode::Flags{{0x00}}
            },
        }
    };
}; // namespace Lpf2DeviceDescriptors