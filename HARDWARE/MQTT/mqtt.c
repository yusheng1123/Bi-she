
/*-------------------------------------------------*/
/*                                                 */
/*            实现MQTT协议功能的源文件             */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"    //包含需要的头文件
#include "mqtt.h"         //包含需要的头文件
#include "string.h"       //包含需要的头文件
#include "stdio.h"        //包含需要的头文件
#include "usart1.h"       //包含需要的头文件
#include "utils_hmac.h"   //包含需要的头文件
#include "wifi.h"         //包含需要的头文件
#include "delay.h"
#include "usart2.h" 

u8 *mqtt_rxbuf;
u8 *mqtt_txbuf;
u16 mqtt_rxlen;
u16 mqtt_txlen;
u8 _mqtt_txbuf[256];//发送数据缓存区
u8 _mqtt_rxbuf[256];//接收数据缓存区

typedef enum
{
	//名字 	    值 			报文流动方向 	描述
	M_RESERVED1	=0	,	//	禁止	保留
	M_CONNECT		,	//	客户端到服务端	客户端请求连接服务端
	M_CONNACK		,	//	服务端到客户端	连接报文确认
	M_PUBLISH		,	//	两个方向都允许	发布消息
	M_PUBACK		,	//	两个方向都允许	QoS 1消息发布收到确认
	M_PUBREC		,	//	两个方向都允许	发布收到（保证交付第一步）
	M_PUBREL		,	//	两个方向都允许	发布释放（保证交付第二步）
	M_PUBCOMP		,	//	两个方向都允许	QoS 2消息发布完成（保证交互第三步）
	M_SUBSCRIBE		,	//	客户端到服务端	客户端订阅请求
	M_SUBACK		,	//	服务端到客户端	订阅请求报文确认
	M_UNSUBSCRIBE	,	//	客户端到服务端	客户端取消订阅请求
	M_UNSUBACK		,	//	服务端到客户端	取消订阅报文确认
	M_PINGREQ		,	//	客户端到服务端	心跳请求
	M_PINGRESP		,	//	服务端到客户端	心跳响应
	M_DISCONNECT	,	//	客户端到服务端	客户端断开连接
	M_RESERVED2		,	//	禁止	保留
}_typdef_mqtt_message;

//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00
const u8 parket_connetAck[] = {0x20,0x02,0x00,0x00};
const u8 parket_disconnet[] = {0xe0,0x00};
const u8 parket_heart[] = {0xc0,0x00};
const u8 parket_heart_reply[] = {0xc0,0x00};
const u8 parket_subAck[] = {0x90,0x03};

void MQTT_Init(void)
{
    //缓冲区赋值
	mqtt_rxbuf = _mqtt_rxbuf;
	mqtt_rxlen = sizeof(_mqtt_rxbuf);
	mqtt_txbuf = _mqtt_txbuf;
	mqtt_txlen = sizeof(_mqtt_txbuf);
	memset(mqtt_rxbuf,0,mqtt_rxlen);
	memset(mqtt_txbuf,0,mqtt_txlen);
	
	//无条件先主动断开
//	MQTT_Disconnect();
//	Delay_Ms(100);
//	MQTT_Disconnect();
//	Delay_Ms(100);
}

/*
函数功能: 登录服务器
函数返回值: 0表示成功 1表示失败
*/
u8 MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	u16 i,j;
	int ClientIDLen = strlen(ClientID);
	int UsernameLen = strlen(Username);
	int PasswordLen = strlen(Password);
	int DataLen;
	mqtt_txlen=0;
	//可变报头+Payload  每个字段包含两个字节的长度标识
	DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);

	//固定报头
	//控制报文类型
	mqtt_txbuf[mqtt_txlen++] = 0x10;		//MQTT Message Type CONNECT
	//剩余长度(不包括固定头部)
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		mqtt_txbuf[mqtt_txlen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//可变报头
	//协议名
	mqtt_txbuf[mqtt_txlen++] = 0;        	// Protocol Name Length MSB    
	mqtt_txbuf[mqtt_txlen++] = 4;           // Protocol Name Length LSB    
	mqtt_txbuf[mqtt_txlen++] = 'M';        	// ASCII Code for M    
	mqtt_txbuf[mqtt_txlen++] = 'Q';        	// ASCII Code for Q    
	mqtt_txbuf[mqtt_txlen++] = 'T';        	// ASCII Code for T    
	mqtt_txbuf[mqtt_txlen++] = 'T';        	// ASCII Code for T    
	//协议级别
	mqtt_txbuf[mqtt_txlen++] = 4;        		// MQTT Protocol version = 4    
	//连接标志
	mqtt_txbuf[mqtt_txlen++] = 0xc2;        	// conn flags 
	mqtt_txbuf[mqtt_txlen++] = 0;        		// Keep-alive Time Length MSB    
	mqtt_txbuf[mqtt_txlen++] = 100;        	// Keep-alive Time Length LSB  100S心跳包  

	mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen);// Client ID length MSB    
	mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen);// Client ID length LSB  	
	memcpy(&mqtt_txbuf[mqtt_txlen],ClientID,ClientIDLen);
	mqtt_txlen += ClientIDLen;
	
	if(UsernameLen > 0)
	{   
		mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen);		//username length MSB    
		mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen);    	//username length LSB    
		memcpy(&mqtt_txbuf[mqtt_txlen],Username,UsernameLen);
		mqtt_txlen += UsernameLen;
	}
	
	if(PasswordLen > 0)
	{    
		mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen);		//password length MSB    
		mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen);    	//password length LSB  
		memcpy(&mqtt_txbuf[mqtt_txlen],Password,PasswordLen);
		mqtt_txlen += PasswordLen; 
	}   
