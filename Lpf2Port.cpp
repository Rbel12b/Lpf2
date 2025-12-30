#include "Lpf2Port.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>

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
    m_serial->uartPinsOn();
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
    if (m_deviceType == DeviceType::UNKNOWNDEVICE)
        return false;
    if (m_status != LPF2_STATUS::STATUS_DATA)
        return false;
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
    int baudRate = 2400;

    log_i("Initialization done.");

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

    auto messages = m_parser.update();

    for (const auto &msg : messages)
    {
        if (m_status == LPF2_STATUS::STATUS_SYNCING)
        {
            if (msg.msg == MESSAGE_SYS)
            {
                break; // system messages are one byte so they are not useful for syncing
            }
            LPF2_LOG_D("Synced with device.");
            m_status = m_new_status;
            if (m_new_status == LPF2_STATUS::STATUS_ACK_WAIT)
                m_new_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
            else
                m_new_status = LPF2_STATUS::STATUS_INFO;
        }

        LPF2_DEBUG_EXPR_D(
            parser.printMessage(msg););

        parseMessage(msg);

        if (process(millis()) != 0)
        {
            goto end_loop;
        }
    }
end_loop:

    process(millis());

    if (!deviceConnected())
    {
        resetDevice();
    }
}

uint8_t Lpf2Port::process(unsigned long now)
{
    if (now - m_timeStart > 2000 &&
        (m_status != LPF2_STATUS::STATUS_SPEED_CHANGE) &&
        !(m_status == LPF2_STATUS::STATUS_ACK_WAIT && m_new_status == LPF2_STATUS::STATUS_SPEED))
    {
        LPF2_LOG_D("Device disconnected.");
        resetDevice();
        sendACK(true);
        m_timeStart = now;
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
        if (now - m_timeStart > 100)
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

uint8_t Lpf2Port::getDataSize(uint8_t format)
{
    switch (format)
    {
    case DATA8:
        return 1;

    case DATA16:
        return 2;

    case DATA32:
        return 4;

    case DATAF:
        return 4;
    default:
        break;
    }
    return 0;
}

bool Lpf2Port::deviceIsAbsMotor(DeviceType id)
{
    switch (id)
    {
    case DeviceType::TECHNIC_LARGE_LINEAR_MOTOR:
    case DeviceType::TECHNIC_XLARGE_LINEAR_MOTOR:
    case DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR:
    case DeviceType::TECHNIC_LARGE_ANGULAR_MOTOR_GREY:
    case DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR:
    case DeviceType::TECHNIC_MEDIUM_ANGULAR_MOTOR_GREY:
    case DeviceType::MEDIUM_LINEAR_MOTOR:
    case DeviceType::SIMPLE_MEDIUM_LINEAR_MOTOR:
        return true;
    default:
        return false;
    }
}

void Lpf2Port::setMode(uint8_t num)
{
    if (num >= modes)
    {
        LPF2_LOG_W("Tried to set invalid mode %i (max %i)", num, modes - 1);
        return;
    }

    m_mode = num;
    uint8_t header = MESSAGE_CMD | CMD_SELECT;
    uint8_t checksum = header ^ 0xFF;
    checksum ^= (uint8_t)num;

    {
        MutexLock lock(m_serialMutex);
        m_serial->write(header);
        m_serial->write((uint8_t)num);
        m_serial->write(checksum);
        m_serial->flush();
    }

    if (modeData[num].flags.pin1())
    {
        m_pwm->out(255, 0);
    }
    if (modeData[num].flags.pin2())
    {
        m_pwm->out(0, 255);
    }
}

void Lpf2Port::requestSpeedChange(uint32_t speed)
{
    LPF2_LOG_D("Trying to change speed to %i", speed);
    uint8_t header = MESSAGE_CMD | CMD_SPEED | (2 << 3);
    uint8_t checksum = header ^ 0xFF;
    uint8_t b;
    {
        MutexLock lock(m_serialMutex);
        m_serial->write(header);
        b = (speed & 0xFF) >> 0;
        checksum ^= b;
        m_serial->write(b);
        b = (speed & 0xFF00) >> 8;
        checksum ^= b;
        m_serial->write(b);
        b = (speed & 0xFF0000) >> 16;
        checksum ^= b;
        m_serial->write(b);
        b = (speed & 0xFF000000) >> 24;
        checksum ^= b;
        m_serial->write(b);
        m_serial->write(checksum);
        m_serial->flush();
    }
    m_status = LPF2_STATUS::STATUS_ACK_WAIT;
    m_new_status = LPF2_STATUS::STATUS_SPEED;
    m_timeStart = millis();
}

void Lpf2Port::resetDevice()
{
    baud = 115200;
    changeBaud(baud);
    m_deviceType = DeviceType::UNKNOWNDEVICE;
    modes = views = 0;
    comboNum = 0;
    for (int i = 0; i < modes; i++)
    {
        modeData[i] = Mode();
    }
    for (size_t i = 0; i < 16; i++)
    {
        modeCombos[i] = 0;
    }
    nextModeExt = false;
    m_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
    m_new_status = LPF2_STATUS::STATUS_SPEED_CHANGE;
    m_start = millis();
}

ModeNum Lpf2Port::getDefaultMode(DeviceType id)
{

    if (deviceIsAbsMotor(id))
    {
        return ModeNum::MOTOR__CALIB;
    }

    switch (id)
    {
    case DeviceType::COLOR_DISTANCE_SENSOR:
        return ModeNum::COLOR_DISTANCE_SENSOR__RGB_I;
    default:
        return ModeNum::_DEFAULT;
    }
}

void Lpf2Port::parseMessage(const Lpf2Message &msg)
{
    switch (msg.msg)
    {
    case MESSAGE_CMD:
    {
        if (m_status == LPF2_STATUS::STATUS_ACK_WAIT && m_new_status == LPF2_STATUS::STATUS_SPEED)
        {
            // device does not support speed change
            baud = 2400;
            changeBaud(2400);
            LPF2_LOG_W("Speed change not supported, continuing at %i baud", baud);
            m_status = LPF2_STATUS::STATUS_SYNC_WAIT;
            m_new_status = LPF2_STATUS::STATUS_INFO;
        }
        m_timeStart = millis();
        parseMessageCMD(msg);
        break;
    }
    case MESSAGE_INFO:
    {
        m_timeStart = millis();
        parseMessageInfo(msg);
        break;
    }
    case MESSAGE_SYS:
    {
        switch (msg.header)
        {
        case BYTE_ACK:
            if (m_status == LPF2_STATUS::STATUS_ACK_WAIT)
            {
                LPF2_LOG_D("ACK received.");
                m_status = m_new_status;
            }
            else if (m_status == LPF2_STATUS::STATUS_INFO)
            {
                LPF2_LOG_D("Info end ACK received.");
                m_status = LPF2_STATUS::STATUS_ACK_SENDING;
                m_new_status = LPF2_STATUS::STATUS_DATA_START;
            }
            break;
        case BYTE_SYNC:
            if (m_status == LPF2_STATUS::STATUS_SYNC_WAIT)
            {
                LPF2_LOG_D("SYNC received.");
                m_status = m_new_status;
            }
            break;

        default:
            break;
        }
        break;
    }
    case MESSAGE_DATA:
    {
        if (m_status == LPF2_STATUS::STATUS_DATA_START)
        {
            m_status = LPF2_STATUS::STATUS_DATA_RECEIVED;
        }
        m_timeStart = millis();
        uint8_t mode = GET_MODE(msg.header);

        if (nextModeExt)
        {
            mode += 8;
            nextModeExt = false;
        }

        if (mode >= modes)
        {
            break;
        }

        uint8_t size = modeData[mode].data_sets * getDataSize(modeData[mode].format);

        uint8_t readLen = size;
        if (msg.length < size)
        {
            readLen = msg.length;
        }

        for (int i = 0; i < readLen; i++)
        {
            modeData[mode].rawData[i] = msg.data[i];
        }
        break;
    }
    default:
    {
        log_e("Unknown message type: 0x%02X", msg.msg);
    }
    }
}

void Lpf2Port::parseMessageCMD(const Lpf2Message &msg)
{
    switch (msg.cmd)
    {
    case CMD_TYPE:
    {
        m_deviceType = (DeviceType)msg.data[0];
        m_status = LPF2_STATUS::STATUS_INFO;
        nextModeExt = false;
        break;
    }
    case CMD_MODES:
    {
        if (msg.length == 1)
        {
            modes = views = msg.data[1] + 1;
        }
        else if (msg.length == 2)
        {
            modes = msg.data[0] + 1;
            views = msg.data[1] + 1;
        }
        else if (msg.length == 4)
        {
            modes = msg.data[2] + 1;
            views = msg.data[3] + 1;
        }
        break;
    }
    case CMD_SPEED:
    {
        if (msg.length != 4)
        {
            break;
        }
        baud = msg.data[0] | ((uint64_t)msg.data[1] << 8) | ((uint64_t)msg.data[2] << 16) | ((uint64_t)msg.data[3] << 24);
        break;
    }
    case CMD_VERSION:
    {
        break;
    }
    case CMD_EXT_MODE:
    {
        if (msg.data[0])
        {
            nextModeExt = true;
        }
        break;
    }
    default:
    {
        log_w("Unknown command: 0x%02X", msg.cmd);
        break;
    }
    }
}

void Lpf2Port::parseMessageInfo(const Lpf2Message &msg)
{
    switch (msg.data[0] & 0xDF)
    {
    case INFO_NAME:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes)
        {
            break;
        }
        std::string name;
        int i = 0;
        for (i = 1; i < msg.length; i++)
        {
            if (msg.data[i] == '\0')
            {
                break;
            }
            name += msg.data[i];
        }
        modeData[mode].name = name;
        i++;
        if ((i + 6) <= msg.length && msg.data.size() >= static_cast<size_t>(i + 6))
        {
            std::memcpy(&modeData[mode].flags.bytes, msg.data.data() + i, 6);
        }
        break;
    }
    case INFO_RAW:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes || msg.length < 9)
        {
            break;
        }
        if (msg.data.size() >= 9)
        {
            std::memcpy(&modeData[mode].min, msg.data.data() + 1, 4);
            std::memcpy(&modeData[mode].max, msg.data.data() + 5, 4);
        }
        break;
    }
    case INFO_PCT:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes || msg.length < 5)
        {
            break;
        }
        float val;
        std::memcpy(&val, msg.data.data() + 1, 4);
        if (val == -100.0f)
        {
            modeData[mode].negativePCT = true;
        }
        break;
    }
    case INFO_SI:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes || msg.length < 9)
        {
            break;
        }
        std::memcpy(&modeData[mode].SImin, msg.data.data() + 1, 4);
        std::memcpy(&modeData[mode].SImax, msg.data.data() + 5, 4);
        break;
    }
    case INFO_UNITS:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes)
        {
            break;
        }
        std::string unit;
        int i = 0;
        for (i = 1; i < msg.length; i++)
        {
            if (msg.data[i] == '\0')
            {
                break;
            }
            unit += msg.data[i];
        }
        modeData[mode].unit = unit;
        break;
    }
    case INFO_MAPPING:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes || msg.length < 3)
        {
            break;
        }
        modeData[mode].in.val = msg.data[1];
        modeData[mode].out.val = msg.data[2];
        break;
    }
    case INFO_MODE_COMBOS:
    {
        uint8_t num = (msg.length - 1) / 2;
        if (num > 15)
        {
            break;
        }
        for (int i = 0; i < num; i++)
        {
            std::memcpy(&modeCombos[i], msg.data.data() + 1 + (i * 2), 2);
            if (comboNum == 0 && modeCombos[num] == 0)
            {
                comboNum = i;
            }
        }
    }
    case INFO_FORMAT:
    {
        uint8_t mode = GET_MODE(msg.cmd) + ((msg.data[0] & INFO_MODE_PLUS_8) ? 8 : 0);
        if (mode >= modes || msg.length < 5)
        {
            break;
        }
        modeData[mode].data_sets = msg.data[1];
        modeData[mode].format = msg.data[2];
        modeData[mode].figures = msg.data[3];
        modeData[mode].decimals = msg.data[4];
        break;
    }
    default:
    {
        log_w("Unknown info: 0x%02X", msg.data[0]);
        break;
    }
    }
}

