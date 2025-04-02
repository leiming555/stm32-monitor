#include "led.h"
#include "Servo.h"

extern u8 curtain;


//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PE端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 //LED0-->PB.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
 GPIO_SetBits(GPIOA,GPIO_Pin_6);						 //PB.5 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	    		 //LED1-->PE.5 端口配置, 推挽输出
 GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOC,GPIO_Pin_13); 						 //PE.5 输出高 
}

/**
   * @brief 状态翻转
   * @param 
   * @retval 
*/
void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_6) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_6);					//则设置PA1引脚为高电平
	}
	else													//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_6);					//则设置PA1引脚为低电平
	}
}

/**
   * @brief 状态翻转
   * @param 
   * @retval 
*/
void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);					//则设置PA1引脚为高电平
	}
	else													//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);					//则设置PA1引脚为低电平
	}
}






extern u8 mode;



void mode_deal(void)
{
	switch(mode)
	{
		case 0:
					 LED1=1;     
					 break;
		
		case 1:
					  CURTAINON();
		        curtain=1;//打开窗帘
					 break;
		
		case 2:
		       CURTAINOFF();//关窗帘
					 curtain=0;
		        break;
		
		case 3:
					      //蓝灯
					 break;
		
		case 4:
					     //绿灯
					 break;
		
		case 5:
					     //白灯
					 break;
		
		case 6:
			      LED1=0;
					  //开灯
					 break;
		
		case 7:
			 //  MOTOR_STOP();
		       break;
		
		
		
		default:break;
	}
}
















