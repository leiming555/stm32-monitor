#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED PCout(13)	// LED接口
#define Buzzer PAout(5)	
#define Buzzer_ON  1
#define Buzzer_OFF 0
#define LED_OFF		1
#define LED_ON		0

void LED_Init(void);//初始化
void Buzzer_Init(void);//蜂鸣器初始化
			    
#endif
