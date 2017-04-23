/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.7.1
  * @date    20-May-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

    
#include "stm32f4xx.h"

/*ucos-iii header*/
#include  <includes.h>
    
#include "stdio.h"
#include "printf.h"

#include "systick.h"
#include "timer.h"
#include "led.h"
#include "usart.h"
#include "sram.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"

#include "netconn_udp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/*START TASK*/
#define  START_TASK_PRIO                           4u   /*TASK PRIORITIES*/
#define  START_TASK_STK_SIZE                     128u   /*TASK STACK SIZES*/

static  OS_TCB   StartTaskTCB;
static  CPU_STK  StartTaskStk[START_TASK_STK_SIZE];

//static  void  AppTaskStart (void *p_arg);

/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

void StartTask(void *pdata);


/* Private functions ---------------------------------------------------------*/
//mode:1 显示DHCP获取到的地址
//	  其他 显示静态地址
void show_address(u8 mode)
{
	if(mode==1)
	{
		printf("MAC :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址

		printf("DHCP IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址

		printf("DHCP GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址

		printf("DHCP IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址

	}
	else 
	{
		printf("MAC :%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);//打印MAC地址

		printf("Static IP:%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址

		printf("Static GW:%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址

		printf("Static IP:%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址

	}	
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  
    OS_ERR	err;
    
    OSInit(&err);   /* Init uC/OS-III.   */


    systick_init(168); 
    usart_init(115200);
    led_gpio_init();
    fsms_sram_init();   //初始化外部SRAM
    
    ow_mem_init(SRAMIN);		//初始化内部内存池
	ow_mem_init(SRAMEX);		//初始化外部内存池
	ow_mem_init(SRAMCCM);	//初始化CCM内存池
    
    //tim3_int_init(999,839); //100hz的频率
    
    OSInit(&err); 					//UCOS初始化

    printf("\r\nInit finished!!!\r\n");   
    
    /* Infinite loop */
    while(lwip_comm_init())
	{
		printf("LWIP Init Falied!\r\n");
		delay_ms(1200);
		printf("Retrying...\r\n");  
	}
	printf("LWIP Init Success!\r\n");
    
    /*初始化udp_demo(创建udp_demo线程)*/
    udp_demo_init();    

    
    OSTaskCreate((OS_TCB       *)&StartTaskTCB,              /* Create the start task                                */
             (CPU_CHAR     *)"Start Task",
             (OS_TASK_PTR   )StartTask,
             (void         *)0u,
             (OS_PRIO       )START_TASK_PRIO,
             (CPU_STK      *)&StartTaskStk[0u],
             (CPU_STK_SIZE  )StartTaskStk[START_TASK_STK_SIZE / 10u],
             (CPU_STK_SIZE  )START_TASK_STK_SIZE,
             (OS_MSG_QTY    )0u,
             (OS_TICK       )0u,
             (void         *)0u,
             (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
             (OS_ERR       *)&err);

    OSStart(&err);  /* Start multitasking (i.e. give control to uC/OS-III). */
    
    
}

/*Start Task*/
void StartTask(void *pdata)
{
	CPU_SR  cpu_sr = (CPU_SR)0;
    OS_ERR  *p_err;
	(void)pdata ;
	
	OS_StatTaskInit(p_err);  			//初始化统计任务
	CPU_CRITICAL_ENTER();  	//关中断
    
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();	//创建DHCP任务
#endif
	
	OSTaskSuspend(NULL, p_err); //挂起start_task任务
	CPU_CRITICAL_EXIT();  //开中断
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
