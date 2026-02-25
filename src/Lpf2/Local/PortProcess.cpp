#include "Lpf2/Local/Port.hpp"
#include <string>
#include <cstring>

namespace Lpf2::Local
{
    void Port::init(
#if defined(LPF2_USE_FREERTOS)
        bool useFreeRTOSTask,
        std::string taskName
#endif
    )
    {

#ifdef LPF2_MUTEX_INVALID
        if (m_serialMutex == LPF2_MUTEX_INVALID)
        {
            m_serialMutex = LPF2_MUTEX_CREATE();
            configASSERT(m_serialMutex != LPF2_MUTEX_INVALID);
        }
#endif

        if (!m_IO.ready())
        {
            return;
        }
        m_serial = m_IO.getUart();
        m_pwm = m_IO.getPWM();
        m_parser.init(m_serial);
        resetDevice();

#if defined(LPF2_USE_FREERTOS)

        if (!useFreeRTOSTask)
            return;

        xTaskCreate(
            &Port::taskEntryPoint, // Static entry point
            taskName.c_str(),      // Task name
            4096,
            this, // Pass this pointer
            5,
            nullptr);
#endif
    }

    bool Port::deviceConnected()
    {
        if (m_deviceType == DeviceType::UNKNOWNDEVICE)
        {
            return false;
        }
        else if (m_status != LPF2_STATUS::STATUS_DATA && m_status != LPF2_STATUS::STATUS_ANALOD_ID)
        {
            return false;
        }
        return true;
    }

#if defined(LPF2_USE_FREERTOS)

    void Port::taskEntryPoint(void *pvParameters)
    {
        Port *self = static_cast<Port *>(pvParameters);
        self->uartTask(); // Call actual member function
    }

    void Port::uartTask()
    {
        LPF2_LOG_I("Initialization done.");

        resetDevice();

        Message message;

        while (1)
        {
            vTaskDelay(1);
            if (!m_IO.ready())
            {
                return;
            }
            update();
        }
    }
#endif

    void Port::update()
    {
        if (!m_IO.ready())
        {
            return;
        }

#ifdef LPF2_MUTEX_INVALID
        if (m_serialMutex == LPF2_MUTEX_INVALID)
        {
            LPF2_LOG_E("Serial mutex not initialized!");
            return;
        }
#endif

        if (m_status == LPF2_STATUS::STATUS_ANALOD_ID)
        {
            // m_serial->uartPinsOff();
            doAnalogID();
            return;
        }

        auto messages = m_parser.update();

        for (const auto &msg : messages)
        {
            LPF2_DEBUG_EXPR_V(
                if ((m_status == LPF2_STATUS::STATUS_SPEED_CHANGE || m_status == LPF2_STATUS::STATUS_ACK_WAIT) &&
                    (!msg.system || msg.header != BYTE_ACK)) {
                    // do not print SYNC and other messages because, they're not relevant in this state
                    // (Speed change - lot of garbage is received).
                    break;
                }
                else if (m_status == LPF2_STATUS::STATUS_DATA_START && msg.header == BYTE_SYNC) {
                    break;
                }  m_parser.printMessage(msg););
            if (m_status == LPF2_STATUS::STATUS_SYNCING)
            {
                if (msg.msg == MESSAGE_SYS)
                {
                    continue; // system messages are one byte so they are not useful for syncing
                }
                LPF2_LOG_D("Synced with device.");
                m_startRec = LPF2_GET_TIME();
                m_status = m_new_status;
                if (m_new_status == LPF2_STATUS::STATUS_ACK_WAIT)
                    m_new_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
                else if (m_new_status == LPF2_STATUS::STATUS_SYNC_WAIT)
                    m_new_status = LPF2_STATUS::STATUS_INFO;
                else
                    m_new_status = LPF2_STATUS::STATUS_INFO;
            }

            parseMessage(msg);
            auto now = LPF2_GET_TIME();
            if (msg.header != BYTE_SYNC)
            {
                // when speed is mismatched we receive 0 bytes or garbage, so we should not update the last received time because of that.
                m_startRec = now;
                continue;
            }

            if (process(now) != 0)
            {
                goto end_loop;
            }
        }
    end_loop:

        process(LPF2_GET_TIME());
    }

