#include "jlink_rtt_log.h"
#include "cmsis_os2.h"

osMutexId_t rtt_log_mutex = NULL;

void osal_log_init(void) {
    rtt_log_mutex = osMutexNew(NULL);
}
