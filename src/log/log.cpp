#include "./log.h"

#ifdef LPF2_LOG_IMPL

const char *pathToFileName(const char *path)
{
    int i = 0;
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            break;
        }
    }
    return path + i + 1;
}

#endif // LPF2_LOG_IMPL

int lpf2_log_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Serial.write((uint8_t *)buffer, len);
    return len;
}