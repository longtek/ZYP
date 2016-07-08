#include "CAN.h"
#include "def1.h"
#include "2440lib.h"
#include "2440slib.h"
#include "option.h"
#include "CANSPI.h"

void CAN_SetBandRate(CanBandRate bandrate,int IsBackNormal)
{
    U8 value=0;
    U8 ReadBackCNT=0;
    CAN_Write(MCP2510REG_CANCTRL,MODE_CONFIG);
    while(ReadBackCNT<8)
    {
        value=(CAN_Read(MCP2510REG_CANSTAT)&0xe0);
        if(value==MODE_CONFIG)
        {
            break;
        }
        ReadBackCNT++;
    }
    if(ReadBackCNT==8)
    {
         CAN_Reset();
         CAN_Write(MCP2510REG_CANCTRL, MODE_CONFIG);
         Delay(150);
         value=(CAN_Read(MCP2510REG_CANCTRL)&0xe0);    
     }
     switch(bandrate)
     {
         case BandRate_10kbps:
              CAN_Write(CNF1,0x31);
              CAN_Write(CNF2,0xb0);
              CAN_Write(CNF3,0x06);
              break;
         case BandRate_125kbps:
		      CAN_Write(CNF1, SJW1|BRP4);	//Synchronization Jump Width Length =1 TQ
		      CAN_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		      CAN_Write(CNF3, SEG4);// Phase Seg 2 = 4
		      break;
	     case BandRate_250kbps:
		      CAN_Write(CNF1, SJW1|BRP2);	//Synchronization Jump Width Length =1 TQ
		      CAN_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		      CAN_Write(CNF3, SEG4);// Phase Seg 2 = 4
		      break;
	     case BandRate_500kbps:
		      CAN_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		      CAN_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		      CAN_Write(CNF3, SEG4);// Phase Seg 2 = 4
		      break;
	     case BandRate_1Mbps:
		      CAN_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		      CAN_Write(CNF2, BTLMODE_CNF3|(SEG3<<3)|SEG2); // Phase Seg 1 = 2, Prop Seg = 3
		      CAN_Write(CNF3, SEG2);// Phase Seg 2 = 1
		      break;
     }
     if(IsBackNormal==TRUE)
     {
         CAN_Write(CLKCTRL, MODE_NORMAL | CLKEN | CLK8);
     }

}
/****************************************************************************
【function information】 read the ID of CAN,
       referrence:  address of CAN 's regester,
                    can_id is the ID 's value of return;
       
TRUE，表示是扩展ID(29位)
FALSE，表示非扩展ID(11位)
****************************************************************************/
int CAN_Read_ID(U8 address,U32 * can_id)
{
     U32 tbufdata;
     unsigned char *p=(unsigned char *)&tbufdata;
     CAN_SRead(address,p,4);
     *can_id=(tbufdata<<3)|((tbufdata>>13)&0x7);
     *can_id&=0x7ff;
     if((p[MCP2510LREG_SIDL]&TXB_EXIDE_M)==TXB_EXIDE_M)
     {
        *can_id = (*can_id<<2) | (p[MCP2510LREG_SIDL] & 0x03);
		*can_id <<= 16;
		*can_id |= tbufdata>>16;
		return TRUE;
     }
     return FALSE;
}
/***********************************************************\
*	read data of CAN have received;						*
*	reference: nbuffer  is the number of receive buffer	*
*			can_id  is the ID's value of returnning							*
*			rxRTR is no or yes RXRTR						*
*			data is the value of received						*
*			dlc is the data of length code							*
*	return 												*
*		TRUE，表示是扩展总线						*
*		FALSE，表示非扩展总线						*
\***********************************************************/
int Read_CAN_Buffer(U8 nbuffer,int* rxRTR,U32 *can_id,U8*data,U8*dlc)
{
   U8 mcp_addr=(nbuffer<<4)+0x31,ctrl;
   int IsExt;
   IsExt= CAN_Read_ID(mcp_addr,can_id);
   ctrl = CAN_Read(mcp_addr-1);
   *dlc = CAN_Read(mcp_addr+4);
   if(ctrl&0x08)
   {
      *rxRTR=TRUE;
   }
   else
   {
      *rxRTR = FALSE;
   }
   *dlc&=DLC_MASK;
   CAN_SRead(mcp_addr+5,data,8);
   return IsExt;
}
void Write_Can_ID(U8 address,U32 can_id,int IsExt)
{
    U32 tbufdata;
    if(IsExt)
    {
       can_id&=0x1fffffff;
       tbufdata=can_id&0xffff;
       tbufdata<<=16;
       tbufdata|=(can_id>>(18-5)&(~0x1f));
       tbufdata |= TXB_EXIDE_M;
    }
    else{
		  can_id&=0x7ff;	//11位
		  tbufdata= (can_id>>3)|((can_id&0x7)<<13);
		}
	CAN_Swrite(address,(unsigned char*)&tbufdata,4);
}

