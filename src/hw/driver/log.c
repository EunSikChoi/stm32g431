/*
 * log.c
 *
 *  Created on: 2021. 8. 4.
 *      Author: 82109
 */




#include "log.h"
#include "uart.h"
#ifdef _USE_HW_CLI
#include "cli.h"
#include "i2c.h"
#include "led.h"
#endif


#ifdef _USE_HW_LOG


typedef struct
{
  uint16_t line_index;
  uint16_t buf_length;
  uint16_t buf_length_max;
  uint16_t buf_index;
  uint8_t *buf;
} log_buf_t;


log_buf_t log_buf_boot;
log_buf_t log_buf_list;

static uint8_t buf_boot[LOG_BOOT_BUF_MAX];
static uint8_t buf_list[LOG_LIST_BUF_MAX];
static uint8_t buf_flash[1024];

static bool is_init = false;
static bool is_boot_log = true;
static bool is_enable = true;
static bool is_open = false;

static uint8_t  log_ch = LOG_CH;
static uint32_t log_baud = 57600;

static char print_buf[256];

#ifdef _USE_HW_ROTS
static osMutexId mutex_lock;
#endif



#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif





bool logInit(void)
{
#ifdef _USE_HW_ROTS
  osMutexDef(mutex_lock);
  mutex_lock = osMutexCreate (osMutex(mutex_lock));
#endif

  log_buf_boot.line_index     = 0;
  log_buf_boot.buf_length     = 0;
  log_buf_boot.buf_length_max = LOG_BOOT_BUF_MAX;
  log_buf_boot.buf_index      = 0;
  log_buf_boot.buf            = buf_boot;


  log_buf_list.line_index     = 0;
  log_buf_list.buf_length     = 0;
  log_buf_list.buf_length_max = LOG_LIST_BUF_MAX;
  log_buf_list.buf_index      = 0;
  log_buf_list.buf            = buf_list;


  is_init = true;

#ifdef _USE_HW_CLI
  cliAdd("log", cliCmd);
#endif

  return true;
}

void logEnable(void)
{
  is_enable = true;
}

void logDisable(void)
{
  is_enable = false;
}

void logBoot(uint8_t enable)
{
  is_boot_log = enable;
}

bool logOpen(uint8_t ch, uint32_t baud)
{
  log_ch   = ch;
  log_baud = baud;
  is_open  = true;
  return uartOpen(ch, baud);
}


bool logtoi2cWrite(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
   return i2cWriteByte(ch, dev_addr,  reg_addr, data, timeout);

}

bool logtoi2cWrites(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
    return i2cWriteBytes( ch, dev_addr, reg_addr, p_data,  length, timeout);
}



bool logtoi2cRead(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
   return i2cReadByte(ch,  dev_addr,  reg_addr, p_data, timeout);
}

bool logBufPrintf(log_buf_t *p_log, char *p_data, uint32_t length)
{
  uint32_t buf_last;
  uint8_t *p_buf;
  int buf_len;


  buf_last = p_log->buf_index + length + 6;
  if (buf_last > p_log->buf_length_max)
  {
    p_log->buf_index = 0;
  }

  p_buf = &p_log->buf[p_log->buf_index];// set pointer add //

  // +6 mean  %04x = 0001,0002 4Byte , /t(tap) 1Byte , /n 1Byte Total 6Byte add //
  buf_len = snprintf((char *)p_buf, length + 6, "%04X\t%s", p_log->line_index, p_data);
  p_log->line_index++;
  p_log->buf_index += buf_len;


  if (buf_len + p_log->buf_length <= p_log->buf_length_max)
  {
    p_log->buf_length += buf_len;
  }

  return true;
}

