/*
 * uart.c
 *
 *  Created on: 2021. 8. 4.
 *      Author: 82109
 */




#include "uart.h"
#include "cdc.h"



#ifdef _USE_HW_UART

#define _USE_UART1  //-  CDC
//#define _USE_UART2

typedef struct
{

  bool is_open;
  bool is_tx_done;
  bool is_tx_error;
  uint32_t baud;
  //qbuffer_t qbuffer;
  //UART_HandleTypeDef *p_huart;
  //DMA_HandleTypeDef  *p_hdma_tx;
}uart_tbl_t;

static uart_tbl_t uart_tbl[UART_MAX_CH];




bool   uartInit(void)
{
  for( int i =0; i < UART_MAX_CH ; i++)
  {
      uart_tbl[i].is_open     = false;
      uart_tbl[i].baud        = 57600;
      uart_tbl[i].is_tx_done  = true;
      uart_tbl[i].is_tx_error = false;
  }

  return true;

}
bool   uartOpen(uint8_t ch , uint32_t baud)
{
  bool ret = false;

  switch(ch)
  {
      case _DEF_UART1:
        uart_tbl[ch].is_open = true;
        uart_tbl[ch].baud = baud;
        ret = true;
       break;
  }

  return ret;

}

bool uartIsopen(void)
{
  return cdcIsConnect();
}

uint32_t uartAvailable(uint8_t ch)
{

  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
     ret = cdcAvailable();
    break;
  }

  return ret;

}

uint8_t uartRead(uint8_t ch)
{

  uint8_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
     ret = cdcRead();
    break;
  }

  return ret;

}
uint32_t uartWrite(uint8_t ch, uint8_t *p_data , uint32_t length)
{

  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
     ret = cdcWrite(p_data, length);
    break;
  }

  return ret;

}




uint32_t uartPrintf(uint8_t ch, char *fmt, ...)
{
  char buf[256];
  va_list args;
  int len;
  uint32_t ret;

  va_start(args, fmt);
  len = vsnprintf(buf, 256, fmt, args);
  ret = uartWrite(ch, (uint8_t *)buf, len);

  va_end(args);

  return ret;

}

uint32_t uartGetBaud(uint8_t ch)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
     ret = cdcGetBaud();
    break;
  }

  return ret;

}


#endif // USE_HW_UART
