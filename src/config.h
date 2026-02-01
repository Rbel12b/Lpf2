#pragma once

#define CONFIG_ARDUHAL_LOG_COLORS 1
#define LPF2_LOG_LEVEL 4

#include <Arduino.h>
#include "./log/log.h"

#define LPF2_GET_TIME() ((size_t)millis())