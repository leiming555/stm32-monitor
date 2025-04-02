#ifndef __BEEP_H
#define __BEEP_H	 

#include "sys.h"

#define BEEP PAout(7)	// PA7
//位带操作，在变量后写0或1，就可以直接输出高低电平
	

void BEEP_Init(void);//初始化

		 				    
#endif
