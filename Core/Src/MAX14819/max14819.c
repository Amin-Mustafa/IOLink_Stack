#include "MAX14819.h"
#include "MAX14819_registers.h"
#include "../System/priorities.h"
#include "../GPIO/gpio.h"
#include "../SPI/spi.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include <string.h>

uint8_t MAX14819_Init() {
    // IRQ -> PA3
    // CS  -> PA4
    GPIO_EnableClock(MAX14819_CS_PORT);

    HAL_GPIO_WritePin(MAX14819_CS_PORT, MAX14819_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MAX14819_IRQ_PORT, MAX14819_IRQ_PIN, GPIO_PIN_SET);

    GPIO_InitPin(MAX14819_CS_PORT, MAX14819_CS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_MEDIUM, GPIO_NOPULL);
    GPIO_InitPin(MAX14819_IRQ_PORT, MAX14819_IRQ_PIN, GPIO_MODE_IT_FALLING, 0, GPIO_PULLUP);

    HAL_NVIC_SetPriority(EXTI3_IRQn, MAX14819_IRQ_PRIORITY, 0); 
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    SPI_Init(SPI_MODE_MASTER, SPI_DATASIZE_8BIT, SPI_BAUDRATEPRESCALER_32, SPI_DMA_TX_RX);
    
    // Read chip id (connection check)
    uint8_t rev_id = MAX14819_ReadRegister(MAX14819_REV_ID) & 0x0F;

    if (rev_id != MAX14819_REVID && rev_id != MAX14819A_REVID) {
        return 0; 
    }

    MAX14819_ReadRegister(MAX14819_STATUS);    // 0x1E
    MAX14819_ReadRegister(MAX14819_INTERRUPT); // 0x02

    // Enable crystal
    uint8_t clock = MAX14819_CLOCK_XTAL_EN | MAX14819_CLOCK_TXTX_EN_DIS;
    MAX14819_WriteRegister(MAX14819_CLOCK, clock);

    // Enable interrupts
    uint8_t int_en = MAX14819_INT_WU_RQ_INT_EN | MAX14819_INT_STATUS_INT_EN;
                     
    MAX14819_WriteRegister(MAX14819_INTERRUPT_EN, int_en);

    return 1;
}


void MAX14819_PowerOnPort(MAX14819_Port_t port) {
    uint8_t lcnfg = MAX14819_ReadPortReg(MAX14819_L_CNFG, port);

    if (lcnfg == 0xFF) return;

    lcnfg |= MAX14819_L_CNFG_EN;
    MAX14819_WritePortReg(MAX14819_L_CNFG, port, lcnfg);
}

void MAX14819_PowerOffPort(MAX14819_Port_t port) {
    uint8_t lcnfg = MAX14819_ReadPortReg(MAX14819_L_CNFG, port);
    lcnfg &= ~MAX14819_L_CNFG_EN;
    MAX14819_WritePortReg(MAX14819_L_CNFG, port, lcnfg);
}

static uint8_t MAX14819_ReadRegister(MAX14819_Register_t reg) {
    // MSB is 1 for read (0x80)
    reg |= 0x80;
    uint8_t tx_buf[2] = { reg, 0x00 }; 
    uint8_t rx_buf[2] = { 0 };

    SPI_TransmitReceive(MAX14819_CS_PORT, MAX14819_CS_PIN, tx_buf, rx_buf, 2);
    
    return rx_buf[1]; 
}

static void MAX14819_WriteRegister(MAX14819_Register_t reg, uint8_t val) {
    // MSB is 0 for write 
    reg &= 0x7F;
    uint8_t tx_buf[2] = { reg, val }; 
    
    SPI_Transmit(MAX14819_CS_PORT, MAX14819_CS_PIN, tx_buf, 2);
}

static uint8_t MAX14819_ReadPortReg(MAX14819_Register_t reg, MAX14819_Port_t port) {
    if(port == MAX14819_PORT_B) reg |= 1;
    return MAX14819_ReadRegister(reg);
}

static void MAX14819_WritePortReg(MAX14819_Register_t reg, MAX14819_Port_t port, uint8_t val) {
    if(port == MAX14819_PORT_B) reg |= 1;
    MAX14819_WriteRegister(reg, val);
}

void MAX14819_ActivatePort(MAX14819_Port_t port, uint32_t boot_time, MAX14819_CurrentLimit_t clim) {
    MAX14819_PowerOffPort(port);
    osDelay(1000);

    //enable auto-retry; set current limit
    uint8_t drvr_curr_lim = MAX14819_DRVR_CURR_LIM_AR_EN | (clim << 6);
    MAX14819_WriteRegister(MAX14819_DRVR_CURR_LIM, drvr_curr_lim);

    uint8_t lcnfg = MAX14819_L_CNFG_EN | MAX14819_L_CNFG_DYN_BL;
    MAX14819_WritePortReg(MAX14819_L_CNFG, port, lcnfg);
    //wait for the sensor to boot
    osDelay(boot_time);

    //enable internal framer
    MAX14819_WritePortReg(MAX14819_CHAN_STAT, port, MAX14819_CHAN_STAT_FRAMER_EN);
    
    //enable driver and filter
    uint8_t cq_cfg = MAX14819_CQ_CFG_PUSH_PULL | MAX14819_CQ_CFG_FILTER_EN | MAX14819_CQ_CFG_SINK_SEL0;
    MAX14819_WritePortReg(MAX14819_CQ_CFG, port, cq_cfg);

    //insert checksum on transmit, check on receive, and only interrupt when whole message received
    uint8_t msg_ctrl = MAX14819_MSG_CTRL_INS_CHKSUM | MAX14819_MSG_CTRL_RX_CHKSUM_EN | 
                        MAX14819_MSG_CTRL_RMSG_READY_EN | MAX14819_MSG_CTRL_TSIZE_EN;
    MAX14819_WritePortReg(MAX14819_MSG_CTRL, port, msg_ctrl);

    // Set IO-Link compliance delay
    uint8_t dev_dly = MAX14819_DEVICE_DLY_B_DELAY1 | MAX14819_DEVICE_DLY_D_DELAY0 | MAX14819_DEVICE_DLY_RSPNS_TMR_EN;
    MAX14819_WritePortReg(MAX14819_DEVICE_DLY, port, dev_dly);

    uint8_t current_int_en = MAX14819_ReadRegister(MAX14819_INTERRUPT_EN);
    if (port == MAX14819_PORT_A) {
        current_int_en |= (MAX14819_INT_RX_DATA_RDY_A_EN | MAX14819_INT_RX_ERROR_A_EN);
    } else {
        current_int_en |= (MAX14819_INT_RX_DATA_RDY_B_EN | MAX14819_INT_RX_ERROR_B_EN);
    }
    MAX14819_WriteRegister(MAX14819_INTERRUPT_EN, current_int_en);
}