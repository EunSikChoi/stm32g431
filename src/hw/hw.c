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

  MX_FDCAN1_Init();

  if( resetGetCount() == 2)
  {
    //jump to system bootloader //  시스템 부트 로더 실행 //
    resetToSysBoot();
  }

  usbInit();
  usbBegin(USB_CDC_MODE);

  uartInit();
  uartOpen(_DEF_UART1, 57600);

}