    uint8_t Port::process(unsigned long now)
    {
        if (now - m_startRec > 2000)
        {
            if (m_deviceConnected)
            {
                LPF2_LOG_I("Device disconnected.");
                m_deviceConnected = false;
            }
            resetDevice();
            sendACK(true);
            m_startRec = now;
        }

        if (now - m_startRec > 1000 && m_status == LPF2_STATUS::STATUS_SPEED_CHANGE)
        {
            // device does not support speed change
            baud = 2400;
            changeBaud(baud);
            LPF2_LOG_W("Speed change not supported, continuing at %i baud", baud);
            m_status = LPF2_STATUS::STATUS_SYNC_WAIT;
            m_new_status = LPF2_STATUS::STATUS_INFO;
            m_startRec = now;
        }

        if (m_lastStatus != m_status)
        {
            m_lastStatus = m_status;
            LPF2_LOG_V("New status: %i", (int)m_status);
        }

        switch (m_status)
        {
        case LPF2_STATUS::STATUS_SPEED_CHANGE:
            baud = 115200;
            requestSpeedChange(baud);
            break;
        case LPF2_STATUS::STATUS_SPEED:
            if (!m_deviceDataReceived && m_new_status != LPF2_STATUS::STATUS_SPEED)
            {
                break; // we don't know the device yet.
            }
            changeBaud(baud);
            sendACK(true);
            LPF2_LOG_D("Succesfully changed speed to %i baud", baud);
            if (m_new_status == LPF2_STATUS::STATUS_SPEED)
            {
                m_status = LPF2_STATUS::STATUS_SYNC_WAIT;
                m_new_status = LPF2_STATUS::STATUS_INFO;
            }
            else
            {
                m_status = LPF2_STATUS::STATUS_DATA_START;
            }
            break;

        case LPF2_STATUS::STATUS_ERR:
            LPF2_LOG_W("Error state, resetting device.");
            resetDevice();
            sendACK(true);
            m_status = LPF2_STATUS::STATUS_SYNC_WAIT;
            m_new_status = LPF2_STATUS::STATUS_INFO;
            break;

        case LPF2_STATUS::STATUS_DATA_START:
            m_rawDataSizeEnsured = false;
        case LPF2_STATUS::STATUS_DATA:
            if (now - m_start >= 100)
            {
                m_start = now;
                LPF2_LOG_V("heartbeat");
                sendACK(true);
            }
            break;

        case LPF2_STATUS::STATUS_ACK_WAIT:
            if (now - m_start > 25)
            {
                // if (m_status == LPF2_STATUS::STATUS_ACK_WAIT && m_new_status == LPF2_STATUS::STATUS_SPEED)
                // {
                //     // device does not support speed change
                //     baud = 2400;
                //     changeBaud(2400);
                //     LPF2_LOG_W("Speed change not supported, continuing at %i baud", baud);
                //     m_status = LPF2_STATUS::STATUS_SYNC_WAIT;
                //     m_new_status = LPF2_STATUS::STATUS_INFO;
                // }
                switch (m_new_status)
                {
                case LPF2_STATUS::STATUS_SPEED:
                    m_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
                    break;

                default:
                    break;
                }
            }
            break;

        case LPF2_STATUS::STATUS_DATA_RECEIVED:
            LPF2_LOG_D("Succesfully changed speed to %i baud", baud);
            LPF2_LOG_D("Setting default mode: %i", getDefaultMode(m_deviceType));
            setMode(getDefaultMode(m_deviceType));
            sendACK(true);
            m_status = LPF2_STATUS::STATUS_DATA;
            break;

        case LPF2_STATUS::STATUS_ACK_SENDING:
            sendACK(false);
            sendACK(false);
            sendACK(false);
            sendACK(false);
            LPF2_LOG_D("Sent ACK after info, changing speed.");
            m_status = LPF2_STATUS::STATUS_DATA_START;
            changeBaud(baud);
            sendACK(true);
            m_start = now;
            break;

        default:
            break;
        }
        return 0;
    }

    void Port::resetDevice()
    {
        // enterUartState();
        // return;
        m_pwm->off();
        {
            Utils::MutexLock lock(m_serialMutex);
            m_serial->uartPinsOff();
        }
        baud = 115200;
        m_deviceType = DeviceType::UNKNOWNDEVICE;
        m_deviceDataReceived = false;
        modes = views = 0;
        comboNum = 0;
        modeData.resize(0);
        modeCombos.clear();
        modeCombos.resize(16);
        for (size_t i = 0; i < 16; i++)
        {
            modeCombos[i] = 0;
        }
        nextModeExt = false;
        measurementNum = 0;
        m_status = LPF2_STATUS::STATUS_ANALOD_ID;
        m_start = LPF2_GET_TIME();
        m_startRec = m_start;
    }

    void Port::enterUartState()
    {
        m_pwm->off();
        {
            Utils::MutexLock lock(m_serialMutex);
            m_serial->uartPinsOn();
        }
        baud = 115200;
        changeBaud(baud);
        m_deviceType = DeviceType::UNKNOWNDEVICE;
        modes = views = 0;
        comboNum = 0;
        modeData.resize(0);
        for (size_t i = 0; i < 16; i++)
        {
            modeCombos[i] = 0;
        }
        nextModeExt = false;
        measurementNum = 0;
        m_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
        m_new_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
        m_start = LPF2_GET_TIME();
        m_startRec = m_start;
    }
}; // namespace Lpf2::Local