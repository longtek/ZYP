#ifndef __DMA_H_
#define __DMA_H_

#define SRC_STOCK_SIZE 64
void DMA_Init(unsigned char  *pData,unsigned int nSoundLend);
void  DMA2Audioout(void); 
#endif
/*
typedef struct tagDMA
{
    volatile U32 DISRC;   //要传送的源数据地址寄存器; 
    volatile U32 DISRCC;  //要传送的源数据配置寄存器
    volatile U32 DIDST;   //目标数据寄存器
    volatile U32 DIDSTC;  //目标数据配置寄存器
    volatile U32 DCON;    //控制寄存器
    volatile U32 DSTAT;   //状态寄存器
    volatile U32 DCSRC;   //当前源寄存器
    volatile U32 DCDST;   //当前目的寄存器
    volatile U32 DMASKTRIG; //屏蔽寄存器
} DMA;*/