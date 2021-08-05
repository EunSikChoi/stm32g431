/*
 * ap.h
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */

#ifndef SRC_AP_AP_H_
#define SRC_AP_AP_H_



#include "hw.h"


void apInit(void);
void apMain(void);

typedef struct
{
  bool (*keepLoop)(void);
} mode_args_t;


#endif /* SRC_AP_AP_H_ */
