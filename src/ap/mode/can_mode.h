/*
 * can_mode.h
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */

#ifndef SRC_AP_MODE_CAN_MODE_H_
#define SRC_AP_MODE_CAN_MODE_H_



#include "hw.h"
#include "ap.h"

bool canModeInit(void);
void canModeMain(mode_args_t *args);


#endif /* SRC_AP_MODE_CAN_MODE_H_ */
