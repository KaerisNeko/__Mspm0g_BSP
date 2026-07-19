#ifndef __COM_H__
#define __COM_H__

#include "ti_msp_dl_config.h"
#include "HRT.h"
#include <string.h>

#define COM_UART_INST           UART_COM_INST
#define COM_UART_IRQN_HANDLER   UART_COM_INST_IRQHandler
#define COM_NVIC_IRQN           UART_COM_INST_INT_IRQN
#define COM_BUF_SIZE            256
#define COM_RX_END_CHAR         ';'

void COM_Init(void);
uint8_t COM_TransmitBusy(void);
void COM_Transmit(uint8_t* data, uint16_t size);
void COM_TransmitBlocking(uint8_t* data, uint16_t size);
uint8_t COM_ReceiveValid(void);
void COM_Receive(uint8_t* data, uint16_t* size);

#endif
