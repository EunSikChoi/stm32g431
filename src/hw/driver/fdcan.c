/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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

/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"
#include "log.h"
#include "cli.h"
#include "qbuffer.h"

/* USER CODE BEGIN 0 */
uint8_t ubKeyNumber = 0x0;
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];

static volatile uint32_t err_int_cnt = 0;

static void canErrUpdate(uint8_t ch);

FDCAN_HandleTypeDef hfdcan1;

typedef struct
{
  uint32_t prescalse;
  uint32_t sjw;
  uint32_t tseg1;
  uint32_t tseg2;
}can_baud_cfg_t ;




can_baud_cfg_t can_baud_cfg_80m_normal[] =
{
    {50, 8, 13, 2}, // 100K, 87.5%
    {40, 8, 13, 2}, // 125K, 87.5%
    {20, 8, 13, 2}, // 250K, 87.5%
    {10, 8, 13, 2}, // 500K, 87.5%
    {5,  8, 13, 2}, // 1M,   87.5%
};

can_baud_cfg_t can_baud_cfg_80m_data[] =
{
    {40, 8, 11, 8}, // 100K, 60%
    {32, 8, 11, 8}, // 125K, 60%
    {16, 8, 11, 8}, // 250K, 60%
    {8,  8, 11, 8}, // 500K, 60%
    {4,  8, 11, 8}, // 1M,   60%
    {2,  8, 11, 8}, // 2M    60%
    {1,  8, 11, 8}, // 4M    60%
    {1,  8,  9, 6}, // 5M    62.5%
};


