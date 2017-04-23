#ifndef _LWIP_CFG_H
#define _LWIP_CFG_H

#include "stm32f4xx.h"

//#define ETH_TX_BUF_SIZE 1500
//#define ETH_RX_BUF_SIZE 1500

/*for task stack*/
#define  LWIP_TASK_TCPIP_PRIO                 17

#define LWIP_TASK_STK_POOL_SIZE					256
#define LWIP_TASK_TCPIP_STK_SIZE					256

/*for lwip task*/
#define LWIP_TASK_MAX	3

#endif
