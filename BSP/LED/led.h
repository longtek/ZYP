#ifndef __LED_H__
#define __LED_H__

#include "2440addr.h"

#define Led0_Off()   (rGPFDAT |= (1 <<0))
#define Led0_On()    (rGPFDAT &= ~(1 <<0)) 
#define Led1_Off()   (rGPFDAT |= (1 <<1))
#define Led1_On()    (rGPFDAT &= ~(1 <<1)) 
#define Led2_Off()   (rGPFDAT |= (1 <<2))
#define Led2_On()    (rGPFDAT &= ~(1 <<2)) 

extern void led_Init(void);

#endif





