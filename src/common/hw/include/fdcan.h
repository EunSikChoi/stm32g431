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


typedef enum
{
  CAN_DLC_0,
  CAN_DLC_1,
  CAN_DLC_2,
  CAN_DLC_3,
  CAN_DLC_4,
  CAN_DLC_5,
  CAN_DLC_6,
  CAN_DLC_7,
  CAN_DLC_8,
  CAN_DLC_12,
  CAN_DLC_16,
  CAN_DLC_20,
  CAN_DLC_24,
  CAN_DLC_32,
  CAN_DLC_48,
  CAN_DLC_64
} can_dlc_t;

typedef enum
{
  CAN_ERR_NONE      = 0x00000000,
  CAN_ERR_PASSIVE   = 0x00000001,
  CAN_ERR_WARNING   = 0x00000002,
  CAN_ERR_BUS_OFF   = 0x00000004,
  CAN_ERR_BUS_FAULT = 0x00000008,
} can_err_t;


typedef struct
{
  uint32_t id;
  uint16_t length;
  uint8_t  data[64];

  can_dlc_t      dlc;
  can_id_type_t  id_type;
  can_frame_t    frame;
} can_msg_t;

void FDCAN_Config(void);

bool canInit(void);
bool canOpen(uint8_t ch, can_mode_t mode, can_frame_t frame,  can_baud_t baud, can_baud_t baud_data );
bool canConfigFilter(uint8_t ch, uint8_t index, can_id_type_t id_type, uint32_t id, uint32_t id_mask);
uint32_t canMsgAvailable(uint8_t ch);
bool canMsgWrite(uint8_t ch, can_msg_t *p_msg, uint32_t timeout);
bool canMsgRead(uint8_t ch, can_msg_t *p_msg);
bool canMsgInit(can_msg_t *p_msg, can_frame_t frame, can_id_type_t  id_type, can_dlc_t dlc);



#endif


#ifdef __cplusplus
}
#endif



#endif /* __FDCAN_H__ */


