#include "GYRO.h"
#include <string.h>

uint8_t GYRO_txBuf[GYRO_I2C_BUF_SIZE];
uint8_t GYRO_rxBuf[GYRO_I2C_BUF_SIZE];

typedef enum GYRO_I2cState_t {
    GYRO_i2cStateIdle = 0,

    GYRO_i2cStateTxStarted,
    GYRO_i2cStateTxInPgr,
    GYRO_i2cStateTxCplt,
    
    GYRO_i2cStateRxStarted,
    GYRO_i2cStateRxInPgr,
    GYRO_i2cStateRxCplt,
}GYRO_I2cState;

volatile GYRO_I2cState GYRO_i2cState = GYRO_i2cStateIdle;

// Tx vars:
volatile static uint16_t GYRO_txSize = 0;
volatile static uint16_t GYRO_txferedLen = 0;

// Rx vars:
volatile static uint16_t GYRO_rxSize = 0;
volatile static uint16_t GYRO_rxferedLen = 0;
static uint8_t* GYRO_rxDest = NULL;
static uint8_t GYRO_rxValid = 0;

void GYRO_Init(void) {
    DL_I2C_enableInterrupt(GYRO_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER |
        DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER
    );
    NVIC_EnableIRQ(GYRO_I2C_IRQN);
}

void GYRO_Update(void) {
    switch (GYRO_i2cState) {
        case GYRO_i2cStateTxStarted: {
            if (DL_I2C_getControllerStatus(GYRO_I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE) {
                DL_I2C_startControllerTransfer(
                        GYRO_I2C_INST, GYRO_I2C_ADDR,
                        DL_I2C_CONTROLLER_DIRECTION_TX, GYRO_txSize);
                GYRO_i2cState = GYRO_i2cStateTxInPgr;
            }
            break;
        }
        case GYRO_i2cStateTxInPgr: {
            break;
        }
        case GYRO_i2cStateTxCplt: {
            if (DL_I2C_getControllerStatus(GYRO_I2C_INST) &
                    DL_I2C_CONTROLLER_STATUS_IDLE) {
                GYRO_txSize = GYRO_txferedLen = 0;
                GYRO_i2cState = GYRO_i2cStateIdle;
            }
            break;
        }
        case GYRO_i2cStateRxCplt: {
            if (DL_I2C_getControllerStatus(GYRO_I2C_INST) &
                    DL_I2C_CONTROLLER_STATUS_IDLE) {
                if (GYRO_rxDest != NULL) {
                    memcpy(GYRO_rxDest, GYRO_rxBuf, GYRO_rxSize);
                    GYRO_rxDest = NULL;
                    GYRO_rxValid = 1;
                }
                GYRO_rxSize = GYRO_rxferedLen = 0;
                
                GYRO_i2cState = GYRO_i2cStateIdle;
            }
            break;
        }

        default: break;
    }
}

uint8_t GYRO_I2CIsBusy(void) {
    if (GYRO_i2cState != GYRO_i2cStateIdle) {
        return 1;
    }
    return 0;
}

static void GYRO_I2CTransmitITStart(uint16_t size) {
    GYRO_txSize = size;
    GYRO_txferedLen = DL_I2C_fillControllerTXFIFO(GYRO_I2C_INST, GYRO_txBuf, size);
    if (GYRO_txferedLen < size) {
        DL_I2C_enableInterrupt(GYRO_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    } else {
        DL_I2C_disableInterrupt(GYRO_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    }
    GYRO_i2cState = GYRO_i2cStateTxStarted;
}

void GYRO_I2CWrite_IT(uint8_t* data, uint16_t size) {
    if (GYRO_i2cState != GYRO_i2cStateIdle) {
        return;
    }
    if (size > GYRO_I2C_BUF_SIZE) {
        return;
    }
    
    memcpy(GYRO_txBuf, data, size);
    GYRO_I2CTransmitITStart(size);
}

static void GYRO_I2CReceiveITStart(uint16_t size) {
    GYRO_rxSize = size;
    GYRO_rxferedLen = 0;
    DL_I2C_startControllerTransfer(
            GYRO_I2C_INST, GYRO_I2C_ADDR,
            DL_I2C_CONTROLLER_DIRECTION_RX, GYRO_rxSize);
    GYRO_i2cState = GYRO_i2cStateRxStarted;
}

void GYRO_I2CRead_IT(uint8_t* data, uint16_t size) {
    if (GYRO_i2cState != GYRO_i2cStateIdle) {
        return;
    }
    if (size > GYRO_I2C_BUF_SIZE) {
        return;
    }

    GYRO_rxDest = data;
    GYRO_I2CReceiveITStart(size);
}

uint8_t GYRO_I2CReceiveValid(void) {
    uint8_t res = GYRO_rxValid;
    GYRO_rxValid = 0;
    return res;
}



void GYRO_I2C_IRQ_HANDLER(void) {
    uint32_t iidx = DL_I2C_getPendingInterrupt(GYRO_I2C_INST);

    if (iidx == DL_I2C_IIDX_CONTROLLER_TX_DONE) {
        DL_I2C_disableInterrupt(
                GYRO_I2C_INST,
                DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
        GYRO_i2cState = GYRO_i2cStateTxCplt;
    }
    if (iidx == DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER) {
        if (GYRO_txferedLen < GYRO_txSize) {
            GYRO_txferedLen += DL_I2C_fillControllerTXFIFO(
                    GYRO_I2C_INST, &GYRO_txBuf[GYRO_txferedLen],
                    GYRO_txSize - GYRO_txferedLen);
        }
    }
    
    if (iidx == DL_I2C_IIDX_CONTROLLER_RX_DONE) {
        while (DL_I2C_isControllerRXFIFOEmpty(GYRO_I2C_INST) != true) {
            if (GYRO_rxferedLen < GYRO_rxSize) {
                GYRO_rxBuf[GYRO_rxferedLen++] =
                        DL_I2C_receiveControllerData(GYRO_I2C_INST);
            } else {
                DL_I2C_receiveControllerData(GYRO_I2C_INST);
            }
        }
        GYRO_i2cState = GYRO_i2cStateRxCplt;
    }
    if (iidx == DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER) {
        GYRO_i2cState = GYRO_i2cStateRxInPgr;
        while (DL_I2C_isControllerRXFIFOEmpty(GYRO_I2C_INST) != true) {
            if (GYRO_rxferedLen < GYRO_rxSize) {
                GYRO_rxBuf[GYRO_rxferedLen++] =
                        DL_I2C_receiveControllerData(GYRO_I2C_INST);
            } else {
                DL_I2C_receiveControllerData(GYRO_I2C_INST);
            }
        }
    }
}
