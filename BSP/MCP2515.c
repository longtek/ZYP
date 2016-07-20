//======================================================================
//  �ļ�����  MCP2515.c
//  ����������	ͨ��SPI�ӿڣ�����MCP2515
//	ά����¼��	2009-08-14	v1.0    
//				2009-10-28  v1.1    
//======================================================================
#include "def.h"
#include "option.h"
#include "2416addr.h"
#include "2416lib.h"
#include "2416slib.h" 
#include "MCP2515.h"
extern CanConfig canconfig;
extern float m_Rpm,m_Speed,m_Throttle;
extern U8 m_RpmIndex,m_OldRpmIndex;
extern U8 m_ToChange;
/****************************************************************************
MCP2515_CS		GPB7		output		( nSS0 )
MCP2515_SI		GPE12		output		( SPIMOSI0 )
MCP2515_SO		GPE11		input		( SPIMISO0 )
MCP2515_SCK		GPE13		output		( SPICLK0 )
MCP2515_INT		GPG0		input		( EINT8 )
****************************************************************************/
#define MCP2515_DEBUG    1
#define DELAY_TIME		800

#define MCP2515_CS_OUT       ( rGPLCON = rGPLCON & (~(3<<26)) | (1<<26) )        //GPL13
#define MCP2515_CS_PULLUP    ( rGPLUDP = rGPLUDP &(~(3<<26))|(2<<26)) 
#define MCP2515_CS_DISPULLUP ( rGPLUDP = rGPLUDP &(~(3<<26)))  
#define MCP2515_CS_H         ( rGPLDAT |= (1<<13))    
#define MCP2515_CS_L         ( rGPLDAT = rGPLDAT & (~(1<<13))) 

#define MCP2515_SI_OUT		( rGPECON = rGPECON & (~(3<<22)) | (1<<22) )		//GPE11
#define MCP2515_SI_PULLUP   ( rGPEUDP = rGPEUDP & (~(3<<22)))
#define MCP2515_SI_H		( rGPEDAT = rGPEDAT | (1<<11))
#define MCP2515_SI_L		( rGPEDAT = rGPEDAT & (~(1<<11)) )

#define MCP2515_SCK_OUT		( rGPECON = rGPECON & (~(3<<26)) | (1<<26) )		//GPE13
#define MCP2515_SCK_PULLUP  ( rGPEUDP = rGPEUDP & (~(1<<26)) )
#define MCP2515_SCK_H		( rGPEDAT = rGPEDAT | (1<<13) )
#define MCP2515_SCK_L		( rGPEDAT = rGPEDAT & (~(1<<13)) )

#define MCP2515_SO_IN		 ( rGPECON = rGPECON & (~(3<<24)) | (0<<24))		//GPE12
#define MCP2515_SO_GET		 ( rGPEDAT &= (1<<12))	
#define MCP2515_SO_DISPULLUP ( rGPEUDP = rGPEUDP & (~(3<<24)) )
#define MCP2515_SO_PULLUP	 ( rGPEUDP = rGPEUDP | (1<<24) )

	
/********************** MCP2515 Instruction *********************************/
#define MCP2515INSTR_RESET		0xc0		//��λΪȱʡ״̬�����趨Ϊ����ģʽ
#define MCP2515INSTR_READ		0x03		//�ӼĴ����ж�������
#define MCP2515INSTR_WRITE		0x02		//��Ĵ���д������
#define MCP2515INSTR_RTS		0x80		//����һ���������ͻ������ı��ķ���
#define MCP2515INSTR_RDSTAT		0xa0		//��ȡ״̬
#define MCP2515INSTR_BITMDFY	0x05		//λ�޸�
//***************************************************************************


