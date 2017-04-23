#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "includes.h"

#include "lwip_comm.h"
#include "usart.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"

#include "netconn_udp.h"

/*Sequential Udp Task*/
#define  SERQU_UDP_TASK_PRIO                        5u   /*TASK PRIORITIES*/
#define  SERQU_UDP_STK_SIZE                         128u   /*TASK STACK SIZES*/

OS_TCB Sequ_Udp_TCB;
CPU_STK Sequ_Udp_Stk[SERQU_UDP_STK_SIZE];




u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区
//UDP发送数据内容
const u8 *udp_demo_sendbuf="Explorer STM32F407 NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP数据发送标志位

//udp任务函数
static void sequ_udp_task(void *arg)
{
	CPU_SR cpu_sr = (CPU_SR)0;
	OS_ERR err;
	static struct netconn *udpconn;
	static struct netbuf  *recvbuf;
	static struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u32 data_len = 0;
	struct pbuf *q;
	
	LWIP_UNUSED_ARG(arg);
    
	udpconn = netconn_new(NETCONN_UDP);  //创建一个UDP链接
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //创建UDP连接成功
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //构造目的IP地址
		netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//连接到远端主机
		if(err == ERR_OK)//绑定完成
		{
			while(1)
			{
				if((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf, strlen((char *)udp_demo_sendbuf));
					memcpy(sentbuf->p->payload, (void*)udp_demo_sendbuf, strlen((char*)udp_demo_sendbuf));
					err = netconn_send(udpconn, sentbuf);  	//将netbuf中的数据发送出去
					if(err != ERR_OK)
					{
						printf("发送失败\r\n");
						netbuf_delete(sentbuf);      //删除buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//清除数据发送标志
					netbuf_delete(sentbuf);      	//删除buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //接收数据
                
				if(recvbuf != NULL)          //接收到数据
				{ 
					CPU_CRITICAL_ENTER(); //关中断
                    
					memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
					for(q=recvbuf->p; q!=NULL; q=q->next)  //遍历完整个pbuf链表
					{
						//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) 
                            memcpy(udp_demo_recvbuf+data_len, q->payload, (UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
						else 
                            memcpy(udp_demo_recvbuf+data_len, q->payload, q->len);
                        
						data_len += q->len;  	
						if(data_len > UDP_DEMO_RX_BUFSIZE) 
                            break; //超出TCP客户端接收数组,跳出	
					}
                    
					CPU_CRITICAL_EXIT();  //开中断
                    
					data_len = 0;  //复制完成后data_len要清零。
					printf("%s\r\n", udp_demo_recvbuf);  //打印接收到的数据
					netbuf_delete(recvbuf);      //删除buf
				}
                else 
                  OSTimeDlyHMSM(0, 0, 0, 5,
                                OS_OPT_TIME_HMSM_STRICT, &err);  //延时5ms
			}
		}
        else 
          printf("UDP绑定失败\r\n");
	}
    else 
      printf("UDP连接创建失败\r\n");
}

//创建UDP线程
//返回值:0 UDP创建成功
//		其他 UDP创建失败
void udp_demo_init(void)
{

	CPU_SR cpu_sr;
    
    OS_ERR err;
	
	CPU_CRITICAL_ENTER();	//关中断
  
    /* Create Sequential Udp Task */
    OSTaskCreate((OS_TCB       *)&Sequ_Udp_TCB,              /*任务控制块指针 */
             (CPU_CHAR     *)"Sequential Udp Task",
             (OS_TASK_PTR   )sequ_udp_task,
             (void         *)0u,
             (OS_PRIO       )SERQU_UDP_TASK_PRIO,   /*任务优先级*/
             (CPU_STK      *)&Sequ_Udp_Stk[0u], /*任务堆栈基地址*/
             (CPU_STK_SIZE  )Sequ_Udp_Stk[SERQU_UDP_STK_SIZE / 10u],    /*堆栈剩余警戒线*/
             (CPU_STK_SIZE  )SERQU_UDP_STK_SIZE,    /*堆栈大小*/
             (OS_MSG_QTY    )0u,    /*可接收的最大消息队列数*/
             (OS_TICK       )0u,    /*时间片轮转时间*/
             (void         *)0u,    /*任务控制块扩展信息*/
             (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),   /*任务选项*/
             (OS_ERR       *)&err);
    
	CPU_CRITICAL_EXIT();		//开中断	

}
