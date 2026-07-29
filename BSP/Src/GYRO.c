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

typedef enum GYRO_I2cReadRegState_t {
    GYRO_i2cReadRegStateIdle = 0,

    GYRO_i2cReadRegStateWaitWriteAddr,
    GYRO_i2cReadRegStateWaitRead,
}GYRO_I2cReadRegState;

volatile GYRO_I2cState GYRO_i2cState = GYRO_i2cStateIdle;
GYRO_I2cReadRegState GYRO_i2cReadRegState = GYRO_i2cReadRegStateIdle;

// Tx vars:
volatile static uint16_t GYRO_txSize = 0;
volatile static uint16_t GYRO_txferedLen = 0;

// Rx vars:
volatile static uint16_t GYRO_rxSize = 0;
volatile static uint16_t GYRO_rxferedLen = 0;
static uint8_t* GYRO_rxDest = NULL;
static uint8_t GYRO_rxValid = 0;

// Read reg vars:
static uint8_t* GYRO_readRegDest = NULL;
static uint8_t GYRO_readRegSize = 0;
static uint8_t GYRO_readRegValid = 0;

void GYRO_Init(void) {
    DL_I2C_enableInterrupt(GYRO_I2C_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE |
        DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER |
        DL_I2C_INTERRUPT_CONTROLLER_RXFIFO_TRIGGER
    );
    NVIC_EnableIRQ(GYRO_I2C_IRQN);
}

static void GYRO_I2CTransmitITStart(uint16_t size);
static void GYRO_I2CReceiveITStart(uint16_t size);

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
    
    switch (GYRO_i2cReadRegState) {
        case GYRO_i2cReadRegStateWaitWriteAddr: {
            if (GYRO_i2cState == GYRO_i2cStateIdle) {
                GYRO_rxDest = GYRO_readRegDest;
                GYRO_i2cReadRegState = GYRO_i2cReadRegStateWaitRead;
                GYRO_I2CReceiveITStart(GYRO_readRegSize);
            }
            break;
        }
        case GYRO_i2cReadRegStateWaitRead: {
            if (GYRO_I2CReceiveValid()) {
                GYRO_readRegValid = 1;
                GYRO_readRegDest = NULL;
                GYRO_readRegSize = 0;
                GYRO_i2cReadRegState = GYRO_i2cReadRegStateIdle;
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
    if (GYRO_i2cReadRegState != GYRO_i2cReadRegStateIdle) {
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



void GYRO_WaitForI2CIdle(void) {
    while (GYRO_I2CIsBusy()) {
        GYRO_Update();
    }
}

void GYRO_I2CWriteReg(uint8_t addr, uint8_t* data, uint16_t size) {
    if (GYRO_I2CIsBusy()) {
        return;
    }
    
    GYRO_txBuf[0] = addr;
    memcpy(GYRO_txBuf + 1, data, size);
    GYRO_I2CTransmitITStart(size + 1);
}

void GYRO_I2CWriteRegBlocking(uint8_t addr, uint8_t* data, uint16_t size) {
    GYRO_WaitForI2CIdle();
    GYRO_I2CWriteReg(addr, data, size);
    GYRO_WaitForI2CIdle();
}

void GYRO_I2CReadReg(uint8_t addr, uint8_t* data, uint16_t size) {
    if (GYRO_I2CIsBusy()) {
        return;
    }
    
    GYRO_txBuf[0] = addr;
    
    GYRO_readRegDest = data;
    GYRO_readRegSize = size;
    // GYRO_readRegValid = 0;
    GYRO_i2cReadRegState = GYRO_i2cReadRegStateWaitWriteAddr;
    GYRO_I2CTransmitITStart(1);
}

void GYRO_I2CReadRegBlocking(uint8_t addr, uint8_t* data, uint16_t size) {
    GYRO_WaitForI2CIdle();
    GYRO_I2CReadReg(addr, data, size);
    GYRO_WaitForI2CIdle();
}

uint8_t GYRO_I2CReadRegValid(void) {
    uint8_t res = GYRO_readRegValid;
    GYRO_readRegValid = 0;
    return res;
}



static void GYRO_I2CWriteRegByteBlocking(uint8_t addr, uint8_t data) {
    uint8_t d = 0;
    d = data;
    GYRO_I2CWriteRegBlocking(addr, &d, 1);
}

void GYRO_InitSys(void) {
    // Reset
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_RA_PWR_MGMT_1, 0x80);
    delay_cycles(CPUCLK_FREQ / 10);

    // Power Management Registers
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_RA_PWR_MGMT_1, 0x00);

    // Set FSR
    GYRO_SetGyroFSR(3);
    GYRO_WaitForI2CIdle();

    GYRO_SetAccelFSR(0);
    GYRO_WaitForI2CIdle();

    // Set rate
    GYRO_SetRate(50);
    GYRO_WaitForI2CIdle();

    GYRO_SetLPF(25);
    GYRO_WaitForI2CIdle();

    // Disable all interrupts
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_INT_EN_REG, 0x00);
    // I2C Master Mode Off
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_USER_CTRL_REG, 0x00);
    // FIFO Close FIFO
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_FIFO_EN_REG, 0x00);
    // INT pin low level is effective
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_INTBP_CFG_REG, 0x80);

    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_RA_PWR_MGMT_1, 0x01);
    GYRO_I2CWriteRegByteBlocking(GYRO_MPU6050_RA_PWR_MGMT_2, 0x00);
    GYRO_SetRate(50);
    GYRO_WaitForI2CIdle();
}

void GYRO_SetGyroFSR(uint8_t fsr) {
    static uint8_t data = 0;
    data = fsr << 3;
    GYRO_I2CWriteReg(GYRO_MPU6050_GYRO_CFG_REG, &data, 1);
}

void GYRO_SetAccelFSR(uint8_t fsr) {
    static uint8_t data = 0;
    data = fsr << 3;
    GYRO_I2CWriteReg(GYRO_MPU6050_ACCEL_CFG_REG, &data, 1);
}

void GYRO_SetRate(uint16_t rate) {
    static uint8_t data = 0;
    if (rate > 1000) {
        rate = 1000;
    }
    if (rate < 4) {
        rate = 4;
    }
    data = 1000 / rate - 1;
    GYRO_I2CWriteReg(GYRO_MPU6050_SAMPLE_RATE_REG, &data, 1);
}

void GYRO_SetLPF(uint16_t freq) {
    static uint8_t data = 0;
    if (freq >= 188) {
        data = 1;
    } else if (freq >= 98) {
        data = 2;
    } else if (freq >= 42) {
        data = 3;
    } else if (freq >= 20) {
        data = 4;
    } else if (freq >= 10) {
        data = 5;
    } else {
        data = 6;
    }
    GYRO_I2CWriteReg(GYRO_MPU6050_CFG_REG, &data, 1);
}

static uint8_t* GYRO_gyroDataDest = NULL;

void GYRO_GetGyroData(uint8_t* data) {
    if (data == NULL) {
        return;
    }
    GYRO_gyroDataDest = data;
    GYRO_I2CReadReg(GYRO_MPU6050_GYRO_OUT, GYRO_gyroDataDest, 6);
}