/****************************************************************************   
������˵����SPI�ӿ�IOƬѡ��ʼ��   
****************************************************************************/   
static void MCP2515_IO_CS_Init( void )    
{
   U16 k;   
   MCP2515_CS_OUT ;  
  // MCP2515_CS_PULLUP; 
   MCP2515_SI_OUT ;   
   MCP2515_SCK_OUT ;   
   MCP2515_SO_IN ;   
   //MCP2515_SO_PULLUP ;      //��������    
   //MCP2515_SO_DISPULLUP ;     //��ֹ����    
   MCP2515_SI_L ;       //SI put 0     
   MCP2515_SCK_L ;      //SCK put 0    
   for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
   MCP2515_CS_H ;           // unselect the MCP2515    
   for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
}   
   
/****************************************************************************  
������˵����SPI�ӿڶ�д��ʼ��Ƭѡ��Ч  
****************************************************************************/   
static void MCP2515_RW_Start( void )    
{
   U16 k;   
   MCP2515_SI_L ;       //SI put 0    
   MCP2515_SCK_L ;      //SCK put 0    
   for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
   MCP2515_CS_L ;           // Select the MCP2515    
   for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
}   
   
/****************************************************************************  
������˵����SPI�ӿ�д������  
****************************************************************************/   
static void Spi_Write( U8 Data )    
{   
    U8 m ;
    U16 k;   
   
    for( m = 0; m < 8; m++ )   
    {   
        if( (Data&0x80)==0x80 )
        {  
            MCP2515_SI_H;       //SI put 1 
        }   
        else
        {   
            MCP2515_SI_L;       //SI put 0  
        }  
   
		for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
        MCP2515_SCK_H ;     //SCK put 1    
        Data = Data<<1 ;   
        MCP2515_SCK_L ;     //SCK put 0    
		for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
    }   
}   
   
/****************************************************************************  
������˵����SPI�ӿڶ�������  
****************************************************************************/   
static U8 Spi_Read( void)   
{   
    U8 m ;   
    U8 data = 0 ;
    U16 k;   
   
    for( m = 0; m < 8; m++ )   
    {   
        MCP2515_SCK_H ;     //SCK put 1    
		for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
        data = data<<1;  
        if( MCP2515_SO_GET != 0 )   
            data |= 0x01 ;   
        else   
            data &= 0xfe;   
   
		for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
        MCP2515_SCK_L ;     //SCK put 0    
		for (k = 0; k <= DELAY_TIME; k++);  //��ʱ����300ns    
    }   
   
    return (data);   
}   

/****************************************************************************
������˵���� Send Command to MCP2515 via SPI 
****************************************************************************/
static void SendCMDMCP2515( U8 CMD )
{
   MCP2515_RW_Start() ;		//Initial IO port and CS is select
   Spi_Write( CMD );
   MCP2515_CS_H ;			// Deselect the MCP2515
}

/****************************************************************************
������˵���������λMCP2515
****************************************************************************/
static void MCP2515_Reset()
{
	MCP2515_RW_Start() ;
	Spi_Write( MCP2515INSTR_RESET );
	MCP2515_CS_H ;
}

/****************************************************************************
������˵������MCP2515ָ����ַд��һ���ֽ�
****************************************************************************/
static void MCP2515_Write( U8 address, U8 value)
{
	MCP2515_RW_Start() ;

	Spi_Write(MCP2515INSTR_WRITE);
	Spi_Write( address );
	Spi_Write( value );

	MCP2515_CS_H ;
}

/****************************************************************************
������˵�����޸�ָ����ַ�Ĵ�����ĳЩλ
****************************************************************************/
static void MCP2515_WriteBits( U8 address, U8 data, U8 mask )
{
	MCP2515_RW_Start() ;

	Spi_Write( MCP2515INSTR_BITMDFY );
	Spi_Write( address);
	Spi_Write( mask);
	Spi_Write( data);

	MCP2515_CS_H ;
}

