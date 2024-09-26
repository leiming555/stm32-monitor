#include "oled.h"
#include "delay.h"
#include "codetab.h"

void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	/*STM32F103C8T6芯片的硬件I2C: PB6 -- SCL; PB7 -- SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//I2C必须开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_DeInit(I2C1);//使用I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x30;//主机的I2C地址,随便写的
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;//400K

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
}

void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//寄存器地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//发送数据
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2C1, ENABLE);//关闭I2C1总线
}

void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	I2C_Configuration();
	delay_ms(100); //这里的延时很重要
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}
//y0:  y方向初始坐标(0-7)
//q:   进度条值(0-51)
void OLED_progress_bar(unsigned char y0,unsigned char q)
{
	unsigned int j=0;
	unsigned char x,y;
	
	switch(q)
	{
		case 0:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage0[j++]);
							}	
						}break;
		case 1:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage1[j++]);
							}	
						}break;
	  case 2:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage2[j++]);
							}	
						}break;
		case 3:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage3[j++]);
							}	
						}break;
	   case 4:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage4[j++]);
							}	
						}break;
		case 5:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage5[j++]);
							}	
						}break;
	  case 6:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage6[j++]);
							}	
						}break;
		case 7:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage7[j++]);
							}	
						}break;
		case 8:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage8[j++]);
							}	
						}break;
		case 9:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage9[j++]);
							}	
						}break;
	  case 10:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage10[j++]);
							}	
						}break;
		case 11:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage11[j++]);
							}	
						}break;
		case 12:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage12[j++]);
							}	
						}break;
		case 13:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage13[j++]);
							}	
						}break;
	  case 14:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage14[j++]);
							}	
						}break;
		case 15:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage15[j++]);
							}	
						}break;
    case 16:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage16[j++]);
							}	
						}break;
	  case 17:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage17[j++]);
							}	
						}break;
		case 18:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage18[j++]);
							}	
						}break;
		case 19:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage19[j++]);
							}	
						}break;
		case 20:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage20[j++]);
							}	
						}break;
	  case 21:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage21[j++]);
							}	
						}break;
		case 22:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage22[j++]);
							}	
						}break;	
    case 23:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage23[j++]);
							}	
						}break;
	  case 24:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage24[j++]);
							}	
						}break;
		case 25:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage25[j++]);
							}	
						}break;	
    case 26:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage26[j++]);
							}	
						}break;
		case 27:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage27[j++]);
							}	
						}break;
	  case 28:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage28[j++]);
							}	
						}break;
		case 29:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage29[j++]);
							}	
						}break;
	   case 30:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage30[j++]);
							}	
						}break;
		case 31:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage31[j++]);
							}	
						}break;
	  case 32:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage32[j++]);
							}	
						}break;
		case 33:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage33[j++]);
							}	
						}break;
		case 34:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage34[j++]);
							}	
						}break;
		case 35:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage35[j++]);
							}	
						}break;
	  case 36:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage36[j++]);
							}	
						}break;
		case 37:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage37[j++]);
							}	
						}break;
		case 38:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage38[j++]);
							}	
						}break;
		case 39:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage39[j++]);
							}	
						}break;
	  case 40:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage40[j++]);
							}	
						}break;
		case 41:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage41[j++]);
							}	
						}break;
    case 42:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage42[j++]);
							}	
						}break;
	  case 43:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage43[j++]);
							}	
						}break;
		case 44:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage44[j++]);
							}	
						}break;
		case 45:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage45[j++]);
							}	
						}break;
		case 46:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage46[j++]);
							}	
						}break;
	  case 47:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage47[j++]);
							}	
						}break;
		case 48:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage48[j++]);
							}	
						}break;	
    case 49:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage49[j++]);
							}	
						}break;
	  case 50:
						for(y=y0;y-y0<2;y++)
						{
							OLED_SetPos(0,y);
							for(x=0;x<128;x++)
							{
								WriteDat(gImage50[j++]);
							}	
						}break;						
  }
}


void show(void)
{

	OLED_CLS();//OLED清屏
	OLED_ShowCN(0,0,2); OLED_ShowCN(16,0,4);OLED_ShowCN(65,0,39);//温度
	OLED_ShowCN(0,2,3); OLED_ShowCN(16,2,4);//湿度
	OLED_ShowCN(0,4,5); OLED_ShowCN(16,4,6);//光照
	OLED_ShowCN(0,6,40); OLED_ShowCN(16,6,41);OLED_ShowCN(32,6,42);OLED_ShowCN(48,6,43);//空气质量检测

	
}
