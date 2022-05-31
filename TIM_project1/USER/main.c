#include<stm32f4xx.h>
void LedInit(void)
{
    /*
     * 初始化LED，对PC3配置相关寄存器，设置 PC3 为通用推挽输出、高速
     */
    //（使用任何外设都要使能对应时钟，因为32为了低功耗，所有外设时钟都是disable）
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC 时钟，外设时钟使能寄存器AHB1ENR第2位赋1使能
    GPIOC->MODER = (GPIOA->MODER & ~(0x3<<6))|(0x1<<6); //模式寄存器MODER第7、6位赋01，配PC3为通用输出模式
    GPIOC->OTYPER &=~(0x1<<3);//输出类型寄存器OTYPER第3位赋0，配PC3为推挽输出类型
    GPIOC->OSPEEDR = (GPIOA->OSPEEDR & ~(0x3<<6))|(0x2<<6);//输出速率寄存器OSPEEDR第7、6位赋10，配PC3为高速输出
    GPIOC->ODR |= 1<<3; //数据输出寄存器ODR第3位赋1，PC3输出高电平
}
void LedSw()
{
    /*
     * 控制 PA5 电平翻转
     */
    GPIOC->ODR ^= 1<<3;//ODR第3位与1异或运算实现该输出数据取反操作
}

void TimerInit(void)
{
    RCC->APB1ENR |=1<<1; //使能 TIM3
    TIM3->CR1 = 1<<7;//配置为自重载预载使能、边沿对齐、向上计数工作模式
    TIM3->ARR = 15999; //设定计数器分频数
    TIM3->PSC = 999; //设定预分频数
    TIM3->DIER |= 1<<0; //设置中断更新使能
    NVIC->ISER[0] |= 1<<29; //在 NVIC 中设置 TIM3 中断使能
    TIM3->CR1 |= 1<<0; //开启 TIM3
}

void TIM3_IRQHandler(void)
{
    TIM3->SR &= ~(1<<0); //清除当前中断事件;
    LedSw(); //LED 亮灭反转
}

int main(){
    LedInit(); //LED 初始化
    TimerInit(); //定时器初始化
    while(1);
}