/****************************************************************************
������˵����              Read often used status
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
static unsigned char MCP2515_ReadStatus()
{
	unsigned char result;

	MCP2515_RW_Start() ;

	Spi_Write(MCP2515INSTR_RDSTAT);
	result = Spi_Read() ;
	Spi_Write( 0 ) ;		//�����ظ����
	MCP2515_CS_H ;
	 // if( MCP2515_DEBUG )		Uart_Printf( "StatusREG = 0x%x\n", result ) ;
	return result;
}

/****************************************************************************
������˵������MCP2515ָ����ַ�ж���һ���ֽ�
****************************************************************************/
static unsigned char MCP2515_Read( U8 address )
{
	unsigned char result;
	MCP2515_RW_Start() ;

	Spi_Write(MCP2515INSTR_READ) ;		//0x03
	Spi_Write( address ) ;
	result = Spi_Read() ;

	MCP2515_CS_H ;
	return result ;
}

/****************************************************************************
������˵�������ж�ȡMCP2515����	
****************************************************************************/
static void MCP2515_SRead( U8 address, unsigned char* pdata, U8 nlength )
{
	int i;

	MCP2515_RW_Start() ;
	Spi_Write(MCP2515INSTR_READ);
	Spi_Write( address );

	for (i=0; i<nlength; i++)
	{
		*pdata=Spi_Read();
		   //if( MCP2515_DEBUG )    Uart_Printf( "  0x%x\n", (unsigned char)*pdata ) ;
		pdata++;
	}
	MCP2515_CS_H ;
}


/****************************************************************************
������˵��������д��MCP2515����	
****************************************************************************/
static void MCP2515_Swrite( U8 address, unsigned char* pdata, U8 nlength)
{
	int i;
	MCP2515_RW_Start() ;

	Spi_Write(MCP2515INSTR_WRITE);
	Spi_Write((unsigned char)address);

	for (i=0; i < nlength; i++) 
	{
		Spi_Write( (unsigned char)*pdata );
		//if( MCP2515_DEBUG )    Uart_Printf( "0x%x\n", (unsigned char)*pdata ) ;
		pdata++;
	}
	MCP2515_CS_H ;
}

