#include "hexUtils.h"

std::string bytes_to_hexString(const std::vector<uint8_t> &data)
{
    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i)
    {
        if (i > 0)
            oss << ", ";
        oss << "0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    return oss.str();
}