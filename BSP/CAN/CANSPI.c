#include "CANSPI.h"
#include "2440addr.h"
#include "def1.h"
#define DELAY_TIME		500

#define CAN_CS_OUT      ( rGPBCON = rGPBCON & (~(3<<16)) | (1<<16) )      //GPB7    
#define CAN_CS_H        ( rGPBDAT = rGPBDAT | (1<<8) )    
#define CAN_CS_L        ( rGPBDAT = rGPBDAT & (~(1<<8))  ) 

#define CAN_SI_OUT		( rGPGCON = rGPGCON & (~(3<<12)) | (1<<12) )		//GPE12
#define CAN_SI_H		( rGPGDAT = rGPGDAT | (1<<6) )
#define CAN_SI_L		( rGPGDAT = rGPGDAT & (~(1<<6)) )

#define CAN_SCK_OUT		( rGPGCON = rGPGCON & (~(3<<14)) | (1<<14) )		//GPE13
#define CAN_SCK_H		( rGPGDAT = rGPGDAT | (1<<7) )
#define CAN_SCK_L		( rGPGDAT = rGPGDAT & (~(1<<7)) )

#define CAN_SO_IN		( rGPGCON = rGPGCON & (~(3<<10)) | (0<<10) )		//GPG5
#define CAN_SO_GET		( rGPGDAT & (1<<5) )	
#define CAN_SO_PULLUP		( rGPGUP = rGPGUP & (~(1<<5)) )
#define CAN_SO_DISPULLUP		( rGPGUP = rGPGUP | (1<<5) )

#define CAN_INT_IN		( rGPGCON = rGPGCON & (~(3<<4)) )		//GPG13
#define CAN_INT_GET		( rGPGDAT & (1<<2) )
#define CAN_INT_PULLUP		( rGPGUP = rGPGUP & (~(1<<2)) )
/********************** MCP2510 Instruction *********************************/
#define MCP2510INSTR_RESET		0xc0		//复位为缺省状态，并设定为配置模式
#define MCP2510INSTR_READ		0x03		//从寄存器中读出数据
#define MCP2510INSTR_WRITE		0x02		//向寄存器写入数据
#define MCP2510INSTR_RTS		0x80		//启动一个或多个发送缓冲器的报文发送
#define MCP2510INSTR_RDSTAT		0xa0		//读取状态
#define MCP2510INSTR_BITMDFY	0x05		//位修改
//***************************************************************************
/****************************************************************************   
【功能说明】SPI接口IO片选初始化   
****************************************************************************/   
void CAN_IO_CS_Init( void )    
{
   U16 k;   
   CAN_CS_OUT;   
   CAN_SI_OUT;   
   CAN_SCK_OUT;   
   CAN_SO_IN;   
   CAN_SO_PULLUP ;      //允许上拉    
   //MCP2510_SO_DISPULLUP ;     //禁止上拉    
   CAN_SI_L ;       //SI put 0     
   CAN_SCK_L ;      //SCK put 0    
   for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns    
   CAN_CS_H ;           // unselect the MCP2510    
   for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns    
} 
/****************************************************************************  
【功能说明】SPI接口读写开始，片选有效  
****************************************************************************/
void CAN_RW_Start(void)
{
   U16 k;
   CAN_SI_L;    //SI put 0 
   CAN_SCK_L;   //SCK put 0
   for(k=0;k<=DELAY_TIME;k++);   //延时至少300ns
   CAN_CS_L;    //abled;
   for(k=0;k<=DELAY_TIME;k++); //延时至少300ns;
}


/****************************************************************************  
【功能说明】SPI接口写入数据  
****************************************************************************/   
void Spi_Write(U8 Data)
{
     U8 m;
     U16 k;
     for(m=0;m<8;m++)
     {
         if((Data&0x80)==0x80)
         {
            CAN_SI_H;
         }
         else
         {
            CAN_SI_L;
         }
         for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns
         CAN_SCK_H;
         Data=Data<<1;
         CAN_SCK_L;
          for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns
     }
}
U8 Spi_Read()
{
   U8 m;
   U16 k=0;
   U8 data=0;
   for(m=0;m<8;m++)
   {
      CAN_SCK_H;
      for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns
      data = data<<1;
         if(CAN_SO_GET != 0 )   
            data |= 0x01 ;   
        else   
            data &= 0xfe; 
       for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns
        CAN_SCK_L ;     //SCK put 0   
       for (k = 0; k <= DELAY_TIME; k++);  //延时至少300ns    
   }
   return (data);
}
/****************************************************************************
【功能说明】 Send Command to CAN via SPI 
****************************************************************************/
void SendCMDCAN( U8 CMD )
{
   CAN_RW_Start() ;		//Initial IO port and CS is select
   Spi_Write( CMD );
   CAN_CS_H ;			// Deselect 
}
/****************************************************************************
【功能说明】软件复位
****************************************************************************/
void CAN_Reset()
{
	CAN_RW_Start() ;
	Spi_Write( MCP2510INSTR_RESET );
	CAN_CS_H ;
}
/****************************************************************************
【功能说明】向MCP2510指定地址写入一个字节
****************************************************************************/
void CAN_Write( U8 address, U8 value)
{
	CAN_RW_Start() ;

	Spi_Write(MCP2510INSTR_WRITE);
	Spi_Write( address );
	Spi_Write( value );

	CAN_CS_H ;
}

void CAN_WriteBits( U8 address, U8 data, U8 mask )
{
	CAN_RW_Start() ;

	Spi_Write( MCP2510INSTR_BITMDFY );
	Spi_Write( address);
	Spi_Write( mask);
	Spi_Write( data);

	CAN_CS_H ;
}
/****************************************************************************
【功能说明】              Read often used status
//Status 	 7    	6    	5    	4    	3    	2  	1	0
//		|	|	|	|	|	|	|	|									
//		|	|	|	|	|	|	|	|___CANINTF.RX0IF
//		|	|	|	|	|	|	|_______CANINTF.RX1IF
//		|	|	|	|	|	|___________TXB0CTRL.TXREQ
//		|	|	|	|	|_______________CANINTF.TX0IF
//		|	|	|	|___________________TXB1CTRL.TXREQ
//		|	|	|_______________________CANINTF.TX1IF
//		|	|___________________________TXB2CTRL.TXREQ
//		|_______________________________CANINTF.TX2IF
****************************************************************************/
unsigned char CAN_ReadStatus()
{
    unsigned char result;
    CAN_RW_Start();
    Spi_Write(MCP2510INSTR_RDSTAT);
    result=Spi_Read();
    Spi_Write(0);
    CAN_CS_H;
    return result;
}
/****************************************************************************
【fuction instruction】 read a byte of data from the addreass  
****************************************************************************/

unsigned char CAN_Read(U8 address)
{
    unsigned char result;
    CAN_RW_Start();
    Spi_Write(MCP2510INSTR_READ);
    Spi_Write(address);
    result =Spi_Read();
    CAN_CS_H;
    return result;
}
/****************************************************************************
【function instruction 】read a	series of data from  the address of CAN to the specifc buffer 
and the length of data is specific. 
****************************************************************************/

void CAN_SRead(U8 address,unsigned char *pdata,U8 nlength)
{
      int i;
      CAN_RW_Start();
      Spi_Write(MCP2510INSTR_READ);
      Spi_Write(address);
      for(i=0;i<nlength;i++)
      {
          *pdata=Spi_Read();
          pdata++;
      }
      CAN_CS_H;
}
/****************************************************************************
【function instruction】write a series of data to the specific address of CAN

****************************************************************************/
void CAN_Swrite(U8 address,unsigned char *pdata,U8 nlength)
{
     int i;
     CAN_RW_Start();
     Spi_Write(MCP2510INSTR_WRITE);
     Spi_Write((unsigned char)address);
     
     for(i=0;i<nlength;i++)
     {
         Spi_Write((unsigned char)*pdata);
         pdata++;
     }
     CAN_CS_H;    
}



   