#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "delay.h"
#include "OLED.h"
#include "LED.h"
#include "dht11.h"
#include "usart.h"
#include "bh1750.h"
#include "timer.h"
#include "beep.h"
#include "Key.h"
#include "extix.h"
#include "esp8266.h"
#include "onenet.h"
#include "MqttKit.h"
#include "ASRPRO.h"
#include "Servo.h"


volatile u8 mode=0;
u8 humidityH;	  //湿度整数部分
u8 humidityL;	  //湿度小数部分
u8 temperatureH;   //温度整数部分
u8 temperatureL;   //温度小数部分
float Light; //光照度
uint8_t LED_Status = 0; //PA4LED的状态

extern char oledBuf[20];


u8 curtain;
u8 alarmFlag; //是否报警的标志
u8 alarm_is_free = 10;//报警器是否被手动操作，如果被手动操作即设置为0

char PUB_BUF[256];//上传数据的buf（温度，湿度，光照强度,LED和蜂鸣器）

const char *devSubTopic[] = {"/mysmarthomeLM666/sub"}; // 设备订阅命令地址，MQTTX发送命令到服务器，stm32接收命令
const char devPubTopic[] = "/mysmarthomeLM666/pub"; // 设备上行数据地址，stm32传感器向服务器传输数据并使用MQTTX接收

u8 ESP8266_INIT_OK = 0;//esp8266初始化完成标志

int main(void)
{
	unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
	
	delay_init();
	
	OLED_Init();
	Servo_Init();
	Usart1_Init(115200); // 传感器串口
	Usart2_Init(115200); // 8266通讯串口
	
	LED_Init();
	ASRPRO_Init(); 
	//DHT11_Init();
	
	//BH1750_Init();
	
	// TIM2_Int_Init(2499, 7199);// 中断执行时间250ms,主频72MHz
	TIM3_Int_Init(2499, 7199);
	
	OLED_ShowString(1,1,"   STM32+8266");
	
	BEEP_Init();
	
	EXTIX_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	OLED_ShowString(1,1,"NVIC_PriorityGroup_2");
	ESP8266_Init();
	OLED_ShowString(1,1,"   Hardware Init");
	UsartPrintf(USART2, "Hardware Init \r\n");
	
	while(OneNet_DevLink() == 1)  // 循环连接服务器
	{delay_ms(500);}
	
	BEEP = 0;
	delay_ms(250); // 鸣叫表示接入成功
	BEEP = 1;
	
	OneNet_Subscribe(devSubTopic, 1); // 参数1：
	
	
	while(1)
	{
		
		  mode = RxFlag_deal();
      mode_deal();

		if(timeCount % 40 == 0) // timeCount每25ms加一次，timeCount加40次就是1000ms，就是1秒
		{
			// 温湿度传感器获取数据
			DHT11_Read_Data(&humidityH, &humidityL,&temperatureH,&temperatureL);
			UsartPrintf(USART_DEBUG,"温度：%d.%d  湿度：%d.%d", temperatureH, temperatureL, humidityH, humidityL);
			
		
			
			if(alarm_is_free == 10)
			{
				if(humidityH > 70 || temperatureH > 40 )
				{
					alarmFlag = 1;
				}
				else
				{
					alarmFlag = 0;
				}
			}
			if(alarm_is_free < 10)
			{
				alarm_is_free++;
			}
			
		sprintf(oledBuf,"Temp:%d.%d",temperatureH,temperatureL); //将内容写入到数组中
		OLED_ShowString(2,1,oledBuf);
		OLED_ShowString(2,11,"C"); //单位
		
		sprintf(oledBuf,"Humi:%d.%d",humidityH,humidityL); //将内容写入到数组中
		OLED_ShowString(3,1,oledBuf);
		OLED_ShowString(3,11,"%");//单位
		
	
			
		}
		
		if(++timeCount >= 200) // 五秒发送间隔
		{
			// 返回PA4的LED状态
			LED_Status = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6);
			
			UsartPrintf(USART_DEBUG, "OneNet_Publish \r\n");
			
			sprintf(PUB_BUF, "{\"Temp\":%d.%d, \"Hum\":%d.%d, \"curtain\":%d, \"LED\":%d, \"BEEP\":%d}",
			temperatureH,temperatureL,humidityH,humidityL,curtain,LED_Status?0:1,alarmFlag);// 使用alarmFlag描述蜂鸣器状态
			// LED_Status?0:1 如果LED_Status返回值是0，则输出1，如果是1，则输出0
			
			OneNet_Publish(devPubTopic, PUB_BUF);
			timeCount = 0;
			ESP8266_Clear();
		}
		 // 检查有没有下发指令
		dataPtr = ESP8266_GetIPD(3); // 15ms执行一次
		// 用来检查ESP8266有没有收到信息，如果有返回值就说明收到信息，没有就是没收到
		if(dataPtr != NULL)
		
			OneNet_RevPro(dataPtr);
		
		delay_ms(10);
		
	}
}


