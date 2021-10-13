/*
 * i2c.c
 *
 *  Created on: 2021. 9. 23.
 *      Author: 82109
 */


#include "i2c.h"
#include "cli.h"

//I2C_HandleTypeDef hi2c1;


#ifdef _USE_HW_CLI
static void cliI2C(cli_args_t *args);
#endif


bool i2cInit(void)
{
  bool ret = false;



#ifdef _USE_HW_CLI
  cliAdd("i2c", cliI2C);
#endif

  logPrintf("I2c_Init     \t\t: I2C_Ok\r\n");

  return ret;
}

bool i2cOpen(void)
{

  bool ret = false;

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00F07BFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    ret = true;
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    ret = true;
  }

  return ret;
}

bool i2cIsDeviceReady(uint8_t ch, uint8_t dev_addr)
{

  if (HAL_I2C_IsDeviceReady(&hi2c1, dev_addr << 1, 10, 10) == HAL_OK)
  {
    return true;
  }

  return false;
}

bool i2cReadByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
  return i2cReadBytes(ch, dev_addr, reg_addr, p_data, 1, timeout);
}

bool i2cReadBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret;
  HAL_StatusTypeDef i2c_ret;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }


  i2c_ret = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, p_data, length, timeout);

  if( i2c_ret == HAL_OK )
  {
    ret = true;
  }
  else
  {
    ret = false;
  }

  return ret;
}

bool i2cWriteByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
  return i2cWriteBytes(ch, dev_addr, reg_addr, &data, 1, timeout);
}

bool i2cWriteBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret;
  HAL_StatusTypeDef i2c_ret;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, p_data, length, timeout);

  if(i2c_ret == HAL_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }

  return ret;
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PA15     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PA15     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

#ifdef _USE_HW_CLI
void cliI2C(cli_args_t *args)
{
  bool ret = false;
  bool i2c_ret;
  uint8_t print_ch;
  uint8_t ch;
  uint16_t dev_addr;
  uint16_t reg_addr;
  uint16_t length;
  uint32_t i;
  uint8_t aRxBuffer[13];
  uint8_t i2c_data[128];
  uint32_t pre_time;




  if (args->argc == 1 && args->isStr(0, "info"))
  {

    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "scan"))
  {

    print_ch = (uint16_t) args->getData(1);

    print_ch = constrain(print_ch, 1, I2C_MAX_CH);
    print_ch -= 1;

    for (i=0x00; i<= 0x7F; i++) //7f  = 7bit ID //
    {
      if (i2cIsDeviceReady(print_ch, i) == true) // check ready i2c device //
      {
        cliPrintf("I2C CH%d Addr 0x%X : OK\n", print_ch+1, i);
      }
    }

    ret = true;
  }

  if (args->argc == 5)
  {

    print_ch = (uint16_t) args->getData(1);
    print_ch = constrain(print_ch, 1, I2C_MAX_CH);

    dev_addr = (uint16_t) args->getData(2);
    reg_addr = (uint16_t) args->getData(3);
    length   = (uint16_t) args->getData(4);
    ch       = print_ch - 1;

    if(args->isStr(0, "read") == true)
    {
      for (i=0; i<length; i++)
      {
        i2c_ret = i2cReadByte(ch, dev_addr, reg_addr+i, i2c_data, 100);

        if (i2c_ret == true)
        {
          cliPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, reg_addr+i, i2c_data[0]);
        }
        else
        {
          cliPrintf("%d I2C - Fail \n", print_ch);
          break;
        }
      }
    }
    else if(args->isStr(0, "write") == true)
    {
      pre_time = millis();
      i2c_ret = i2cWriteByte(ch, dev_addr, reg_addr, (uint8_t)length, 100);

      if (i2c_ret == true)
      {
        cliPrintf("%d I2C - 0x%02X : 0x%02X, %d ms\n", print_ch, reg_addr, length, millis()-pre_time);
      }
      else
      {
        cliPrintf("%d I2C - Fail \n", print_ch);
      }
    }
    else
    {
        ret = false;
    }

    ret = true;
  }



  if (args->argc == 1 && args->isStr(0, "send"))
  {

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "read"))
  {

    HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(0x38 << 1), 0x92, I2C_MEMADD_SIZE_8BIT, aRxBuffer, 1, 100);
    cliPrintf("I2C READ OK \n");

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf( "i2c info\n");
    cliPrintf( "i2c scan channel[1~%d]\n", I2C_MAX_CH);
    cliPrintf( "i2c send\n");
    cliPrintf( "i2c read\n");


    /*cliPrintf( "i2c open channel[1~%d]\n", I2C_MAX_CH);
    cliPrintf( "i2c read channel dev_addr reg_addr length\n");
    cliPrintf( "i2c write channel dev_addr reg_addr data\n");
    cliPrintf( "i2c read16 channel dev_addr reg_addr length\n");
    cliPrintf( "i2c write16 channel dev_addr reg_addr data\n");
    cliPrintf( "i2c writeData channel dev_addr reg_addr data\n");*/
  }


}

#endif



