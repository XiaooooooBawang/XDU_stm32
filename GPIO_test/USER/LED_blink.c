#include<stm32f4xx.h>
void LedInit(void)
{
    /*
     * 初始化LED，对PA5配置相关寄存器，设置 PA5 为通用推挽输出、高速
     */
    //（使用任何外设都要使能对应时钟，因为32为了低功耗，所有外设时钟都是disable）
	RCC->AHB1ENR |= 1<<0; //使能 GPIOA 时钟，外设时钟使能寄存器AHB1ENR第0位赋1使能
	GPIOA->MODER = (GPIOA->MODER & ~(0x3<<10))|(0x1<<10); //模式寄存器MODER第11、10位赋01，配PA5为通用输出模式
	GPIOA->OTYPER &=~(0x1<<5);//输出类型寄存器OTYPER第5位赋0，配PA5为推挽输出类型
	GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(0x3<<10))|(0x2<<10);//输出速率寄存器OSPEEDR第11、10位赋10，配PA5为高速输出
	GPIOA->ODR |= 1<<5; //数据输出寄存器ODR第5位赋1，PA5输出高电平
}
void LedSw()
{
    /*
     * 控制 PA5 电平翻转
     */
	GPIOA->ODR ^= 1<<5;//ODR第5位与1异或运算实现该输出数据取反操作
}
void BtnInit()
{
    /*
     * 初始化B1按钮，将 PC13 配置为数字输入并上拉
     */
	RCC->AHB1ENR |= 1<<2;//使能 GPIOC 时钟，外设时钟使能寄存器AHB1ENR第2位赋1使能
	GPIOC->MODER &= ~(0x3<<26);//模式寄存器MODER第27、26位赋00，配PA5为通用输入模式
	GPIOC->PUPDR = (GPIOC->PUPDR & ~(0x3<<26))|(0x1<<26); //上拉下拉寄存器PUPDR第27、26位赋01，配PA5为上拉
}
int BtnGet() {
    /*
     * 对 PC13 电平的读取
     */
	return (GPIOC->IDR >> 13)& 0x1;//若为高电平则返回1表示按钮没按下，若为低电平则返回0表示按钮按下
}
void LedFlash(int on) 
{
    /*
     * 控制 PA5 电平输出
     */
	if(on)
		GPIOA->ODR |= 1<<5;//on==1让led亮，赋1输出高电平
	else 
		GPIOA ->ODR &= ~(1<<5); //on==0让led灭，赋0输出低电平
}
void Delay(void)
{
    /*
     * 延时函数
     */
	unsigned int i;
	for ( i=4000000; i>0; i-- );//空循环,从C程序编译结果反汇编可知循环一次需要执行 4 指令周期，故计算可得循环次数约为 4000000
}
void GPIO_design1()
{
    /*
     * GPIO应用1
     */
    while(1)
    {
        LedFlash(1); //灯亮
        Delay();         //延时1s
        LedFlash(0);  //灯灭
        Delay();        //延时1s
    }
}
int btn_previous, btn_current;
void GPIO_design2()
{
    /*
     * GPIO应用2
     */
    btn_previous =1;//初始时PC13输入是1
    while(1)     //循环读取电平，监听按钮状态
    {
        btn_current = BtnGet();  //对当前 PC13 电平的读取
        if (!btn_current)      //若为0则按钮按下了
        {
            if (btn_previous) //若上一刻PC13输入值为1，则表示PC13输入从高变低电平
            {
                LedSw();  //PA5输出取反
            }
        }
        btn_previous = btn_current; //动态更新现在为上一刻
    }
}
int main( )
{
	LedInit(); //led初始化
	BtnInit(); //按钮B1初始化
	//GPIO_design1();   //GPIO应用1
    GPIO_design2();    //GPIO应用2
}
