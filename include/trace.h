#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

#define TRACE_LEVEL_NONE 0
#define TRACE_LEVEL_ERROR 1
#define TRACE_LEVEL_WARNING 2
#define TRACE_LEVEL_INFO 3
#define TRACE_LEVEL_DEBUG 4

static int trace_level = TRACE_LEVEL_DEBUG;

#define TRACE_SET(level)     \
    do                       \
    {                        \
        trace_level = level; \
    } while (0)

static const char *TRACE_LEVEL_STR[] = {
    "",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG"};

#define TRACE(level, fmt, ...)                                                 \
    do                                                                         \
    {                                                                          \
        if (level <= trace_level)                                              \
        {                                                                      \
            printf("%s: %s:%d: ", TRACE_LEVEL_STR[level], __FILE__, __LINE__); \
            printf(fmt, ##__VA_ARGS__);                                        \
        }                                                                      \
    } while (0)

#endif // TRACE_H