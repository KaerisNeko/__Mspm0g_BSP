#ifndef __MEM_H__
#define __MEM_H__

#include "ti_msp_dl_config.h"

#define MEM_PORT_CS     GPIO_MEM_PORT
#define MEM_PIN_CS      GPIO_MEM_CS_PIN
#define MEM_SPI_INST    SPI_MEM_INST

#define MEM_DMA                 DMA
#define MEM_SPI_TX_CH           DMA_SPI_TX_CHAN_ID
#define MEM_SPI_RX_CH           DMA_SPI_RX_CHAN_ID

#define MEM_IRQN                SPI_MEM_INST_INT_IRQN
#define MEM_IRQN_HANDLER        SPI_MEM_INST_IRQHandler

#define MEM_SPI_CS(x) ((x) ?\
        DL_GPIO_setPins(MEM_PORT_CS, MEM_PIN_CS) :\
        DL_GPIO_clearPins(MEM_PORT_CS, MEM_PIN_CS))

void MEM_Init(void);

void MEM_SeqStart(void);
void MEM_SeqStop(void);
uint8_t MEM_RWByteBlocking(uint8_t data);

void MEM_TxRx_DMA(uint8_t* txData, uint8_t* rxData, uint16_t size);

uint8_t MEM_IsTxDone(void);
uint8_t MEM_IsRxDone(void);

#endif
