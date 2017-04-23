#ifndef _NETCONN_UDP_H
#define _NETCONN_UDP_H


#define UDP_DEMO_RX_BUFSIZE		2000	//定义udp最大接收数据长度
#define UDP_DEMO_PORT			8089	//定义udp连接的本地端口号
#define LWIP_SEND_DATA			0X80    //定义有数据发送

extern u8 udp_flag;		//UDP数据发送标志位

void udp_demo_init(void);


#endif
