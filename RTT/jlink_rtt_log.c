#include "jlink_rtt_log.h"
#include "cmsis_os2.h"

osMutexId_t rtt_log_mutex = NULL;

void JLINK_RTT_LogInit(void) {
    rtt_log_mutex = osMutexNew(NULL);
}
