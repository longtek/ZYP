#ifndef __NAND_H
#define __NAND_H
#include "def.h"
//====================================================
// 常量定义区
//====================================================

#define	EnNandFlash()	(rNFCONF |= (1<<0))  //bit0=1 enable NAND flash controller
#define	DsNandFlash()	(rNFCONF &= ~(1<<)) //bit0=1 disable NAND flash controller
#define	InitEcc()		(rNFCONF |= (1<<4))  //bit4=1 initialize ECC
#define	NoEcc()			(rNFCONT &= ~(1<<4)) //bit4=0 initialize ECC
#define	NF_ChipEn()		(rNFCONT&=~(1<<1))  //bit1=0 NAND flash nFCE = L (active)
#define	NF_ChipDs()		(rNFCONT|=(1<<1))   //bit1=1 NAND flash nFCE = H (inactive)
#define NF_MECC_UnLock()         {rNFCONT&=~(1<<5);}
#define NF_MECC_Lock()         {rNFCONT|=(1<<5);}
#define NF_SECC_UnLock()         {rNFCONT&=~(1<<6);}
#define NF_SECC_Lock()         {rNFCONT|=(1<<6);}

#define NF_CMD(cmd)			{rNFCMD=cmd;}
#define NF_ADDR(addr)		{rNFADDR=addr;}	
#define NF_RSTECC()			{rNFCONT|=(1<<4);}
#define NF_RDDATA() 		(rNFDATA)
#define NF_RDDATA8() 		((*(volatile unsigned char*)0x4E000010) )
#define NF_WRDATA(data) 	{rNFDATA=data;}
#define NF_WRDATA8(data) 	{rNFDATA8=data;}

#define	NFIsBusy()		(!(rNFSTAT&1))     //whether nand flash is busy?
#define	NFIsReady()		(rNFSTAT&1)        //whether nand flash is ready?

#define	READCMD0	0      //Read0 model  command  == Page addr  0~127
#define	READCMD1	1      //Read1 model  command  == Page addr  128~511
#define	READCMD2	0x50   //Read2 model  command  == Page addr  512~527
#define READCMD3    0x30   //Read3 model  for Large Page 
#define	ERASECMD0	0x60   //Block erase  command 0
#define	ERASECMD1	0xd0   //Block erase  command 1
#define	PROGCMD0	0x80   //page write   command 0
#define	PROGCMD1	0x10   //page write   command 1
#define	QUERYCMD	0x70   //query command
#define	RdIDCMD		0x90   //read id command
#define RESETCMD    0xFF   //reset command

// RnB Signal
#define NF_CLEAR_RB()    		{rNFSTAT |= (1<<2);}	// Have write '1' to clear this bit.
#define NF_DETECT_RB()    		{while(!(rNFSTAT&(1<<2)));}

#define TACLS		7	// 1-clk(0ns) 
#define TWRPH0		7	// 3-clk(25ns)
#define TWRPH1		7	// 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

#define   OK        1
#define   FAIL      0

#define ASM		1
#define C_LANG	2
#define DMA		3
#define TRANS_MODE 2

#define AMT_SIZE 10320
////////////////////////////// 8-bit ////////////////////////////////

// Main function
void Test_Nand(void);
// Sub function
void Test_K9F2G08(void);

void Test_Page_Write(U32 block,U32 page,U8 *buffer,U32 Sumbyte);
void Test_Page_Read(U32 block,U32 page,U8 *buffer,U32 Sumbyte);
void Test_Block_Erase(U32 block);
void PrintBadBlockNum(void);

U16  Read_Status(void);
void Nand_Reset(void);
void Print_Id(void);
static U32 ReadChipId(void);

static int EraseBlock_2G08(U32 block);
static int ReadPage_2G08(U32 block,U32 page,U8 *buffer);
static int WritePage_2G08(U32 block,U32 page,U8 *buffer);
static int IsBadBlock_2G08(U32 block);
static int MarkBadBlock_2G08(U32 block);

static void InitNandFlash(void);
//*******************************************************

#endif /*__NAND_H*/

