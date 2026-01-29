#include "Lpf2Port.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>

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