#ifndef DHT11_H
#define DHT11_H

#include "stm32f10x.h"


#include "iobit.h"
//IO方向设置
//IO方向设置
#define DHT11_IO_IN()  {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=8<<0;}
#define DHT11_IO_OUT() {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=3<<0;}
////IO操作函数											   
#define	DHT11_DQ_OUT PBout(0) //数据端口	PB0
#define	DHT11_DQ_IN  PBin(0)  //数据端口	PB0


u8 DHT11_Init(void);//初始化DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//读取温湿度
u8 DHT11_Read_Byte(void);//读出一个字节
u8 DHT11_Read_Bit(void);//读出一个位
u8 DHT11_Check(void);//检测是否存在DHT11
void DHT11_Rst(void);//复位DHT11   


void DHT11_Show_Data(void);
#endif
