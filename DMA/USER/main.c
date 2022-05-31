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
void UartDmaTxEn()
{
    /*
     * UartDmaTxEn 函数使能 USART2 的 DMA 发。
     */
    RCC->AHB1ENR |= (1<<21); //使能 DMA1 时钟
    USART2->CR3 |= (1<<7); //使能 UART2 的 DMAT
    DMA1_Stream6->CR = 0;//寄存器重置,全部清0。
    DMA1_Stream6->PAR = (int) &USART2->DR;//USART2->DR数据寄存器的地址作为外设地址
    DMA1_Stream6->CR = (4<<25)|(3<<16)|(1<<10)|(1<<6);
    //选择通道4，优先级非常高，存储器采用字节传输（00就是字节传输，所以不需要位操作），地址自增，数据传输方向为存储器到外设
}
void UartDmaTx( char * tx_buf, int tx_len )
{
    /*
     * UartDmaTx 函数启动 USART2 的 DMA 发送。
     */
    DMA1_Stream6->CR &= ~1;            //先disable，解除后续的寄存器的写保护
    DMA1_Stream6->M0AR = (int)tx_buf;  //发送数据的地址
    DMA1_Stream6->NDTR = tx_len;       //发送数据的宽度
    DMA1_Stream6->CR |= 1;             //enable
}
int UartDmaTxSts()
{
    /*
     * UartDmaTxSts 函数返回 DMA 发送是否完成。
     */
    if(!(DMA1->HISR & (1<<21)))
        return 0;      //没发送就返回0使while轮询
    DMA1->HIFCR |= 1<<21;  //发送成功，将HISR第21位流通道6清0
    return 1;
}
void UartDmaRxEn()
{
    /*
     * UartDmaRxEn 函数使能 USART2 的 DMA 收。
     */
    RCC->AHB1ENR |= (1<<21); //使能 DMA1 时钟
    USART2->CR3 |= (1<<6); //使能 UART2 的 DMAR
    DMA1_Stream5->CR = 0;//寄存器重置,全部清0。
    DMA1_Stream5->PAR = (int)&USART2->DR;//USART2->DR数据寄存器的地址作为外设源地址
    DMA1_Stream5->CR = (4<<25) | (3<<16) | (1<<10);
    //选择通道4，优先级非常高，外设采用字节传输（00是字节传输，不需位操作），地址自增，数据传输方向为外设到存储器（00是外设到存储器，不需位操作）
}
void UartDmaRx( char * rx_buf, int rx_len )
{
    /*
     * 启动 USART2 的 DMA 接收
     */
    DMA1_Stream5->CR &= ~1;             //先disable，解除后续的寄存器的写保护
    DMA1_Stream5->M0AR = (int) rx_buf;   //接收数据的地址
    DMA1_Stream5->NDTR = rx_len;       //接收数据的宽度
    DMA1_Stream5->CR |= 1;              //enable
}
int UartDmaRxSts()
{
    /*
     * UartDmaRxSts 函数返回 DMA 接收是否完成。
     */
    if(!(DMA1->HISR & (1<<11)))
        return 0;       //没接收到就返回0使while轮询
    DMA1->HIFCR |= 1<<11;   //接收成功，将HISR第11位流通道5清0
    return 1;
}
int main(void)
{
    /*
     * 使能时钟，初始化 GPIO、USART1、DMA1，并反复执行接收回发操作。
     */
    char buf[8]; //定义数据缓冲
    UartInit();
    UartDmaTxEn();
    UartDmaRxEn();
    while (1) {
        UartDmaRx(buf, 8);  //数据宽度是8
        while (!UartDmaRxSts());
        UartDmaTx(buf, 8);
        while (!UartDmaTxSts());
    }
}
