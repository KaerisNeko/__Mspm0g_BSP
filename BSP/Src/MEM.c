#include "MEM.h"

uint8_t MEM_txDmaDone = 0;
uint8_t MEM_rxDmaDone = 0;

void MEM_Init(void) {
    NVIC_EnableIRQ(MEM_IRQN);
    NVIC_EnableIRQ(DMA_INT_IRQn);
}

void MEM_SeqStart(void) {
    MEM_SPI_CS(0);
}

void MEM_SeqStop(void) {
    MEM_SPI_CS(1);
}

uint8_t MEM_RWByteBlocking(uint8_t data) {
    uint8_t recv = 0;

    DL_SPI_transmitData8(MEM_SPI_INST, data);
    while (DL_SPI_isBusy(MEM_SPI_INST));
    
    recv = DL_SPI_receiveData8(MEM_SPI_INST);
    while (DL_SPI_isBusy(MEM_SPI_INST));
    
    return recv;
}

void MEM_TxRx_DMA(uint8_t* txData, uint8_t* rxData, uint16_t size) {
    // Tx
    DL_DMA_setSrcAddr(MEM_DMA, MEM_SPI_TX_CH, (uint32_t)txData);
    DL_DMA_setDestAddr(MEM_DMA, MEM_SPI_TX_CH, (uint32_t)&(MEM_SPI_INST->TXDATA));
    DL_DMA_setTransferSize(MEM_DMA, MEM_SPI_TX_CH, size);
    DL_DMA_enableChannel(MEM_DMA, MEM_SPI_TX_CH);
    DL_DMA_startTransfer(MEM_DMA, MEM_SPI_TX_CH);
    
    // Rx
    DL_DMA_setSrcAddr(MEM_DMA, MEM_SPI_RX_CH, (uint32_t)&(MEM_SPI_INST->RXDATA));
    DL_DMA_setDestAddr(MEM_DMA, MEM_SPI_RX_CH, (uint32_t)rxData);
    DL_DMA_setTransferSize(MEM_DMA, MEM_SPI_RX_CH, size);
    DL_DMA_enableChannel(MEM_DMA, MEM_SPI_RX_CH);
    DL_DMA_startTransfer(MEM_DMA, MEM_SPI_RX_CH);
}

uint8_t MEM_IsTxDone(void) {
    uint8_t res = MEM_txDmaDone;
    MEM_txDmaDone = 0;
    return res;
}

uint8_t MEM_IsRxDone(void) {
    uint8_t res = MEM_rxDmaDone;
    MEM_rxDmaDone = 0;
    return res;
}



void MEM_IRQN_HANDLER(void) {
    uint32_t iidx = DL_SPI_getPendingInterrupt(MEM_SPI_INST);
    if (iidx == DL_SPI_IIDX_DMA_DONE_TX) {
        MEM_txDmaDone = 1;
    } else if (iidx == DL_SPI_IIDX_DMA_DONE_RX) {
        MEM_rxDmaDone = 1;
    }
}
