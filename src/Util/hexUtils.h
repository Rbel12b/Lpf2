#pragma once
#ifndef _LPF2_HEX_UTILS_
#define _LPF2_HEX_UTILS_
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>

std::string bytes_to_hexString(const std::vector<uint8_t> &data);

#endif