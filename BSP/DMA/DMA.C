#include "2440addr.h" 
#include "dma.h"
#include "2440lib.h"
#include "config.h"
extern OS_EVENT *Sem_DMA;
extern U32  WhFlag;
extern unsigned char SoundData[]; //DMA 根据Whflag从 SoundData[]读取到IISFIFO  
void DMA_Init(unsigned char *pData,unsigned int nSoundLend)
{
  rINTMOD = 0x0;  
  pISR_DMA2 = (U32)DMA2Audioout;   
  EnableIrq(BIT_DMA2);
  rDISRC2 = (U32)(pData);  
  rDISRCC2=(0<<1)|(0<<0);//AHB,crease;
  rDIDST2 = (U32)IISFIFO; 
  rDIDSTC2=(1<<1)|(1<<0);//APB,fixed;
  rDCON2=(1<<31)+(0<<30)+(1<<29)+(0<<28)+(0<<27)+(0<<24)+(1<<23)+(0<<22)+(1<<20)+nSoundLend/2;
  //Handshake,sync PLCK,TC int,single tx,27 single service,24I2SSDO,23I2Srequest,22 Auto-reload,half-word,size/2;
  rDMASKTRIG2=(0<<2)|(1<<1)|0;
  //No-stop,DMA2 channel on,No-sw trigger
}
/*reload accordding to nSoundLend/2 */
void  DMA2Audioout(void) 
{    
     #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
     OS_CPU_SR  cpu_sr;
     #endif      
     OS_ENTER_CRITICAL();     
     rSRCPND |= BIT_DMA2 ;
     rINTPND = rINTPND;
     OS_EXIT_CRITICAL();
     rDISRC2 = (U32)(SoundData+WhFlag);              
     OSSemPost(Sem_DMA); 
}
