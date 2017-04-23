#include "stm32F4xx.h"
#include "stm32f4xx_gpio.h"

#include "led.h"

void led_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
}

void led_ctrl(LedData led_enum, LedAction ctrl_val)
{
    switch(led_enum)
    {
        case Led1:
          GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)ctrl_val);
          break;
          
        case Led2:
          GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)ctrl_val);
          break;
          
        case Led3:
          GPIO_WriteBit(GPIOB, GPIO_Pin_6, (BitAction)ctrl_val);
          break;
          
        case Led4:
          GPIO_WriteBit(GPIOC, GPIO_Pin_6, (BitAction)ctrl_val);
          break;
          
        default:
          break;
    }
      
}