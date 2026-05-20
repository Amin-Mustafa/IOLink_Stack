#include "iolink_master.h"
#include "iolink_max14819.h"
#include "iolink_main.h"
#include "cmsis_os2.h"

static iolink_m_t* master_stack = NULL;
static iolink_hw_drv_t* phy_driver = NULL;
static iolink_pl_mode_t port_modes[2] = {iolink_mode_INACTIVE, iolink_mode_INACTIVE};

static IOLink_Sensor_Driver_t* attached_sensors[2] = {NULL, NULL};

static void Master_PD_Callback(uint8_t portnumber, void* arg, uint8_t data_len, const uint8_t* data) {
    if (portnumber >= 2) return;
    const IOLink_Sensor_Driver_t* sensor = attached_sensors[portnumber];
    if(sensor && sensor->pd_handler) {
        sensor->pd_handler(sensor->sensor_ctx, data_len, data);
    }
}

static void Master_SMI_Callback(void *arg, uint8_t portnumber, iolink_arg_block_id_t ref_id, uint16_t len, arg_block_t *block) {
    if (portnumber >= 2) return;
    const IOLink_Sensor_Driver_t* sensor = attached_sensors[portnumber];
    if(sensor && sensor->smi_handler) {
        sensor->smi_handler(sensor->sensor_ctx, ref_id, len, block);
    }
}

bool IOLink_Master_Init(const IOLink_Master_Cfg_t* cfg) {
    iolink_14819_cfg_t max_cfg = {0};
    max_cfg.chip_address    = cfg->spi_addr;
    max_cfg.spi_slave_name  = cfg->spi_slave_name;
    max_cfg.chip_irq        = cfg->chip_irq;
    max_cfg.DrvCurrLim      = cfg->current_limit;
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

bool IOLink_Master_AttachSensor(uint8_t port, IOLink_Sensor_Driver_t* sensor_driver) {
    if((port >= 2) || (sensor_driver == NULL)) {
        return false;
    }

    sensor_driver->port_num = port;
    attached_sensors[port] = sensor_driver;
    return true;
}

void IOLink_Master_WakePort(uint8_t port) {
    // Set request for seding port configuration
    arg_block_portconfiglist_t req = {0};
    req.arg_block.id = IOLINK_ARG_BLOCK_ID_PORT_CFG_LIST;   

    // Set port mode to auto
    req.configlist.portmode = IOLINK_PORTMODE_IOL_AUTO; 

    // Send request
    SMI_PortConfiguration_req(port, IOLINK_ARG_BLOCK_ID_PORT_CFG_LIST, sizeof(req), (arg_block_t *)&req);
}