void Lpf2Port::changeBaud(uint32_t baud)
{
    m_serial->flush();
    m_serial->setBaudrate(baud);
}

void Lpf2Port::sendACK(bool NACK)
{
    MutexLock lock(m_serialMutex);
    m_serial->write(NACK ? BYTE_NACK : BYTE_ACK);
    m_serial->flush();
}

float Lpf2Port::getValue(const Mode &modeData, uint8_t dataSet)
{
    if (dataSet > modeData.data_sets)
        return 0.0f;

    const std::vector<uint8_t> &raw = modeData.rawData;
    std::string result;

    // Determine byte size per dataset based on format
    size_t bytesPerDataset;
    switch (modeData.format)
    {
    case DATA8:
        bytesPerDataset = 1;
        break;
    case DATA16:
        bytesPerDataset = 2;
        break;
    case DATA32:
    case DATAF:
        bytesPerDataset = 4;
        break;
    default:
        return 0.0f;
    }

    // Check that rawData contains enough bytes
    size_t expectedSize = static_cast<size_t>(modeData.data_sets) * bytesPerDataset;
    if (raw.size() < expectedSize)
    {
        return 0.0f;
    }

    const uint8_t *ptr = raw.data() + (bytesPerDataset * dataSet);
    float value = 0.0f;

    // Parse based on format
    switch (modeData.format)
    {
    case DATA8:
        value = parseData8(ptr) / pow(10, modeData.decimals);
        break;
    case DATA16:
        value = parseData16(ptr) / pow(10, modeData.decimals);
        break;
    case DATA32:
        value = parseData32(ptr) / pow(10, modeData.decimals);
        break;
    case DATAF:
        value = parseDataF(ptr);
        break;
    }
    return value;
}

