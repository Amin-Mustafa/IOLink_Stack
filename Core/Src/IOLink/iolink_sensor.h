#ifndef IOLINK_SENSOR_H
#define IOLINK_SENSOR_H

#include <stdint.h>
#include "iolink.h"

typedef struct IOLink_Sensor_Driver IOLink_Sensor_Driver_t;

typedef void (*Sensor_PD_Handler_t)(void* sensor_ctx, uint8_t data_len, const uint8_t *data);
typedef void (*Sensor_SMI_Handler_t)(void* sensor_ctx, iolink_arg_block_id_t ref_id, uint16_t len, arg_block_t* block);

struct IOLink_Sensor_Driver {
    void *sensor_ctx;                  // Pointer to the specific sensor struct (e.g., O1D100_t)
    uint8_t port_num;                  // Which port this sensor is attached to
    Sensor_PD_Handler_t pd_handler;    // The sensor's cyclic data parser
    Sensor_SMI_Handler_t smi_handler;  // The sensor's acyclic data parser
};

#endif  // IOLINK_SENSOR_H