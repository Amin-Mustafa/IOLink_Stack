#ifndef IOLINK_MASTER_H
#define IOLINK_MASTER_H

#include <stddef.h>
#include <stdbool.h>
#include "iolink_sensor.h"

typedef enum {
    MAX14819_CURRLIM_100mA = 0x00,
    MAX14819_CURRLIM_200mA = 0x01,
    MAX14819_CURRLIM_300mA = 0x02,
    MAX14819_CURRLIM_500mA = 0x03,
} MAX14819_CurrLim_t;

typedef struct {
    const char* spi_slave_name;
    uint8_t current_limit;      // Maps to MAX14819 REG_DrvrCurrLim

    size_t master_thread_stack;
    size_t dl_thread_stack;
} IOLink_Master_Cfg_t;

bool IOLink_Master_Init(const IOLink_Master_Cfg_t* cfg);
bool IOLink_Master_AttachSensor(uint8_t port, IOLink_Sensor_Driver_t* sensor_driver);
void IOLink_Master_WakePort(uint8_t port);
bool IOLink_Master_ReadISDU(uint8_t port, uint16_t index, uint8_t subindex, uint8_t *buffer, uint16_t *len, uint32_t timeout_ms);
bool IOLink_Master_WriteISDU(uint8_t port, uint16_t index, uint8_t subindex, const uint8_t *data, uint16_t len, uint32_t timeout_ms);

#endif  // IOLINK_MASTER_H