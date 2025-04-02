#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY0 PBin(1) //LED KEY
#define KEY1 PBin(11) //BEEP KEY
  

#define KEY0_PRES	1		//KEY0  
#define KEY1_PRES	2		//KEY1 

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数					    
#endif
