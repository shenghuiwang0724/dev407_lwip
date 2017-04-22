
#ifndef _SYSTICK_H_
#define _SYSTICK_H_

void systick_init(u8 SYSCLK);
void delay_us(u32 nus);
void delay_ms(u16 nms);

#endif