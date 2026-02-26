#pragma once

#include <Arduino.h>
#include "Lpf2/log/log.h"

#define LPF2_GET_TIME() ((size_t)millis())

#define HUB_EMULATION_MSG_RECEIVE_TASK_PRIORITY 20