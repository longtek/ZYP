#include "config.h"
void TargetInit(void)
{
    int i;          
/*1. �趨ϵͳʱ��*/
	U8  key;
	U32 mpll_val=0;
    #if ADS10   
    __rt_lib_init(0,0); //for ADS 1.0
 
    #endif
			
	//ChangeMPllValue((mpll_val>>14)&0x1ff, (mpll_val>>5)&0x3f, mpll_val&3);
	//ChangeClockDivider(key,12,12);   

    
/*�˿ڳ�ʼ��*/
  	Port_Init();

/* ���ڳ�ʼ��*/
   // Delay(0);    
    Uart_Init(0,115200);
    Uart_Select(0);
    Uart_SendString("\nhello world!\n");
    CalcBusClk();    
	Bluetooth_Serial_Init(0,9600);
    BToothCS(TRUE);
    /* LEDָʾ�Ƴ�ʼ��  */
    led_Init();
    Led1_On();
 }