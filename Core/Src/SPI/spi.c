#include "spi.h"
#include "stm32h5xx_hal.h"
#include "cmsis_os2.h"
#include "../System/system.h"
#include "../System/priorities.h"
#include <string.h>

#define DMA_BUF_SIZE    256
#define HARD_TIMEOUT    100
#define NUM_SPI         5

typedef struct {
    uint32_t request;
    DMA_Channel_TypeDef* channel;
    uint32_t irqn;
    enum {SPI_DMACONFIG_RX, SPI_DMACONFIG_TX} type;
} SPI_DmaConfig_t;

typedef struct {
    SPI_TypeDef* instance;
    SPI_HandleTypeDef hspi;

    osMutexId_t bus_mutex;
    osSemaphoreId_t transfer_cplt_smphr;

    SPI_DMA_t dma_mode;

    DMA_HandleTypeDef hdma_rx;
    DMA_HandleTypeDef hdma_tx;
    
    SPI_DmaConfig_t rx_dma_config;
    SPI_DmaConfig_t tx_dma_config;

    SPI_DMA_t dma_line;

} SPI_Context_t;


static SPI_Context_t ctx = {
    .instance = SPI3,
    .rx_dma_config = {
        .request = GPDMA1_REQUEST_SPI3_RX,
        .channel = GPDMA1_Channel0,
        .irqn = GPDMA1_Channel0_IRQn,
        .type = SPI_DMACONFIG_RX
    },
    .tx_dma_config = {
        .request = GPDMA1_REQUEST_SPI3_TX, 
        .channel = GPDMA1_Channel1, 
        .irqn = GPDMA1_Channel1_IRQn, 
        .type = SPI_DMACONFIG_TX
    }
};


static uint8_t enable_spi_dma(DMA_HandleTypeDef* hdma, SPI_DmaConfig_t config) {
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    hdma->Instance          = config.channel;
    hdma->Init.Request      = config.request;

    hdma->Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    hdma->Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    hdma->Init.DestDataWidth= DMA_DEST_DATAWIDTH_BYTE;
    hdma->Init.Priority     = DMA_LOW_PRIORITY_LOW_WEIGHT;
    hdma->Init.SrcBurstLength   = 1;
    hdma->Init.DestBurstLength  = 1;
    hdma->Init.TransferAllocatedPort    = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    hdma->Init.TransferEventMode        = DMA_TCEM_BLOCK_TRANSFER;
    hdma->Init.Mode         = DMA_NORMAL;

    if(config.type == SPI_DMACONFIG_RX) {
        //fix periph, increment memory
        hdma->Init.Direction    = DMA_PERIPH_TO_MEMORY;
        hdma->Init.SrcInc       = DMA_SINC_FIXED;
        hdma->Init.DestInc      = DMA_DINC_INCREMENTED;
    } else {
        hdma->Init.Direction    = DMA_MEMORY_TO_PERIPH;
        hdma->Init.SrcInc       = DMA_SINC_INCREMENTED;
        hdma->Init.DestInc      = DMA_DINC_FIXED;
    }
    if (HAL_DMA_Init(hdma) != HAL_OK) {
      Error_Handler();
    }

    return 1;
} 

