#include "Lpf2Port.h"

void Lpf2Port::doAnalogID()
{
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

        static int detectionCounter = 0;
        static const int detectionThreshold = 5; // Number of consecutive detections required - 1, so 2 means 3 times
        static int lastDetectedType = -1;
        if (ch1diff >= 2.5f)
        {
            if (lastDetectedType == 0)
            {
                detectionCounter++;
            }
            else
            {
                lastDetectedType = 0;
                detectionCounter = 0;
            }
            if (detectionCounter >= 2) // Uart thresshold is lower on purpose, for faster communication
            {
                // Serial protocol
                m_dumb = false;
                enterUartState();
                LPF2_LOG_D("Uart detected");
            }
            goto end_analog_check;
        }
        else if (ch0max >= 3.0f && ch0diff < 0.5f)
        {
            if (ch1min <= 0.5f && ch1diff < 0.5f)
            {
                if (lastDetectedType == 1)
                {
                    detectionCounter++;
                }
                else
                {
                    lastDetectedType = 1;
                    detectionCounter = 0;
                }
                if (detectionCounter >= detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::TRAIN_MOTOR;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Train Motor");
                }
                goto end_analog_check;
            }
        }
        else if (ch0min <= 1.0f && ch0diff < 0.5f)
        {
            if (ch1min <= 0.5f && ch1diff < 0.5f)
            {
                if (lastDetectedType == 2)
                {
                    detectionCounter++;
                }
                else
                {
                    lastDetectedType = 2;
                    detectionCounter = 0;
                }
                if (detectionCounter >= detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Simple Motor");
                }
                goto end_analog_check;
            }
            else if (ch1min >= 2.5f && ch1diff < 0.5f)
            {
                if (lastDetectedType == 3)
                {
                    detectionCounter++;
                }
                else
                {
                    lastDetectedType = 3;
                    detectionCounter = 0;
                }
                if (detectionCounter >= detectionThreshold)
                {
                    m_deviceType = Lpf2DeviceType::LIGHT;
                    m_dumb = true;
                    LPF2_LOG_V("Analog: Light");
                }
                goto end_analog_check;
            }
        }
        LPF2_LOG_V("Analog: No device detected");
        detectionCounter = 0;
        m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
        lastDetectedType = -1;
    }
end_analog_check:
    vTaskDelay(5 / portTICK_PERIOD_MS);
}