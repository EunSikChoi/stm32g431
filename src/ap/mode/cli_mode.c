/*
 * cli_mode.c
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */


#include "cli_mode.h"



bool cliModeInit(void)
{
  return true;
}


 void cliModeMain(mode_args_t *args)
 {
   uint32_t pre_time = millis();

   logPrintf("cliMode in\n");

   while(args->keepLoop())
   {
     if(millis() - pre_time >= 500)
     {
       pre_time = millis();
       ledToggle(_DEF_LED1);
     }

     cliMain();

   }// END While

   logPrintf("cliMode out\n");

 }
