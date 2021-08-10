/*
 * cli_mode.c
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */


#include "cli_mode.h"



extern uint8_t ubKeyNumber;
FDCAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
extern uint8_t TxData[8];


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
     if(millis() - pre_time >= 1000)
     {
       pre_time = millis();
       ledToggle(_DEF_LED1);

       /* Set the data to be transmitted */
       TxData[0] = ubKeyNumber;
       TxData[1] = 0xAD;

       /* Start the Transmission process */
       if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK)
       {
         /* Transmission request Error */
         Error_Handler();
       }

       HAL_Delay(10);




     }

     cliMain();

   }// END While

   logPrintf("cliMode out\n");

 }
