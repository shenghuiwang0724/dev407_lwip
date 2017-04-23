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




u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ�����
//UDP������������
const u8 *udp_demo_sendbuf="Explorer STM32F407 NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP���ݷ��ͱ�־λ

//udp������
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
    
	udpconn = netconn_new(NETCONN_UDP);  //����һ��UDP����
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //����UDP���ӳɹ�
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //����Ŀ��IP��ַ
		netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//���ӵ�Զ������
		if(err == ERR_OK)//�����
		{
			while(1)
			{
				if((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //������Ҫ����
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf, strlen((char *)udp_demo_sendbuf));
					memcpy(sentbuf->p->payload, (void*)udp_demo_sendbuf, strlen((char*)udp_demo_sendbuf));
					err = netconn_send(udpconn, sentbuf);  	//��netbuf�е����ݷ��ͳ�ȥ
					if(err != ERR_OK)
					{
						printf("����ʧ��\r\n");
						netbuf_delete(sentbuf);      //ɾ��buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//������ݷ��ͱ�־
					netbuf_delete(sentbuf);      	//ɾ��buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //��������
                
				if(recvbuf != NULL)          //���յ�����
				{ 
					CPU_CRITICAL_ENTER(); //���ж�
                    
					memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
					for(q=recvbuf->p; q!=NULL; q=q->next)  //����������pbuf����
					{
						//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
						//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
						if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) 
                            memcpy(udp_demo_recvbuf+data_len, q->payload, (UDP_DEMO_RX_BUFSIZE-data_len));//��������
						else 
                            memcpy(udp_demo_recvbuf+data_len, q->payload, q->len);
                        
						data_len += q->len;  	
						if(data_len > UDP_DEMO_RX_BUFSIZE) 
                            break; //����TCP�ͻ��˽�������,����	
					}
                    
					CPU_CRITICAL_EXIT();  //���ж�
                    
					data_len = 0;  //������ɺ�data_lenҪ���㡣
					printf("%s\r\n", udp_demo_recvbuf);  //��ӡ���յ�������
					netbuf_delete(recvbuf);      //ɾ��buf
				}
                else 
                  OSTimeDlyHMSM(0, 0, 0, 5,
                                OS_OPT_TIME_HMSM_STRICT, &err);  //��ʱ5ms
			}
		}
        else 
          printf("UDP��ʧ��\r\n");
	}
    else 
      printf("UDP���Ӵ���ʧ��\r\n");
}

//����UDP�߳�
//����ֵ:0 UDP�����ɹ�
//		���� UDP����ʧ��
void udp_demo_init(void)
{

	CPU_SR cpu_sr;
    
    OS_ERR err;
	
	CPU_CRITICAL_ENTER();	//���ж�
  
    /* Create Sequential Udp Task */
    OSTaskCreate((OS_TCB       *)&Sequ_Udp_TCB,              /*������ƿ�ָ�� */
             (CPU_CHAR     *)"Sequential Udp Task",
             (OS_TASK_PTR   )sequ_udp_task,
             (void         *)0u,
             (OS_PRIO       )SERQU_UDP_TASK_PRIO,   /*�������ȼ�*/
             (CPU_STK      *)&Sequ_Udp_Stk[0u], /*�����ջ����ַ*/
             (CPU_STK_SIZE  )Sequ_Udp_Stk[SERQU_UDP_STK_SIZE / 10u],    /*��ջʣ�ྯ����*/
             (CPU_STK_SIZE  )SERQU_UDP_STK_SIZE,    /*��ջ��С*/
             (OS_MSG_QTY    )0u,    /*�ɽ��յ������Ϣ������*/
             (OS_TICK       )0u,    /*ʱ��Ƭ��תʱ��*/
             (void         *)0u,    /*������ƿ���չ��Ϣ*/
             (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),   /*����ѡ��*/
             (OS_ERR       *)&err);
    
	CPU_CRITICAL_EXIT();		//���ж�	

}
