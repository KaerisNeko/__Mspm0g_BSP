#ifndef __GYRO_H__
#define __GYRO_H__

#include "ti_msp_dl_config.h"

#define GYRO_I2C_ADDR           0x68
#define GYRO_I2C_BUF_SIZE       256

#define GYRO_I2C_INST           I2C_GYRO_INST
#define GYRO_I2C_IRQ_HANDLER    I2C_GYRO_INST_IRQHandler
#define GYRO_I2C_IRQN           I2C_GYRO_INST_INT_IRQN

void GYRO_Init(void);
void GYRO_Update(void);

uint8_t GYRO_I2CIsBusy(void);
void GYRO_I2CWrite_IT(uint8_t* data, uint16_t size);
void GYRO_I2CRead_IT(uint8_t* data, uint16_t size);
uint8_t GYRO_I2CReceiveValid(void);

void GYRO_I2CWriteReg(uint8_t addr, uint8_t* data, uint16_t size);
void GYRO_I2CReadReg(uint8_t addr, uint8_t* data, uint16_t size);
uint8_t GYRO_I2CReadRegValid(void);

#endif
