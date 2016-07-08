#ifndef _CANSPI_H_
#define _CANSPI_H_
void CAN_IO_CS_Init( void );  
void CAN_RW_Start(void);
unsigned char  Spi_Read(void );
void Spi_Write(unsigned char Data); 
void SendCMDCAN( unsigned char CMD );
void CAN_Reset(void);
void CAN_WriteBits( unsigned char address, unsigned char data, unsigned char mask );
unsigned char CAN_ReadStatus(void);
void CAN_Write( unsigned char address, unsigned char value);
unsigned char CAN_Read(unsigned char address);
void CAN_SRead(unsigned char address,unsigned char *pdata,unsigned char nlength);
void CAN_Swrite(unsigned char address,unsigned char *pdata,unsigned char nlength);
#endif