#include "log.h"

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

int log_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(stdout, fmt, args);
    va_end(args);
    return result;
}

#endif // LPF2_LOG_IMPL