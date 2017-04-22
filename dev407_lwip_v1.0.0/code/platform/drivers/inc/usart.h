#ifndef __USART_H
#define __USART_H

#include "base_type.h"

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	

void usart_init(uint32_t baudrate);
uint8_t usart_sendbyte(int ch);
void usart_putc(uint8_t ch);
void usart_puts(const char *str);


#endif