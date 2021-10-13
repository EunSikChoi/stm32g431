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
  logInit();
  rtcInit();
  resetInit();
  ledInit();

  if( resetGetCount() == 2)
  {
    //jump to system bootloader //  시스템 부트 로더 실행 //
    resetToSysBoot();
  }

  logPrintf("[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);

  usbInit();
  usbBegin(USB_CDC_MODE);
  uartInit();
  uartOpen(_DEF_UART1, 57600);
  canInit();
  i2cInit();

  logBoot(false); // END Boot log //


 // MX_FDCAN1_Init();
  canOpen(_DEF_CAN1, CAN_NORMAL, CAN_CLASSIC, CAN_1M, CAN_2M );

  // if change Config, Change Cli info printf //

  //canConfigFilter(_DEF_CAN1, 25, CAN_STD, CAN_FILTER_CLASSIC, 0x0430, 0x7FF); // if CAN_STD : index 0 -27 //
  //canConfigFilter(_DEF_CAN1,  6, CAN_EXT, CAN_FILTER_CLASSIC, 0x0141, 0x7FF); // if CAN_EXT : index 0 -7  //

  canConfigFilter(_DEF_CAN1, 25, CAN_STD, CAN_FILTER_RANGE, 0x0430, 0x433); // if CAN_STD : index 0 -27 //
  canConfigFilter(_DEF_CAN1,  6, CAN_EXT, CAN_FILTER_RANGE, 0x0141, 0x143); // if CAN_EXT : index 0 -7  //


}
