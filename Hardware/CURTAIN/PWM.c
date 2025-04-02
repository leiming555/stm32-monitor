#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
    // 开启 TIM4 和 GPIOB 的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 初始化 GPIOB 的 PB6 为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  // 使用 PB6
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 选择内部时钟作为 TIM4 的时钟源
    TIM_InternalClockConfig(TIM4);
    
    // 配置 TIM4 基本参数
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;   // 自动重装载寄存器 ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;     // 预分频器 PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    
    // 配置 TIM4 输出比较模式，使用通道1对应 PB6
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;  // CCR 初始值
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);  // 使用通道1
    
    // 使能 TIM4
    TIM_Cmd(TIM4, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare1(TIM4, Compare);
}
