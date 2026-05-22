#include "iolink_master.h"
#include "iolink_max14819.h"
#include "iolink_main.h"
#include "cmsis_os2.h"

static iolink_m_t* master_stack = NULL;
static iolink_hw_drv_t* phy_driver = NULL;
static iolink_pl_mode_t port_modes[2] = {iolink_mode_INACTIVE, iolink_mode_INACTIVE};

static IOLink_Sensor_Driver_t* attached_sensors[IOLINK_PORT_COUNT] = {NULL};

static void Master_PD_Callback(uint8_t portnumber, void* arg, uint8_t data_len, const uint8_t* data) {
    const IOLink_Sensor_Driver_t* sensor = attached_sensors[portnumber - 1];
    if(sensor && sensor->pd_handler) {
        sensor->pd_handler(sensor->sensor_ctx, data_len, data);
    }
}

static void Master_SMI_Callback(void *arg, uint8_t portnumber, iolink_arg_block_id_t ref_id, uint16_t len, arg_block_t *block)  {
    IOLink_Sensor_Driver_t* sensor = attached_sensors[portnumber - 1];
    if(!sensor) return;

    if (ref_id == IOLINK_ARG_BLOCK_ID_OD_RD) {
        // On-demand data
        arg_block_od_t* od_data = (arg_block_od_t*)block;
        if(od_data->index == sensor->pending_index) {
            int payload = len - sizeof(arg_block_od_t);
            if((payload > 0) && (payload <= sensor->isdu_rx_len) && (sensor->isdu_rx_buffer != NULL)) {
                memcpy(sensor->isdu_rx_buffer, od_data->data, payload);
                sensor->isdu_rx_len = payload; // Update with actual length received
                sensor->isdu_success = true;
            } else {
                sensor->isdu_success = false;
            }
        }
    }
    // Handle Write Confirmations
    else if (ref_id == IOLINK_ARG_BLOCK_ID_OD_WR) 
    {
        // The stack echoes back an OD_WR block on success
        arg_block_od_t *od_data = (arg_block_od_t *)block;
        if (od_data->index == sensor->pending_index) 
        {
            sensor->isdu_success = true;
        }
    }
    // Handle Explicit Failures
    else if (ref_id == IOLINK_ARG_BLOCK_ID_JOB_ERROR) 
    {
        sensor->isdu_success = false;
    }

    osSemaphoreRelease(sensor->isdu_sem);
}

// API

bool IOLink_Master_Init(const IOLink_Master_Cfg_t* cfg) {
    iolink_14819_cfg_t max_cfg = {0};
    max_cfg.spi_slave_name  = (cfg == NULL) ? "Default" : cfg->spi_slave_name;
    max_cfg.DrvCurrLim      = (cfg == NULL) ? MAX14819_CURRLIM_100mA : cfg->current_limit;
    max_cfg.CQCfgA          = MAX14819_CQCFG_PUSHPUL;
    max_cfg.CQCfgB          = MAX14819_CQCFG_PUSHPUL;

    phy_driver = iolink_14819_init(&max_cfg);
    if(!phy_driver) return false;

    static iolink_port_cfg_t port_configs[2] = {
        {.name = "Port A", .mode = &port_modes[0], .arg = (void*)0},
        {.name = "Port B", .mode = &port_modes[1], .arg = (void*)1}
    };
    port_configs[0].drv = phy_driver;
    port_configs[1].drv = phy_driver;

    iolink_m_cfg_t master_cfg = {0};
    master_cfg.port_cnt = 2;
    master_cfg.port_cfgs = port_configs;
    master_cfg.master_thread_stack_size = cfg->master_thread_stack ? cfg->master_thread_stack : 2048;
    master_cfg.master_thread_prio       = osPriorityNormal;
    master_cfg.dl_thread_stack_size     = cfg->dl_thread_stack     ? cfg->dl_thread_stack     : 2048;
    master_cfg.dl_thread_prio           = osPriorityHigh;
    master_cfg.cb_pd = Master_PD_Callback;
    master_cfg.cb_smi = Master_SMI_Callback;

    master_stack = iolink_m_init(&master_cfg);
    return (master_stack != NULL);
}

