#include "osal_log.h"
#include "cmsis_os2.h"
#include "SEGGER_RTT.h"
#include <stdarg.h>

void os_log (uint8_t type, const char * fmt, ...) {
    va_list list;

    uint32_t ticks = osKernelGetTickCount();
    uint32_t sec = ticks / 1000;
    uint32_t ms  = ticks % 1000;

    unsigned term_id = 0;

    switch(LOG_LEVEL_GET(type)) {
        case LOG_LEVEL_DEBUG:
            SEGGER_RTT_printf(term_id, "[%6u DEBUG] ", sec, ms);
            break;
        case LOG_LEVEL_INFO:
            SEGGER_RTT_printf(term_id, "[%3u.%03u INFO ] ", sec, ms);
            break;
        case LOG_LEVEL_WARNING:
            SEGGER_RTT_printf(term_id, "[%3u.%03u WARN ] ", sec, ms);
            break;
        case LOG_LEVEL_ERROR:
            SEGGER_RTT_printf(term_id, "[%3u.%03u ERROR] ", sec, ms);
            break;
        case LOG_LEVEL_FATAL:
            SEGGER_RTT_printf(term_id, "[%3u.%03u FATAL] ", sec, ms);
            break;
        default:
            break;
    }

    va_start (list, fmt);
    SEGGER_RTT_vprintf(term_id, fmt, &list);
    va_end (list);
}