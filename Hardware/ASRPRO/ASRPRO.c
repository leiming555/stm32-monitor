#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "OLED.h"
#include "LED.h"

uint8_t ASRPRO_RxFlag=0;  //中断接收标志位

void ASRPRO_Init(void)        //串口1初始化
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA9引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA10引脚初始化为上拉输入
	
	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;					//定义结构体变量
	USART_InitStructure.USART_BaudRate = 9600;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位
	USART_Init(USART1, &USART_InitStructure);				//将结构体变量交给USART_Init，配置USART1
	
	/*中断输出配置*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启串口接收数据的中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//选择配置NVIC的USART1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);							//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*USART使能*/
	USART_Cmd(USART1, ENABLE);	
}

void USART1_IRQHandler(void)    //利用中断接收语音助手的数据改变标志位
{

	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)    //判断是否是USART1的接收事件触发的中断
	{
		uint8_t RxData = USART_ReceiveData(USART1);           //读取数据寄存器，存放在接收的数据变量
		
			if (RxData==0x01)      //0x01 由语音模块发来
			{
				ASRPRO_RxFlag = 1;
			}
			else if (RxData==0x02)  //
			{
				ASRPRO_RxFlag = 2;
			}
			else if (RxData==0x03)  //
			{
				ASRPRO_RxFlag = 3;
			}
			else if (RxData==0x04)  //
			{
				ASRPRO_RxFlag = 4;
			}
			else if (RxData==0x05)  //
			{
				ASRPRO_RxFlag = 5;
			}
			else if (RxData==0x06)  //
			{
				ASRPRO_RxFlag = 6;
			}
			else if (RxData==0x07)  //
			{
				ASRPRO_RxFlag = 7;
			}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);     
	}
}

extern u8 mode;

u8 RxFlag_deal(void)
{
	 switch(ASRPRO_RxFlag)
			{
				case 1:
							mode=1;
							ASRPRO_RxFlag=0;
				      break;
				
				case 2:
							mode=2;
							ASRPRO_RxFlag=0;
				      break;
				
				case 3:
							mode=3;
							ASRPRO_RxFlag=0;
				      break;
				
				case 4:
							mode=4;
							ASRPRO_RxFlag=0;
				      break;
				
				case 5:
							mode=5;
							ASRPRO_RxFlag=0;
				      break;
				
				case 6:
							mode=6;
				      OLED_Clear();
							ASRPRO_RxFlag=0;
				      break;
				
				case 7:                            //关灯
							mode=0;
				      OLED_Clear();
							ASRPRO_RxFlag=0;
				      break;
				
				default:break;
			}
    return mode;
}

