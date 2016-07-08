#include "led.h"
#include "2440addr.h"

void led_Init(void)
{
	rGPFCON &=0xffffff00 ;
	rGPFCON |= (1 << 0)|(1<<2)|(1<<4) ;
	rGPFUP  &=  ~(1 << 0)|~(1<<1)|~(1<<2) ;
	rGPFDAT |= (1 << 0)|(1<<1)|(1<<2) ;
}








