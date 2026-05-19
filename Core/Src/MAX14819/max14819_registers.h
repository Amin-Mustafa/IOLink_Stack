#ifndef MAX14819_REGISTERS_H
#define MAX14819_REGISTERS_H

typedef enum {
    MAX14819_TX_RX_DATA     = 0x00, // Port-specific
    MAX14819_INTERRUPT      = 0x02, // General
    MAX14819_INTERRUPT_EN   = 0x03, // General
    MAX14819_RX_FIFO_LVL    = 0x04, // Port-specific
    MAX14819_CQ_CTRL        = 0x06, // Port-specific
    MAX14819_CQ_ERR         = 0x08, // Port-specific
    MAX14819_MSG_CTRL       = 0x0A, // Port-specific
    MAX14819_CHAN_STAT      = 0x0C, // Port-specific
    MAX14819_LED_CTRL       = 0x0E, // General
    MAX14819_TRIGGER        = 0x0F, // General
    MAX14819_CQ_CFG         = 0x10, // Port-specific
    MAX14819_CYCL_TMR       = 0x12, // Port-specific
    MAX14819_DEVICE_DLY     = 0x14, // Port-specific
    MAX14819_TRIG_ASSGN     = 0x16, // Port-specific
    MAX14819_L_CNFG         = 0x18, // Port-specific
    MAX14819_IO_ST_CFG      = 0x1A, // Port-specific
    MAX14819_DRVR_CURR_LIM  = 0x1C, // General
    MAX14819_CLOCK          = 0x1D, // General
    MAX14819_STATUS         = 0x1E, // General
    MAX14819_REV_ID         = 0x1F, // General
} MAX14819_Register_t;

typedef enum {
    MAX14819_INT_RX_DATA_RDY_A   = 1 << 0,
    MAX14819_INT_RX_DATA_RDY_B   = 1 << 1,
    MAX14819_INT_RX_ERROR_A      = 1 << 2,
    MAX14819_INT_RX_ERROR_B      = 1 << 3,
    MAX14819_INT_TX_ERROR_A      = 1 << 4,
    MAX14819_INT_TX_ERROR_B      = 1 << 5,
    MAX14819_INT_WU_RQ           = 1 << 6,
    MAX14819_INT_STATUS          = 1 << 7,
} MAX14819_Interrupt;

typedef enum {
    MAX14819_INT_RX_DATA_RDY_A_EN   = 1 << 0,
    MAX14819_INT_RX_DATA_RDY_B_EN   = 1 << 1,
    MAX14819_INT_RX_ERROR_A_EN      = 1 << 2,
    MAX14819_INT_RX_ERROR_B_EN      = 1 << 3,
    MAX14819_INT_TX_ERROR_A_EN      = 1 << 4,
    MAX14819_INT_TX_ERROR_B_EN      = 1 << 5,
    MAX14819_INT_WU_RQ_INT_EN       = 1 << 6,
    MAX14819_INT_STATUS_INT_EN      = 1 << 7,
} MAX14819_InterruptEn;

typedef enum {
    MAX14819_CQ_CTRL_CQSEND         = 1 << 0,
    MAX14819_CQ_CTRL_CYCLE_TMR_EN   = 1 << 1,
    MAX14819_CQ_CTRL_RX_FIFO_RST    = 1 << 2,
    MAX14819_CQ_CTRL_TX_FIFO_RST    = 1 << 3,
    MAX14819_CQ_CTRL_WU_PULSE       = 1 << 4,
    MAX14819_CQ_CTRL_EST_COM        = 1 << 5,
    MAX14819_CQ_CTRL_COM_RT1_0      = 0x3 << 6,
} MAX14819_CQCtrl;

typedef enum {
    MAX14819_CQ_ERR_PARITY  = 1 << 0,
    MAX14819_CQ_ERR_FRAME   = 1 << 1,
    MAX14819_CQ_ERR_RSIZE   = 1 << 2,
    MAX14819_CQ_ERR_RCHKSUM = 1 << 3,
    MAX14819_CQ_ERR_TSIZE   = 1 << 4,
    MAX14819_CQ_ERR_TCHKSUM = 1 << 5,
    MAX14819_CQ_ERR_TCYCL   = 1 << 6,
    MAX14819_CQ_ERR_TRANSM  = 1 << 7,
} MAX14819_CQErr;