const uint32_t dlc_len_tbl[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

const uint32_t dlc_tbl[] =
    {
        FDCAN_DLC_BYTES_0,
        FDCAN_DLC_BYTES_1,
        FDCAN_DLC_BYTES_2,
        FDCAN_DLC_BYTES_3,
        FDCAN_DLC_BYTES_4,
        FDCAN_DLC_BYTES_5,
        FDCAN_DLC_BYTES_6,
        FDCAN_DLC_BYTES_7,
        FDCAN_DLC_BYTES_8,
        FDCAN_DLC_BYTES_12,
        FDCAN_DLC_BYTES_16,
        FDCAN_DLC_BYTES_20,
        FDCAN_DLC_BYTES_24,
        FDCAN_DLC_BYTES_32,
        FDCAN_DLC_BYTES_48,
        FDCAN_DLC_BYTES_64
    };

can_baud_cfg_t *p_can_baud = can_baud_cfg_80m_normal;
can_baud_cfg_t *p_can_data = can_baud_cfg_80m_data;

uint32_t frame_tbl[] =
{
    FDCAN_FRAME_CLASSIC,
    FDCAN_FRAME_FD_NO_BRS,
    FDCAN_FRAME_FD_BRS
};

uint32_t mode_tbl[] =
{
    FDCAN_MODE_NORMAL,
    FDCAN_MODE_BUS_MONITORING,
    FDCAN_MODE_INTERNAL_LOOPBACK
};

uint32_t filter_tbl[] =
{
    FDCAN_FILTER_RANGE,
    FDCAN_FILTER_DUAL,
    FDCAN_FILTER_MASK,
    FDCAN_FILTER_RANGE_NO_EIDM
};


typedef struct
{
  bool is_init;
  bool is_open;

  uint32_t err_code;
  uint8_t  state;
  uint32_t recovery_cnt;

  uint32_t q_rx_full_cnt;
  uint32_t q_tx_full_cnt;
  uint32_t fifo_full_cnt;
  uint32_t fifo_lost_cnt;

  uint32_t fifo_idx;
  uint32_t enable_int;
  can_mode_t  mode;
  can_frame_t frame;
  can_baud_t  baud;
  can_baud_t  baud_data;

  FDCAN_HandleTypeDef  hfdcan1;
 // bool (*handler)(can_msg_t *arg);

  qbuffer_t q_msg;
  can_msg_t can_msg[CAN_MSG_RX_BUF_MAX];
} can_tbl_t;


static can_tbl_t can_tbl[CAN_MAX_CH];

#ifdef _USE_HW_CLI
static void cliCan(cli_args_t *args);
#endif


bool canInit(void)
{
  bool ret = true;

  uint8_t i;

  for(i = 0; i < CAN_MAX_CH; i++)
  {
    can_tbl[i].is_init  = true;
    can_tbl[i].is_open  = false;
    can_tbl[i].err_code = CAN_ERR_NONE;
    can_tbl[i].state    = 0;
    can_tbl[i].recovery_cnt = 0;

    can_tbl[i].q_rx_full_cnt = 0;
    can_tbl[i].q_tx_full_cnt = 0;
    can_tbl[i].fifo_full_cnt = 0;
    can_tbl[i].fifo_lost_cnt = 0;

    qbufferCreateBySize(&can_tbl[i].q_msg, (uint8_t *)&can_tbl[i].can_msg[0], sizeof(can_msg_t), CAN_MSG_RX_BUF_MAX);

    // msg block Receive //
    // 1_msg = 1_CAN protocol //
  }

#ifdef _USE_HW_CLI
  cliAdd("can", cliCan);
#endif
  return ret;
}



bool canOpen(uint8_t ch, can_mode_t mode, can_frame_t frame,  can_baud_t baud, can_baud_t baud_data )
{
  bool ret =  true;

  hfdcan1.Instance          = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat  = frame_tbl[frame];
  hfdcan1.Init.Mode         = mode_tbl[mode];

  hfdcan1.Init.AutoRetransmission   = ENABLE;
  hfdcan1.Init.TransmitPause        = ENABLE;
  hfdcan1.Init.ProtocolException    = ENABLE;

  hfdcan1.Init.NominalPrescaler     = p_can_baud[baud].prescalse;
  hfdcan1.Init.NominalSyncJumpWidth = p_can_baud[baud].sjw;
  hfdcan1.Init.NominalTimeSeg1      = p_can_baud[baud].tseg1;
  hfdcan1.Init.NominalTimeSeg2      = p_can_baud[baud].tseg2;


  hfdcan1.Init.DataPrescaler        = p_can_data[baud_data].prescalse;
  hfdcan1.Init.DataSyncJumpWidth    = p_can_data[baud_data].sjw;
  hfdcan1.Init.DataTimeSeg1         = p_can_data[baud_data].tseg1;
  hfdcan1.Init.DataTimeSeg2         = p_can_data[baud_data].tseg2;

  hfdcan1.Init.StdFiltersNbr = 28;
  hfdcan1.Init.ExtFiltersNbr = 8;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;


  can_tbl[ch].mode                  = mode;
  can_tbl[ch].frame                 = frame;
  can_tbl[ch].baud                  = baud;
  can_tbl[ch].baud_data             = baud_data;
  can_tbl[ch].fifo_idx              = FDCAN_RX_FIFO0;
  can_tbl[ch].enable_int            = FDCAN_IT_LIST_RX_FIFO0 |
                                      FDCAN_IT_BUS_OFF       |
                                      FDCAN_IT_ERROR_PASSIVE |
                                      FDCAN_IT_ERROR_WARNING;
                                     // Error CallBack INT Enable //
//FDCAN_IT_LIST_RX_FIFO0
  can_tbl[ch].is_open = true;


  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    ret = false;
  }

  return ret;
}




bool canConfigFilter(uint8_t ch, uint8_t index, can_id_type_t id_type, can_filter_type_t filter_type, uint32_t id, uint32_t id_mask)
{
  bool ret = true;

  FDCAN_FilterTypeDef sFilterConfig;

  if (id_type == CAN_STD)
  {
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
  }
  else
  {
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  }

  sFilterConfig.FilterIndex   = index;  // Filter index 0-27 filter element //

  sFilterConfig.FilterType    = filter_tbl[filter_type];// Set Filter Type


  sFilterConfig.FilterConfig  = FDCAN_FILTER_TO_RXFIFO0;

  sFilterConfig.FilterID1     = id;
  sFilterConfig.FilterID2     = id_mask;


  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }


  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }

  //if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, can_tbl[ch].enable_int, 0) != HAL_OK)  // enable INT Set //
  {
    Error_Handler();
  }

  return ret;
}

