/*
 * i2c.h
 *
 *  Created on: 2021. 9. 23.
 *      Author: 82109
 */

#ifndef SRC_COMMON_HW_INCLUDE_I2C_H_
#define SRC_COMMON_HW_INCLUDE_I2C_H_

#include "hw_def.h"

#define I2C_MAX_CH       HW_I2C_MAX_CH

bool i2cOpen(void);
bool i2cInit(void);
bool i2cIsDeviceReady(uint8_t ch, uint8_t dev_addr);
bool i2cReadByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout);
bool i2cReadBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout);
bool i2cWriteByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout);
bool i2cWriteBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout);

I2C_HandleTypeDef hi2c1;


#endif /* SRC_COMMON_HW_INCLUDE_I2C_H_ */