typedef enum {
    MAX14819_MSG_CTRL_INVCQ         = 1 << 0,
    MAX14819_MSG_CTRL_RMSG_READY_EN = 1 << 1,
    MAX14819_MSG_CTRL_RX_CHKSUM_EN  = 1 << 2,
    MAX14819_MSG_CTRL_TX_KEEP_MSG   = 1 << 3,
    MAX14819_MSG_CTRL_TSIZE_EN      = 1 << 4,
    MAX14819_MSG_CTRL_INS_CHKSUM    = 1 << 5,
    MAX14819_MSG_CTRL_SPI_CHKSUM    = 1 << 6,
    MAX14819_MSG_CTRL_TX_ER_DESTROY = 1 << 7,
} MAX14819_MsgCtrl;

typedef enum {
    MAX14819_CHAN_STAT_CQ_FAULT     = 1 << 0,
    MAX14819_CHAN_STAT_UVL          = 1 << 1,
    MAX14819_CHAN_STAT_LCLIM        = 1 << 2,
    MAX14819_CHAN_STAT_CQ_FAULT_COR = 1 << 3,
    MAX14819_CHAN_STAT_UVL_COR      = 1 << 4,
    MAX14819_CHAN_STAT_L_CLIM_COR   = 1 << 5,
    MAX14819_CHAN_STAT_FRAMER_EN    = 1 << 6,
    MAX14819_CHAN_STAT_RST          = 1 << 7,
} MAX14819_ChanStat;

typedef enum {
    MAX14819_LED_CTRL_RX_RDY_EN_A  = 1 << 0,
    MAX14819_LED_CTRL_LED_EN1_A    = 1 << 1,
    MAX14819_LED_CTRL_RX_ERR_EN_A  = 1 << 2,
    MAX14819_LED_CTRL_LED_EN2_A    = 1 << 3,
    MAX14819_LED_CTRL_RX_RDY_EN_B  = 1 << 4,
    MAX14819_LED_CTRL_LED_EN1_B    = 1 << 5,
    MAX14819_LED_CTRL_RX_ERR_EN_B  = 1 << 6,
    MAX14819_LED_CTRL_LED_EN2_B    = 1 << 7,
} MAX14819_LedCtrl;

typedef enum {
    MAX14819_TRIGGER_TRIG_INIT0 = 1 << 0,
    MAX14819_TRIGGER_TRIG_INIT1 = 1 << 1,
    MAX14819_TRIGGER_TRIG_INIT2 = 1 << 2,
    MAX14819_TRIGGER_TRIG_INIT3 = 1 << 3,
} MAX14819_Trigger;

typedef enum {
    MAX14819_CQ_CFG_FILTER_EN   = 1 << 0,
    MAX14819_CQ_CFG_DRV_DIS     = 1 << 1,
    MAX14819_CQ_CFG_PUSH_PULL   = 1 << 2,
    MAX14819_CQ_CFG_NPN         = 1 << 3,
    MAX14819_CQ_CFG_SINK_SEL0   = 1 << 4,
    MAX14819_CQ_CFG_SINK_SEL1   = 1 << 5,
    MAX14819_CQ_CFG_SOURCE_SINK = 1 << 6,
    MAX14819_CQ_CFG_IEC3TH      = 1 << 7,
} MAX14819_CQCfg;

typedef enum {
    MAX14819_CYCL_TMR_TCYCL_IM0   = 1 << 0,
    MAX14819_CYCL_TMR_TCYCL_IM1   = 1 << 1,
    MAX14819_CYCL_TMR_TCYCL_IM2   = 1 << 2,
    MAX14819_CYCL_TMR_TCYCL_IM3   = 1 << 3,
    MAX14819_CYCL_TMR_TCYCL_IM4   = 1 << 4,
    MAX14819_CYCL_TMR_TCYCL_IM5   = 1 << 5,
    MAX14819_CYCL_TMR_TCYCL_BS0   = 1 << 6,
    MAX14819_CYCL_TMR_TCYCL_BS1   = 1 << 7,
} MAX14819_CyclTmr;

typedef enum {
    MAX14819_DEVICE_DLY_RSPNS_TMR_EN = 1 << 0,
    MAX14819_DEVICE_DLY_D_DELAY0     = 1 << 1,
    MAX14819_DEVICE_DLY_D_DELAY1     = 1 << 2,
    MAX14819_DEVICE_DLY_D_DELAY2     = 1 << 3,
    MAX14819_DEVICE_DLY_D_DELAY3     = 1 << 4,
    MAX14819_DEVICE_DLY_B_DELAY0     = 1 << 5,
    MAX14819_DEVICE_DLY_B_DELAY1     = 1 << 6,
    MAX14819_DEVICE_DLY_ERR          = 1 << 7,
} MAX14819_DeviceDly;

