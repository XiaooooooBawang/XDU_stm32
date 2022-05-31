#include<stm32f4xx.h>

void TimerInit(void)
{
    RCC->APB1ENR |=1<<1; //使能 TIM3
    TIM3->CR1 = 1<<7; //配置为自重载预载使能、边沿对齐、向上计数工作模式
    TIM3->ARR = 11999; //配置 PWM 周期为 1.5 秒
    TIM3->PSC = 1999;
    TIM3->CCMR2 = (TIM3->CCMR2 & ~(0xFF00)) | 0x6800;//CCMR2是对应CH3/4的，配置 CH4 为 PWM1 输出模式
    TIM3->CCER |= 1<<12; //使能比较通道 4 作为输出，第12位对应CH4
    TIM3->CCR4 = 8000; //CH4 占空比为 67%，CCR4对应CH4
    TIM3->CR1 |= 1<<0; //开启 TIM3
}

void PwmOutInit(void)
{   //PC9 ------> TIM3_CH4
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC
    //设置 PC9 为 AF 复用、推挽输出、无上拉无下拉、中速
    GPIOC->MODER = (GPIOC->MODER & ~(0x3<<18)) | (0x2<<18);
    GPIOC->OTYPER &= ~(1<<9);
    GPIOC->PUPDR &= ~(0x3<<18);
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR & ~(0x3<<18)) | (1<<18);
    /*
     * 设置 AFRH 寄存器，将 PC9 复用为 TIM3 的 CH4 输出，PC9->AF02
     * 复用功能寄存器4位为1小组，分别是AFL0~AFL7、AFH8~AFH15。
     * AFL0~AFL7 和GPIOx0~GPIOx7对应，AFH8~AFH15和GPIOx8~GPIOx15对应。
     * STM32底层没有AFRH和AFRL寄存器，底层是封装成一个数组AFR[2]，ARF[0]代表AFRL寄存器，AFR[1]代码AFRH寄存器。
     */
    GPIOC->AFR[1] = (GPIOC->AFR[1] & ~(0xF<<4)) | (1<<5);
}

int main(){
    PwmOutInit();
    TimerInit();
    while (1);
}
