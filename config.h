#pragma once

#define CONFIG_ARDUHAL_LOG_COLORS 1
#define LPF2_LOG_LEVEL 2

#include <Arduino.h>
#include "log/log.h"

#if defined(ESP32)
#define LPF2_USE_FREERTOS
#endif