void Write_Can_Buffer( U8 nbuffer, int ext, U32 can_id, int rxRTR, U8* data,U8 dlc )
{
	U8 mcp_addr = (nbuffer<<4) + 0x31;
	CAN_Swrite(mcp_addr+5, data, dlc );  // write data bytes
	Write_Can_ID( mcp_addr, can_id,ext);  // write CAN id
	if (rxRTR)
		dlc |= RTR_MASK;  // if RTR set bit in byte
	CAN_Write((mcp_addr+4), dlc);            // write the RTR and DLC
}
void Write_Data_To_Can(U32 id,U8*pdata,unsigned char dlc,int IsExt,int rxRTR)
{
   unsigned char err;
   static int ntxbuffer=0;
   Write_Can_Buffer(ntxbuffer,IsExt,id,rxRTR,pdata,dlc);
   switch(ntxbuffer)
   {
      case 0:
          CAN_WriteBits(TXB0CTRL,(TXB_TXREQ_M|TXB_TXP10_M),0xff);
          do{err=CAN_Read(TXB0CTRL);}
          while((err&0x08));
         // if((err&0x70)!=0)
          ntxbuffer=1;
          break;
      case 1:
            CAN_WriteBits(TXB1CTRL, (TXB_TXREQ_M|TXB_TXP10_M), 0xff);
            do { err = CAN_Read(TXB1CTRL) ; }
		    while( (err &0x08)==0x08 )  ;
		    ntxbuffer=2;
		    break;
      case 2:
           CAN_WriteBits(TXB2CTRL, (TXB_TXREQ_M|TXB_TXP10_M), 0xff);
		    do { err = CAN_Read(TXB2CTRL) ; }
		    while( (err &0x08)==0x08 )  ;
		    //if( (err &0x70) != 0 )  Uart_Printf( "  Can Send Err = 0x%x\n", err  );
		    ntxbuffer=0;
		    break;
	}	    		    
}
int Can_Poll()//which receive buffer
{
	if( CAN_ReadStatus()&RX0INT )
		return 0;
	
	if( CAN_ReadStatus()&RX1INT )
		return 1;

	return -1;
}
int Read_Data_From_Can(int n,U32 *id,U8 *pdata,U8 *dlc,int *rxRTR,int*isExt)
{
    U8 byte;
    byte=CAN_Read(CANINTF);
    if(n==0)
    {
        if(byte & RX0INT)
        {
           *isExt=Read_CAN_Buffer(n+3, rxRTR, id, pdata, dlc);
           CAN_WriteBits(CANINTF, (U8)(~(RX0INT)), RX0INT); // Clear interrupt
           return TRUE;
        }
        return FALSE;
    }
    else if(n ==1 )
	{
		if(byte & RX1INT)
		{
			*isExt=Read_CAN_Buffer(n+4, rxRTR, id, pdata, dlc);
			CAN_WriteBits(CANINTF, (U8)(~(RX1INT)), RX1INT); // Clear interrupt
			return TRUE ;
		}
		return FALSE;
	}
}
void Can_Setup(void)
{
    CAN_WriteBits(RXB0CTRL, (RXB_BUKT+RXB_RX_STD), 0xFF);
    CAN_WriteBits(RXB1CTRL, RXB_RX_STD, 0xFF);
}
void CAN_Init()
{
    unsigned char i,j,a;
    CanBandRate bandrate=BandRate_125kbps;    
    CAN_IO_CS_Init();
    CAN_Reset();
    CAN_SetBandRate(bandrate,FALSE);
    CAN_Write(CANINTE,NO_IE);
    Write_Can_ID(RXM0SIDH,0x7ff,0);
    Write_Can_ID(RXM1SIDH,0x7ff,0);
    
    Write_Can_ID(RXF0SIDH,0,0);
    Write_Can_ID(RXF1SIDH,0,0);
    Write_Can_ID(RXF2SIDH,0,0);
    Write_Can_ID(RXF3SIDH,0,0);
    Write_Can_ID(RXF4SIDH,0,0);
    Write_Can_ID(RXF5SIDH,0x789,0);
    
    //CAN_Write(CLKCTRL, MODE_LOOPBACK| CLKEN | CLK8);//回环模式
    //如果不能用两台设备联机实验的话，可以选择回环模式
	CAN_Write(CLKCTRL, MODE_NORMAL| CLKEN | CLK8);//标准模式
    a = TXB0CTRL;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 14; j++) {
			CAN_Write(a, 0);
			a++;
	        }
       	a += 2; // We did not clear CANSTAT or CANCTRL
	}
	CAN_Write(RXB0CTRL, 0);
	CAN_Write(RXB1CTRL, 0);
	CAN_Write(BFPCTRL, 0x3C);
	CAN_Write(CANINTE, RX0IE|RX1IE);
}
void GetData_FromCan(U32 *can_id,U8 *can_data,int *isdata)
{	
    int i=-2;
    int temp;
    unsigned char dlc;
    int rxRTR,isExt;
	if((i=Can_Poll())>-1 )	
	{
	   *isdata=1;
	   for( temp=0; temp<8; temp++)  can_data[temp] = 0 ;
	   temp = Read_Data_From_Can(i, can_id, can_data, &dlc, &rxRTR, &isExt);	
	}
}






