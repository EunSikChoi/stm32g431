/*
 * ap.c
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */





#include "ap.h"
#include "mode/cli_mode.h"
#include "mode/can_mode.h"


typedef enum
{
  MODE_IDEL,
  MODE_CLI,
  MODE_CAN,
} ap_mode_t;




ap_mode_t mode = MODE_IDEL;
ap_mode_t mode_next = MODE_IDEL;

static void apLedUpdate(void);
static bool apLoopIdle(void);
static void apGetModeNext(ap_mode_t *p_mode_next);

static mode_args_t mode_args;


void apInit(void)
{
  cliOpen(_DEF_UART1, 57600);  //CDC//FOR USB

  i2cOpen();

  mode_args.keepLoop = apLoopIdle; // define point func // keepLoop = apLoopIdle

}

void apMain(void)
{

  while(1)
  {
    switch(mode)
    {
      case MODE_CLI:
        cliModeMain(&mode_args);
        break;

      case MODE_CAN:
        canModeMain(&mode_args);
        break;

      default:
        apLoopIdle();
        break;
    }
  }// END While
}// END apMain

void apLedUpdate(void)
{

  static uint32_t pre_time = 0;

  if(millis()- pre_time >= 2000)
  {
    pre_time = millis();
    //ledToggle(_DEF_LED1);//RED
  }


  if (usbIsOpen() == true)
  {
      ledOn(_DEF_LED2);
  }
  else
  {
      ledOff(_DEF_LED2);//GREEN
  }

}

bool apLoopIdle(void)
{

  bool ret = true;
  apGetModeNext(&mode_next);

  if (mode == MODE_IDEL )
  {
    apLedUpdate();
  }

  if(mode != mode_next )
  {
    uartPrintf(_DEF_UART1, "\n Change bps: %d\n", uartGetBaud(_DEF_UART1));
    logPrintf("BPS Log : %d\n", uartGetBaud(_DEF_UART1));

    mode = mode_next;
    ret = false;

  }

  return ret;

}

void apGetModeNext(ap_mode_t *p_mode_next)
{

 if( uartGetBaud(_DEF_UART1) == 1400 )
 {
   *p_mode_next = MODE_CAN;
 }
 else
 {
   *p_mode_next = MODE_CLI;
 }

 if (usbIsOpen() != true)
 {
   *p_mode_next = MODE_IDEL;
   //ledOff(_DEF_LED1); //RED
 }


}



