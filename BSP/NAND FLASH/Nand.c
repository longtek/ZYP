//=============================================================
// 文件名称：	Nand.c
// 功能描述：	NandFlash驱动
//              选择Nand类型：1.K9F1208(small page) 2.K9F2G08(large page)
//              选择要执行的操作：
//              0: 读取Chip ID
//              1: 复位Nand
//              2: 块擦除，输入要擦除的块
//              3: 读Nand，输入要读取的块，页
//              4: 写Nand，输入要写入的块，页，偏移量
//              5: 检查坏块
//
// 维护记录：	2009-08-15	V1.0 
//              2009-10-14  V1.1 支持大页Nand，还需完善ECC校验
//=============================================================
#include "def.h"
#include "2440lib.h"
#include "2440addr.h"
#include "Nand.h"
#include "wav.h"

U16 pR_AMT[129][40]={0};
volatile int NFConDone;
U8 Spare_Data_2G08[64];

static U8 se8Buf[16]={
	0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff
};

void __irq NFCon_Int(void);

void Test_Nand(void)
{
	U32 gpacon;	
	Uart_Printf("Nand test\n");
	gpacon = rGPACON;
	rGPACON = (rGPACON &~(0x3f<<17)) | (0x3f<<17);//111 1110 0000 0000 0000 0000
	// GPA     22         21           20      19     18     17
	//           nFCE   nRSTOUT  nFRE  nFWE   ALE   CLE
	Test_K9F2G08();
	rGPACON = gpacon;
}
void Test_K9F2G08(void)
{
	U8 Start_block,Start_Page;
	U8 *pW_addr,*pR_addr;
	U32 WriteByteCnt,i,j;
	InitNandFlash();	   
    pW_addr=(unsigned char *)m_RpmAmt;
    pR_addr=(unsigned char *)pR_AMT;
    WriteByteCnt=AMT_SIZE;
    Start_block=0;
    Start_Page=0;
    Test_Page_Write(Start_block,Start_Page,pW_addr,WriteByteCnt); 
    Test_Page_Read(Start_block,Start_Page,pR_addr,WriteByteCnt);
    Uart_Printf("\nK9F2G08 finished!!\n");
}

static void InitNandCfg(void)
{
//  for S3C2440
	rNFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);		
	rNFCONT = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);
//	rNFSTAT = 0;    
    Nand_Reset();	
}
void Nand_Reset(void)
{
    int i;  
	NF_ChipEn();
	NF_CLEAR_RB();
	NF_CMD(RESETCMD);	//reset command
	for(i=0;i<10;i++);  //tWB = 100ns. //??????
	NF_DETECT_RB();
	NF_ChipDs();		
	Uart_Printf("\nNand Reset!\n");
}
void Test_Block_Erase(U32 block)
{
	if((Read_Status()&0x80)==0) {
		Uart_Printf("Write protected.\n");
			return;
		}
	Uart_Printf("\n%d-block erased.\n", block);
	if(EraseBlock_2G08(block)==FAIL) return;
}

void Test_Page_Read(U32 block,U32 page,U8 *buffer,U32 Sumbyte)
{
	U32 i,count=0;
	unsigned char downPt[2048],flag=1;
	while(flag)	
	{
	    for(;block<2048;block++)
	    {
	        if(IsBadBlock_2G08(block))
	        {
	           break;  
	        }
	    }
		if(ReadPage_2G08(block, page, (U8 *)downPt )==FAIL) {
			Uart_Printf("\nRead error.\n");
		} else {
			Uart_Printf("\nRead OK.\n");
		}
		Uart_Printf("Read data(%d-block,%d-page)\n", block, page);
		for(i=0;i<2048;i++)
		{
		   *(buffer+count)=downPt[i];
		    if(++count>=Sumbyte)
		    {
		       flag=0;
		       break;
		    }
		}
		if(++page>=64)
		{
		    page=0;
		    block++;
		}
	}
	for(i=0; i<2048; i++) {
		if((i%16)==0) Uart_Printf("\n%4x: ", i);
			Uart_Printf("%02x ", downPt[i]);
			}
	Uart_Printf("\n");
}

