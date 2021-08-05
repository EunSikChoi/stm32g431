/*
 * ap.c
 *
 *  Created on: Jul 27, 2021
 *      Author: 82109
 */





#include "ap.h"


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
static void apModeCli(void);
static void apModeCan(void);


void apInit(void)
{
  cliOpen(_DEF_UART1, 57600);  //CDC//FOR USB

}

void apMain(void)
{

  while(1)
  {

    apLoopIdle();

    switch(mode)
    {
    case MODE_CLI:
      apModeCli();
      break;

    case MODE_CAN:
      apModeCan();
      break;

    default:
      break;
    }
  }// END While
}// END apMain

void apLedUpdate(void)
{

  uint32_t pre_time;
  pre_time = millis();

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

}

bool apLoopIdle(void)
{

  bool ret = true;
  apGetModeNext(&mode_next);

  apLedUpdate();

  if(mode != mode_next )
  {
    uartPrintf(_DEF_UART1, "\n Change bps: %d\n", uartGetBaud(_DEF_UART1));
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

}



void apModeCli(void)
{
#ifdef _USE_HW_CLI
  cliMain();
#endif
}

void apModeCan(void)
{
  if (uartAvailable(_DEF_UART1) > 0)
  {
    uartPrintf(_DEF_UART1, "RX : 0x%X\n", uartRead(_DEF_UART1));
  }
}

