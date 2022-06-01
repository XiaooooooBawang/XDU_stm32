#include"stm32f4xx.h"
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
int UartTx( unsigned char txd )
{
    /*
     * 串口数据发送函数
     */
    if(!(USART2->SR & (0x1<<7)))
        return 0;
    USART2->DR = txd;
    return 1;
}
void AdcInit(void)
{
    /*
     * ADC 通道配置函数
     * 配置 ADC 参数：ADC1 转换通道采用内部温度传感器通道 ADC_IN18
     * 时钟源采用 HCLK，利用软件触发，单次转换模式，转换精度 8bit，数据对齐采用右对齐方式。
     */
    RCC->APB2ENR |= 1<<8; //使能 ADC1 的时钟
    ADC->CCR |= (1<<23); //使能内部温度传感器参考电源
    ADC1->SMPR1 |= 0x7<<24; //设置传感器通道18的采样时间为 480 周期
    ADC1->SQR1 &= ~(0xF<<20); //设置常规通道序列长度为 1 conversion
    ADC1->SQR3 = (ADC1->SQR3 & ~(0x1F)) | 18; //设置常规通道 1 为 ADC_IN 18
    ADC1->CR1 = (2<<24)|(1<<11); //常规转换为非连续模式，8bit，通道数为 1（bit 15:13=000->1个通道）。
    ADC1->CR2 = 1; //设置常规转换为单次模式（bit 2=0），右对齐（bit 11=0），非外部触发（bit 29:28=00）,使能 ADC（bit 1=1）。
}
void AdcRun()
{
    /*
     * AdcRun 函数启动 ADC。
     */
    ADC1->SR &= ~2;     //转换未完成
    ADC1->CR2 |= 1<<30; //开始 ADC 转换
}
int AdcGet( unsigned char *pdat )
{
    /*
     * AdcGet 函数获取 ADC 数据，转换完成时读取数据并返回成功，未完成时返回失败。
     */
    if(!(ADC1->SR & 2))
        return 0; //未完成转换，返回失败
    *pdat = (unsigned char)ADC1->DR; //转换完成，读取转换值
    return 1; //返回成功
}
void Delay(void)
{
    /*
     * 延时函数
     */
    unsigned int i;
    for ( i=4000000; i>0; i-- );//空循环,从C程序编译结果反汇编可知循环一次需要执行 4 指令周期，故计算可得循环次数约为 4000000
}
int main(void)
{
    unsigned char dat;
    UartInit();
    AdcInit();
    while(1)
    {
        AdcRun();//启动 ADC
        while(!AdcGet(&dat));//读取转换数据
        while(!UartTx(dat));//串口发送数据
        Delay(); //延时 1s。
    }
}
