#include<stm32f4xx.h>
void LedInit() 
{
    /*
     * 初始化LED，对PC12配置相关寄存器，设置 PC12 为推挽低速无上下拉输出管脚
     */
	RCC->AHB1ENR |= (0x1<<2);//使能 GPIOC 时钟，外设时钟使能寄存器AHB1ENR第2位赋1使能
	GPIOC->MODER = (GPIOC->MODER & ~(0x3<<24))|(0x1<<24);//模式寄存器MODER第25、24位赋01，配PC12为通用输出模式
	GPIOC->OTYPER &= ~(0x1<<12);//输出类型寄存器OTYPER第12位赋0，配PC12为推挽输出类型
	GPIOC->OSPEEDR &= ~(0x3<<24);//输出速率寄存器OSPEEDR第25、24位赋00，配PC12为低速输出
	GPIOC->PUPDR &= ~(0x3<<24);//上拉下拉寄存器PUPDR第25、24位赋00，配PC5为无上拉上拉
	GPIOC->ODR &= ~(0x1<<12); //数据输出寄存器ODR第12位赋0，PC12输出低电平，该灯亮
}
void LedSw() 
{
    /*
     * 控制 PC12 电平翻转
     */
	GPIOC->ODR ^= (0x1<<12);//ODR第12位与1异或运算实现该输出数据取反操作
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
}
void BtnInit()
{   //同GPIO实验中的BtnInit（）
    RCC->AHB1ENR |= 1<<2;
    GPIOC->MODER &= ~(0x3<<26);
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(0x3<<26))|(0x1<<26);
}
int BtnGet() {
    //同GPIO实验中的BtnGet（）
    return (GPIOC->IDR >> 13)& 0x1;
}

static int disp_bits[]={0x800, 0xF08, 0x420, 0x600, 0x308, 0x210, 0x010, 0xF00, 0x000, 0x200};
static int number[2][11]={2,1,0,0,9,1,9,0,0,7,5,
                          0,1,2,3,4,5,6,7,8,9,10};
static int liushuideng[]={0xF30, 0xF28, 0xF18, 0xE38, 0xD38, 0xB38};//流水灯的电平组合


void SegDisp( int num ) 
{
    /*
     * 数码管显示数字
     */
	if (0<=num && num<=9)//对于有效数值，根据该数值所对应的电平组合分别对GPIOC中的相应位进行置位或清零操作
		GPIOC->ODR = (GPIOC->ODR & ~0xF38)|disp_bits[num]; 
	else //对于无效数值，将所有相应位进行置位操作，使数码管无显示
		GPIOC->ODR |= 0xF38; 
}
void Liushuideng_Disp( int index )
{
    /*
     * 数码管显示流水灯
     */
    GPIOC->ODR = (GPIOC->ODR & ~0xF38)|liushuideng[index];    //根据该数值所对应的电平组合分别对GPIOC中的相应位进行置位或清零操作
}
int main() 
{
    int i,indexi,indexj,len[3]={11,10,6};
    int btn_previous, btn_current;
    btn_previous=1;
    indexi=0;
    LedInit();//小数点led初始化
    SegInit();//七段管初始化
    BtnInit();//按钮B1初始化
    while(1)
    {
        /*
         * 先进行各引脚的初始化工作。一开始是滚动显示学号模式，每个数字显示后会循环延时约0.5秒，
         * 在这期间会循环读取PC13电平，若检测到 PC13 的输入从高电平变为低电平，改变数码管显示内容，
         * 实现改变显示模式的功能，三种模式根据按钮按下的次数按顺序改变。
         * 同时在这0.5秒延时后会翻转一次PC12的值，使小数点达到呼吸灯效果。如此循环。
         */
        for(indexj=0; indexj<len[indexi%3]; indexj++)
        {
            if(indexi%3<2)//第一第二种模式显示数字
                SegDisp(number[indexi%3][indexj]);//显示规定行索引的数组的内容
            else//第三种模式显示流水灯
                Liushuideng_Disp(indexj);//显示流水灯
            for(i=0;i<400000;i++) //延时约0.5秒，同时监听按钮
            {
                btn_current = BtnGet();
                if (!btn_current)
                {
                    if (btn_previous)
                    {
                        indexi ++;//按钮按下后改变输出数组的行索引，即改变要输出的内容，改变显示模式
                        indexj=-1; //从第0位开始显示，因为是在外层的for中indexj先自加再执行，所以赋值-1
                    }
                }
                btn_previous = btn_current;
            }
            LedSw();//小数点亮灭
        }
    }
}
