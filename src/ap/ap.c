/*
 * ap.c
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */





#include "ap.h"


void apInit(void)
{

}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();

  while(1)
  {


    if(millis()- pre_time >= 1100)
    {
      pre_time = millis();
     // ledToggle(0);
      ledToggle(1);
    }


    if (uartIsopen() == true)
    {
        ledOn(0);
    }
    else
    {
        ledOff(0);
    }

    if(uartAvailable(_DEF_UART1) > 0)
    {
      uint8_t rx_data;
      rx_data = uartRead(_DEF_UART1);

      uartWrite(_DEF_UART1, "RxData : " , 10);
      uartWrite(_DEF_UART1, &rx_data, 1);
      uartWrite(_DEF_UART1, "\n", 2);
    }



  }
}

