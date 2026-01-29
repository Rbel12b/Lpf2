#pragma once
#ifndef _LPF2_PORT_LOCAL_H_
#define _LPF2_PORT_LOCAL_H_

#include "config.h"
#include "Lpf2Port.h"
#include "Lpf2Serial.h"
#include "Lpf2SerialDef.h"
#include "Util/mutex.h"

#define MEASUREMENTS 20

class Lpf2PortLocal : public Lpf2Port
{
public:
    Lpf2PortLocal(Lpf2IO *IO) : m_IO(IO), m_serial(m_IO->getUart()), m_pwm(m_IO->getPWM()), m_parser(m_serial) {};

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
    void setPower(uint8_t pin1, uint8_t pin2) override;
    bool deviceConnected() override;

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

    void setMode(uint8_t num);
    void requestSpeedChange(uint32_t speed);

    void resetDevice();
    void enterUartState();

    uint8_t process(unsigned long now);

    void doAnalogID();

private:
    LPF2_STATUS m_status = LPF2_STATUS::STATUS_ERR;
    LPF2_STATUS m_new_status = LPF2_STATUS::STATUS_ERR;
    LPF2_STATUS m_lastStatus = LPF2_STATUS::STATUS_ERR;
    bool m_deviceConnected = false; // do not rely on this, use deviceConnected() instead
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