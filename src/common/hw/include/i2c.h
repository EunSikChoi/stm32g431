/*
 * i2c.h
 *
 *  Created on: 2021. 9. 23.
 *      Author: 82109
 */

#ifndef SRC_COMMON_HW_INCLUDE_I2C_H_
#define SRC_COMMON_HW_INCLUDE_I2C_H_

#include "hw_def.h"

bool i2cOpen(void);

I2C_HandleTypeDef hi2c1;


#endif /* SRC_COMMON_HW_INCLUDE_I2C_H_ */