void Test_Page_Write(U32 block,U32 page,U8 *buffer,U32 Sumbyte)
{
	int i, offset;
	unsigned char srcPt[2048],flag=1;
    unsigned int count=0;
	for(;block<2048;block++)
	{
	    if(IsBadBlock_2G08(block))
	    {
	       Test_Block_Erase(block);
	           break;  
	    }
	} 
    while(flag)
	{
	    memset(srcPt,0,2048);
		for(i=0; i<2048; i++)
		{
			srcPt[i]=*buffer++;
			if(++count>=Sumbyte)
			{
			   flag=0;
			   break;
			}
		}
		Uart_Printf("\nWrite data[%d block, %d page].\n", block, page);		
		if(WritePage_2G08(block, page, srcPt)==FAIL)
		{
			Uart_Printf("Write Error.\n");
		} 
		else
		{
			Uart_Printf("Write OK.\n");
		}
		if(++page>=64)
		{
		    page=0;
		    block++;
		    for(;block<2048;block++)
	        {
	           if(IsBadBlock_2G08(block))
	           {
	               Test_Block_Erase(block);
	                  break;  
	            }
	        }
		}
	}
	for(i=0; i<2048; i++)
	{
		if((i%16)==0) Uart_Printf("\n%4x: ", i);
		Uart_Printf("%02x ", srcPt[i]);
	}
		Uart_Printf("\n");
}
void PrintBadBlockNum(void)
{
    int i;
    U16 id;
    int badblock = 0;
	{
		Uart_Printf("\n[K9F2G08 NAND Flash bad block check]\n");		
		id=ReadChipId();
		Uart_Printf("ID=%x(0xec76)\n",id);
		for(i=0;i<2048;i++)
		{
			if (IsBadBlock_2G08(i) == FAIL)   // Print bad block
				badblock ++;
		}
		if (badblock == 0)
			Uart_Printf("no bad block\n");	
	}
}
static int EraseBlock_2G08(U32 block)
{
	U32 blockPage=(block<<5);
    int i;
#if BAD_CHECK
    if(IsBadBlock_2G08(block))
	return FAIL;
#endif
	NF_ChipEn(); 
	NF_CMD(ERASECMD0);   // Erase one block 1st command, Block Addr:A9-A25
	// Address 3-cycle
	NF_ADDR(blockPage&0xff);	    // Page number=0
	NF_ADDR((blockPage>>8)&0xff);   
	NF_ADDR((blockPage>>16)&0xff);
	NF_CLEAR_RB();
	NF_CMD(ERASECMD1);	// Erase one blcok 2nd command
	NF_DETECT_RB();
	if(rNFSTAT&0x8) 
		return FAIL;

	NF_CMD(QUERYCMD);   // Read status command

    if (NF_RDDATA()&0x1) // Erase error
    {	
    	NF_ChipDs();
		Uart_Printf("[ERASE_ERROR:block#=%d]\n",block);
		return FAIL;
    }
    else 
    {
    	NF_ChipDs();
		return OK;
    }
}

