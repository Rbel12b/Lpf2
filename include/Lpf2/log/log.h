// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
// Copyright 2026 Rbel12b - Modifications for Lpf2
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

#ifndef __LPF2_LOG_H__
#define __LPF2_LOG_H__

#include "Lpf2/config.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LPF2_LOG_LEVEL
#define LPF2_LOG_LEVEL 2
#endif

#if LPF2_LOG_LEVEL > 0
#define LPF2_LOG_E(format, ...) lpf2_log_printf(LPF2_LOG_FORMAT(E, format), ##__VA_ARGS__)
#define LPF2_DEBUG_EXPR_E(...) \
    do                         \
    {                          \
        __VA_ARGS__            \
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
#define LPF2_LOG_W(format, ...) lpf2_log_printf(LPF2_LOG_FORMAT(W, format), ##__VA_ARGS__)
#define LPF2_DEBUG_EXPR_W(...) \
    do                         \
    {                          \
        __VA_ARGS__            \
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
#define LPF2_LOG_I(format, ...) lpf2_log_printf(LPF2_LOG_FORMAT(I, format), ##__VA_ARGS__)
#define LPF2_DEBUG_EXPR_I(...) \
    do                         \
    {                          \
        __VA_ARGS__            \
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
#define LPF2_LOG_D(format, ...) lpf2_log_printf(LPF2_LOG_FORMAT(D, format), ##__VA_ARGS__)
#define LPF2_DEBUG_EXPR_D(...) \
    do                         \
    {                          \
        __VA_ARGS__            \
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
#define LPF2_LOG_V(format, ...) lpf2_log_printf(LPF2_LOG_FORMAT(V, format), ##__VA_ARGS__)
#define LPF2_DEBUG_EXPR_V(...) \
    do                         \
    {                          \
        __VA_ARGS__            \
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

#define LPF2_LOG_IMPL 1

#define LPF2_LOG_LEVEL_NONE (0)
#define LPF2_LOG_LEVEL_ERROR (1)
#define LPF2_LOG_LEVEL_WARN (2)
#define LPF2_LOG_LEVEL_INFO (3)
#define LPF2_LOG_LEVEL_DEBUG (4)
#define LPF2_LOG_LEVEL_VERBOSE (5)

#ifndef LPF2_LOG_LEVEL
#define LPF2_LOG_LEVEL CORE_DEBUG_LEVEL
#endif

#ifndef CONFIG_LPF2_LOG_COLORS
#define CONFIG_LPF2_LOG_COLORS 1
#endif

#if CONFIG_LPF2_LOG_COLORS
#define LPF2_LOG_COLOR_BLACK "30"
#define LPF2_LOG_COLOR_RED "31"    // ERROR
#define LPF2_LOG_COLOR_GREEN "32"  // INFO
#define LPF2_LOG_COLOR_YELLOW "33" // WARNING
#define LPF2_LOG_COLOR_BLUE "34"
#define LPF2_LOG_COLOR_MAGENTA "35" // VERBOSE
#define LPF2_LOG_COLOR_CYAN "36" // DEBUG
#define LPF2_LOG_COLOR_GRAY "37"
#define LPF2_LOG_COLOR_WHITE "38"

#define LPF2_LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LPF2_LOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define LPF2_LOG_RESET_COLOR "\033[0m"

#define LPF2_LOG_COLOR_E LPF2_LOG_COLOR(LPF2_LOG_COLOR_RED)
#define LPF2_LOG_COLOR_W LPF2_LOG_COLOR(LPF2_LOG_COLOR_YELLOW)
#define LPF2_LOG_COLOR_I LPF2_LOG_COLOR(LPF2_LOG_COLOR_GREEN)
#define LPF2_LOG_COLOR_D LPF2_LOG_COLOR(LPF2_LOG_COLOR_CYAN)
#define LPF2_LOG_COLOR_V LPF2_LOG_COLOR(LPF2_LOG_COLOR_MAGENTA)
#define LPF2_LOG_COLOR_PRINT(letter) lpf2_log_printf(LPF2_LOG_COLOR_##letter)
#define LPF2_LOG_COLOR_PRINT_END lpf2_log_printf(LPF2_LOG_RESET_COLOR)
#else
#define LPF2_LOG_COLOR_E
#define LPF2_LOG_COLOR_W
#define LPF2_LOG_COLOR_I
#define LPF2_LOG_COLOR_D
#define LPF2_LOG_COLOR_V
#define LPF2_LOG_RESET_COLOR
#define LPF2_LOG_COLOR_PRINT(letter)
#define LPF2_LOG_COLOR_PRINT_END
#endif

#define LPF2_SHORT_LOG_FORMAT(letter, format) LPF2_LOG_COLOR_##letter format LPF2_LOG_RESET_COLOR "\r\n"
#define LPF2_LOG_FORMAT(letter, format) LPF2_LOG_COLOR_##letter "[%6u][" #letter "][%s:%u] %s(): " format LPF2_LOG_RESET_COLOR "\r\n", (unsigned long)LPF2_GET_TIME(), lpf2_pathToFileName(__FILE__), __LINE__, __FUNCTION__

const char *lpf2_pathToFileName(const char *path);


int lpf2_log_printf(const char *fmt, ...);
esp_err_t lpf2_log_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */
