/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"
#include "fdcan.h"


#ifdef _USE_HW_CAN

#define CAN_MAX_CH            HW_CAN_MAX_CH
#define CAN_MSG_RX_BUF_MAX    HW_CAN_MSG_RX_BUF_MAX



typedef enum
{
  CAN_NORMAL,
  CAN_MONITOR,
  CAN_LOOPBACK
}can_mode_t;

typedef enum
{
  CAN_CLASSIC,
  CAN_FD_NO_BRS,
  CAN_FD_BRS
}can_frame_t;

typedef enum
{
  CAN_STD,
  CAN_EXT
} can_id_type_t;

typedef enum
{
  CAN_100K,
  CAN_125K,
  CAN_250K,
  CAN_500K,
  CAN_1M,
  CAN_2M,//
  CAN_4M,
  CAN_5M
} can_baud_t;

void FDCAN_Config(void);

bool canOpen(uint8_t ch, can_mode_t mode, can_frame_t frame,  can_baud_t baud, can_baud_t data );
bool canConfigFilter(uint8_t ch, uint8_t index, can_id_type_t id_type, uint32_t id, uint32_t id_mask);



#endif


#ifdef __cplusplus
}
#endif



#endif /* __FDCAN_H__ */


