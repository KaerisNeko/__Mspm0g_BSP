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

void GYRO_WaitForI2CIdle(void);
void GYRO_I2CWriteReg(uint8_t addr, uint8_t* data, uint16_t size);
void GYRO_I2CWriteRegBlocking(uint8_t addr, uint8_t* data, uint16_t size);
void GYRO_I2CReadReg(uint8_t addr, uint8_t* data, uint16_t size);
void GYRO_I2CReadRegBlocking(uint8_t addr, uint8_t* data, uint16_t size);
uint8_t GYRO_I2CReadRegValid(void);



#define GYRO_MPU6050_INT_EN_REG             0x38
#define GYRO_MPU6050_USER_CTRL_REG          0x6A
#define GYRO_MPU6050_FIFO_EN_REG            0x23
#define GYRO_MPU6050_INTBP_CFG_REG          0x37

#define GYRO_MPU6050_GYRO_CFG_REG           0x1B
#define GYRO_MPU6050_ACCEL_CFG_REG          0x1C
#define GYRO_MPU6050_CFG_REG                0x1A
#define GYRO_MPU6050_SAMPLE_RATE_REG        0x19

#define GYRO_MPU6050_RA_PWR_MGMT_1          0x6B
#define GYRO_MPU6050_RA_PWR_MGMT_2          0x6C

#define GYRO_MPU6050_GYRO_OUT               0x43

void GYRO_InitSys(void);

void GYRO_SetGyroFSR(uint8_t fsr);
void GYRO_SetAccelFSR(uint8_t fsr);
void GYRO_SetRate(uint16_t rate);
void GYRO_SetLPF(uint16_t freq);

void GYRO_GetGyroData(uint8_t* data);

#endif
