#include "lwip.h"
#include "lwip/sockets.h"
#include "iolink_server.h"
#include "../../Core/Src/O1D100/o1d100.h"
#include "../../Core/Src/IOLink/iolink_master.h"
#include "cmsis_os2.h"
#include <string.h>

osThreadId_t iolink_server_taskhandle;
const osThreadAttr_t iolink_server_attr = {
    .name = "IO-Link Server",
    .stack_size = 4096,
    .priority = osPriorityNormal
};


#define ETH_DBG 1

#define LASER_PORT IOLINK_PORT_A

enum Dbg {
    NOTHING, 
    TASK_STARTED,
    LWIP_INITIALIZED,
    SOCKET_INITIALIZED,
    SOCKET_BOUND,
    LISTENING,
    BIND_FAILED,
    ACCEPTING,
    ACCEPTED_CLIENT,
};

extern struct netif gnetif;
volatile enum Dbg check = 0;
volatile char my_debug_ip[16] = {0};

O1D100_t laser;

void iolink_server_task(void* args) {
    // Init sensor
    O1D100_Init(&laser);

    // Init master
    const IOLink_Master_Cfg_t cfg = {
        .spi_slave_name = "SPI1",
        .current_limit = MAX14819_CURRLIM_300mA,
    };

    IOLink_Master_Init(&cfg);
    IOLink_Master_AttachSensor(LASER_PORT, &laser.sensor_drv);
    IOLink_Master_WakePort(LASER_PORT);

    osDelay(2000);

    O1D100_SetUnits(&laser, O1D100_UNIT_MM);
    char vendor_name[20];
    O1D100_GetVendorName(&laser, vendor_name, 20);

    while(1) {
        volatile uint16_t __attribute__((unused)) dist = O1D100_GetDistance(&laser);
        osDelay(10);
    }
}

void start_iolink_server(void) {
    if(iolink_server_taskhandle == NULL) {
        iolink_server_taskhandle = osThreadNew(iolink_server_task, NULL, &iolink_server_attr);
    }
}


// static void process_cmd(char cmd[static 1], char reply[static 1]) {
//     static O1D100_Data_t laser_data;
//     if (strcmp(cmd, "on") == 0) {
//         O1D100_Start(MAX14819_PORT_B);
//         sprintf(reply, "Laser waking...\n> ");
//     }
//     else if (strcmp(cmd, "off") == 0) {
//         O1D100_Stop(MAX14819_PORT_B);
//         sprintf(reply, "Laser sleeping...\n> ");
//     }
//     else if (strcmp(cmd, "dist") == 0) {
//         O1D100_ReadData(MAX14819_PORT_B, &laser_data);
//         uint16_t dist = laser_data.distance_mm;
//         sprintf(reply, "Distance: %u mm\n> ", dist);
//     }
// }