#include <string.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "usart.h"	 
#include "timer.h"
#include "dht11.h"
#include "HC_SR04.h"
#include "adc.h"
#include "onenet.h"//����Э���
#include "esp8266.h"//�����豸


u8 temperature;            //�¶�
u8 fan;                    //���ȿ���
u8 humidity;               //ʪ��
u16 ADC_Value[30];         //adc���ݻ�����
u16 value1,value2,value3;  //DMA���ݴ���������м�������ֱ���ȡ���գ����������������
u16 n=0;
float Distance;            //����
u8 Lighting,Voice,Gas;     //���գ����������������
float temp1;
u8 LED_value;              //LED�Ƶ�ֵ��0��1
u8 buf1[10]={0};
u8 buf2[10]={0};
u8 buf3[10]={0};
u8 buf4[10]={0};
u8 buf5[10]={0};
u8 buf6[10]={0};
u16 i;   //forѭ���õ��ı�������


int main(void)
{
	unsigned char *dataPtr = NULL;
	unsigned short timeCount = 300;	//���ͼ������
	
//==================================Ƭ�������ʼ��==============================================
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����
	delay_init();			          //��ʱ������ʼ��
	Usart1_Init(9600);	        //����1��ʼ��Ϊ115200
	Usart2_Init(115200); 	      //����2����ESP8266ͨ��
	LED_Init();				          //LED��ʼ��
	OLED_Init(); 	              //OLED��ʼ��  
	TIM3_Int_Init(2000,7200);   //��ʼ����ʱ�������������õ�
  HC_SR04_Init(0xffff,72-1);  //������ģ���ʼ��
	DHT11_Init();               //��ʼ����ʪ��ģ��
	ADCx_Init();                //��ʼ��ADC�����ڲɼ����������գ�����Ũ��
	Buzzer_Init();              //��������ʼ��
	
//===================================����Э���ʼ��===============================================	
	ESP8266_Init();					    //��ʼ��ESP8266
	delay_ms(500);
	while(OneNet_DevLink())			//����OneNET
	delay_ms(500);
	LED = LED_ON;						    //�����ɹ�����������LED
  show();                     //OLED��ʾ������
			
	while(1) 
	{
	
//=====================================��ʪ��ģ��===================================================	
	  if(DHT11_Read_Data(&temperature,&humidity) == 0)
		{
			sprintf((char *)buf1,":%d",temperature);
			sprintf((char *)buf2,":%d%%RH",humidity);
			OLED_ShowStr(32,0,buf1,2);
			OLED_ShowStr(32,2,buf2,2);
		}
		
//=======================�����գ��������к����壩�������������ݵĻ�ȡ��ͨ��ADC����ͨ����ѯ��ȡ========		
    for(i=0,value1=0,value2=0,value3=0;i<30;)
		{
			value1+=ADC_Value[i++];	
			value2+=ADC_Value[i++];
			value3+=ADC_Value[i++];
		}			
		Lighting=(4096-value1/10)*100/4096; //����
		temp1=(float)(value3/10)*(3.3/4096);//��������
		value1=0;
		value2=0;
		value3=0;
		
		if(temp1<0.9)
			{
				n=temp1/0.001;
				Gas = n*0.012;
				printf("һ����̼Ũ��:%dppm\n",Gas);
			}
			else
			{
				Gas = ((temp1-0.9)/0.1)*100;
				Gas =Gas+9;
				printf("һ����̼Ũ��:%dppm\n",Gas);
			}
		sprintf((char *)buf3,":%d%%",Lighting);
		sprintf((char *)buf5,":%d   ",Gas);
		OLED_ShowStr(32,4,buf3,2);//OLED��ʾ����
		OLED_ShowStr(64,6,buf5,2);//OLED��ʾ��������
			
//===========================================================����������===================================
		if(Gas<60)
		{
			Buzzer=Buzzer_ON;
		}		
    else
		{
			Buzzer=Buzzer_OFF;
		}			
			
//===========================================================������ģ��===================================			
		//Get_distance();
		//sprintf((char *)buf6,":%.1fCM",Distance);
		//OLED_ShowStr(64,6,buf6,2);//��ʾ����
		//if(Distance<20)
		//	LED = LED_ON;
		//else LED = LED_OFF;
//=============================================�ϴ����ݵ�OneNet�����������Բ��ùܣ�=================================			
   	delay_ms(10);
		timeCount ++;
		if(timeCount >= 300)	//���ͼ��3S
		{	
			timeCount = 0;
			OneNet_SendData();	//��������
			ESP8266_Clear();    //��ջ���
		}
		
//==============================================����OneNet���������������ݣ����Բ��ùܣ�===============================			
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
		{
			OneNet_RevPro(dataPtr);
		}

		

	}	
}



















