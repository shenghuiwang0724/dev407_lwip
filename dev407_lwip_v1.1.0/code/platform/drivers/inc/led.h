#ifndef __LED_H
#define __LED_H

typedef enum
{ 
  Off = 0,
  On
}LedAction;

typedef enum
{ 
  Led1 = 1,
  Led2,
  Led3,
  Led4
}LedData;

void led_gpio_init(void);
void led_ctrl(LedData led_enum, LedAction ctrl_val);


#endif