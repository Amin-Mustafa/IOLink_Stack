#ifndef IOLINK_MASTER_H
#define IOLINK_MASTER_H

#include <stddef.h>
#include <stdbool.h>
#include "iolink_sensor.h"

typedef struct {
    uint8_t spi_addr;
    const char* spi_slave_name;
    uint32_t chip_irq;

    uint8_t current_limit;      // Maps to MAX14819 REG_DrvrCurrLim
    uint8_t cycle_timer_base;   // Cycle time base config

    size_t master_thread_stack;
    size_t dl_thread_stack;
} IOLink_Master_Cfg_t;

bool IOLink_Master_Init(const IOLink_Master_Cfg_t *cfg);
bool IOLink_Master_AttachSensor(uint8_t port, IOLink_Sensor_Driver_t *sensor_driver);
void IOLink_Master_WakePort(uint8_t port);

#endif  // IOLINK_MASTER_H