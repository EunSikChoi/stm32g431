/*
 * led.c
 *
 *  Created on: 2021. 7. 27.
 *      Author: 82109
 */

#include "led.h"


typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState on_state;
  GPIO_PinState off_state;
}led_tb_t;

led_tb_t led_tbl[LED_MAX_CH] =
{
    {GPIOC, GPIO_PIN_13, GPIO_PIN_SET, GPIO_PIN_RESET},
    {GPIOB, GPIO_PIN_9 , GPIO_PIN_SET, GPIO_PIN_RESET}
};

bool ledInit(void)
{
  bool ret = 0;

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  for (int i=0; i < LED_MAX_CH; i++)
  {
    GPIO_InitStruct.Pin = led_tbl[i].pin;
    HAL_GPIO_Init(led_tbl[i].port, &GPIO_InitStruct);

    ledOff(i);
  }

  return ret;

}


void ledOn(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;

  HAL_GPIO_WritePin(led_tbl[ch].port, led_tbl[ch].pin, led_tbl[ch].on_state);
}

void ledOff(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;

  HAL_GPIO_WritePin(led_tbl[ch].port, led_tbl[ch].pin, led_tbl[ch].off_state);
}

void ledToggle(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;

  HAL_GPIO_TogglePin(led_tbl[ch].port, led_tbl[ch].pin);
}
