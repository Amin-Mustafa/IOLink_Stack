#include "o1d100.h"
#include "../IOLink/iolink_master.h"
#include <string.h>

#define O1D100_FAR 32760

static void O1D100_PD_Handler(void* sensor_ctx, uint8_t data_len, const uint8_t* data) {
    O1D100_t* sensor = (O1D100_t*)sensor_ctx;

    if(data_len >= 8) {
        uint16_t raw_dist = (data[0] << 8) | data[1];
        uint16_t raw_refl = (data[4] << 8) | data[5];

        if(raw_dist > O1D100_FAR) { 
            sensor->is_valid = false;
        } else {
            sensor->is_valid = true;
            sensor->distance_mm = raw_dist;
            sensor->reflectivity_pct = raw_refl;
        }
    }
}

// API

void O1D100_Init(O1D100_t *sensor) {
    memset(sensor, 0, sizeof(O1D100_t));
    
    sensor->drv_interface.sensor_ctx = sensor;
    sensor->drv_interface.pd_handler = O1D100_PD_Handler;
    
    sensor->drv_interface.isdu_mutex = osMutexNew(NULL);
    sensor->drv_interface.isdu_sem = osSemaphoreNew(1, 0, NULL);
}

uint16_t O1D100_GetDistance(O1D100_t *sensor) {
    return sensor->is_valid ? sensor->distance_mm : 0xFFFF;
}

bool O1D100_GetVendorName(O1D100_t *sensor, char *out_name, uint16_t max_len) {
    uint16_t len = max_len - 1;
    if (IOLink_Master_ReadISDU(sensor->drv_interface.port_num, 16, 0, (uint8_t*)out_name, &len, 1000)) {
        out_name[len] = '\0'; 
        return true;
    }
    return false;
}

uint32_t O1D100_GetOperatingHours(O1D100_t *sensor) {
    uint32_t buffer;
    uint16_t len = 4;
    
    if (IOLink_Master_ReadISDU(sensor->drv_interface.port_num, 542, 0, ((uint8_t*)&buffer), &len, 1000)) {
        // Parse Big-Endian payload
        return __builtin_bswap32(buffer);
    }
    return 0xFFFFFFFF; // Error
}

bool O1D100_SetUnit(O1D100_t *sensor, O1D100_Unit_t unit)  {
    if (unit > O1D100_UNIT_IN) return false;
    
    uint8_t payload = (uint8_t)unit;
    
    // Dispatch the write request (Index 551, Subindex 0, Length 1 byte)
    return IOLink_Master_WriteISDU(
        sensor->drv_interface.port_num, 
        551, 
        0, 
        &payload, 
        1, 
        1000
    );
}