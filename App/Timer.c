#include "config.h"
U8 IntCnt;
extern U32 m_pclk;
//#define OSTickISR() Timer0_ISR()
/*********************************************************************************************************
Timer_Isr()
********************************************************************************************************/
void  Timer0_ISR(void)
{
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif 
    OS_ENTER_CRITICAL();
	rSRCPND1 =rSRCPND1|( 1 << 10);
	rINTPND1 = rINTPND1;
	OS_EXIT_CRITICAL();
	IntCnt++;
	OSTimeTick();
}

/*********************************************************************************************************
Initial Timer0 use for ucos time tick
********************************************************************************************************/
void Timer0Init(void)
{
	// 定时器设置	
	rTCON = rTCON & (~0xf) ;			// clear manual update bit, stop Timer0
	rTCFG0 	&= 0xffffff00;				// set Timer 0&1 prescaler 0
	rTCFG0 |= 15;						//prescaler = 15+1

	rTCFG1 	&= 0xfffffff0;				//set Timer 0 MUX 1/4
	rTCFG1  |= 0x00000001;				//set Timer 0 MUX 1/4
    rTCNTB0 = (m_pclk/ (4 *15* OS_TICKS_PER_SEC)) - 1;
    Uart_Printf("m_pclk=%d\n",m_pclk);
    
    rTCON = rTCON & (~0xf) |0x02;   // updata 		
	rTCON = rTCON & (~0xf) |0x09; 	// star
 }

/*********************************************************************************************************
system IsrInit
********************************************************************************************************/
extern void OSTickISR(void);
void ISRInit(void)
{
	//设置中断控制器
	rPRIORITY = 0x00000000;		// 使用默认的固定的优先级
	rINTMOD1 = 0x00000000;		// 所有中断均为IRQ中断
	pISR_TIMER0= (uint32)OSTickISR;
	rINTMSK1 &= ~(1<<10);			// 打开TIMER0中断允许
	IntCnt=0;
 }