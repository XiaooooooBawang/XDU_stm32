#include<stm32f4xx.h>
void DmaInit()
{
    /*
     * DmaInit 函数使能 DMA2 时钟。
     */
    RCC->AHB1ENR |= 1<<22;
    DMA2_Stream4->CR = (7<<25)|(1<<10)|(1<<9)|(2<<6);//通道 7，外设和存储器均采用字节传输，地址均递增，方向为存储器至存储器。
}
void MemCpy(char * sbuf, char * dbuf, int len)
{
    /*
     * MemCpy 函数实现两个存储区的数据复制。
     */
    DMA2_Stream4->CR &= ~1; //先disable DMA，解除后续的寄存器的写保护
    DMA2_Stream4->PAR = (int)sbuf;//源存储区首地址
    DMA2_Stream4->M0AR = (int)dbuf;//目的存储区首地址
    DMA2_Stream4->NDTR = len;//拷贝字节数
    DMA2_Stream4->CR |= (1<<0); //enable DMA
    while(!(DMA2->HISR & (1<<5)));//传输完成
    DMA2->HIFCR |= 1<<5;  //将HISR第5位流通道4清0
}
int main(void)
{
    /*
     * 使能 DMA2，将一定义字符串复制到别一个存储区。
     */
    char mytext[16];
    DmaInit();
    MemCpy("Hello world!",mytext,13);
    while(1);
}