/****************************************************************************
������˵����
****************************************************************************/
static void MCP2515_SetBandRate(CanBandRate bandrate, int IsBackNormal)
{
	U8 value=0;
	U8 ReadBackCNT = 0;

	// Bit rate calculations.
	//
	//Input clock fre=16MHz
	// In this case, we'll use a speed of 125 kbit/s, 250 kbit/s, 500 kbit/s.
	// If we set the length of the propagation segment to 7 bit time quanta,
	// and we set both the phase segments to 4 quanta each,
	// one bit will be 1+7+4+4 = 16 quanta in length.
	//
	// setting the prescaler (BRP) to 0 => 500 kbit/s.
	// setting the prescaler (BRP) to 1 => 250 kbit/s.
	// setting the prescaler (BRP) to 3 => 125 kbit/s.
	//
	// If we set the length of the propagation segment to 3 bit time quanta,
	// and we set both the phase segments to 1 quanta each,
	// one bit will be 1+3+2+2 = 8 quanta in length.
	// setting the prescaler (BRP) to 0 => 1 Mbit/s.

	// Go into configuration mode
	MCP2515_Write(MCP2515REG_CANCTRL, MODE_CONFIG);

	if( MCP2515_DEBUG )  Uart_Printf( "MCP2515REG_CANCTRL =  0x%x\n", MCP2515_Read(MCP2515REG_CANCTRL) );

	while( ReadBackCNT<8 )
	{
		value = ( MCP2515_Read( MCP2515REG_CANSTAT ) & 0xe0 );
		if(value == MODE_CONFIG ){
			Uart_Printf( "ReadBackCNT = 0x%x\n", ReadBackCNT );
			break;
		}
		ReadBackCNT++ ;
		Uart_Printf( "ReadBackCNT = 0x%x,value=0x%x\n", ReadBackCNT,value );
	}
	
	if( ReadBackCNT == 8 ) 			//Set mcp2515's mode failed,redo it again
	{
		Uart_Printf( "Set config mode is failed! CANCTRL = 0x%x\n", value );
		MCP2515_Reset();
		MCP2515_Write(MCP2515REG_CANCTRL, MODE_CONFIG);		//redo to set mcp2515 mode
		Delay( 150 );
		value = ( MCP2515_Read(MCP2515REG_CANCTRL) & 0xe0 );	//read back mode from CANSTAT Register
		Uart_Printf( "Set is 0x%x , Read is 0x%x\n", MODE_CONFIG, value ) ;
	}
    //Uart_Printf( "\n bandrate =%d !\n",bandrate ) ;	
	switch(bandrate){
	case BandRate_10kbps:
		MCP2515_Write(CNF1, 0x31);	//10k	16TQ
		MCP2515_Write(CNF2, 0xb0);  //PS1=7 TQ  PSeg=1 TQ
		MCP2515_Write(CNF3, 0x06);  //PS2=7 TQ SYNC=1 TQ	
		//if( MCP2515_DEBUG )  Uart_Printf( "CNF1 =  0x%x\n", MCP2515_Read(CNF1) );
		//if( MCP2515_DEBUG )  Uart_Printf( "CNF2 =  0x%x\n", MCP2515_Read(CNF2) );
		//if( MCP2515_DEBUG )  Uart_Printf( "CNF3 =  0x%x\n", MCP2515_Read(CNF3) );
		break;
	case BandRate_125kbps:
		MCP2515_Write(CNF1, SJW1|BRP4);	//Synchronization Jump Width Length =1 TQ
		MCP2515_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2515_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_250kbps:
		MCP2515_Write(CNF1, SJW1|BRP2);	//Synchronization Jump Width Length =1 TQ
		MCP2515_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2515_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_500kbps:
		MCP2515_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		MCP2515_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2515_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_1Mbps:
		MCP2515_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		MCP2515_Write(CNF2, BTLMODE_CNF3|(SEG3<<3)|SEG2); // Phase Seg 1 = 2, Prop Seg = 3
		MCP2515_Write(CNF3, SEG2);// Phase Seg 2 = 1
		break;
	}

	if( IsBackNormal == TRUE  )
	{
		//Enable clock output
		MCP2515_Write(CLKCTRL, MODE_NORMAL | CLKEN | CLK8);
	}

}

/****************************************************************************
������˵������ȡMCP2515 CAN����ID
����: addressΪMCP2515�Ĵ�����ַ
	can_idΪ���ص�IDֵ
����ֵ
TRUE����ʾ����չID(29λ)
FALSE����ʾ����չID(11λ)
****************************************************************************/
static int MCP2515_Read_Can_ID( U8 address, U32* can_id)
{
	U32 tbufdata;
	unsigned char* p=(unsigned char*)&tbufdata;

	MCP2515_SRead(address, p, 4);
	*can_id = (tbufdata<<3)|((tbufdata>>13)&0x7);
	*can_id &= 0x7ff;

	if ( (p[MCP2515LREG_SIDL] & TXB_EXIDE_M) ==  TXB_EXIDE_M ) {
		*can_id = (*can_id<<2) | (p[MCP2515LREG_SIDL] & 0x03);
		*can_id <<= 16;
		*can_id |= tbufdata>>16;
		return TRUE;
	}
	return FALSE;
}

/***********************************************************\
*	��ȡMCP2515 ���յ�����							*
*	����: nbufferΪ�ڼ�������������Ϊ3����4	*
*			can_idΪ���ص�IDֵ							*
*			rxRTR��ʾ�Ƿ���RXRTR						*
*			data��ʾ��ȡ������						*
*			dlc��ʾdata length code							*
*	����ֵ												*
*		TRUE����ʾ����չ����						*
*		FALSE����ʾ����չ����						*
\***********************************************************/
static int MCP2515_Read_Can(U8 nbuffer, int* rxRTR, U32* can_id, U8* data , U8* dlc)
{

	U8 mcp_addr = (nbuffer<<4) + 0x31, ctrl;
	int IsExt;

	IsExt=MCP2515_Read_Can_ID( mcp_addr, can_id);
	ctrl=MCP2515_Read(mcp_addr-1);
	*dlc=MCP2515_Read( mcp_addr+4);
	if ((ctrl & 0x08)) {
		*rxRTR = TRUE;
	}
	else{
		*rxRTR = FALSE;
	}
	*dlc &= DLC_MASK;	
	//MCP2515_SRead(mcp_addr+5, data, *dlc);
	MCP2515_SRead(mcp_addr+5, data, 8); 
	return IsExt;
}


/***********************************************************\
*	д��MCP2515 ���͵�����							*
*	����: nbufferΪ�ڼ�������������Ϊ0��1��2	*
*			ext��ʾ�Ƿ�����չ����					*
*			can_idΪ���ص�IDֵ							*
*			rxRTR��ʾ�Ƿ���RXRTR						*
*			data��ʾ��ȡ������						*
*			dlc��ʾdata length code							*
*		FALSE����ʾ����չ����						*
\***********************************************************/
static void MCP2515_Write_Can( U8 nbuffer, int ext, U32 can_id, int rxRTR, U8* data,U8 dlc )
{
	U8 mcp_addr = (nbuffer<<4) + 0x31;
	MCP2515_Swrite(mcp_addr+5, data, dlc );  // write data bytes
	MCP2515_Write_Can_ID( mcp_addr, can_id,ext);  // write CAN id
	Uart_Printf("MCP2515_Write_Can_ID\n");
	if (rxRTR)
		dlc |= RTR_MASK;  // if RTR set bit in byte
	MCP2515_Write((mcp_addr+4), dlc);            // write the RTR and DLC
	Uart_Printf("dlc\n");
}

/*******************************************\
*	����MCP2515 CAN����ID				*
*	����: addressΪMCP2515�Ĵ�����ַ*
*			can_idΪ���õ�IDֵ			*
*			IsExt��ʾ�Ƿ�Ϊ��չID	*
\*******************************************/
static void MCP2515_Write_Can_ID(U8 address, U32 can_id, int IsExt)
{
	U32 tbufdata;

	if (IsExt) {
		can_id&=0x1fffffff;	//29λ
		tbufdata=can_id &0xffff;
		tbufdata<<=16;
		tbufdata|=(can_id>>(18-5)&(~0x1f));
		tbufdata |= TXB_EXIDE_M;
	}
	else{
		can_id&=0x7ff;	//11λ
		tbufdata= (can_id>>3)|((can_id&0x7)<<13);
	}
	MCP2515_Swrite(address, (unsigned char*)&tbufdata, 4);
}

/***********************************************************************************\
								��������
	����:
		data����������

	Note: ʹ������������ѭ�����ͣ�û������������Ч���
\***********************************************************************************/
static void Can_Write(U32 id, U8 *pdata, unsigned char dlc, int IsExt, int rxRTR)
{
	unsigned char err ;
	static int ntxbuffer=0;
	MCP2515_Write_Can(ntxbuffer, IsExt, id, rxRTR, pdata, dlc);

	switch(ntxbuffer){
	case 0:
		MCP2515_WriteBits(TXB0CTRL, (TXB_TXREQ_M|TXB_TXP10_M), 0xff) ;
		//do { err = MCP2515_Read(TXB0CTRL) ; }
		//while( (err &0x08)==0x08 )  ;
		if( (err &0x70) != 0 )  Uart_Printf( "  Can Send Err = 0x%x\n", err  );
		ntxbuffer=1;
		break;
	case 1:
		MCP2515_WriteBits(TXB1CTRL, (TXB_TXREQ_M|TXB_TXP10_M), 0xff) ;
		//do { err = MCP2515_Read(TXB1CTRL) ; }
		//while( (err &0x08)==0x08 )  ;
		if( (err &0x70) != 0 )  Uart_Printf( "  Can Send Err = 0x%x\n", err  );
		ntxbuffer=2;
		break;
	case 2:
		MCP2515_WriteBits(TXB2CTRL, (TXB_TXREQ_M|TXB_TXP10_M), 0xff) ;
		//do { err = MCP2515_Read(TXB2CTRL) ; }
		//while( (err &0x08)==0x08 )  ;
		if( (err &0x70) != 0 )  Uart_Printf( "  Can Send Err = 0x%x\n", err  );
		ntxbuffer=0;
		break;
	}

}


