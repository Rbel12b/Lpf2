// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef __LOG_H__
#define __LOG_H__

#include "../config.h"
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LPF2_LOG_LEVEL
#define LPF2_LOG_LEVEL 2
#endif

#if LPF2_LOG_LEVEL > 0
#define LPF2_LOG_E log_e
#define LPF2_DEBUG_EXPR_E(...) \
    do                         \
    {                          \
        __VA_ARGS__;           \
    } while (0)
#else
#define LPF2_LOG_E(...) \
    do                  \
    {                   \
    } while (0)
#define LPF2_DEBUG_EXPR_E(...) \
    do                         \
    {                          \
    } while (0)
#endif
#if LPF2_LOG_LEVEL > 1
#define LPF2_LOG_W log_w
#define LPF2_DEBUG_EXPR_W(...) \
    do                         \
    {                          \
        __VA_ARGS__;           \
    } while (0)
#else
#define LPF2_LOG_W(...) \
    do                  \
    {                   \
    } while (0)
#define LPF2_DEBUG_EXPR_W(...) \
    do                         \
    {                          \
    } while (0)
#endif
#if LPF2_LOG_LEVEL > 2
#define LPF2_LOG_I log_i
#define LPF2_DEBUG_EXPR_I(...) \
    do                         \
    {                          \
        __VA_ARGS__;           \
    } while (0)
#else
#define LPF2_LOG_I(...) \
    do                  \
    {                   \
    } while (0)
#define LPF2_DEBUG_EXPR_I(...) \
    do                         \
    {                          \
    } while (0)
#endif
#if LPF2_LOG_LEVEL > 3
#define LPF2_LOG_D log_d
#define LPF2_DEBUG_EXPR_D(...) \
    do                         \
    {                          \
        __VA_ARGS__;           \
    } while (0)
#else
#define LPF2_LOG_D(...) \
    do                  \
    {                   \
    } while (0)
#define LPF2_DEBUG_EXPR_D(...) \
    do                         \
    {                          \
    } while (0)
#endif
#if LPF2_LOG_LEVEL > 4
#define LPF2_LOG_V log_v
#define LPF2_DEBUG_EXPR_V(...) \
    do                         \
    {                          \
        __VA_ARGS__;           \
    } while (0)
#else
#define LPF2_LOG_V(...) \
    do                  \
    {                   \
    } while (0)
#define LPF2_DEBUG_EXPR_V(...) \
    do                         \
    {                          \
    } while (0)
#endif

#ifndef log_n

#define LPF2_LOG_IMPL 1

#define ARDUHAL_LOG_LEVEL_NONE (0)
#define ARDUHAL_LOG_LEVEL_ERROR (1)
#define ARDUHAL_LOG_LEVEL_WARN (2)
#define ARDUHAL_LOG_LEVEL_INFO (3)
#define ARDUHAL_LOG_LEVEL_DEBUG (4)
#define ARDUHAL_LOG_LEVEL_VERBOSE (5)

#ifndef CORE_DEBUG_LEVEL
#define ARDUHAL_LOG_LEVEL LPF2_LOG_LEVEL
#else
#define ARDUHAL_LOG_LEVEL CORE_DEBUG_LEVEL
#ifdef USE_ESP_IDF_LOG
#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CORE_DEBUG_LEVEL
#endif
#endif
#endif

#ifndef CONFIG_ARDUHAL_LOG_COLORS
#define CONFIG_ARDUHAL_LOG_COLORS 1
#endif

#if CONFIG_ARDUHAL_LOG_COLORS
#define ARDUHAL_LOG_COLOR_BLACK "30"
#define ARDUHAL_LOG_COLOR_RED "31"    // ERROR
#define ARDUHAL_LOG_COLOR_GREEN "32"  // INFO
#define ARDUHAL_LOG_COLOR_YELLOW "33" // WARNING
#define ARDUHAL_LOG_COLOR_BLUE "34"
#define ARDUHAL_LOG_COLOR_MAGENTA "35"
#define ARDUHAL_LOG_COLOR_CYAN "36" // DEBUG
#define ARDUHAL_LOG_COLOR_GRAY "37" // VERBOSE
#define ARDUHAL_LOG_COLOR_WHITE "38"

#define ARDUHAL_LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define ARDUHAL_LOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define ARDUHAL_LOG_RESET_COLOR "\033[0m"

#define ARDUHAL_LOG_COLOR_E ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_RED)
#define ARDUHAL_LOG_COLOR_W ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_YELLOW)
#define ARDUHAL_LOG_COLOR_I ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_GREEN)
#define ARDUHAL_LOG_COLOR_D ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_CYAN)
#define ARDUHAL_LOG_COLOR_V ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_GRAY)
#define ARDUHAL_LOG_COLOR_PRINT(letter) log_printf(ARDUHAL_LOG_COLOR_##letter)
#define ARDUHAL_LOG_COLOR_PRINT_END log_printf(ARDUHAL_LOG_RESET_COLOR)
#else
#define ARDUHAL_LOG_COLOR_E
#define ARDUHAL_LOG_COLOR_W
#define ARDUHAL_LOG_COLOR_I
#define ARDUHAL_LOG_COLOR_D
#define ARDUHAL_LOG_COLOR_V
#define ARDUHAL_LOG_RESET_COLOR
#define ARDUHAL_LOG_COLOR_PRINT(letter)
#define ARDUHAL_LOG_COLOR_PRINT_END
#endif

    const char *pathToFileName(const char *path);
    int log_printf(const char *fmt, ...);

#define ARDUHAL_SHORT_LOG_FORMAT(letter, format) ARDUHAL_LOG_COLOR_##letter format ARDUHAL_LOG_RESET_COLOR "\r\n"
#define ARDUHAL_LOG_FORMAT(letter, format) ARDUHAL_LOG_COLOR_##letter "[%6u][" #letter "][%s:%u] %s(): " format ARDUHAL_LOG_RESET_COLOR "\r\n", (unsigned long)(millis() / 1000ULL), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#define log_v(format, ...) log_printf(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define log_v(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#define log_d(format, ...) log_printf(ARDUHAL_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define log_d(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#define log_i(format, ...) log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define log_i(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#define log_w(format, ...) log_printf(ARDUHAL_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define log_w(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#define log_e(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define log_e(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_NONE
#define log_n(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define log_n(format, ...) \
    do                     \
    {                      \
    } while (0)
#endif

#endif // log_n

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */
