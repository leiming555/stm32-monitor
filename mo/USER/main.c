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
#include "onenet.h"//网络协议层
#include "esp8266.h"//网络设备


u8 temperature;            //温度
u8 fan;                    //风扇开关
u8 humidity;               //湿度
u16 ADC_Value[30];         //adc数据缓冲区
u16 value1,value2,value3;  //DMA数据处理的三个中间变量，分别提取光照，音量，烟雾的数据
u16 n=0;
float Distance;            //距离
u8 Lighting,Voice,Gas;     //光照，音量，烟雾的数据
float temp1;
u8 LED_value;              //LED灯的值，0或1
u8 buf1[10]={0};
u8 buf2[10]={0};
u8 buf3[10]={0};
u8 buf4[10]={0};
u8 buf5[10]={0};
u8 buf6[10]={0};
u16 i;   //for循环用到的变量而已


int main(void)
{
	unsigned char *dataPtr = NULL;
	unsigned short timeCount = 300;	//发送间隔变量
	
//==================================片内外设初始化==============================================
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组
	delay_init();			          //延时函数初始化
	Usart1_Init(9600);	        //串口1初始化为115200
	Usart2_Init(115200); 	      //串口2，与ESP8266通信
	LED_Init();				          //LED初始化
	OLED_Init(); 	              //OLED初始化  
	TIM3_Int_Init(2000,7200);   //初始化定时器，驱动风扇用的
  HC_SR04_Init(0xffff,72-1);  //超声波模块初始化
	DHT11_Init();               //初始化温湿度模块
	ADCx_Init();                //初始化ADC，用于采集音量，光照，烟雾浓度
	Buzzer_Init();              //蜂鸣器初始化
	
//===================================网络协议初始化===============================================	
	ESP8266_Init();					    //初始化ESP8266
	delay_ms(500);
	while(OneNet_DevLink())			//接入OneNET
	delay_ms(500);
	LED = LED_ON;						    //入网成功，点亮板子LED
  show();                     //OLED显示主界面
			
	while(1) 
	{
	
//=====================================温湿度模块===================================================	
	  if(DHT11_Read_Data(&temperature,&humidity) == 0)
		{
			sprintf((char *)buf1,":%d",temperature);
			sprintf((char *)buf2,":%d%%RH",humidity);
			OLED_ShowStr(32,0,buf1,2);
			OLED_ShowStr(32,2,buf2,2);
		}
		
//=======================（光照，声音，有害气体）三个传感器数据的获取，通过ADC规则通道查询获取========		
    for(i=0,value1=0,value2=0,value3=0;i<30;)
		{
			value1+=ADC_Value[i++];	
			value2+=ADC_Value[i++];
			value3+=ADC_Value[i++];
		}			
		Lighting=(4096-value1/10)*100/4096; //亮度
		temp1=(float)(value3/10)*(3.3/4096);//空气质量
		value1=0;
		value2=0;
		value3=0;
		
		if(temp1<0.9)
			{
				n=temp1/0.001;
				Gas = n*0.012;
				printf("一氧化碳浓度:%dppm\n",Gas);
			}
			else
			{
				Gas = ((temp1-0.9)/0.1)*100;
				Gas =Gas+9;
				printf("一氧化碳浓度:%dppm\n",Gas);
			}
		sprintf((char *)buf3,":%d%%",Lighting);
		sprintf((char *)buf5,":%d   ",Gas);
		OLED_ShowStr(32,4,buf3,2);//OLED显示亮度
		OLED_ShowStr(64,6,buf5,2);//OLED显示空气质量
			
//===========================================================蜂鸣器报警===================================
		if(Gas<60)
		{
			Buzzer=Buzzer_ON;
		}		
    else
		{
			Buzzer=Buzzer_OFF;
		}			
			
//===========================================================超声波模块===================================			
		//Get_distance();
		//sprintf((char *)buf6,":%.1fCM",Distance);
		//OLED_ShowStr(64,6,buf6,2);//显示距离
		//if(Distance<20)
		//	LED = LED_ON;
		//else LED = LED_OFF;
//=============================================上传数据到OneNet服务器（可以不用管）=================================			
   	delay_ms(10);
		timeCount ++;
		if(timeCount >= 300)	//发送间隔3S
		{	
			timeCount = 0;
			OneNet_SendData();	//发送数据
			ESP8266_Clear();    //清空缓存
		}
		
//==============================================接受OneNet服务器发来的数据（可以不用管）===============================			
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
		{
			OneNet_RevPro(dataPtr);
		}

		

	}	
}



















