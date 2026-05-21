#ifndef O1D100_H
#define O1D100_H

#include "../IOLink/iolink_sensor.h"
#include <stdbool.h>

typedef struct {
    IOLink_Sensor_Driver_t sensor_drv;

    // Process data
    uint16_t distance_mm;
    uint16_t reflectivity_pct;
    bool is_valid;
} O1D100_t;

typedef enum {
    O1D100_UNIT_MM = 0,
    O1D100_UNIT_M  = 1,
    O1D100_UNIT_IN = 2
} O1D100_Unit_t;

void O1D100_Init(O1D100_t* sensor);
uint16_t O1D100_GetDistance(O1D100_t* sensor);
uint32_t O1D100_GetOperatingHours(O1D100_t* sensor);
bool O1D100_GetVendorName(O1D100_t *sensor, char *out_name, uint16_t max_len);
bool O1D100_SetUnits(O1D100_t *sensor, O1D100_Unit_t unit);

#endif