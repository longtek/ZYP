#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#define Bluetooth 	0x02

void put_byte(unsigned char chd);
void Select_Device(char Device);
char bluetooth_getdata(void);
void bluetooth_serial_Init(int pclk,int baud);
void bluetooth_SendByte(char ch);
void bluetooth_Send(char *str);
void bluetooth_printf(char *fmt,...);
#endif