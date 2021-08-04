/*
 * ap.c
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */





#include "ap.h"


void apInit(void)
{
  cliOpen(_DEF_UART1, 57600);  //CDC//FOR USB

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


  #ifdef _USE_HW_CLI
    cliMain();
  #endif

  }
}