uint32_t canMsgAvailable(uint8_t ch)
{
  if(ch > CAN_MAX_CH) return 0;

  return qbufferAvailable(&can_tbl[ch].q_msg);
}

bool canMsgInit(can_msg_t *p_msg, can_frame_t frame, can_id_type_t  id_type, can_dlc_t dlc)
{
  p_msg->frame   = frame;
  p_msg->id_type = id_type;
  p_msg->dlc     = dlc;
  p_msg->length  = dlc_len_tbl[dlc];
  return true;
}

bool canMsgWrite(uint8_t ch, can_msg_t *p_msg, uint32_t timeout)
{
  //FDCAN_HandleTypeDef  *p_can;
  uint32_t pre_time;
  bool ret = true;


  if(ch > CAN_MAX_CH) return false;

  if (can_tbl[ch].err_code & CAN_ERR_BUS_OFF) return false;


 //p_can = &can_tbl[ch].hfdcan1;

  switch(p_msg->id_type)
  {
    case CAN_STD :
      TxHeader.IdType = FDCAN_STANDARD_ID;
      break;

    case CAN_EXT :
      TxHeader.IdType = FDCAN_EXTENDED_ID;
      break;
  }

  switch(p_msg->frame)
  {
    case CAN_CLASSIC:
      TxHeader.FDFormat      = FDCAN_CLASSIC_CAN;
      TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
      break;

    case CAN_FD_NO_BRS:
      TxHeader.FDFormat      = FDCAN_FD_CAN;
      TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
      break;

    case CAN_FD_BRS:
      TxHeader.FDFormat      = FDCAN_FD_CAN;
      TxHeader.BitRateSwitch = FDCAN_BRS_ON;
      break;
  }

  TxHeader.Identifier          = p_msg->id;
  TxHeader.MessageMarker       = 0;
  TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.TxFrameType         = FDCAN_DATA_FRAME;
  TxHeader.DataLength          = dlc_tbl[p_msg->dlc];

  pre_time = millis();

  if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, p_msg->data) == HAL_OK)  //&hfdcan1
  {
    /* Wait transmission complete */
    while(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0)
    {
      if (millis()-pre_time >= timeout)
      {
        ret = false;
        break;
      }
    }
  }
  else
  {
    ret = false;
  }

  return ret;
}


bool canMsgRead(uint8_t ch, can_msg_t *p_msg)
{
  bool ret = true;


  if(ch > CAN_MAX_CH) return 0;

  ret = qbufferRead(&can_tbl[ch].q_msg, (uint8_t *)p_msg, 1);


  return ret;
}

uint16_t canGetRxErrCount(uint8_t ch) // For REC Error Counter
{
  uint16_t ret = 0;
  HAL_StatusTypeDef status;
  FDCAN_ErrorCountersTypeDef error_counters;

  if(ch > CAN_MAX_CH) return 0;

  status = HAL_FDCAN_GetErrorCounters(&hfdcan1, &error_counters);
  if (status == HAL_OK)
  {
    ret = error_counters.RxErrorCnt;
  }

  return ret;
}

uint16_t canGetTxErrCount(uint8_t ch) // For TEC Error Counter
{
  uint16_t ret = 0;
  HAL_StatusTypeDef status;
  FDCAN_ErrorCountersTypeDef error_counters;

  if(ch > CAN_MAX_CH) return 0;

  status = HAL_FDCAN_GetErrorCounters(&hfdcan1, &error_counters);
  if (status == HAL_OK)
  {
    ret = error_counters.TxErrorCnt;
  }

  return ret;
}


