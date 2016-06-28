#ifndef _DMA_H_
#define _DMA_H_
void DMA_Init(unsigned char *pData,unsigned int nSoundLend);
void DMA_IRQ(void);
void InitDMATxMOde(unsigned char *pData,unsigned int nDataLend,unsigned char index); 
#endif