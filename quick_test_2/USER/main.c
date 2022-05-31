#include "stm32f4xx.h"
void UartInit(void)
{
    RCC->AHB1ENR |= 1; //使能 GPIOA 时钟
    RCC->APB1ENR |= 1<<17; //使能 USART2 时钟
    //配置 PA2、PA3 为复用、推挽输出、高速
    GPIOA->MODER = (GPIOA->MODER & ~(0xF<<4))|(0xA<<4);
    GPIOA->OTYPER &= ~(0xF<<4);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(0xF<<4))|(0xA<<4);
    //设置 AFRL 寄存器，PA2 和 PA3 复用模式为 AF7，分别为 U2TX 和 U2RX1
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF<<8))|(0x77<<8);
    USART2->BRR = 1667;
    USART2->CR1 = (1<<13)|(1<<3)|(1<<2); //使能接收和发送功能
}
int UartRx( char* prxd )
{
    if(!(USART2->SR & (0x1<<5)))
        return 0;
    *prxd = (char)USART2->DR;
    return 1;
}
int UartTx( char txd )
{
    if(!(USART2->SR & (0x1<<7)))
        return 0;
    USART2->DR = txd;
    return 1;
}
void UartRxIntEn(void)
{
    USART2->CR1 |= 1<<5;
    NVIC->ISER[1] |= 1<<6;
}
void USART2_IRQHandler(void)
{   char ch;
    while(!UartRx(&ch));
    if(ch>='1'&&ch<='5'){
        while(!UartTx(ch));
        TIM4->PSC = 1000*(ch-'0')-1; //设定预分频数
    }
    else{
        while(!UartTx('E'));
    }
}
char lf[2] = {'L','R'};
int lf_index=0;
void SegInit()
{   //配置 PC3、PC4、PC5、PC8、PC9、PC10、PC11 为低速开漏无上下拉输出
    RCC->AHB1ENR |= 0x4; //使能 GPIOC 时钟
    GPIOC->MODER = (GPIOC->MODER & ~0xFF0FC0)|0x550540; //输出管脚
    GPIOC->OTYPER |= 0xF38; //开漏输出
    GPIOC->OSPEEDR &= ~ 0xFF0FC0; //低速
    GPIOC->PUPDR &= ~ 0xFF0FC0; //无上拉无下拉
    GPIOC->ODR |= 0xF38; //置高（不亮）
    while(!UartTx(lf[lf_index]));
}
static int disp_bits[]={0x800, 0xF08, 0x420, 0x600, 0x308, 0x210, 0x010, 0xF00, 0x000,
                        0x200};
void SegDisp( unsigned char num )
{
    if (num<=9)
        GPIOC->ODR = (GPIOC->ODR & ~0xF38)|disp_bits[num];
    else
        GPIOC->ODR |= 0xF38;
}
void BtnInit()
{
    RCC->AHB1ENR |= 1<<2;//使能 GPIOC 时钟
    GPIOC->MODER &= ~(0x3<<26);//配 PC13 为输入
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(0x3<<26))|(0x1<<26); //配 PC13 上拉
}
void ExIntInit(void)
{
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC 时钟
    RCC->APB2ENR |= 1<<14; //使能 SYSCFG 时钟
    //EXTI13 信号源为 PC13
    SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~(0xF<<4))| (2<<4);
    EXTI->IMR |= 1<<13; //取消对 EXTI13 信号线的屏蔽
    EXTI->FTSR |= 1<<13; //设定 EXTI13 中断触发信号为下降沿
    NVIC->ISER[1] |= 1<<8; //在 NVIC 中设置 EXTI15_10 中断使能
}

void EXTI15_10_IRQHandler(void)
{
    EXTI->PR |= 1<<13; //清除当前已经产生的 EXTI13 中断
    lf_index=1-lf_index;
    while(!UartTx(lf[lf_index]));
}
void TimerInit(void)
{
    RCC->APB1ENR |=1<<2; //使能 TIM4
    TIM4->CR1 = 1<<7; //配置为自重载预载使能、边沿对齐、向上计数工作模式
    TIM4->ARR = 15999; //设定计数器分频数
    TIM4->PSC = 999; //设定预分频数
    TIM4->DIER |= 1<<0; //设置中断更新使能
    NVIC->ISER[0] |= 1<<30; //在 NVIC 中设置 TIM4 中断使能
    TIM4->CR1 |= 1<<0; //开启 TIM4
}
int disp_index=0;
void TIM4_IRQHandler(void)
{
    TIM4->SR &= ~(1<<0); //清除当前中断事件;
    SegDisp(disp_index%10);
    if(!lf_index) {
        disp_index++;
    }else {
        disp_index--;
    }
    if(disp_index<0){
        disp_index+=10;
    }
}
int main(){
    UartInit();
    UartRxIntEn();
    SegInit();
    BtnInit();
    TimerInit();
    ExIntInit();
    while (1);
}
