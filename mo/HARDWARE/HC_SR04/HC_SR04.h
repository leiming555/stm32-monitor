#ifndef  _HC_SR04_
#define  _HC_SR04_

#include "stm32f10x.h"
#include "sys.h"

#define Trig PBout(15)

void HC_SR04_Init(u16 arr,u16 psc);
void Get_distance(void);

#endif