bool IOLink_Master_AttachSensor(IOLink_Port_t port, IOLink_Sensor_Driver_t* sensor_driver) { 
    if((port >= IOLINK_PORT_COUNT) || (sensor_driver == NULL)) {
        return false;
    }

    sensor_driver->port_num = port;
    attached_sensors[port] = sensor_driver;
    return true;
}

void IOLink_Master_WakePort(IOLink_Port_t port) {
    if(port >= IOLINK_PORT_COUNT) return;
    // Set request for seding port configuration
    arg_block_portconfiglist_t req = {0};
    req.arg_block.id = IOLINK_ARG_BLOCK_ID_PORT_CFG_LIST;   

    // Set port mode to auto
    req.configlist.portmode = IOLINK_PORTMODE_IOL_AUTO; 

    // Send request
    SMI_PortConfiguration_req(port + 1, IOLINK_ARG_BLOCK_ID_PORT_CFG_LIST, sizeof(req), (arg_block_t *)&req);
}

bool IOLink_Master_ReadISDU(IOLink_Port_t port, uint16_t index, uint8_t subindex, uint8_t *buffer, uint16_t *len, uint32_t timeout_ms) {
    if (port >= IOLINK_PORT_COUNT || !buffer || !len) return false;

    IOLink_Sensor_Driver_t* sensor = attached_sensors[port];
    if (!sensor) return false;

    osMutexAcquire(sensor->isdu_mutex, osWaitForever);

    // Setup transaction state
    sensor->pending_index = index;
    sensor->isdu_rx_buffer = buffer;
    sensor->isdu_rx_len = *len;
    sensor->isdu_success = false;

    // Dispatch request
    arg_block_od_t read_req; 
    read_req.arg_block.id = IOLINK_ARG_BLOCK_ID_OD_RD;
    read_req.index = index;   
    read_req.subindex = subindex; 
    
    SMI_DeviceRead_req(port + 1, IOLINK_ARG_BLOCK_ID_OD_RD, sizeof(arg_block_od_t), (arg_block_t *)&read_req);

    // Block until Master_SMI_Callback releases the semaphore or timeout
    osStatus_t status = osSemaphoreAcquire(sensor->isdu_sem, timeout_ms);

    sensor->isdu_rx_buffer = NULL;
    *len = sensor->isdu_rx_len; // Pass received length back to the caller
    
    osMutexRelease(sensor->isdu_mutex);

    return (status == osOK && sensor->isdu_success);
}

bool IOLink_Master_WriteISDU(IOLink_Port_t port, uint16_t index, uint8_t subindex, const uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    if (port >= IOLINK_PORT_COUNT || !data || !len) return false;
    
    IOLink_Sensor_Driver_t* sensor = attached_sensors[port];
    if (!sensor) return false;

    osMutexAcquire(sensor->isdu_mutex, osWaitForever);

    sensor->pending_index = index;
    sensor->isdu_success = false;

    // Allocate a buffer large enough to hold the header + payload
    uint8_t req_buffer[sizeof(arg_block_od_t) + IOLINK_OD_MAX_SIZE];
    arg_block_od_t *write_req = (arg_block_od_t *)req_buffer;
    
    // Setup request
    write_req->arg_block.id = IOLINK_ARG_BLOCK_ID_OD_WR;
    write_req->index = index;   
    write_req->subindex = subindex; 
    // Copy user payload to tx data
    memcpy(write_req->data, data, len);

    // Dispatch write request
    SMI_DeviceWrite_req(port + 1, IOLINK_ARG_BLOCK_ID_OD_WR, sizeof(req_buffer), (arg_block_t *)write_req);

    // Block until the SMI callback catches the confirmation or error
    osStatus_t status = osSemaphoreAcquire(sensor->isdu_sem, timeout_ms);
    
    osMutexRelease(sensor->isdu_mutex);

    return (status == osOK && sensor->isdu_success);
}