//    u1_printf("mqtt_txlen=%d\r\n",mqtt_txlen);
//    for(i=0;i<mqtt_txlen;i++)
//    {
//        u1_printf("%#x ",mqtt_txbuf[i]);
//    }
	for(i=0;i<10;i++)
	{
		memset(mqtt_rxbuf,0,mqtt_rxlen);
		MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);
		for(j=0;j<10;j++)
		{
			Delay_Ms(50);
			if(usart2_data)
			{
				memcpy(mqtt_rxbuf,usart2_arrary,usart2_index);
				
				//memcpy
				usart2_data=0;
				usart2_index=0;
			}
			//CONNECT
			if(mqtt_rxbuf[0]==parket_connetAck[0] && mqtt_rxbuf[1]==parket_connetAck[1]) //连接成功			   
			{
				return 0;//连接成功
			}
		}
	}
	return 1;
}

/*
函数功能: MQTT订阅/取消订阅数据打包函数
函数参数:
    topic       主题   
    qos         消息等级 0:最多分发一次  1: 至少分发一次  2: 仅分发一次
    whether     订阅/取消订阅请求包 (1表示订阅,0表示取消订阅)
返回值: 0表示成功 1表示失败
*/
u8 MQTT_SubscribeTopic(char *topic,u8 qos,u8 whether)
{    
	u8 i,j;
	mqtt_txlen=0;
	int topiclen = strlen(topic);
	int DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
	//固定报头
	//控制报文类型
	if(whether)mqtt_txbuf[mqtt_txlen++] = 0x82; //消息类型和标志订阅
	else	mqtt_txbuf[mqtt_txlen++] = 0xA2;    //取消订阅
	//剩余长度
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
		encodedByte = encodedByte | 128;
		mqtt_txbuf[mqtt_txlen++] = encodedByte;
	}while ( DataLen > 0 );	
	//可变报头
	mqtt_txbuf[mqtt_txlen++] = 0;			//消息标识符 MSB
	mqtt_txbuf[mqtt_txlen++] = 0x01;        //消息标识符 LSB
	//有效载荷
	mqtt_txbuf[mqtt_txlen++] = BYTE1(topiclen);//主题长度 MSB
	mqtt_txbuf[mqtt_txlen++] = BYTE0(topiclen);//主题长度 LSB   
	memcpy(&mqtt_txbuf[mqtt_txlen],topic,topiclen);
	mqtt_txlen += topiclen;  
	if(whether)
	{
		 mqtt_txbuf[mqtt_txlen++] = qos;//QoS级别
	}
	for(i=0;i<10;i++)
	{
		memset(mqtt_rxbuf,0,mqtt_rxlen);
		MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);
		for(j=0;j<10;j++)
		{
			Delay_Ms(50);
			if(usart2_data)
			{
//				for(i=0;i<usart2_index;i++)
//				{
//					u1_printf("%x ",usart2_arrary[i]);
//				}
				u1_printf("\n");
				memcpy(mqtt_rxbuf,usart2_arrary,usart2_index);
				usart2_data=0;
				usart2_index=0;
			}
			if(mqtt_rxbuf[0]==parket_subAck[0] && mqtt_rxbuf[1]==parket_subAck[1]) //订阅成功			   
			{
				return 0;//订阅成功
			}
		}
	}
	return 1; //失败
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
u8 MQTT_PublishData(char *topic, char *message, u8 qos)
{  
	int topicLength = strlen(topic);    
	int messageLength = strlen(message);     
	u16 id=0;
	int DataLen;
	mqtt_txlen=0;
	//有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
	//QOS为0时没有标识符
	//数据长度             主题名   报文标识符   有效载荷
    if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
    else	DataLen = (2+topicLength) + messageLength;   

    //固定报头
	//控制报文类型
    mqtt_txbuf[mqtt_txlen++] = 0x30;    // MQTT Message Type PUBLISH  

	//剩余长度
	do
	{
		u8 encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		mqtt_txbuf[mqtt_txlen++] = encodedByte;
	}while ( DataLen > 0 );	
	mqtt_txbuf[mqtt_txlen++] = BYTE1(topicLength);//主题长度MSB
	mqtt_txbuf[mqtt_txlen++] = BYTE0(topicLength);//主题长度LSB 
	memcpy(&mqtt_txbuf[mqtt_txlen],topic,topicLength);//拷贝主题
	mqtt_txlen += topicLength;       
	//报文标识符
	if(qos)
	{
		mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
		mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
		id++;
	}
	memcpy(&mqtt_txbuf[mqtt_txlen],message,messageLength);
	mqtt_txlen += messageLength;

	MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);
	return mqtt_txlen;
}

void MQTT_SentHeart(void)
{
	MQTT_SendBuf((u8 *)parket_heart,sizeof(parket_heart));
}

void MQTT_Disconnect(void)
{
	MQTT_SendBuf((u8 *)parket_disconnet,sizeof(parket_disconnet));
}

void MQTT_SendBuf(u8 *buf,u16 len)
{
	Usartx_SendData(USART2,buf,len);
}	