/***********************************************************************************\
								��ѯ�Ƿ��յ�����
	����ֵ:���û�����ݣ��򷵻�-1��
			���򣬷����յ����ݵĻ�������
	Note: ����������������յ����ݣ��򷵻ص�һ��������
\***********************************************************************************/
static int Can2515_Poll()
{
	if( MCP2515_ReadStatus()&RX0INT )
		return 0;
	
	if( MCP2515_ReadStatus()&RX1INT )
		return 1;

	return -1;
}

/****************************************************************************
������˵����
****************************************************************************/
static int Can_Read(int n, U32* id, U8 *pdata,  U8*dlc, int* rxRTR, int *isExt)
{
	U8 byte;
	byte = MCP2515_Read(CANINTF);

	if(n==0)
	{
		if(byte & RX0INT)
		{
			*isExt=MCP2515_Read_Can(n+3, rxRTR, id, pdata, dlc);
			MCP2515_WriteBits(CANINTF, (U8)(~(RX0INT)), RX0INT); // Clear interrupt
			return TRUE ;
		}
		Uart_Printf( "Error! 0 bytes is Read!!! CANINTF=0x%x\n", byte ) ;
		return FALSE;
	}
	else if(n ==1 )
	{
		if(byte & RX1INT)
		{
			*isExt=MCP2515_Read_Can(n+3, rxRTR, id, pdata, dlc);
			MCP2515_WriteBits(CANINTF, (U8)(~(RX1INT)), RX1INT); // Clear interrupt
			return TRUE ;
		}
		Uart_Printf( "0 bytes is Read!!! CANINTF=0x%x\n", byte ) ;
		return FALSE;
	}

	Uart_Printf( "Error! Receive channel=0x%x\n", n ) ;
	return FALSE;
}

/****************************************************************************
������˵����
****************************************************************************/
// Setup the CAN buffers used by the application.
// We currently use only one for reception and one for transmission.
// It is possible to use several to get a simple form of queue.
//
// We setup the unit to receive all CAN messages.
// As we only have at most 4 different messages to receive, we could use the
// filters to select them for us.
//
// Init_MCP2515() should already have been called.
void Can_2515Setup(void)
{
    // As no filters are active, all messages will be stored in RXB0 only if
    // no roll-over is active. We want to recieve all CAN messages (standard and extended)
    // (RXM<1:0> = 11).
    // SPI_mcp_write_bits(RXB0CTRL, RXB_RX_ANY, 0xFF);
    // SPI_mcp_write_bits(RXB1CTRL, RXB_RX_ANY, 0xFF);
    // But there is a bug in the chip, so we have to activate roll-over.
	MCP2515_WriteBits(RXB0CTRL, RXB_RX_STD|RXB_BUKT, 0xFF);		//�ر������˲����ܣ��������б��ģ�������� 
	MCP2515_WriteBits(RXB1CTRL, RXB_RX_STD, 0xFF);		//�ر������˲����ܣ��������б���
}

