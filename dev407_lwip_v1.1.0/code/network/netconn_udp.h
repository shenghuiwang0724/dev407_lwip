#ifndef _NETCONN_UDP_H
#define _NETCONN_UDP_H


#define UDP_DEMO_RX_BUFSIZE		2000	//����udp���������ݳ���
#define UDP_DEMO_PORT			8089	//����udp���ӵı��ض˿ں�
#define LWIP_SEND_DATA			0X80    //���������ݷ���

extern u8 udp_flag;		//UDP���ݷ��ͱ�־λ

void udp_demo_init(void);


#endif
