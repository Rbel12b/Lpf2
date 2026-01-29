#include "Lpf2Port.h"

void Lpf2Port::doAnalogID()
{
    uint64_t now = millis();
    if (now - lastMeasurement < 5)
    {
        return;
    }
    lastMeasurement = now;
    ch0Measurements[measurementNum] = m_serial->readCh(0);
    ch1Measurements[measurementNum] = m_serial->readCh(1);
    measurementNum++;
    if (measurementNum >= MEASUREMENTS)
    {
        measurementNum = 0;
        float ch0min = 5, ch0max = 0, ch1min = 5, ch1max = 0;
        for (int i = 0; i < MEASUREMENTS; i++)
        {
            if (ch0Measurements[i] < ch0min)
            {
                ch0min = ch0Measurements[i];
            }
            if (ch0Measurements[i] > ch0max)
            {
                ch0max = ch0Measurements[i];
            }

            if (ch1Measurements[i] < ch1min)
            {
                ch1min = ch1Measurements[i];
            }
            if (ch1Measurements[i] > ch1max)
            {
                ch1max = ch1Measurements[i];
            }
        }
        float ch0diff = ch0max - ch0min;
        float ch1diff = ch1max - ch1min;
        LPF2_LOG_V("Analog ID results: ch0min=%.2f ch0max=%.2f ch0diff=%.2f | ch1min=%.2f ch1max=%.2f ch1diff=%.2f",
                   ch0min, ch0max, ch0diff,
                   ch1min, ch1max, ch1diff);

        if (ch1diff >= 2.5f)
        {
            if (m_lastDetectedType == 0)
            {
                m_detectionCounter++;
            }
            else
            {
                m_lastDetectedType = 0;
                m_detectionCounter = 0;
            }
            if (m_detectionCounter >= 2) // Uart thresshold is lower on purpose, for faster communication
            {
                // Serial protocol
                m_dumb = false;
                enterUartState();
                LPF2_LOG_D("Uart detected");
            }
            return;
        }
        else if (ch0max >= 3.0f && ch0diff < 0.5f)
        {
            if (ch1min <= 0.5f && ch1diff < 0.5f)
            {
                if (m_lastDetectedType == 1)
                {
                    m_detectionCounter++;
                }
                else
                {
                    m_lastDetectedType = 1;
                    m_detectionCounter = 0;
                }
                if (m_detectionCounter >= m_detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::TRAIN_MOTOR;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Train Motor");
                }
                return;
            }
        }
        else if (ch0min <= 1.0f && ch0diff < 0.5f)
        {
            if (ch1min <= 0.5f && ch1diff < 0.5f)
            {
                if (m_lastDetectedType == 2)
                {
                    m_detectionCounter++;
                }
                else
                {
                    m_lastDetectedType = 2;
                    m_detectionCounter = 0;
                }
                if (m_detectionCounter >= m_detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Simple Motor");
                }
                return;
            }
            else if (ch1min >= 2.5f && ch1diff < 0.5f)
            {
                if (m_lastDetectedType == 3)
                {
                    m_detectionCounter++;
                }
                else
                {
                    m_lastDetectedType = 3;
                    m_detectionCounter = 0;
                }
                if (m_detectionCounter >= m_detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::LIGHT;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Light");
                }
                return;
            }
        }
        LPF2_LOG_V("Analog: No device detected");
        m_detectionCounter = 0;
        m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
        m_lastDetectedType = -1;
    }
}