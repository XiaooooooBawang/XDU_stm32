#include<stm32f4xx.h>
void UartInit(void)
{
    /*
     * USART初始化函数
     */
    RCC->AHB1ENR |= 1; //使能 GPIOA 时钟
    RCC->APB1ENR |= 1<<17; //使能 USART2 时钟
    //配置 PA2、PA3 为复用、推挽输出、高速
    GPIOA->MODER = (GPIOA->MODER & ~(0xF<<4))|(0xA<<4);
    GPIOA->OTYPER &= ~(0xF<<4);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(0xF<<4))|(0xA<<4);
    //设置 AFRL 寄存器，PA2 和 PA3 复用模式为 AF7，分别为 U2TX 和 U2RX
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF<<8))|(0x77<<8);
    USART2->BRR = 1667;//波特率9600
    USART2->CR1 = (1<<13)|(1<<3)|(1<<2); //使能接收和发送功能
}
int UartRx( char* prxd )
{
    /*
     * 串口数据接收函数
     */
    if(!(USART2->SR & (0x1<<5)))
        return 0;
    *prxd = (char)USART2->DR;
    return 1;
}
int UartTx( char txd )
{
    /*
     * 串口数据发送函数
     */
    if(!(USART2->SR & (0x1<<7)))
        return 0;
    USART2->DR = txd;
    return 1;
}
void BtnInit()
{
    //BtnInit 函数将 PC13 配置为数字输入并上拉。
    RCC->AHB1ENR |= 1<<2;//使能 GPIOC 时钟
    GPIOC->MODER &= ~(0x3<<26);//配 PC13 为输入
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(0x3<<26))|(0x1<<26); //配 PC13 上拉
}
void ExIntInit(void)
{
    /*
     * PC13触发EXTI中断
     */
    RCC->AHB1ENR |= 1<<2; //使能 GPIOC 时钟
    RCC->APB2ENR |= 1<<14; //使能 SYSCFG 时钟
    SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~(0xF<<4))| (2<<4);//EXTI13 信号源为 PC13
    EXTI->IMR |= 1<<13; //取消对 EXTI13 信号线的屏蔽
    EXTI->FTSR |= 1<<13; //设定 EXTI13 中断触发信号为下降沿
    NVIC->ISER[1] |= 1<<8; //在 NVIC 中设置 EXTI15_10 中断使能
}
int lf_index=0;
char lf_ch[2]={'L','R'};//显示顺序的变量
void EXTI15_10_IRQHandler(void)
{
    EXTI->PR |= 1<<13; //清除当前已经产生的 EXTI13 中断
    lf_index=1-lf_index;  //改变显示顺序
    while(!UartTx(lf_ch[lf_index])); //发送显示顺序到上位机
}
void SegInit()
{
    /*
     * 初始化七段数码管的驱动电路，主要完成使能 GPIOC时钟、
     * 配置 PC3、PC4、PC5、PC8、PC9、PC10、PC11 为低速开漏无上下拉输出并对各管脚置位。
     */
    RCC->AHB1ENR |= (0x1<<2);//使能 GPIOC 时钟，外设时钟使能寄存器AHB1ENR第2位赋1使能
    GPIOC->MODER = (GPIOC->MODER & ~0xFF0FC0)|0x550540; //模式寄存器MODER数码管各对应两位赋01，配为通用输出模式
    GPIOC->OTYPER |= 0xF38; //输出类型寄存器OTYPER数码管各对应位赋1，配为开漏输出类型
    GPIOC->OSPEEDR &= ~0xFF0FC0;//输出速率寄存器OSPEEDR数码管各对应两位赋00，配为低速输出
    GPIOC->PUPDR &= ~0xFF0FC0;//上拉下拉寄存器PUPDR数码管各对应两位赋00，配PC12为无上拉上拉
    GPIOC->ODR |= 0xF38; //数据输出寄存器ODR数码管各对应位赋1，PC12输出高电平，所有灯都不亮
    while(!UartTx(lf_ch[lf_index]));//一开始就发送显示顺序到上位机
}
static int disp_bits[]={0x800, 0xF08, 0x420, 0x600, 0x308, 0x210, 0x010, 0xF00, 0x000, 0x200};
void SegDisp(int num )
{
    /*
     * 数码管显示数字
     */
    if (0<=num && num<=9)//对于有效数值，根据该数值所对应的电平组合分别对GPIOC中的相应位进行置位或清零操作
        GPIOC->ODR = (GPIOC->ODR & ~0xF38)|disp_bits[num];
    else //对于无效数值，将所有相应位进行置位操作，使数码管无显示
        GPIOC->ODR |= 0xF38;
}
void TimerInit(void)
{
    /*
     * TIM3控制周期
     */
    RCC->APB1ENR |=1<<1; //使能 TIM3
    TIM3->CR1 = 1<<7; //配置为自重载预载使能、边沿对齐、向上计数工作模式
    TIM3->ARR = 15999; //设定计数器分频数
    TIM3->PSC = 999; //设定预分频数
    TIM3->DIER |= 1<<0; //设置中断更新使能
    NVIC->ISER[0] |= 1<<29; //在 NVIC 中设置 TIM3 中断使能
    TIM3->CR1 |= 1<<0; //开启 TIM3
}
int disp_index=0;
void TIM3_IRQHandler(void)
{
    /*
     * 在TIM3中断中循环显示或改变显示顺序
     */
    TIM3->SR &= ~(1<<0); //清除当前中断事件;
    if(!lf_index){
        //从左到右
        disp_index++;
    }
    else{
        //从右到左
        disp_index--;
    }
    if(disp_index<0){
        //防止从右到左减的时候出现负数，导致取余出问题
        disp_index+=10;
    }
    SegDisp(disp_index%10);
}
void UartRxIntEn(void)
{
    /*
     * 使能接收中断
     */
    USART2->CR1 |= 1<<5;
    NVIC->ISER[1] |= 1<<6;
}
void USART2_IRQHandler(void)
{
    /*
     * USART2中断服务函数，根据接收到的数字字符决定周期
     */
    switch ((char)USART2->DR) {
        case '1' :
            //1s
            TIM3->ARR = 15999; //设定计数器分频数
            TIM3->PSC = 999; //设定预分频数
            break;
        case '2' :
            //2s
            TIM3->ARR = 31999; //设定计数器分频数
            TIM3->PSC = 999; //设定预分频数
            break;
        case '3' :
            //3s
            TIM3->ARR = 47999; //设定计数器分频数
            TIM3->PSC = 999; //设定预分频数
            break;
        case '4' :
            //4s
            TIM3->ARR = 15999; //设定计数器分频数
            TIM3->PSC = 3999; //设定预分频数
            break;
        case '5' :
            //5s
            TIM3->ARR = 15999; //设定计数器分频数
            TIM3->PSC = 4999; //设定预分频数
            break;
        default:
            break;
    }
}
int main(void) {
    UartInit();
    UartRxIntEn();
    BtnInit();
    ExIntInit();
    SegInit();
    TimerInit();
    while (1);
}

