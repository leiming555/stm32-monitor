//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"
#include "delay.h"
//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

//Ӳ������
#include "usart.h"
#include "oled.h"
#include "led.h"
//C��
#include <string.h>
#include <stdio.h>


#define PROID		"598988"   //��ƷID

#define AUTH_INFO	"12345678"		//��Ȩ��Ϣ

#define DEVID		"1075649520"	//�豸ID

extern unsigned char esp8266_buf[128];
extern u8 temperature;
extern u8 humidity; 
extern u8 LED_value;
extern u8 Lighting,Voice,Gas; 
extern float Distance;   
extern u8 fan;
//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

	unsigned char *dataPtr;
	
	_Bool status = 1;
	OLED_CLS();//OLED����
  OLED_ShowCN(15,2,15);OLED_ShowCN(31,2,16);OLED_ShowCN(47,2,17);OLED_ShowCN(63,2,13);OLED_ShowCN(79,2,14);OLED_ShowCN(95,2,10);//oled��ʾ�������������С�
	printf("OneNet_DevLink\r\nPROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ϴ�ƽ̨	
		dataPtr = ESP8266_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:printf("Tips:	���ӳɹ�\r\n");status = 0;
					       OLED_CLS();//OLED����
					       OLED_ShowCN(31,4,13);OLED_ShowCN(47,4,14);OLED_ShowCN(63,4,12);OLED_ShowCN(79,4,18);//oled��ʾ�����ӳɹ���	
                 delay_ms(500);					
					       break;
					case 1:printf("WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 2:printf("WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
					case 3:printf("WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 4:printf("WARN:	����ʧ�ܣ��û������������\r\n");break;
					case 5:printf("WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
					
					default:printf("ERR:	����ʧ�ܣ�δ֪����\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else
		printf("WARN:	MQTT_PacketConnect Failed\r\n");
	  OLED_ShowCN(47,4,26);OLED_ShowCN(63,4,27);OLED_ShowCN(47,6,28);OLED_ShowCN(63,6,29);//����ʧ�ܣ�����
	return status;
	
}

unsigned char OneNet_FillBuf(char *buf)
{
	char text[32];
	
	memset(text, 0, sizeof(text));
	
	strcpy(buf, ",;");
	//============================��onenet��������==============================================================			
	memset(text, 0, sizeof(text));
	sprintf(text, "temperature,%d;",temperature); //�¶�����
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "humidity,%d;", humidity);  //ʪ������
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "Lighting,%d;", Lighting); //��������
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "air quality,%d;", Gas);          //����Ũ������
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "fan,%d;", fan); //���ȿ��ص�ֵ
	strcat(buf, text);
	
	return strlen(buf);
}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//Э���
	
	char buf[128];
	
	short body_len = 0, i = 0;
	
//	printf("Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//��ȡ��ǰ��Ҫ���͵����������ܳ���
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)							//���
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];
			
			ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//�ϴ����ݵ�ƽ̨
//			printf("Send %d Bytes\r\n", mqttPacket._len);
			
			MQTT_DeleteBuffer(&mqttPacket);															//ɾ��
		}
		else
			printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//Э���
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//�����·�
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//���topic����Ϣ��
			if(result == 0)
			{
				printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//����ظ����
				{
					printf("Tips:	Send CmdResp\r\n");
					
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ظ�����
					MQTT_DeleteBuffer(&mqttPacket);									//ɾ��
				}
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
//				printf("Tips:	MQTT Publish Send OK\r\n");
			
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//��ջ���
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, ':');					//����':'

	if(dataPtr != NULL && result != -1)					//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		numBuf[num] = 0;
		
		num = atoi((const char *)numBuf);			  //תΪ��ֵ��ʽ
		
//============================����onenet����������=================================================================	
	 if(strstr((char *)req_payload, "fan"))		//����"fan"
		{
			if(num==0)//�رշ���
			{
				LED = LED_OFF;//�����Թصƴ���
			}
			else if(num==1)//�򿪷���
			{
				LED = LED_ON;//�����Կ��ƴ���
			}
		}
		else if(strstr((char *)req_payload, "   "))		//�������� �����õı��� �Դ�����  �������������ӵĹ���
		{
			//�������Ҫ�Ĺ���
		}
		
	}
//====================================================================================================================
	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