typedef enum {
    MAX14819_TRIG_ASSGN_TRIG_EN = 1 << 0,
    MAX14819_TRIG_ASSGN_TRIG0   = 1 << 4,
    MAX14819_TRIG_ASSGN_TRIG1   = 1 << 5,
    MAX14819_TRIG_ASSGN_TRIG2   = 1 << 6,
    MAX14819_TRIG_ASSGN_TRIG3   = 1 << 7,
} MAX14819_TrigAssgn;

typedef enum {
    MAX14819_L_CNFG_EN       = 1 << 0,
    MAX14819_L_CNFG_CLIM_DIS = 1 << 1,
    MAX14819_L_CNFG_CL2X     = 1 << 2,
    MAX14819_L_CNFG_BL0      = 1 << 3,
    MAX14819_L_CNFG_BL1      = 1 << 4,
    MAX14819_L_CNFG_DYN_BL   = 1 << 5,
    MAX14819_L_CNFG_RT0      = 1 << 6,
    MAX14819_L_CNFG_RT1      = 1 << 7,
} MAX14819_LCnfg;

typedef enum {
    MAX14819_IO_ST_CFG_DI_CSINK     = 1 << 0,
    MAX14819_IO_ST_CFG_DI_CSOURCE   = 1 << 1,    
    MAX14819_IO_ST_CFG_DI_EC3TH     = 1 << 2,
    MAX14819_IO_ST_CFG_DI_FILTER_EN = 1 << 3,    
    MAX14819_IO_ST_CFG_TX           = 1 << 4,
    MAX14819_IO_ST_CFG_TX_EN        = 1 << 5,
    MAX14819_IO_ST_CFG_CQ_LEVEL     = 1 << 6,
    MAX14819_IO_ST_CFG_DI_LEVEL     = 1 << 7,
} MAX14819_IOSStCfg;

typedef enum {
    MAX14819_DRVR_CURR_LIM_AR_EN    = 1 << 0,
    MAX14819_DRVR_CURR_LIM_T_AR0    = 1 << 1,   
    MAX14819_DRVR_CURR_LIM_T_AR1    = 1 << 2,
    MAX14819_DRVR_CURR_LIM_CL_BL0   = 1 << 3,    
    MAX14819_DRVR_CURR_LIM_CL_BL1   = 1 << 4,
    MAX14819_DRVR_CURR_LIM_CL_DIS   = 1 << 5,
    MAX14819_DRVR_CURR_LIM_CL0      = 1 << 6,
    MAX14819_DRVR_CURR_LIM_CL1      = 1 << 7,
} MAX14819_DrvrCurrLim;

typedef enum {
    MAX14819_CLOCK_XTAL_EN      = 1 << 0,
    MAX14819_CLOCK_EXT_CLK_EN   = 1 << 1, 
    MAX14819_CLOCK_DIV0         = 1 << 2,
    MAX14819_CLOCK_DIV1         = 1 << 3,
    MAX14819_CLOCK_OEN          = 1 << 4,
    MAX14819_CLOCK_EXT_CLK_MIS  = 1 << 5,
    MAX14819_CLOCK_TXTX_EN_DIS  = 1 << 6,
    MAX14819_CLOCK_VCC_WARN_EN  = 1 << 7,
} MAX14819_Clock;

typedef enum {
    MAX14819_STATUS_VCC_WARN     = 1 << 0,
    MAX14819_STATUS_VCC_UV       = 1 << 1, 
    MAX14819_STATUS_TEMP_WARN    = 1 << 2,
    MAX14819_STATUS_TH_SHDN      = 1 << 3,
    MAX14819_STATUS_VCC_WARN_COR = 1 << 4,
    MAX14819_STATUS_VCC_UV_COR   = 1 << 5,
    MAX14819_STATUS_TH_WARN_COR  = 1 << 6,
    MAX14819_STATUS_TH_SHDN_COR  = 1 << 7,
} MAX14819_Status;

typedef enum {
    MAX14819_REV_ID_ID0 = 1 << 0,
    MAX14819_REV_ID_ID1 = 1 << 1,
    MAX14819_REV_ID_ID2 = 1 << 2,
    MAX14819_REV_ID_ID3 = 1 << 3,
} MAX14819_RevID;

#endif