void SPI_Init(uint32_t mode, uint32_t data_size, uint32_t baudrate_psc, SPI_DMA_t dma_line) {
    ctx.dma_line = SPI_DMA_NONE;

    if(ctx.bus_mutex == NULL) {
        ctx.bus_mutex = osMutexNew(NULL);
    }
    if(ctx.transfer_cplt_smphr == NULL) {
        ctx.transfer_cplt_smphr = osSemaphoreNew(1, 0, NULL);
    }

    ctx.hspi.Instance = ctx.instance;
    ctx.hspi.Init.Mode = mode;
    ctx.hspi.Init.Direction = SPI_DIRECTION_2LINES;
    ctx.hspi.Init.DataSize = data_size;
    ctx.hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    ctx.hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    ctx.hspi.Init.NSS = SPI_NSS_SOFT;
    ctx.hspi.Init.BaudRatePrescaler = baudrate_psc;
    ctx.hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    ctx.hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    ctx.hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    ctx.hspi.Init.CRCPolynomial = 0x7;
    ctx.hspi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    ctx.hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    ctx.hspi.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    ctx.hspi.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    ctx.hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    ctx.hspi.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    ctx.hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    ctx.hspi.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    ctx.hspi.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    ctx.hspi.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;

    if (HAL_SPI_Init(&ctx.hspi) != HAL_OK) {
        return; 
    }

    if(dma_line & SPI_DMA_RX) {
        if(enable_spi_dma(&ctx.hdma_rx, ctx.rx_dma_config)) {
            ctx.dma_line |= SPI_DMA_RX;
            __HAL_LINKDMA(&ctx.hspi, hdmarx, ctx.hdma_rx);
            HAL_NVIC_SetPriority(ctx.rx_dma_config.irqn, SPI3_RX_DMA_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(ctx.rx_dma_config.irqn);
        } else {
            ctx.dma_line &= ~(SPI_DMA_RX);
        }
    }
    if(dma_line & SPI_DMA_TX) {
        if(enable_spi_dma(&ctx.hdma_tx, ctx.tx_dma_config)) {
            ctx.dma_line |= SPI_DMA_TX;
            __HAL_LINKDMA(&ctx.hspi, hdmatx, ctx.hdma_tx);
            HAL_NVIC_SetPriority(ctx.tx_dma_config.irqn, SPI3_TX_DMA_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(ctx.tx_dma_config.irqn);
        } else {
            ctx.dma_line &= ~(SPI_DMA_TX);
        }
    }
}

static uint8_t spi_transfer_success(SPI_Context_t* spi_ctx) {
    uint8_t timeout = osSemaphoreAcquire(spi_ctx->transfer_cplt_smphr, HARD_TIMEOUT) != osOK;
    uint8_t error   =  spi_ctx->hspi.ErrorCode != HAL_SPI_ERROR_NONE;
    return !timeout && !error;
}

void SPI_Select(GPIO_TypeDef* port, uint16_t pin) {
    if (osKernelGetState() == osKernelRunning) {
        osMutexAcquire(ctx.bus_mutex, osWaitForever);
    }

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void SPI_Release(GPIO_TypeDef* port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

    if (osKernelGetState() == osKernelRunning) {
        osMutexRelease(ctx.bus_mutex);
    }
}


uint8_t SPI_Transmit(const void* data, uint16_t len) {
    if (osKernelGetState() != osKernelRunning) {
        uint8_t status = (HAL_SPI_Transmit(&ctx.hspi, (uint8_t*)data, len, HARD_TIMEOUT) == HAL_OK);
        return status;
    }
    
    uint8_t success = 1;
    if (ctx.dma_line & SPI_DMA_TX) {
        // DMA mode
        if (HAL_SPI_Transmit_DMA(&ctx.hspi, (uint8_t*)data, len) == HAL_OK) { 
            if(!spi_transfer_success(&ctx)) {
                HAL_SPI_Abort(&ctx.hspi);
                success = 0; 
            } 
        } else {
            success = 0;
        }
    } else {
        // IT mode
        if (HAL_SPI_Transmit_IT(&ctx.hspi, (uint8_t*)data, len) == HAL_OK) {
            if(!spi_transfer_success(&ctx)) {
                HAL_SPI_Abort(&ctx.hspi);
                success = 0;
            }
        } else {
            success = 0;
        }
    }

    return success;
}

uint16_t SPI_Receive(void* data, uint16_t len) {
    
    if (osKernelGetState() != osKernelRunning) {
        HAL_StatusTypeDef status = HAL_SPI_Receive(&ctx.hspi, (uint8_t*)data, len, HARD_TIMEOUT);
        return (status == HAL_OK) ? len : 0;
    }

    uint16_t bytes_received = 0;
    
    if (ctx.dma_line & SPI_DMA_RX) {
        // DMA mode
        if(HAL_SPI_Receive_DMA(&ctx.hspi, (uint8_t*)data, len) == HAL_OK) { 
            if(spi_transfer_success(&ctx)) {
                bytes_received = len;
            } else {
                HAL_SPI_Abort(&ctx.hspi);
            }
        }
    } else {
        if (HAL_SPI_Receive_IT(&ctx.hspi, (uint8_t*)data, len) == HAL_OK) {
            if (spi_transfer_success(&ctx)) {
                bytes_received = len;
            } else {
                HAL_SPI_Abort(&ctx.hspi);
            }
        }
    }

    return bytes_received;
}

uint16_t SPI_TransmitReceive(const void* tx_data, void* rx_data, uint16_t len) {
    if (osKernelGetState() != osKernelRunning) {
        HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&ctx.hspi, (uint8_t*)tx_data, (uint8_t*)rx_data, len, HARD_TIMEOUT);
        return (status == HAL_OK) ? len : 0;
    }

    uint16_t bytes_transferred = 0;

    if ((ctx.dma_line & SPI_DMA_TX_RX) == SPI_DMA_TX_RX) {        
        // DMA mode
        if (HAL_SPI_TransmitReceive_DMA(&ctx.hspi, (uint8_t*)tx_data, (uint8_t*)rx_data, len) == HAL_OK) { 
            if (spi_transfer_success(&ctx)) {
                bytes_transferred = len;
            } else {
                HAL_SPI_Abort(&ctx.hspi);
            }
        }
    } else {
        // IT mode
        if (HAL_SPI_TransmitReceive_IT(&ctx.hspi, (uint8_t*)tx_data, (uint8_t*)rx_data, len) == HAL_OK) {
            if (spi_transfer_success(&ctx)) {
                bytes_transferred = len;
            } else {
                HAL_SPI_Abort(&ctx.hspi);
            }
        }
    }
    
    return bytes_transferred;
}

SPI_HandleTypeDef* SPI_GetHandle() {
    return &ctx.hspi;
}

//interrupt/error handlers
static void signal_spi_transfer_cplt(SPI_HandleTypeDef *hspi) {
    osSemaphoreRelease(ctx.transfer_cplt_smphr);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
    signal_spi_transfer_cplt(hspi);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
    signal_spi_transfer_cplt(hspi);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi) {
    signal_spi_transfer_cplt(hspi);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    signal_spi_transfer_cplt(hspi);
}

void SPI3_IRQHandler(void) {
    HAL_SPI_IRQHandler(&ctx.hspi);
}

void GPDMA1_Channel0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&ctx.hdma_rx);
}

void GPDMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(&ctx.hdma_tx);
}