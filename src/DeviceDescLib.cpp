#include "Lpf2/DeviceDescLib.hpp"

namespace Lpf2
{
    void DeviceDescRegistry::registerDefault()
    {
    #define REGISTER_DEVICE_DESC(dev) \
        DeviceDescRegistry::instance().registerDesc( \
            DeviceType::dev, &DeviceDescriptors::LPF2_DEVICE_ ## dev)

        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_GEST_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_TILT_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_GYRO_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_ACCELEROMETER);
        REGISTER_DEVICE_DESC(HUB_LED);
        REGISTER_DEVICE_DESC(TRAIN_MOTOR);
        REGISTER_DEVICE_DESC(CURRENT_SENSOR);
        REGISTER_DEVICE_DESC(VOLTAGE_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_LARGE_ANGULAR_MOTOR_GREY);
        REGISTER_DEVICE_DESC(TECHNIC_DISTANCE_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY);
        REGISTER_DEVICE_DESC(TECHNIC_COLOR_SENSOR);
        REGISTER_DEVICE_DESC(TECHNIC_LARGE_LINEAR_MOTOR);

    #undef REGISTER_DEVICE_DESC
    }
};

namespace Lpf2::DeviceDescriptors
{
    // Device 0x36
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GEST_SENSOR =
    {
        .type = DeviceType::TECHNIC_MEDIUM_HUB_GEST_SENSOR,
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
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3B
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TILT_SENSOR =
    {
        .type = DeviceType::TECHNIC_MEDIUM_HUB_TILT_SENSOR,
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
                Mode::Flags{{0x00}}
            },
            {
                "IMP",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "CNT",
                0x08, 0x00,
                1, DATA32, 3, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
            {
                "CFG",
                0.0f, 255.0f,
                0.0f, 100.0f,
                0.0f, 255.0f,
                "",
                0x00, 0x10,
                2, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3A
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_GYRO_SENSOR =
    {
        .type = DeviceType::TECHNIC_MEDIUM_HUB_GYRO_SENSOR,
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
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x39
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_ACCELEROMETER =
    {
        .type = DeviceType::TECHNIC_MEDIUM_HUB_ACCELEROMETER,
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
                Mode::Flags{{0x00}}
            },
            {
                "CAL",
                1.0f, 1.0f,
                -100.0f, 100.0f,
                1.0f, 1.0f,
                "",
                0x50, 0x00,
                1, DATA8, 0, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x17
    const DeviceDescriptor LPF2_DEVICE_HUB_LED =
    {
        .type = DeviceType::HUB_LED,
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
                Mode::Flags{{0x00}}
            },
            {
                "RGB O",
                0.0f, 255.0f,
                0.0f, 100.0f,
                0.0f, 255.0f,
                "",
                0x00, 0x10,
                3, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x02
    const DeviceDescriptor LPF2_DEVICE_TRAIN_MOTOR =
    {
        .type = DeviceType::TRAIN_MOTOR,
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
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x15
    const DeviceDescriptor LPF2_DEVICE_CURRENT_SENSOR =
    {
        .type = DeviceType::CURRENT_SENSOR,
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
                Mode::Flags{{0x00}}
            },
            {
                "CUR S",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 4175.0f,
                "mA",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x14
    const DeviceDescriptor LPF2_DEVICE_VOLTAGE_SENSOR =
    {
        .type = DeviceType::VOLTAGE_SENSOR,
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
                Mode::Flags{{0x00}}
            },
            {
                "VLT S",
                0.0f, 4095.0f,
                0.0f, 100.0f,
                0.0f, 9615.0f,
                "mV",
                0x10, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{0x00}}
            },
        }
    };

    // Device 0x3C
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR =
    {
        .type = DeviceType::TECHNIC_MEDIUM_HUB_TEMPERATURE_SENSOR,
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
                Mode::Flags{{0x00}}
            },
        }
    };
    // Device 0x4C
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_ANGULAR_MOTOR_GREY =
    {
        .type = DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY,
        .inModes = 0x0000,
        .outModes = 0x0000,
        .caps = 0x00,
        .combos = { 0x0000 },
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x30 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x21 }}
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
                Mode::Flags{{ 0x00, 0x00, 0x00, 0x24, 0x00, 0x00 }}
            },
            {
                "APOS",
                -180.0f, 179.0f,
                -200.0f, 200.0f,
                -180.0f, 179.0f,
                "DEG",
                0x32, 0x72,
                1, DATA16, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x05, 0x00, 0x00, 0x00, 0x22, 0x00 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x40, 0x22 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x00 }}
            },
        }
    };

    // Device 0x3E
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_DISTANCE_SENSOR =
    {
        .type = DeviceType::TECHNIC_DISTANCE_SENSOR,
        .inModes = 0x0000,
        .outModes = 0x0000,
        .caps = 0x00,
        .combos = { 0x0000 },
        .modes =
        {
            {
                "DISTL",
                0.0f, 2500.0f,
                0.0f, 100.0f,
                0.0f, 250.0f,
                "CM",
                0x91, 0x00,
                1, DATA16, 5, 1,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
            },
            {
                "DISTS",
                0.0f, 320.0f,
                0.0f, 100.0f,
                0.0f, 32.0f,
                "CM",
                0xF1, 0x00,
                1, DATA16, 4, 1,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
            },
            {
                "SINGL",
                0.0f, 2500.0f,
                0.0f, 100.0f,
                0.0f, 250.0f,
                "CM",
                0x90, 0x00,
                1, DATA16, 5, 1,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
            },
            {
                "LISTN",
                0.0f, 1.0f,
                0.0f, 100.0f,
                0.0f, 1.0f,
                "ST",
                0x10, 0x00,
                1, DATA8, 1, 0,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
            },
            {
                "TRAW",
                0.0f, 14577.0f,
                0.0f, 100.0f,
                0.0f, 14577.0f,
                "uS",
                0x90, 0x00,
                1, DATA32, 5, 0,
                {},
                0x00,
                Mode::Flags{{ 0x04, 0x00, 0x00, 0x00, 0x40, 0x00 }}
            },
            {
                "LIGHT",
                0.0f, 100.0f,
                0.0f, 100.0f,
                0.0f, 100.0f,
                "PCT",
                0x00, 0x10,
                4, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x20, 0x40 }}
            },
            {
                "PING",
                0.0f, 1.0f,
                0.0f, 100.0f,
                0.0f, 1.0f,
                "PCT",
                0x00, 0x90,
                1, DATA8, 1, 0,
                {},
                0x00,
                Mode::Flags{{ 0x04, 0x00, 0x00, 0x80, 0x40, 0x00 }}
            },
            {
                "ADRAW",
                0.0f, 1024.0f,
                0.0f, 100.0f,
                0.0f, 1024.0f,
                "PCT",
                0x90, 0x00,
                1, DATA16, 4, 0,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
            },
            {
                "CALIB",
                0.0f, 255.0f,
                0.0f, 100.0f,
                0.0f, 255.0f,
                "PCT",
                0x00, 0x00,
                7, DATA8, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x40, 0x40 }}
            },
        }
    };

    // Device 0x4B
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY =
    {
        .type = DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY,
        .inModes = 0x0000,
        .outModes = 0x0000,
        .caps = 0x00,
        .combos = { 0x0000 },
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x30 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x21 }}
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
                Mode::Flags{{ 0x00, 0x00, 0x00, 0x24, 0x00, 0x00 }}
            },
            {
                "APOS",
                -180.0f, 179.0f,
                -200.0f, 200.0f,
                -180.0f, 179.0f,
                "DEG",
                0x32, 0x72,
                1, DATA16, 3, 0,
                {},
                0x00,
                Mode::Flags{{ 0x05, 0x00, 0x00, 0x00, 0x22, 0x00 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x40, 0x22 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x00 }}
            },
        }
    };

    // Device 0x3D
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_COLOR_SENSOR =
    {
        .type = DeviceType::TECHNIC_COLOR_SENSOR,
        .inModes = 0x0000,
        .outModes = 0x0000,
        .caps = 0x00,
        .combos = { 0x0000 },
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x04, 0x00, 0x00, 0x00, 0x40, 0x00 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x00, 0x00, 0x00, 0x40, 0x00, 0x00 }}
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
                Mode::Flags{{ 0x04, 0x00, 0x00, 0x00, 0x40, 0x00 }}
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x00, 0x40 }}
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
                Mode::Flags{{ 0x84, 0x04, 0x00, 0x00, 0x40, 0x40 }}
            },
        }
    };

    // Device 0x2E
    const DeviceDescriptor LPF2_DEVICE_TECHNIC_LARGE_LINEAR_MOTOR =
    {
        .type = DeviceType::TECHNIC_LARGE_LINEAR_MOTOR,
        .inModes = 0x001E,
        .outModes = 0x001F,
        .caps = 0x0F,
        .combos = { 0x0000 },
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x30 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x21 }}
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
                Mode::Flags{{ 0x00, 0x00, 0x00, 0x24, 0x00, 0x00 }}
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
                Mode::Flags{{ 0x05, 0x00, 0x00, 0x00, 0x22, 0x00 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x40, 0x22 }}
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
                Mode::Flags{{ 0x04, 0x05, 0x00, 0x00, 0x00, 0x00 }}
            },
        }
    };
}; // namespace Lpf2::DeviceDescriptors