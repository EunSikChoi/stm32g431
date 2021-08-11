/*
 * hw.c
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */




#include "hw.h"

void hwInit(void)
{

  bspInit();

  cliInit();

  rtcInit();
  resetInit();
  ledInit();

  if( resetGetCount() == 2)
  {
    //jump to system bootloader //  시스템 부트 로더 실행 //
    resetToSysBoot();
  }

  usbInit();
  usbBegin(USB_CDC_MODE);

  uartInit();
  uartOpen(_DEF_UART1, 57600);

 // MX_FDCAN1_Init();
  canOpen(_DEF_CAN1, CAN_NORMAL, CAN_CLASSIC, CAN_1M, CAN_2M );

  //FDCAN_Config();

  canConfigFilter(_DEF_CAN1, 0, CAN_STD, 0x0430, 0x7FF);
  canConfigFilter(_DEF_CAN1, 0, CAN_EXT, 0x0141, 0x7FF);
}