/****************************************************************************
������˵����
****************************************************************************/
void Init_MCP2515(CanBandRate bandrate,CanConfig Canfig)
{
	unsigned char i,j,a;

	MCP2515_IO_CS_Init() ;
	MCP2515_Reset();

	MCP2515_SetBandRate(bandrate,FALSE);		//���ò�����

	// Disable interrups.
	MCP2515_Write(CANINTE, NO_IE);  		//��ֹ�����ж�

	// Mark all filter bits as don't care:
	MCP2515_Write_Can_ID(RXM0SIDH, 0x7ff,0);
	MCP2515_Write_Can_ID(RXM1SIDH, 0x7ff,0);
	// Anyway, set all filters to 0:
	MCP2515_Write_Can_ID(RXF0SIDH, Canfig.RPM.ID, 0);
	MCP2515_Write_Can_ID(RXF1SIDH, Canfig.SPEED.ID, 0);
	MCP2515_Write_Can_ID(RXF2SIDH, Canfig.THROTTLE.ID, 0);
	MCP2515_Write_Can_ID(RXF3SIDH, 0x235, 0);
	MCP2515_Write_Can_ID(RXF4SIDH, 0x236, 0);
	MCP2515_Write_Can_ID(RXF5SIDH, 0x237, 0);

	//MCP2515_Write(CLKCTRL, MODE_LOOPBACK| CLKEN | CLK8);//�ػ�ģʽ
    //�����������̨�豸����ʵ��Ļ�������ѡ��ػ�ģʽ
    MCP2515_Write(CLKCTRL, MODE_NORMAL| CLKEN | CLK8);//��׼ģʽ
  
	// Clear, deactivate the three transmit buffers
	a = TXB0CTRL;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 14; j++) {
			MCP2515_Write(a, 0);
			a++;
	        }
       	a += 2; // We did not clear CANSTAT or CANCTRL
	}
	// and the two receive buffers.
	MCP2515_Write(RXB0CTRL, 0);
	MCP2515_Write(RXB1CTRL, 0);

	// The two pins RX0BF and RX1BF are used to control two LEDs; set them as outputs and set them as 00.
	MCP2515_Write(BFPCTRL, 0x3C);	
	//Open Interrupt
	MCP2515_Write(CANINTE, RX0IE|RX1IE);
}

/****************************************************************************
������˵����MCP2515ʵ�����
****************************************************************************/
void CAN_2515_TEXT(void)
{
	int i;
	U32 id;
	unsigned char dlc;
	int rxRTR,isExt;
	int temp;
	static U32 count=0;  
	U8 data_write[8]={1,2,3,4,5,6,7,8};
	U8 data_read[8];   
    {
		Can_Write( 250, data_write, 8, FALSE, FALSE);
		//Uart_Printf( "Data=%x,%x,%x,%x,%x,%x,%x,%x\n",data_read[0],data_read[1],data_read[2],data_read[3],data_read[4],data_read[5],data_read[6],data_read[7] );	
		if((i=Can2515_Poll())!=-1 ) 
	    {
            count++;
            for( temp=0; temp<8; temp++)
        	   data_read[temp] = 0 ;   
            temp = Can_Read(i, &id, data_read, &dlc, &rxRTR, &isExt);
            Uart_Printf( "\ni=%d ID=%d \n",i ,id );   
            Uart_Printf( "count=%d,Reveice Data=%x,%x,%x,%x,%x,%x,%x,%x\n",count,data_read[0],data_read[1],data_read[2],data_read[3],data_read[4],data_read[5],data_read[6],data_read[7] );                            
            if(id==canconfig.RPM.ID)
            {
               Can_Data_Process(data_read,canconfig.RPM,&m_Rpm);
            
            }
            else if(id== canconfig.SPEED.ID)
            {
              Can_Data_Process(data_read,canconfig.SPEED,&m_Speed);
            }
            else if(id==canconfig.THROTTLE.ID )
            {
               Can_Data_Process(data_read,canconfig.THROTTLE,&m_Throttle);
            }
		}
	 }	
}


void CAN_2515_TX(void)
{
    static U32 count=0;
	U8 data_write[8]={1,2,3,4,5,6,7,8};
 	{   count++;
    	Can_Write(250, data_write, 8, FALSE, FALSE);    
    	Uart_Printf("count=%d,Send Data=%d,%d,%d,%d,%d,%d,%d,%d\n",count,data_write[0],data_write[1],data_write[2],data_write[3],data_write[4],data_write[5],data_write[6],data_write[7] );   
    }
}

