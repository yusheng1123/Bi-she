
/*                                                 */
/*            实现MQTT协议功能的头文件             */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __MQTT_H
#define __MQTT_H


#include "stm32f10x.h"
#include <stdint.h>


#define Client_ID "dxk1qb1d"   //客户端ID
#define User_name "ya0jwf1ggqknwchz"//用户名
#define Pass_word "Yi6kWXB0dW"////密码

#define SET_TOPIC  "attributes/push"//订阅
#define POST_TOPIC "attributes"//发布

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
    
//阿里云用户名初始化
void Aliyun_LoginInit(char *ProductKey,char *DeviceName,char *DeviceSecret);
//MQTT协议相关函数声明
u8 MQTT_PublishData(char *topic, char *message, u8 qos);
u8 MQTT_SubscribeTopic(char *topic,u8 qos,u8 whether);
void MQTT_Init(void);
u8 MQTT_Connect(char *ClientID,char *Username,char *Password);
void MQTT_SentHeart(void);
void MQTT_Disconnect(void);
void MQTT_SendBuf(u8 *buf,u16 len);

#endif
    
