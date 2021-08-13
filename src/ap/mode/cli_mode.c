/*
 * cli_mode.c
 *
 *  Created on: 2021. 8. 5.
 *      Author: 82109
 */


#include "cli_mode.h"
#include "fdcan.h"



FDCAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
extern uint8_t TxData[8];
extern FDCAN_HandleTypeDef hfdcan1;


bool cliModeInit(void)
{
  return true;
}


 void cliModeMain(mode_args_t *args)
 {
   uint32_t index = 0;
   can_msg_t msg;
   uint32_t pre_time = millis();

   logPrintf("cliMode in\n");

   while(args->keepLoop())
   {
     if(millis() - pre_time >= 1000)
     {
       pre_time = millis();
       ledToggle(_DEF_LED1);


       // set txheader //
       msg.frame   = CAN_CLASSIC;
       msg.id_type = CAN_STD;
       msg.dlc     = CAN_DLC_2;
       msg.id      = 0x119;
       msg.length  = 2;
       msg.data[0] = 0x04;
       msg.data[1] = 0x30;

       // send data //
       if (canMsgWrite(_DEF_CAN1, &msg, 10) > 0)
       {
         index %= 1000;
         cliPrintf("%03d(T) -> id ", index++);
         if (msg.id_type == CAN_STD)
         {
           cliPrintf("std ");
         }
         else
         {
           cliPrintf("ext ");
         }
         cliPrintf(": 0x%08X, L:%02d, ", msg.id, msg.length);
         for (int i=0; i<msg.length; i++)
         {
           cliPrintf("0x%02X ", msg.data[i]);
         }
         cliPrintf("\n");
       }

       // read data //
       if (canMsgAvailable(_DEF_CAN1))
       {
         canMsgRead(_DEF_CAN1, &msg);

         index %= 1000;
         cliPrintf("%03d(R) <- id ", index++);
         if (msg.id_type == CAN_STD)
         {
           cliPrintf("std ");
         }
         else
         {
           cliPrintf("ext ");
         }
         cliPrintf(": 0x%08X, L:%02d, ", msg.id, msg.length);
         for (int i=0; i<msg.length; i++)
         {
           cliPrintf("0x%02X ", msg.data[i]);
         }
         cliPrintf("\n");
       }
     }

     cliMain();

   }// END While

   logPrintf("cliMode out\n");

 }
