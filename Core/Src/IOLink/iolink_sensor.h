#ifndef IOLINK_SENSOR_H
#define IOLINK_SENSOR_H

#include <stdint.h>
#include "iolink.h"
#include "cmsis_os2.h"

typedef struct IOLink_Sensor_Driver IOLink_Sensor_Driver_t;

typedef void (*Sensor_PD_Handler_t)(void* sensor_ctx, uint8_t data_len, const uint8_t *data);
typedef void (*Sensor_SMI_Handler_t)(void* sensor_ctx, iolink_arg_block_id_t ref_id, uint16_t len, arg_block_t* block);

struct IOLink_Sensor_Driver {
    void *sensor_ctx;                  // Pointer to the specific sensor struct (e.g., O1D100_t)
    uint8_t port_num;                  // Which port this sensor is attached to
    Sensor_PD_Handler_t pd_handler;    // The sensor's cyclic data parser
    
    osMutexId_t isdu_mutex;       // Prevents concurrent ISDU requests on the same port
    osSemaphoreId_t isdu_sem;     // Signals when the ISDU transaction is complete
    
    // Temporary storage for the active ISDU transaction
    uint16_t pending_index;
    uint8_t *isdu_rx_buffer;
    uint16_t isdu_rx_len;
    bool isdu_success;
};

#endif  // IOLINK_SENSOR_H