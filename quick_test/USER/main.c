#include<stm32f4xx.h>
void LedInit(void)
{//设置 PC10 为通用推挽输出、高速
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC 时钟;
    GPIOC->MODER = (GPIOC->MODER & ~(0x3<<20))|(0x1<<20); //配 PC10 为输出
    GPIOC->OTYPER &=~(0x1<<10);//配 PC10 推挽输出
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR & ~(0x3<<20))|(0x2<<20);//配 PC10 为高速
    GPIOC->ODR |= 1<<10; //PC10 输出高电平
}
void BtnInit()
{
    //BtnInit 函数将 PC13 配置为数字输入并上拉。
    RCC->AHB1ENR |= 1<<2;//使能 GPIOC 时钟
    GPIOC->MODER &= ~(0x3<<26);//配 PC13 为输入
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(0x3<<26))|(0x1<<26); //配 PC13 上拉
}
void LedSw()
{
    GPIOC->ODR ^= 1<<10;
}

void TimerInit(void)
{
    RCC->APB1ENR |=1<<1; //使能 TIM3
    TIM3->CR1 = 1<<7; //配置为自重载预载使能、边沿对齐、向上计数工作模式
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
void ExIntInit(void)
{
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC 时钟
    RCC->APB2ENR |= 1<<14; //使能 SYSCFG 时钟
    //EXTI13 信号源为 PC13
    SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~(0xF<<4))| (2<<4);
    EXTI->IMR |= 1<<13; //取消对 EXTI10 信号线的屏蔽
    EXTI->FTSR |= 1<<13; //设定 EXTI10 中断触发信号为下降沿
    NVIC->ISER[1] |= 1<<8; //在 NVIC 中设置 EXTI15_10 中断使能
}
int cnt=1;
void EXTI15_10_IRQHandler(void)
{
    EXTI->PR |= 1<<13; //清除当前已经产生的 EXTI13 中断
    cnt+=1;
    /*
     * 注意，ARR和PSC限位16位，所以分频数和预分频数不能太大，要看好
     * 因为ARR会比PSC大，更容易超过限位，所以改变周期的时候，尽量变PSC，而不是ARR
     */
    if(cnt%3==1){
        //1s
        TIM3->ARR = 15999; //设定计数器分频数
        TIM3->PSC = 999; //设定预分频数
    }
    else if(cnt%3==2){
        //3s
        TIM3->ARR = 15999; //设定计数器分频数
        TIM3->PSC = 2999; //设定预分频数
    }
    else if(cnt%3==0){
        //5s
        TIM3->ARR = 15999; //设定计数器分频数
        TIM3->PSC = 4999; //设定预分频数
    }
//
//    比较好的方法如下：
//
//    cnt+=2;
//    if(cnt>5)
//    {
//        cnt=1;
//    }
//    TIM3->PSC = 1000*cnt-1;
}
int main(){
    LedInit();
    BtnInit();
    TimerInit();
    ExIntInit();
    while(1);
}