static int ReadPage_2G08(U32 block,U32 page,U8 *buffer)
{
    int i;
    unsigned int blockPage;
	U32 Mecc, Secc;
	U8 *bufPt=buffer;
	U8 se[64], ecc0, ecc1, ecc2, ecc3,a,b,c,d,e;
    
    blockPage=(block<<5)+page;
	NF_RSTECC();    // Initialize ECC
	NF_MECC_UnLock();
    
	NF_ChipEn();    

	NF_CLEAR_RB();
	NF_CMD(READCMD0);	// Read command
	NF_ADDR(0); 	// Column = 0
	NF_ADDR(0); 
	NF_ADDR(blockPage&0xff);		//
	NF_ADDR((blockPage>>8)&0xff);	// Block & Page num.
	NF_ADDR((blockPage>>16)&0xff);	//
	
	NF_CMD(READCMD3);
	NF_DETECT_RB();
	 
	
#if TRANS_MODE==C_LANG
	for(i=0;i<2048;i++)
	{
		*bufPt++=NF_RDDATA8();	// Read one page
	}
#elif TRANS_MODE==DMA
	// Nand to memory dma setting
	rSRCPND=BIT_DMA0;	// Init DMA src pending.
	rDISRC0=NFDATA; 	// Nand flash data register
	rDISRCC0=(0<<1) | (1<<0); //arc=AHB,src_addr=fix
	rDIDST0=(unsigned)bufPt;
	rDIDSTC0=(0<<1) | (0<<0); //dst=AHB,dst_addr=inc;
	rDCON0=(1<<31)|(1<<30)|(1<<29)|(1<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|(512/4/4);
	//Handshake,AHB,interrupt,(4-burst),whole,S/W,no_autoreload,word,count=128;

	// DMA on and start.
	rDMASKTRIG0=(1<<1)|(1<<0);

	while(!(rSRCPND & BIT_DMA0));	// Wait until Dma transfer is done.
		
	rSRCPND=BIT_DMA0;

#elif TRANS_MODE==ASM
	__RdPage512(bufPt);
#endif
}

static int WritePage_2G08(U32 block,U32 page,U8 *buffer)
{
    int i;
	U32 blockPage, Mecc, Secc;
	U8 *bufPt=buffer;
	  
	NF_RSTECC();    // Initialize ECC
    NF_MECC_UnLock();
	blockPage=(block<<5)+page;

	NF_ChipEn(); 
//	NF_CMD(0x0);//??????
	NF_CMD(PROGCMD0);   // Write 1st command
	NF_ADDR(0);    //Column (A[7:0]) = 0
	NF_ADDR(0);    // A[11:8]
	NF_ADDR((blockPage)&0xff);	// A[19:12]
	NF_ADDR((blockPage>>8)&0xff);	// A[27:20]
	NF_ADDR((blockPage>>16)&0xff);  //

	
#if TRANS_MODE==C_LANG
     
	for(i=0;i<2048;i++)
	{
		NF_WRDATA8(*bufPt++);	// Write one page to NFM from buffer
    }
#endif
/*
    NF_MECC_Lock();
	// Get ECC data.
	// Spare data for 8bit
	// byte  0     1    2     3     4          5               6      7            8         9
	// ecc  [0]  [1]  [2]  [3]    x   [Bad marking]                    SECC0  SECC1
	Mecc = rNFMECC0;
	se8Buf[0]=(U8)(Mecc&0xff);
	se8Buf[1]=(U8)((Mecc>>8) & 0xff);
	se8Buf[2]=(U8)((Mecc>>16) & 0xff);
	se8Buf[3]=(U8)((Mecc>>24) & 0xff);
	se8Buf[5]=0xff;		// Marking good block

	NF_SECC_UnLock();
	//Write extra data(ECC, bad marking)
	for(i=0;i<4;i++)
	{
		NF_WRDATA8(se8Buf[i]);	// Write spare array(Main ECC)
		Spare_Data_1208[i]=se8Buf[i];
    }  
    NF_SECC_Lock(); 
	Secc=rNFSECC; 
	se8Buf[8]=(U8)(Secc&0xff);
	se8Buf[9]=(U8)((Secc>>8) & 0xff);
	for(i=4;i<16;i++)
	{
		NF_WRDATA8(se8Buf[i]);  // Write spare array(Spare ECC and Mark)
		Spare_Data_1208[i]=se8Buf[i];
	} 
*/	
 	NF_CLEAR_RB();
	NF_CMD(PROGCMD1);	 // Write 2nd command
	NF_DETECT_RB();
	if(rNFSTAT&0x8) return FAIL;

	NF_CMD(QUERYCMD);   // Read status command   
    
	for(i=0;i<3;i++);  //twhr=60ns
    
    if (NF_RDDATA()&0x1)  // Page write error
	{
    	NF_ChipDs();
		Uart_Printf("[PROGRAM_ERROR:block#=%d]\n",block);
		return FAIL;
    } 
	else
	{
    	NF_ChipDs();
	    return OK;
	}
}

static int IsBadBlock_2G08(U32 block)
{
    int i;
    unsigned int blockPage;
	U8 data;
    
    blockPage=(block<<5);	// For 2'nd cycle I/O[7:5] 
    
	NF_ChipEn();
	NF_CLEAR_RB();

	NF_CMD(READCMD0);		 // Spare array read command
	NF_ADDR((2048+0)&0xff);		 // Read the mark of bad block in spare array(M addr=0)
	NF_ADDR(((2048+0)>>8)*0xff);
	NF_ADDR(blockPage&0xff);	 // The mark of bad block is in 0 page
	NF_ADDR((blockPage>>8)&0xff);	 // For block number A[24:17]
	NF_ADDR((blockPage>>16)&0xff);  // For block number A[25]
	
	NF_CLEAR_RB();
	NF_CMD(READCMD3);
	NF_DETECT_RB();	 // Wait tR(max 12us)

    data=NF_RDDATA();

	NF_ChipDs();    

    if(data!=0xff)
    {
    	Uart_Printf("[block %d has been marked as a bad block(%x)]\n",block,data);
    	return FAIL;
    }
    else
    {
    	return OK;
    }
}

static int MarkBadBlock_2G08(U32 block)
{
    int i;
	U32 blockPage=(block<<5);
 
    se8Buf[0]=0xff;
    se8Buf[1]=0xff;    
    se8Buf[2]=0xff;    
    se8Buf[5]=0x44;   // Bad blcok mark=44
    
	NF_ChipEn(); 
//	NF_CMD(READCMD2);   //????
	NF_CMD(PROGCMD0);   // Write 1st command
    
	NF_ADDR((2048+0)&0xff);			// 2060 = 0x080c
	NF_ADDR(((2048+0)>>8)&0xff);	// A[10:8]
	NF_ADDR((blockPage)&0xff);	// A[11;18]
	NF_ADDR((blockPage>>8)&0xff);	// A[26:19]
	NF_ADDR((blockPage>>16)&0xff);  //
    
	NF_WRDATA(0);

	NF_CLEAR_RB();
	NF_CMD(PROGCMD1);   // Write 2nd command
	NF_DETECT_RB();

	NF_CMD(QUERYCMD);
    
	for(i=0;i<3;i++);  //twhr=60ns////??????
    
    if (NF_RDDATA()&0x1) // Spare arrray write error
    {	
    	NF_ChipDs();
    	Uart_Printf("[Program error is occurred but ignored]\n");
    }
    else 
    {
    	NF_ChipDs();
    }

	Uart_Printf("[block #%d is marked as a bad block]\n",block);
    return OK;
}

static void InitNandFlash(void)
{
	InitNandCfg();
}
/*void __irq NFCon_Int(void)
{
    NFConDone=1;
	rINTMSK|=BIT_NFCON;
	rSRCPND |= BIT_NFCON;
	rINTPND = rINTPND;
	//ClearPending(BIT_NFCON);
	if(rNFSTAT&0x8) Uart_Printf("Illegal Access is detected!!!\n");
//	else Uart_Printf("RnB is Detected!!!\n"); 
}*/
U16 Read_Status(void)
{
	U16 stat;
	int i;
	
	NF_ChipEn();	
	NF_CMD(QUERYCMD);
	for(i=0; i<10; i++);	
	stat = NF_RDDATA();	
	NF_ChipDs();
	
	return stat;
}

U32 ReadChipId(void)
{
	U32 id;
	
	NF_ChipEn();	
	NF_CMD(RdIDCMD);
	NF_ADDR(0);
	while(NFIsBusy());	
	id  = NF_RDDATA()<<8;
	id |= NF_RDDATA();		
	NF_ChipDs();		
	
	return id;
}

void Print_Id(void)
{
	U16 id;
	U8 maker, device;

	id = ReadChipId();
	
	device = (U8)id;
	maker = (U8)(id>>8);
	
	Uart_Printf("\nMaker:%x, Device:%x\n", maker, device);
}