void logPrintf(const char *fmt, ...)
{
#ifdef _USE_HW_ROTS
  osMutexWait(mutex_lock, osWaitForever);
#endif

  va_list args;
  int len;

  if (is_init != true) return;


  va_start(args, fmt);
  len = vsnprintf(print_buf, 256, fmt, args);

  if (is_open == true && is_enable == true)
  {
    uartWrite(log_ch, (uint8_t *)print_buf, len); // 나중에 다른 통신 채널로 확인하고자 할때//
  }

  if (is_boot_log)
  {
    logBufPrintf(&log_buf_boot, print_buf, len); // for Write to logBuf(&node)// USE Only before Main()
  }
  logBufPrintf(&log_buf_list, print_buf, len); // for Write to logBuf(&node)// any time use //

  va_end(args);

#ifdef _USE_HW_ROTS
  osMutexRelease(mutex_lock);
#endif
}


#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;
  //char data[230] = {0};
  //char data[20] = "12345678901234567890";
  bool i2cret = false;



  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("boot.line_index %d\n", log_buf_boot.line_index);
    cliPrintf("boot.buf_length %d\n", log_buf_boot.buf_length);
    cliPrintf("\n");
    cliPrintf("list.line_index %d\n", log_buf_list.line_index);
    cliPrintf("list.buf_length %d\n", log_buf_list.buf_length);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "boot"))
  {
    uint32_t index = 0;


    while(cliKeepLoop())
    {
      uint32_t buf_len;


      buf_len = log_buf_boot.buf_length - index;
      if (buf_len == 0)
      {
        #if 0
          for( int i = 0; i < log_buf_boot.buf_length ; i++)
          {
            i2cret = logtoi2cWrite(0, 0x50,  i  , (uint8_t)log_buf_boot.buf[i],  100);
            delay(1);
          }

          if(i2cret == 1)
          {
            cliPrintf("i2c OK\n");
          }
          else
          {
            cliPrintf("i2c false\n");
          }
        #endif
          break;
      }

      if (buf_len > 64)
      {
        buf_len = 64;
      }

      #ifdef _USE_HW_ROTS
      osMutexWait(mutex_lock, osWaitForever);
      #endif

      cliWrite((uint8_t *)&log_buf_boot.buf[index], buf_len);

      for( uint32_t i = 0; i < buf_len ; i++)
      {
        i2cret = logtoi2cWrite(0, 0x50,  index+i  , (uint8_t)log_buf_boot.buf[index+i],  100);
        delay(1);
      }

      index += buf_len;

      #ifdef _USE_HW_ROTS
      osMutexRelease(mutex_lock);
      #endif
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "list"))
  {

    uint8_t  offset = 0, H_data = 0, L_data = 0;
    uint16_t flash_addr = 0;
    uint32_t index = 0;


    while(cliKeepLoop())
    {
      uint32_t buf_len;
      uint8_t mem_flag = false;

      buf_len = log_buf_list.buf_length - index;

      if (buf_len == 0)
      {
        // check length //
        if(log_buf_list.buf_length >= 768) // update if change memory 256 + 256 + 256 = 768 //
        {
          cliPrintf("i2c log memory error \n");
          mem_flag = 1;
        }

        #if 1
        if ( mem_flag == 0 )
        {
          for (uint32_t i=0; i<log_buf_list.buf_length; i++)
          {

            if(i <= 255)
            {
              ledOff(_DEF_LED1);
              i2cret = logtoi2cWrite(0, 0x50,  (uint16_t)i   , (uint8_t)log_buf_list.buf[i],  100);
              delay(1);
            }
            else if( i >= 256 && i <= 512)
            {
              offset = i-256;

              ledOn(_DEF_LED1);
              i2cret = logtoi2cWrite(0, 0x51,  (uint16_t)offset   , (uint8_t)log_buf_list.buf[i],  100);
              delay(1);
            }
            else
            {
              offset = i-512;

              ledOn(_DEF_LED1);
              i2cret = logtoi2cWrite(0, 0x52,  (uint16_t)offset   , (uint8_t)log_buf_list.buf[i],  100);
              delay(1);
            }
          }
        #endif

            // Save log Flash Addr //
            flash_addr = log_buf_list.buf_length;
            H_data     = HI_BYTE(flash_addr);
            L_data     = LO_BYTE(flash_addr);

            i2cret = logtoi2cWrite(0, 0x57,  (uint16_t)254   , (uint8_t)H_data,  100);
            if(i2cret != 1)
            {
              cliPrintf("i2c log write false \n");
              break;
            }
            delay(5);

            i2cret = logtoi2cWrite(0, 0x57,  (uint16_t)255   , (uint8_t)L_data,  100);
            if(i2cret != 1)
            {
              cliPrintf("i2c log write false \n");
              break;
            }
            delay(5);
            //  END SAVE //

          #if 0
            i2cret = logtoi2cWrites(0, 0x50, (uint16_t)0, (uint8_t *)&log_buf_list.buf[0]  , 255,  100);
            //delay(10);
          #endif

            // Save log Flash Addr //
            flash_addr = log_buf_list.buf_length;
            H_data     = HI_BYTE(flash_addr);
            L_data     = LO_BYTE(flash_addr);

            i2cret = logtoi2cWrite(0, 0x57,  (uint16_t)254   , (uint8_t)H_data,  100);
            delay(1);

            i2cret = logtoi2cWrite(0, 0x57,  (uint16_t)255   , (uint8_t)L_data,  100);
            delay(1);

          } // END MEM FLAG //

          if(i2cret == 1)
          {
            cliPrintf("i2c OK\n");
          }
          else
          {
            cliPrintf("i2c false\n");
          }

          break;
      }// end buf len //


      if (buf_len > 64)
      {
        buf_len = 64;
      }

      #ifdef _USE_HW_ROTS
      osMutexWait(mutex_lock, osWaitForever);
      #endif

      cliWrite((uint8_t *)&log_buf_list.buf[index], buf_len);

      index += buf_len;

      #ifdef _USE_HW_ROTS
      osMutexRelease(mutex_lock);
      #endif

    }


    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "flash"))
  {
    uint8_t READ[2]={0};
    uint16_t flash_length =0;
    uint32_t offset = 0;

    while(cliKeepLoop())
    {

      cliPrintf("Current list buf len : %d \n", log_buf_list.buf_length);

     // Make log Flash Addr //
      i2cret       = (uint16_t)logtoi2cRead(0, 0x57, 254, &READ[0], 100); // High 8 bit
      i2cret       = (uint16_t)logtoi2cRead(0, 0x57, 255, &READ[1], 100); // low 8 bit
      flash_length = (uint16_t)MAKE_WORD((uint8_t)READ[0], (uint8_t)READ[1]);

      if (i2cret != true)
      {
        cliPrintf("MAKE Addr False \n, ");
        break;
      }

     //cliPrintf("MAKE WORD : %d \n, ", flash_length);

      cliPrintf("   last list buf len : %d \n", flash_length);

     //  END MAKE //


      for (uint32_t i=0; i<flash_length; i++)
      {
        if(i <= 255)
        {
          ledOff(_DEF_LED1);
          i2cret = logtoi2cRead(0, 0x50, i, &buf_flash[i], 100);
          delay(1);
        }
        else if( i >= 256 && i <= 511)
        {
          offset = i-256;
          ledOn(_DEF_LED1);
          i2cret = logtoi2cRead(0, 0x51, offset, &buf_flash[i], 100);
          delay(1);
        }
        else
        {
          offset = i-512;
          ledOn(_DEF_LED1);
          i2cret = logtoi2cRead(0, 0x52, offset, &buf_flash[i], 100);
          delay(1);
        }

      }


      if (i2cret == true)
      {
        cliPrintf("i2c OK\n");
        cliWrite((uint8_t *)&buf_flash[0],  (uint32_t)flash_length);
      }
      else
      {
        cliPrintf("i2c false\n");
      }

      break;

    }

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("log info\n");
    cliPrintf("log boot\n");
    cliPrintf("log list\n");
    cliPrintf("log flash\n");
  }
}
#endif


#endif