void canErrPrint(uint8_t ch) // Error Print//
{
  uint32_t err_code;


  if(ch > CAN_MAX_CH) return;

  err_code = can_tbl[ch].err_code;

  if (err_code & CAN_ERR_PASSIVE) logPrintf("  ERR : CAN_ERR_PASSIVE\n");
  if (err_code & CAN_ERR_WARNING) logPrintf("  ERR : CAN_ERR_WARNING\n");
  if (err_code & CAN_ERR_BUS_OFF) logPrintf("  ERR : CAN_ERR_BUS_OFF\n");
}



//For #define FDCAN_ERROR_MASK (FDCAN_IR_ELO | FDCAN_IR_WDI | FDCAN_IR_PEA | FDCAN_IR_PED | FDCAN_IR_ARA)
// Set Enable Err INT //
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) // update error code status //
{
  uint8_t ch = _DEF_CAN1;

  err_int_cnt++;

  canErrUpdate(ch);

}


// for #define FDCAN_ERROR_STATUS_MASK (FDCAN_IR_EP | FDCAN_IR_EW | FDCAN_IR_BO)
// EP = Error Passitive , EW = Error Warning , BO = Bus Off Err //
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
  uint8_t ch = _DEF_CAN1;

  err_int_cnt++;

  canErrUpdate(ch);
}


void canErrUpdate(uint8_t ch)
{
  FDCAN_ProtocolStatusTypeDef protocol_status;

  HAL_FDCAN_GetProtocolStatus(&hfdcan1, &protocol_status);

  if (protocol_status.ErrorPassive)
  {
    can_tbl[ch].err_code |= CAN_ERR_PASSIVE;
  }
  else
  {
    can_tbl[ch].err_code &= ~CAN_ERR_PASSIVE;
  }

  if (protocol_status.Warning)
  {
    can_tbl[ch].err_code |= CAN_ERR_WARNING;
  }
  else
  {
    can_tbl[ch].err_code &= ~CAN_ERR_WARNING;
  }

  if (protocol_status.BusOff)
  {
    can_tbl[ch].err_code |= CAN_ERR_BUS_OFF;
  }
  else
  {
    can_tbl[ch].err_code &= ~CAN_ERR_BUS_OFF;
  }
}



void canRecovery(uint8_t ch)
{
  if(ch > CAN_MAX_CH) return;

  HAL_FDCAN_Stop(&hfdcan1);
  HAL_FDCAN_Start(&hfdcan1);

  can_tbl[ch].recovery_cnt++;
}

bool canUpdate(void)
{
  bool ret = false;
  can_tbl_t *p_can;

  for (int i=0; i<CAN_MAX_CH; i++)
  {
    p_can = &can_tbl[i];

    //canErrUpdate(i); // check err if callback miss

    switch(p_can->state)
    {
      case 0:
        if (p_can->err_code > 0) //if (p_can->err_code & CAN_ERR_BUS_OFF)
        {
          canRecovery(i);
          p_can->state = 1;
          ret = true;
        }
        break;

      case 1:
        if (p_can->err_code > 0 ) //if ((p_can->err_code & CAN_ERR_BUS_OFF) == 0)
        {
          p_can->state = 0;
        }
        break;
    }
  }

  return ret;
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{

  can_msg_t *rx_buf;
  rx_buf  = (can_msg_t *)qbufferPeekWrite(&can_tbl[_DEF_CAN1].q_msg);  // return node address
                                                                       // q_buf 는 msg_data를 가리키기 떄문에, 데이터형  msg로 msg 크기만큼 데이터 선언
                                                                       // 즉 q_buf 주소에 msg 타입으로 데이터를 저장
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    /* Retrieve Rx messages from RX FIFO0 */

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, rx_buf->data) == HAL_OK)// Data copy //
    {
      if(RxHeader.IdType == FDCAN_STANDARD_ID)
      {
        rx_buf->id      = RxHeader.Identifier;
        rx_buf->id_type = CAN_STD;
      }
      else
      {
        rx_buf->id      = RxHeader.Identifier;
        rx_buf->id_type = CAN_EXT;
      }

      rx_buf->length = dlc_len_tbl[(RxHeader.DataLength >> 16) & 0x0F];

      if (RxHeader.FDFormat == FDCAN_FD_CAN)
      {
        if (RxHeader.BitRateSwitch == FDCAN_BRS_ON)
        {
          rx_buf->frame = CAN_FD_BRS;
        }
        else
        {
          rx_buf->frame = CAN_FD_NO_BRS;
        }
      }
      else
      {
        rx_buf->frame = CAN_CLASSIC;
      }

      if (qbufferWrite(&can_tbl[_DEF_CAN1].q_msg, NULL, 1) != true) // counter +1 for qbufferWrite -> in++ // data not copy only in++ counter//
      {
        can_tbl[_DEF_CAN1].q_rx_full_cnt++; // when full rx buffer //
      }
    }//END GET MSG
  }//END RX CallBack

  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != RESET)
  {
    can_tbl[_DEF_CAN1].fifo_full_cnt++;
  }

  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
  {
    can_tbl[_DEF_CAN1].fifo_lost_cnt++;
  }

}



