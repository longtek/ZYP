#include "bluetooth.h"
#include "option.h"
#include "2440addr.h"  
#include "2440lib.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#define TXD0READY (1 << 2)
#define RXD0READY (1 << 0)
extern U32  m_pclk;
void bluetooth_serial_Init(int pclk,int baud)
{
     if(pclk==0)
            pclk= m_pclk;    	
     rUFCON2=0x0;    
     rUMCON2=0x0;   
     rULCON2=0x3; 
     rUCON2=0x245; 
     rUBRDIV2=((int)(pclk/(baud*16))-1);
     Delay(10);
}
void Select_Device(char Device)
{
	rGPHDAT	=(rGPHDAT&0xfffffffc)|(Device);																	
}
void put_byte(unsigned  char chd)
{	
	rUTXH2 = chd;
	while(!(rUTRSTAT2 & TXD0READY)) ;//等待上个字符发送完毕
}
char bluetooth_getdata(void)
{
     unsigned char get_data=0;
     while(!(rUTRSTAT2 & 0x1));
     {   
         get_data=RdURXH2();
         Uart_Printf("0x%04x\n",get_data);
         put_byte(get_data);
         put_byte('\n');
         return  get_data;
     }
     return 0;
}
void bluetooth_SendByte(char ch)
{
   if(ch=='\n')
   {
       while(!(rUTRSTAT2 &0x2))
       WrUTXH2('\r'); 
   }
   while(!(rUTRSTAT2 &0x2));
   WrUTXH2(ch);
}
void bluetooth_Send(char *str)
{
    while(*str)
    {
       bluetooth_SendByte(*str++);
    }
}
void bluetooth_printf(char *fmt,...)
{
    va_list ap;
    char string[256];
    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    bluetooth_Send(string);
    va_end(ap); 
}
