/*
 * can_mode.c
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */



#include "can_mode.h"

bool canModeInit(void)
{
  return true;
}

void canModeMain(mode_args_t *args)
{
  uint32_t pre_time = millis();

  logPrintf("canMode in\n");

  while(args->keepLoop())
  {
    if(millis() - pre_time >= 100)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }

    if (uartAvailable(_DEF_UART1) > 0)
    {
      uartPrintf(_DEF_UART1, "RX : 0x%X\n", uartRead(_DEF_UART1));
    }

  }// END While

  logPrintf("canMode out\n");

}
