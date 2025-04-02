#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init();
}

void Servo_SetAngle(float Angle)
{
	PWM_SetCompare1(Angle / 180 * 2000 + 500);
}
void CURTAINON(void)
{

Servo_SetAngle(30);

}
void CURTAINOFF(void)
{

Servo_SetAngle(60);

}