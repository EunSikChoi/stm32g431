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

/* USER CODE BEGIN 0 */
uint8_t ubKeyNumber = 0x0;
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];



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




can_baud_cfg_t *p_can_baud = can_baud_cfg_80m_normal;
can_baud_cfg_t *p_can_data = can_baud_cfg_80m_data;


uint32_t mode_tbl[] =
{
    FDCAN_MODE_NORMAL,
    FDCAN_MODE_BUS_MONITORING,
    FDCAN_MODE_INTERNAL_LOOPBACK
};

uint32_t frame_tbl[] =
{
    FDCAN_FRAME_CLASSIC,
    FDCAN_FRAME_FD_NO_BRS,
    FDCAN_FRAME_FD_BRS
};



bool canOpen(uint8_t ch, can_mode_t mode, can_frame_t frame,  can_baud_t baud, can_baud_t data )
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


  hfdcan1.Init.DataPrescaler        = p_can_data[data].prescalse;
  hfdcan1.Init.DataSyncJumpWidth    = p_can_data[data].sjw;
  hfdcan1.Init.DataTimeSeg1         = p_can_data[data].tseg1;
  hfdcan1.Init.DataTimeSeg2         = p_can_data[data].tseg2;

  hfdcan1.Init.StdFiltersNbr = 28;
  hfdcan1.Init.ExtFiltersNbr = 8;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;


  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    ret = false;
  }

  return ret;
}


/* FDCAN1 init function */
//void MX_FDCAN1_Init(void)
//{
//
//  hfdcan1.Instance          = FDCAN1;
//  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
//  hfdcan1.Init.FrameFormat  = FDCAN_FRAME_CLASSIC;
//  hfdcan1.Init.Mode         = FDCAN_MODE_NORMAL;
//
//  hfdcan1.Init.AutoRetransmission   = ENABLE;
//  hfdcan1.Init.TransmitPause        = ENABLE;
//  hfdcan1.Init.ProtocolException    = ENABLE;
//
//  hfdcan1.Init.NominalPrescaler     = 5;
//  hfdcan1.Init.NominalSyncJumpWidth = 8;
//  hfdcan1.Init.NominalTimeSeg1      = 13;
//  hfdcan1.Init.NominalTimeSeg2      = 2;
//
//  hfdcan1.Init.DataPrescaler        = 4;
//  hfdcan1.Init.DataSyncJumpWidth    = 8;
//  hfdcan1.Init.DataTimeSeg1         = 11;
//  hfdcan1.Init.DataTimeSeg2         = 8;
//
//  hfdcan1.Init.StdFiltersNbr = 28;
//  hfdcan1.Init.ExtFiltersNbr = 8;
//  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
//
//
//  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//}


bool canConfigFilter(uint8_t ch, uint8_t index, can_id_type_t id_type, uint32_t id, uint32_t id_mask)
{
  FDCAN_FilterTypeDef sFilterConfig;

  /* Configure Rx filter */
  //sFilterConfig.IdType        = FDCAN_STANDARD_ID;
  if (id_type == CAN_STD)
  {
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
  }
  else
  {
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  }


  //sFilterConfig.FilterIndex   = 0;
  sFilterConfig.FilterIndex   = index;

  sFilterConfig.FilterType    = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig  = FDCAN_FILTER_TO_RXFIFO0;



  // sFilterConfig.FilterID1     = 0x319;
  // sFilterConfig.FilterID2     = 0x7FF;
  sFilterConfig.FilterID1     = id;
  sFilterConfig.FilterID2     = id_mask;




  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure global filter:
     Filter all remote frames with STD and EXT ID
     Reject non matching frames with STD ID and EXT ID */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start the FDCAN module */
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Prepare Tx Header */
  TxHeader.Identifier = 0x117;
  TxHeader.IdType = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader.DataLength = FDCAN_DLC_BYTES_3;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker = 0;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    /* Retrieve Rx messages from RX FIFO0 */
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
    {
    Error_Handler();
    }

    /* Display LEDx */
   // if ((RxHeader.Identifier == 0x314) && (RxHeader.IdType == FDCAN_STANDARD_ID) && (RxHeader.DataLength == FDCAN_DLC_BYTES_2))
  //  {
      //LED_Display(RxData[0]);
      ubKeyNumber = RxData[0];
      logPrintf("CAN RX : %x\n",ubKeyNumber);
   // }
  }
}


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