float Lpf2Port::getValue(uint8_t modeNum, uint8_t dataSet) const
{
    if (modeNum > modeData.size())
        return 0.0f;
    return getValue(modeData[modeNum], dataSet);
}

std::string Lpf2Port::formatValue(float value, const Mode &modeData)
{
    std::ostringstream os;

    // Use fixed precision from modeData.decimals
    os << std::fixed << std::setprecision(modeData.decimals);

    if (modeData.negativePCT)
    {
        value = value - 100;
    }

    os << value;

    // Append unit if present
    if (!modeData.unit.empty())
    {
        os << " " << modeData.unit;
    }

    return os.str();
}

std::string Lpf2Port::convertValue(Mode modeData)
{
    std::string result;

    for (uint8_t i = 0; i < modeData.data_sets; ++i)
    {
        // Format to string
        std::string part = formatValue(getValue(modeData, i), modeData);

        // Append with separator
        if (result.length())
        {
            result += "; ";
        }
        result += part;
    }

    return result;
}

#define CHECK_LENGHT(size, msg_size, length) \
    else if (size <= length)                 \
    {                                        \
        msg_size = LENGTH_##length;          \
    }

int Lpf2Port::writeData(uint8_t modeNum, const std::vector<uint8_t> &data)
{
    if (modeNum >= modeData.size())
    {
        return 1;
    }

    size_t size = data.size();
    uint8_t msg_size = 0;

    if (size == 0)
    {
        return 0;
    }
    CHECK_LENGHT(size, msg_size, 1)
    CHECK_LENGHT(size, msg_size, 2)
    CHECK_LENGHT(size, msg_size, 4)
    CHECK_LENGHT(size, msg_size, 8)
    CHECK_LENGHT(size, msg_size, 16)
    CHECK_LENGHT(size, msg_size, 32)
    CHECK_LENGHT(size, msg_size, 64)
    CHECK_LENGHT(size, msg_size, 128)
    else if (size > 128)
    {
        return 1;
    }

    uint8_t header = MESSAGE_CMD | CMD_EXT_MODE | LENGTH_1;
    uint8_t checksum = header ^ 0xFF;
    uint8_t b = (modeNum >= 8) ? 8 : 0;

    {
        MutexLock lock(m_serialMutex);
        m_serial->write(header);
        checksum ^= b;
        m_serial->write(b);
        m_serial->write(checksum);

        header = MESSAGE_DATA | msg_size | (modeNum & 0x07);
        checksum = header ^ 0xFF;
        m_serial->write(header);

        for (uint8_t i = 0; i < size; i++)
        {
            b = data[i];
            checksum ^= b;
            m_serial->write(b);
        }

        m_serial->write(checksum);
        m_serial->flush();
    }

    return 0;
}

void Lpf2Port::setPower(uint8_t pin1, uint8_t pin2)
{
    if ((modeData[m_mode].flags.power12() || m_dumb) && m_pwm)
    {
        m_pwm->out(pin1, pin2);
    }
}

float Lpf2Port::parseData8(const uint8_t *ptr)
{
    int8_t val = static_cast<int8_t>(*ptr);
    return static_cast<float>(val);
}

float Lpf2Port::parseData16(const uint8_t *ptr)
{
    int16_t val;
    std::memcpy(&val, ptr, sizeof(int16_t));
    return static_cast<float>(val);
}

float Lpf2Port::parseData32(const uint8_t *ptr)
{
    int32_t val;
    std::memcpy(&val, ptr, sizeof(int32_t));
    return static_cast<float>(val);
}

float Lpf2Port::parseDataF(const uint8_t *ptr)
{
    float val;
    std::memcpy(&val, ptr, sizeof(float));
    return val;
}