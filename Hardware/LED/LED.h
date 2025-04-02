#ifndef __LED_H
#define __LED_H	 

#include "sys.h"

#define LED1 PAout(6)	// PA4
#define LED2 PCout(13)	// PC13	

void LED_Init(void);//初始化
void LED1_Turn(void);
void LED2_Turn(void);
void mode_deal(void);






#endif
