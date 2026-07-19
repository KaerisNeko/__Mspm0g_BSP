#include "COM.h"

volatile uint8_t COM_txBuf[COM_BUF_SIZE];
volatile uint8_t COM_txBusy = 0;
volatile uint16_t COM_txCur = 0;
volatile uint16_t COM_txSize = 0;

volatile uint8_t COM_rxBuf[COM_BUF_SIZE];
volatile uint8_t COM_rxBusy = 0;
volatile uint16_t COM_rxSize = 0;

void COM_Init(void) {
    
}

void COM_Transmit(uint8_t* data, uint16_t size) {
    if (COM_txBusy) {
        return;
    }
    
    COM_txCur = 1;
    COM_txSize = size;
    COM_txBusy = 1;
    memcpy(COM_txBuf, data, size);
    DL_UART_Main_transmitData(COM_UART_INST, *COM_txBuf);
}

uint8_t COM_ReceiveValid(void) {
    if (COM_rxSize > 0 && COM_rxBusy == 0) {
        return 1;
    } else {
        return 0;
    }
}

void COM_Receive(uint8_t* data, uint16_t* size) {
    DL_UART_Main_disableInterrupt(COM_UART_INST, DL_UART_INTERRUPT_TX);
    memcpy(data, COM_rxBuf, COM_rxSize);
    *size = COM_rxSize;
    COM_rxSize = 0;
    DL_UART_enableInterrupt(COM_UART_INST, DL_UART_INTERRUPT_TX);
}

void COM_UART_IRQN_HANDLER(void) {
    uint32_t iidx = DL_UART_Main_getPendingInterrupt(COM_UART_INST);
    
    if (iidx == DL_UART_IIDX_TX) {
        if (COM_txCur >= COM_txSize) {
            COM_txBusy = 0;
            goto RET;
        }
        DL_UART_Main_transmitData(COM_UART_INST, COM_txBuf[COM_txCur]);
        COM_txCur++;
    } else if (iidx == DL_UART_IIDX_RX) {
        if (COM_rxSize >= COM_BUF_SIZE) { // buf overrun
            COM_rxSize = 0;
            goto RET;
        }
        
        if (COM_rxBusy == 0) { // RXfer SEQ start
            COM_rxSize = 0;
        }

        uint8_t rxByte = DL_UART_Main_receiveData(COM_UART_INST);
        COM_rxBuf[COM_rxSize] = rxByte;
        COM_rxSize++;
        if (rxByte == COM_RX_END_CHAR) {
            COM_rxBusy = 0;
        } else {
            COM_rxBusy = 1;
        }
    }

    RET:
    return;
}