#ifdef _USE_HW_CLI
void cliCan(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    for (int i=0; i<CAN_MAX_CH; i++)
    {

      cliPrintf(" \n");

      switch(can_tbl[i].is_open )
      {
        case 0 :
          cliPrintf("Ch OPEN : False\n");
          break;
        case 1 :
          cliPrintf("Ch OPEN : OK\n");
          break;
      }

      switch(can_tbl[i].mode )
      {
        case CAN_NORMAL :
          cliPrintf("  Mode  : CAN_NORMAL\n");
          break;
        case CAN_MONITOR :
          cliPrintf("  Mode  : CAN_MONITOR\n");
          break;
        case CAN_LOOPBACK :
          cliPrintf("  Mode  : CAN_LOOPBACK\n");
          break;
      }

      switch(can_tbl[i].frame)
      {
        case CAN_CLASSIC :
          cliPrintf("  Frame : CAN_CLASSIC\n");
          break;
        case CAN_FD_NO_BRS :
          cliPrintf("  Frame : CAN_FD_NO_BRS\n");
          break;
        case CAN_FD_BRS :
          cliPrintf("  Frame : CAN_FD_BRS\n");
          break;
      }

      switch(can_tbl[i].baud)
      {
        case CAN_100K:
          cliPrintf("  Baud   : CAN_100K\n");
          break;
        case CAN_125K:
          cliPrintf("  Baud   : CAN_125K\n");
          break;
        case CAN_250K:
          cliPrintf("  Baud   : CAN_250K\n");
          break;
        case CAN_500K:
          cliPrintf("  Baud   : CAN_500K\n");
          break;
        case CAN_1M:
          cliPrintf("  Baud  : CAN_1M\n");
          break;
        case CAN_2M:
          cliPrintf("  Baud  : CAN_2M\n");
          break;
        case CAN_4M:
          cliPrintf("  Baud  : CAN_4M\n");
          break;
        case CAN_5M:
          cliPrintf("  Baud  : CAN_5M\n");
          break;
      }

      // if change Config, Change Cli info printf //
      cliPrintf("Filter  : CAN_STD, CAN_FILTER_RANGE, 0x0430, 0x433\n");
      cliPrintf("Filter  : CAN_EXT, CAN_FILTER_RANGE, 0x0141, 0x143\n");

    }
    ret = true;
  }


  if (args->argc == 1 && args->isStr(0, "send"))
  {

    uint32_t pre_time;
    uint32_t index = 0;
    can_msg_t msg;
    uint32_t err_code;


    while(cliKeepLoop())
    {

      if (millis()-pre_time >= 1000)
      {
        pre_time = millis();

        msg.frame   = CAN_CLASSIC;
        msg.id_type = CAN_EXT;
        msg.dlc     = CAN_DLC_2;
        msg.id      = 0x314;
        msg.length  = 2;
        msg.data[0] = 0x17;
        msg.data[1] = 0x21;

        // Transfer CAN //
        if (canMsgWrite(_DEF_CAN1, &msg, 10) > 0)
        {
          index %= 1000;
          cliPrintf("%03d(T) -> id ", index++);
          if (msg.id_type == CAN_STD)
          {
            cliPrintf("std ");
          }
          else
          {
            cliPrintf("ext ");
          }
          cliPrintf(": 0x%08X, L:%02d, ", msg.id, msg.length);
          for (int i=0; i<msg.length; i++)
          {
            cliPrintf("0x%02X ", msg.data[i]);
          }
          cliPrintf("\n");
        }

        //  Counter For TEC, REC //
        if (canGetRxErrCount(_DEF_CAN1) > 0 || canGetTxErrCount(_DEF_CAN1) > 0)
        {
          cliPrintf("ErrCnt : REC=%d,  TEC=%d\n", canGetRxErrCount(_DEF_CAN1), canGetTxErrCount(_DEF_CAN1));
        }

        if(err_int_cnt > 0)
        {

          cliPrintf("CallBack Err Cnt : %d\n", err_int_cnt);
          err_int_cnt = 0; // reset int cnt //
        }

        // Print Err Log //
        if (can_tbl[_DEF_CAN1].err_code != err_code)
        {
          cliPrintf("ErrCode : 0x%X\n", can_tbl[_DEF_CAN1].err_code);
          canErrPrint(_DEF_CAN1);
          err_code = can_tbl[_DEF_CAN1].err_code;
        }

        // Receive CAN //
        if (canMsgAvailable(_DEF_CAN1))
        {
          canMsgRead(_DEF_CAN1, &msg);

          index %= 1000;
          cliPrintf("%03d(R) <- id ", index++);
          if (msg.id_type == CAN_STD)
          {
            cliPrintf("std ");
          }
          else
          {
            cliPrintf("ext ");
          }
          cliPrintf(": 0x%08X, L:%02d, ", msg.id, msg.length);
          for (int i=0; i<msg.length; i++)
          {
            cliPrintf("0x%02X ", msg.data[i]);
          }
          cliPrintf("\n");
        }

      }// END PRE TIME

    }// END KEEP LOOP
    ret = true;
  }// END SEND

  if (args->argc == 1 && args->isStr(0, "reset"))
  {

    canErrPrint(_DEF_CAN1);

    // Can Reset //
    if (canUpdate())
    {
      cliPrintf("BusOff Recovery\n");
    }

    canErrPrint(_DEF_CAN1);

    cliPrintf("RESET OK\n");

    ret = true;

  }// END RESET

  if (args->argc == 1 && args->isStr(0, "Errcnt"))
  {
    for (int i=0; i<CAN_MAX_CH; i++)
    {
      cliPrintf("  q_rx_full_cnt : %d\n", can_tbl[i].q_rx_full_cnt);
      cliPrintf("  q_tx_full_cnt : %d\n", can_tbl[i].q_tx_full_cnt);
      cliPrintf("  fifo_full_cnt : %d\n", can_tbl[i].fifo_full_cnt);
      cliPrintf("  fifo_lost_cnt : %d\n", can_tbl[i].fifo_lost_cnt);
      canErrPrint(i);
    }
    ret = true;
  }


  if (ret == false)
  {
    cliPrintf("can info\n");
    cliPrintf("can send\n");
    cliPrintf("can reset\n");
    cliPrintf("can Errcnt\n");
  }
}// END CLI
#endif


void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */
    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PB8-BOOT0     ------> FDCAN1_RX
    PB9     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PB8-BOOT0     ------> FDCAN1_RX
    PB9     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
