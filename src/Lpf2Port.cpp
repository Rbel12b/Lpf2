#include "Lpf2Port.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>

std::string Lpf2Port::getInfoStr()
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase;
    
    oss << "Device: 0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(getDeviceType()) << "\n";
    oss << std::dec << "InModes: 0x" << std::hex << std::setw(4) << std::setfill('0') << getInputModes() << "\n";
    oss << "OutModes: 0x" << std::hex << std::setw(4) << std::setfill('0') << getOutputModes() << "\n";
    oss << "Caps: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(getCapatibilities()) << "\n";
    oss << std::dec << "Combos:\n";
    for (int i = 0; i < getModeComboCount(); i++)
    {
        oss << "\t0x" << std::hex << std::setw(4) << std::setfill('0') << getModeCombo(i);
    }
    oss << "\n";
    for (int i = 0; i < getModes().size(); i++)
    {
        auto &mode = getModes()[i];
        oss << std::dec << "Mode " << i << ":\n";
        oss << "\tname: " << mode.name << "\n";
        oss << "\tunit: " << mode.unit << "\n";
        oss << std::fixed << std::setprecision(6) << "\tmin: " << static_cast<double>(mode.min) << "\n";
        oss << "\tmax: " << static_cast<double>(mode.max) << "\n";
        oss << "\tPCT min: " << mode.PCTmin << "\n";
        oss << "\tPCT max: " << mode.PCTmax << "\n";
        oss << "\tSI min: " << static_cast<double>(mode.SImin) << "\n";
        oss << "\tSI max: " << static_cast<double>(mode.SImax) << "\n";
        oss << std::dec << "\tData sets: " << static_cast<int>(mode.data_sets) << "\n";
        oss << "\tformat: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(mode.format) << "\n";
        oss << std::dec << "\tFigures: " << static_cast<int>(mode.figures) << "\n";
        oss << "\tDecimals: " << static_cast<int>(mode.decimals) << "\n";
        auto in = mode.in;
        oss << "\tin: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(in.val)
            << " (null: " << in.nullSupport() << ", mapping 2.0: " << in.mapping2() << ", m_abs: " << in.m_abs()
            << ", m_rel: " << in.m_rel() << ", m_dis: " << in.m_dis() << ")\n";
        auto out = mode.out;
        oss << "\tout: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(out.val)
            << " (null: " << out.nullSupport() << ", mapping 2.0: " << out.mapping2() << ", m_abs: " << out.m_abs()
            << ", m_rel: " << out.m_rel() << ", m_dis: " << out.m_dis() << ")\n";
        auto flags = mode.flags;
        uint64_t val = 0;
        memcpy(&val, flags.bytes, 6);
        oss << "\tFlags: 0x" << std::hex << std::setw(12) << std::setfill('0') << val << " (speed: " << flags.speed()
            << ", apos: " << flags.apos() << ", power: " << flags.power() << ", motor: " << flags.motor()
            << ", pin1: " << flags.pin1() << ", pin2: " << flags.pin2() << ", calib: " << flags.calib()
            << ", power12: " << flags.power12() << ")\n";
        oss << "\tRaw:";
        for (int n = 0; n < mode.rawData.size(); n++)
        {
            oss << " 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(mode.rawData[n]);
        }
        oss << "\n";
    }
    return oss.str();
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

float Lpf2Port::getValue(const Lpf2Mode &modeData, uint8_t dataSet)
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

std::string Lpf2Port::formatValue(float value, const Lpf2Mode &modeData)
{
    std::ostringstream os;

    // Use fixed precision from modeData.decimals
    os << std::fixed << std::setprecision(modeData.decimals);

    if (modeData.PCTmin < 0.0f)
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

std::string Lpf2Port::convertValue(Lpf2Mode modeData)
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

Lpf2ModeNum Lpf2Port::getDefaultMode(Lpf2DeviceType id)
{

    if (deviceIsAbsMotor(id))
    {
        return Lpf2ModeNum::MOTOR__CALIB;
    }

    switch (id)
    {
    case Lpf2DeviceType::COLOR_DISTANCE_SENSOR:
        return Lpf2ModeNum::COLOR_DISTANCE_SENSOR__RGB_I;
    default:
        return Lpf2ModeNum::_DEFAULT;
    }
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