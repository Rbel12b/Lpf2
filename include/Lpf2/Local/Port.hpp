#pragma once

#include "Lpf2/config.hpp"
#include "Lpf2/Port.hpp"
#include "Lpf2/Local/Serial.hpp"
#include "Lpf2/Local/SerialDef.hpp"
#include "Lpf2/Util/mutex.hpp"

#define MEASUREMENTS 20

namespace Lpf2::Local
{
    class Port : public Lpf2::Port
    {
    public:
        Port() = delete;
        Port(IO &IO) : m_IO(IO), m_serial(m_IO.getUart()), m_pwm(m_IO.getPWM()), m_parser(m_serial) {};

        void init(
#if defined(LPF2_USE_FREERTOS)
            bool useFreeRTOS = true,
            std::string taskName = "uartTask"
#endif
        );

        void update() override;

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

        int writeData(uint8_t modeNum, const std::vector<uint8_t> &data) override;
        void setPower(uint8_t pin1, uint8_t pin2);
        bool deviceConnected() override;

        int setMode(uint8_t mode) override;
        int setModeCombo(uint8_t idx) override;

        void startPower(int8_t pw) override;
        void setAccTime(uint16_t accTime, AccelerationProfile accProfile = 1) override;
        void setDecTime(uint16_t decTime, AccelerationProfile decProfile = 1) override;
        void startSpeed(int8_t speed, uint8_t maxPower, uint8_t useProfile = 0) override;
        void startSpeedForTime(uint16_t time, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void startSpeedForDegrees(uint32_t degrees, int8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void gotoAbsPosition(int32_t absPos, uint8_t speed, uint8_t maxPower, BrakingStyle endState, uint8_t useProfile = 0) override;
        void presetEncoder(int32_t pos) override;

    private:
#if defined(LPF2_USE_FREERTOS)
        static void taskEntryPoint(void *pvParameters);
        void uartTask();
#endif

        void parseMessage(const Message &msg);
        void parseMessageCMD(const Message &msg);
        void parseMessageInfo(const Message &msg);
        void changeBaud(uint32_t baud);
        void sendACK(bool NACK = false);
        void requestSpeedChange(uint32_t speed);

        void resetDevice();
        void enterUartState();

        uint8_t process(unsigned long now);

        void doAnalogID();

        size_t getSpeed() const { return baud; }

    private:
        LPF2_STATUS m_status = LPF2_STATUS::STATUS_ERR;
        LPF2_STATUS m_new_status = LPF2_STATUS::STATUS_ERR;
        LPF2_STATUS m_lastStatus = LPF2_STATUS::STATUS_ERR;
        uint32_t baud = 2400;
        bool m_deviceConnected = false; // do not rely on this, use deviceConnected() instead
        bool nextModeExt = false;
        bool m_dumb = false;
        IO &m_IO;
        Uart *m_serial;
        PWM *m_pwm;
        Parser m_parser;

#ifdef LPF2_MUTEX_INVALID
        Lpf2::Utils::Mutex m_serialMutex = LPF2_MUTEX_INVALID;
#else
        Lpf2::Utils::Mutex m_serialMutex;
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
}; // namespace Lpf2::Local