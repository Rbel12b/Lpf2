/**
 *  Copyright (C) 2026 - Rbel12b
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  */

#include "Lpf2/log/log.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/semphr.h"
#include <stdarg.h>
#include <stdio.h>
#include <cstring>

#ifdef LPF2_LOG_IMPL

extern "C" const char *lpf2_pathToFileName(const char *path)
{
    int i = 0;
    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            break;
        }
    }
    return path + i + 1;
}

#endif // LPF2_LOG_IMPL

QueueHandle_t logMutex = xSemaphoreCreateMutex();
static usb_serial_jtag_driver_config_t usb_jtag_cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();

esp_err_t lpf2_log_init(void)
{
    esp_err_t ret = usb_serial_jtag_driver_install(&usb_jtag_cfg);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

extern "C" int lpf2_log_printf(const char *fmt, ...)
{
    xSemaphoreTake(logMutex, portMAX_DELAY);
    if (!fmt || strlen(fmt) == 0 || !usb_serial_jtag_is_connected())
    {
        xSemaphoreGive(logMutex);
        return 0;
    }
    va_list args;
    va_start(args, fmt);
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    usb_serial_jtag_write_bytes((const uint8_t *)buffer, len, 10);
    xSemaphoreGive(logMutex);
    return len;
}