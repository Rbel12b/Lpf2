#include "Lpf2Port.h"
#include <string>
#include <cstring>

void Lpf2Port::init(
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

    if (!m_IO->ready())
    {
        return;
    }
    resetDevice();

#if defined(LPF2_USE_FREERTOS)

    if (!useFreeRTOSTask)
        return;

    xTaskCreate(
        &Lpf2Port::taskEntryPoint, // Static entry point
        taskName.c_str(),          // Task name
        4096,
        this, // Pass this pointer
        5,
        nullptr);
#endif
}

bool Lpf2Port::deviceConnected()
{
    if (m_deviceType == Lpf2DeviceType::UNKNOWNDEVICE)
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

void Lpf2Port::taskEntryPoint(void *pvParameters)
{
    Lpf2Port *self = static_cast<Lpf2Port *>(pvParameters);
    self->uartTask(); // Call actual member function
}

void Lpf2Port::uartTask()
{
    LPF2_LOG_I("Initialization done.");

    resetDevice();

    Lpf2Message message;

    while (1)
    {
        vTaskDelay(1);
        if (!m_IO->ready())
        {
            return;
        }
        update();
    }
}
#endif

void Lpf2Port::update()
{
    if (!m_IO->ready())
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
            } m_parser.printMessage(msg););
        if (m_status == LPF2_STATUS::STATUS_SYNCING)
        {
            if (msg.msg == MESSAGE_SYS)
            {
                continue; // system messages are one byte so they are not useful for syncing
            }
            LPF2_LOG_D("Synced with device.");
            m_status = m_new_status;
            if (m_new_status == LPF2_STATUS::STATUS_ACK_WAIT)
                m_new_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
            else if (m_new_status == LPF2_STATUS::STATUS_SYNC_WAIT)
                m_new_status = LPF2_STATUS::STATUS_INFO;
            else
                m_new_status = LPF2_STATUS::STATUS_INFO;
        }

        parseMessage(msg);
        auto now = millis();
        m_startRec = now;

        if (process(now) != 0)
        {
            goto end_loop;
        }
    }
end_loop:

    process(millis());
}

uint8_t Lpf2Port::process(unsigned long now)
{
    if (now - m_startRec > 1500)
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
    case LPF2_STATUS::STATUS_DATA:
        if (now - m_start >= 100)
        {
            m_start = now;
            LPF2_LOG_V("heartbeat");
            sendACK(true);
        }
        break;

    case LPF2_STATUS::STATUS_ACK_WAIT:
        if (now - m_start > 100)
        {
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

bool Lpf2Port::deviceIsAbsMotor(Lpf2DeviceType id)
{
    switch (id)
    {
    case Lpf2DeviceType::TECHNIC_LARGE_LINEAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY:
    case Lpf2DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
    case Lpf2DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY:
    case Lpf2DeviceType::MEDIUM_LINEAR_MOTOR:
    case Lpf2DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
        return true;
    default:
        return false;
    }
}

void Lpf2Port::resetDevice()
{
    // enterUartState();
    // return;
    m_pwm->off();
    {
        MutexLock lock(m_serialMutex);
        m_serial->uartPinsOff();
    }
    baud = 115200;
    m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
    modes = views = 0;
    comboNum = 0;
    modeData.resize(0);
    for (size_t i = 0; i < 16; i++)
    {
        modeCombos[i] = 0;
    }
    nextModeExt = false;
    measurementNum = 0;
    m_status = LPF2_STATUS::STATUS_ANALOD_ID;
    m_start = millis();
    m_startRec = m_start;
}

void Lpf2Port::enterUartState()
{
    m_pwm->off();
    {
        MutexLock lock(m_serialMutex);
        m_serial->uartPinsOn();
    }
    baud = 115200;
    changeBaud(baud);
    m_deviceType = Lpf2DeviceType::UNKNOWNDEVICE;
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
    m_start = millis();
    m_startRec = m_start;
}

ModeNum Lpf2Port::getDefaultMode(Lpf2DeviceType id)
{

    if (deviceIsAbsMotor(id))
    {
        return ModeNum::MOTOR__CALIB;
    }

    switch (id)
    {
    case Lpf2DeviceType::COLOR_DISTANCE_SENSOR:
        return ModeNum::COLOR_DISTANCE_SENSOR__RGB_I;
    default:
        return ModeNum::_DEFAULT;
    }
}