void CAN_2515_RX(void)   
{   
    int i;   
    U32 id; 
    static U32 count=0;  
    unsigned char dlc,CTRL;   
    int rxRTR, isExt;   
    int temp;   
    U8 data_read[8];   
    if( (i=Can2515_Poll())!=-1 )  
    {   
        count++; 
        temp = Can_Read(i, &id, data_read, &dlc, &rxRTR, &isExt);
        CTRL=RXB0CTRL+(i<<4); 
        Uart_Printf( "\ni=%d ID=%d \n",i,id );   
        Uart_Printf( "count=%d,Reveice Data=%x,%x,%x,%x,%x,%x,%x,%x\n",count,data_read[0],data_read[1],data_read[2],data_read[3],data_read[4],data_read[5],data_read[6],data_read[7] );                            
        if(id==canconfig.RPM.ID)
        {
            Can_Data_Process(data_read,canconfig.RPM,&m_Rpm);
            m_RpmIndex=((int)m_Rpm>>7)&0xff;
            if(m_RpmIndex<=1)m_RpmIndex=1;
            if(m_RpmIndex>=128)m_RpmIndex=128;    
        }
        else if(id== canconfig.SPEED.ID)
        {
            Can_Data_Process(data_read,canconfig.SPEED,&m_Speed);
        }
        else if(id==canconfig.THROTTLE.ID )
        {
            Can_Data_Process(data_read,canconfig.THROTTLE,&m_Throttle);
        }  
    }
}
/****************************************************************************
������˵����MCP2515 ���ݴ������
    ȡ�����ݵ���Чֵ��
    1���жϴ��С�ˣ�2��������Ч���ֽ����� 3)�Ƴ���Чλ��Ϣ
    0 = big 1 = little
    address    0    1    2    3     4     5     6     7   
    big        MSB  a    b    c     d                 LSB
    little     LSB  d    c    b     a                 MSB
   when can data's in big endian mode,the program will read an int start from 1th to 4th 
 so the  value  of the int is 'dcba' ,it's wrong ,we should exchange their positions
 so that we could read the data we want 'abcd'.
   when can data is in little endian mode,we don't need to exchange their positions.
 If the valible bits isn't start from 7th,we should shift the extra bits before we caculate
 finally value. 
    byte      7  6 5 4 3 2 1 0
                   ^
                   pos   
****************************************************************************/ 
void Can_Data_Process(U8 *data_read,CANELE canelem,float *CanVal)
{
        U8 data[4]={0,0,0,0};
        U32 *p=(U32*)data,j,val=0;
        if(canconfig.CAN_endian==0)         
        {   
            for(j=0;j<4;j++)               //������Ч�ֽڿ�ʼ��4���ֽ�����,��ϳ�һ��int����
            {                              //������little ��Ҫ��������λ�ã
                if(canelem.BYTENUM+2-j>7) //canelem.BYTENUM+2-j =canelem.BYTENUM-1+3-j 
                   data[j]=0;
                else  
                   data[j]=data_read[canelem.BYTENUM+2-j];
            } 
            val=*p<<(7-canelem.BITPOS)>>(32-canelem.DATALEN); //ȡ��Ч�����ݣ��Ƴ�pos֮ǰ��bit,Ȼ����Ч�����Ƶ���λ  
        }
        else
        {  
            
            for(j=0;j<4;j++)
            {
                if(canelem.BYTENUM-1+j>7)
                   data[j]=0;
                else
                   data[j]=data_read[canelem.BYTENUM-1+j];
            }
            data[0]<<=7-canelem.BITPOS;      //Ԥ�ȴ������λ��Ч����; 
            val=(*p>>(7-canelem.BITPOS))&(0xffffffff>>(32-canelem.DATALEN));
        } 
        *CanVal=val*canelem.DATACOEF;
        Uart_Printf("0x%x %x  %f\n",*p,val,(float)*CanVal);      
} 