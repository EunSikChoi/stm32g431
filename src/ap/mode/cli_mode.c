/*
 * cli_mode.c
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */


#include "cli_mode.h"
#include "i2c.h"


bool cliModeInit(void)
{
  return true;
}


//uint8_t aTxBuffer[] = "I2C_TwoBoards";
//uint8_t aRxBuffer[13];

 void cliModeMain(mode_args_t *args)
 {

   uint32_t pre_time = millis();

   logPrintf("cliMode in\n");

   while(args->keepLoop())
   {
     if(millis() - pre_time >= 1000)
     {
       pre_time = millis();
     //  ledToggle(_DEF_LED1);

      // HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)0x38, (uint8_t *)aTxBuffer, 13, 500);
      // HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(0x38 << 1), 0x92, I2C_MEMADD_SIZE_8BIT, aRxBuffer, 1, 100);


     }

     cliMain();

   }// END While

   logPrintf("cliMode out\n");

 }
