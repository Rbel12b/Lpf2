#pragma once
#ifndef _LPF2_PORT_H_
#define _LPF2_PORT_H_

#include "config.h"

#include <Arduino.h>
#include <Stream.h>
#include "Lpf2Const.h"
#include "Lpf2Serial.h"
#include "Lpf2SerialDef.h"
#include "Util/mutex.h"

#define MEASUREMENTS 20
class Lpf2Port
{
public:
    Lpf2Port(Lpf2IO *IO) : m_IO(IO), m_serial(m_IO->getUart()), m_pwm(m_IO->getPWM()), m_parser(m_serial) {};

    void init(
#if defined(LPF2_USE_FREERTOS)
        bool useFreeRTOS = true,
        std::string taskName = "uartTask"
#endif
    );

    void update();

    enum class LPF2_STATUS
    {
        /* Something bad happened. */
        STATUS_ERR,
        /* Waiting for data that looks like LEGO UART protocol. */
        STATUS_SYNCING,
        /* Reading device info before changing baud rate. */
        STATUS_INFO,
        /* Waiting for ACK */
        STATUS_ACK_WAIT,
        /* Waiting for SYNC */
        STATUS_SYNC_WAIT,
        /* Sending ACK, for data phase begin */
        STATUS_ACK_SENDING,
        /* Sending speed change request */
        STATUS_SPEED_CHANGE,
        /* Speed change accepted */
        STATUS_SPEED,
        /* Waiting for first data packet */
        STATUS_DATA_RECEIVED,
        /* First data packet received, sending setup data */
        STATUS_DATA_START,
        /* Normal data receiving state */
        STATUS_DATA,
        /* Analog identification */
        STATUS_ANALOD_ID,
    };

    class Mode
    {
    public:
        std::string name;
        float min = 0.0f, max = 1023.0f;
        float SImin = 0.0f, SImax = 1023.0f;
        bool negativePCT = false;
        std::string unit;
        struct Mapping
        {
            uint8_t val;

            bool nullSupport() const { return val & (1 << 7); }
            bool mapping2() const { return val & (1 << 6); }
            bool m_abs() const { return val & (1 << 4); }
            bool m_rel() const { return val & (1 << 3); }
            bool m_dis() const { return val & (1 << 2); }
        };
        Mapping in, out;
        uint8_t data_sets = 0, format = 0, figures = 0, decimals = 0;
        std::vector<uint8_t> rawData;
        struct Flags
        {
            uint8_t bytes[6] = {0, 0, 0, 0, 0, 0};

            bool speed() const { return bytes[0] & (1 << 0); }
            bool apos() const { return bytes[0] & (1 << 1); }
            bool pos() const { return bytes[0] & (1 << 2); }
            bool power() const { return bytes[0] & (1 << 4); }
            bool motor() const { return bytes[0] & (1 << 5); }
            bool pin1() const { return bytes[0] & (1 << 6); }
            bool pin2() const { return bytes[0] & (1 << 7); }

            bool calib() const { return bytes[1] & (1 << 6); }

            bool power12() const { return bytes[4] & (1 << 0); }
        };
        Flags flags;
    };

    static float getValue(const Mode &modeData, uint8_t dataSet);
    float getValue(uint8_t modeNum, uint8_t dataSet) const;
    static std::string formatValue(float value, const Mode &modeData);
    static std::string convertValue(Mode modeData);
    std::string convertValue(uint8_t modeNum) const
    {
        if (modeNum >= modeData.size())
        {
            return "<mode not found>";
        }
        return convertValue(modeData[modeNum]);
    }

    int writeData(uint8_t modeNum, const std::vector<uint8_t> &data);

    void setPower(uint8_t pin1, uint8_t pin2);

    Lpf2DeviceType getDeviceType() const { return m_deviceType; }
    size_t getSpeed() const { return baud; }
    uint8_t getModeCount() const { return modes; }
    uint8_t getViewCount() const { return views; }
    const std::vector<Mode> &getModes() const { return modeData; }
    uint8_t getModeComboCount() const { return comboNum; }

    uint16_t getModeCombo(uint8_t combo) const
    {
        if (combo >= comboNum || combo >= 16)
            return 0;
        return modeCombos[combo];
    }

    bool deviceConnected();

    std::vector<Mode> modeData;

private:
#if defined(LPF2_USE_FREERTOS)
    static void taskEntryPoint(void *pvParameters);
    void uartTask();
#endif

    void parseMessage(const Lpf2Message &msg);
    void parseMessageCMD(const Lpf2Message &msg);
    void parseMessageInfo(const Lpf2Message &msg);
    void changeBaud(uint32_t baud);
    void sendACK(bool NACK = false);

    static uint8_t getDataSize(uint8_t format);
    static Lpf2ModeNum getDefaultMode(Lpf2DeviceType id);
    static bool deviceIsAbsMotor(Lpf2DeviceType id);

    void setMode(uint8_t num);
    void requestSpeedChange(uint32_t speed);

    void resetDevice();
    void enterUartState();

    uint8_t process(unsigned long now);

    /// Parse a signed 8-bit integer from raw bytes
    static float parseData8(const uint8_t *ptr);

    /// Parse a signed 16-bit little-endian integer from raw bytes
    static float parseData16(const uint8_t *ptr);

    /// Parse a signed 32-bit little-endian integer from raw bytes
    static float parseData32(const uint8_t *ptr);

    /// Parse a 32-bit IEEE-754 little-endian float
    static float parseDataF(const uint8_t *ptr);

    void doAnalogID();

private:
    LPF2_STATUS m_status = LPF2_STATUS::STATUS_ERR;
    LPF2_STATUS m_new_status = LPF2_STATUS::STATUS_ERR;
    LPF2_STATUS m_lastStatus = LPF2_STATUS::STATUS_ERR;
    Lpf2DeviceType m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
    bool m_deviceConnected = false; // do not rely on this, use deviceConnected() instead
    uint8_t modes, views;
    uint32_t baud = 2400;
    uint16_t modeCombos[16];
    uint8_t comboNum = 0;
    bool nextModeExt = false;
    bool m_dumb = false;
    Lpf2IO *m_IO;
    Lpf2Uart *m_serial;
    Lpf2PWM *m_pwm;
    Lpf2Parser m_parser;

#ifdef LPF2_MUTEX_INVALID
    Mutex m_serialMutex = LPF2_MUTEX_INVALID;
#else
    Mutex m_serialMutex;
#endif

    /**
     * Time of the last data received (millis since startup).
     */
    uint64_t m_startRec = 0;

    /**
     * Time of the start of the current operation (millis since startup).
     */
    uint64_t m_start = 0;

    uint8_t m_mode = 0;

    float ch0Measurements[MEASUREMENTS];
    float ch1Measurements[MEASUREMENTS];
    uint8_t measurementNum = 0;
    uint64_t lastMeasurement = 0;

    const int m_detectionThreshold = 5; // Number of consecutive detections required - 1, so 2 means 3 times
    int m_detectionCounter = 0;
    int m_lastDetectedType = -1;
};

#endif