#include"uda1341.h"
#include"2440addr.h"
static void AdjVolume(unsigned short volume)	
{	
	rGPBDAT = rGPBDAT & ~(L3MODE|L3CLOCK|L3DATA) |(L3MODE|L3CLOCK); //Start condition : L3M=H, L3C=H
    rGPBUP  = rGPBUP  & ~(0x7<<2) |(0x7<<2);       //The pull up function is disabled GPB[4:2] 1 1100    
   	rGPBCON = rGPBCON & ~(0x3f<<4) |(0x15<<4);     //GPB[4:2]=Output(L3CLOCK):Output(L3DATA):Output(L3MODE)
	
	volume = (volume*MAX_VOLUME)/0xffff;		    
	    
	uda1341_writeL3addr(0x14 + 0);				//DATA0 (000101xx+00)
   	uda1341_writeL3data(MAX_VOLUME-volume, 0);
}
//====================================================
// 语法格式：void uda1341_init(char mode)
// 功能描述: 初始化 音频芯片的寄存器 L3
// 入口参数: char 记录或是输出;
// 出口参数: 无
//====================================================================
static void uda1341_init(char mode)
{   
       rGPBDAT = rGPBDAT & ~(L3MODE|L3CLOCK|L3DATA) |(L3MODE|L3CLOCK); //Start condition : L3M=H, L3C=H
       rGPBUP  = rGPBUP  & ~(0x7<<2) |(0x7<<2);       //The pull up function is disabled GPB[4:2] 1 1100    
	   rGPBCON = rGPBCON & ~(0x3f<<4) |(0x15<<4);     //GPB[4:2]=Output(L3CLOCK):Output(L3DATA):Output(L3MODE)
       uda1341_writeL3addr(0x14+2);//status register;
       uda1341_writeL3data(0x60,0);//reset,0101000,384fs;
       uda1341_writeL3addr(0x14+2);//status register;
       uda1341_writeL3data(0x10,0);
       uda1341_writeL3addr(0x14+2);//status register;
       uda1341_writeL3data(0x81,0);//power manager ADC off and DAC on;
       //uda1341_writeL3addr(0x14+0);
       //uda1341_writeL3data(0x80,0);
       //AdjVolume(30000);
       if(mode)
       {
          uda1341_writeL3addr(0x14+2);
          uda1341_writeL3data(0xa2,0);
          uda1341_writeL3addr(0x14+0);
          uda1341_writeL3data(0xc2,0);
          uda1341_writeL3data(0x4d,0);
       }       
}
//====================================================
// 语法格式：void uda1341_writeL3addr(unsigned char data)
// 功能描述: 初始化 音频芯片的寄存器 L3
// 入口参数: 音频芯片寄存器地址;
// 出口参数: 无
//====================================================================
static void uda1341_writeL3addr(unsigned char data)
{
     int i,j;
     rGPBDAT  = rGPBDAT & ~(L3DATA | L3MODE | L3CLOCK) | L3CLOCK;
     for(i=0;i<4;i++);
    
     for(i=0;i<8;i++)
     {
        if(data&0x01)
        {
            L3CLOCK_LOW()  
            L3DATA_HIGH()
            for(j=0;j<4;j++);
            L3CLOCK_HIGH()
            L3DATA_HIGH()
            for(j=0;j<4;j++);
        }
        else
        {
            L3CLOCK_LOW()
            L3DATA_LOW()
            for(j=0;j<4;j++);
            L3CLOCK_HIGH()
            L3DATA_LOW()
            for(j=0;j<4;j++);
        }
        data=data>>1;
     }
       rGPBDAT  = rGPBDAT & ~(L3DATA | L3MODE | L3CLOCK) | (L3CLOCK | L3MODE);       //L3M=H,L3C=H   
}
static void uda1341_writeL3data(unsigned char data ,int mode)
{
     unsigned int i,j;
     if(mode)
     {
       rGPBDAT  = rGPBDAT & ~(L3DATA | L3MODE | L3CLOCK) | L3CLOCK;
        for(j=0;j<4;j++);
     }
     rGPBDAT  = rGPBDAT & ~(L3DATA | L3MODE | L3CLOCK) | (L3CLOCK | L3MODE);   //L3M=H(in data transfer mode) 

     for(j=0;j<4;j++);
     for(i=0;i<8;i++)
     {
         if(data&0x01)
         {      
            L3CLOCK_LOW()
            L3DATA_HIGH()
            for(j=0;j<4;j++);
            rGPBDAT |= (L3CLOCK | L3DATA);       //L3C=H,L3D=H              
            for(j=0;j<4;j++);
         }
         else
         {
            L3CLOCK_LOW()
            L3DATA_LOW()
            for(j=0;j<4;j++);           
            L3CLOCK_HIGH()
            L3DATA_LOW()
            for(j=0;j<4;j++);
         }
          data=data>>1;
     }
     rGPBDAT  = rGPBDAT & ~(L3DATA | L3MODE | L3CLOCK) | (L3CLOCK | L3MODE);    //L3M=H,L3C=H
}
static void Init1341port()
{     
       rGPBUP  = rGPBUP  & ~(0x7<<2) | (0x7<<2); //disable pull up function; 
       rGPBCON = rGPBCON & ~(0x3f<<4) | (0x15<<4);//GPBCON clear; GPB2 L3MODE,GPB3 L3DATA,GPB4 L3CLOCK;
       //GPB2.3.4 out;1 0101 0000          
       rGPEUP  = rGPEUP  & ~(0x1f)  | 0x1f;  //disable pull up function;
       rGPECON = rGPECON & ~(0x3ff) | 0x2aa;//GPE0.1.2.3.4 out ;   10 1010 1010               
}
void IIS_Init()
{
       Init1341port();
       uda1341_init(0);
       rIISPSR=(2<<5)|2;
       rIISCON=(1<<5)|(0<<4)|(0<<3)|(1<<2)|(1<<1);//disable DMA,send channal ,enable SPSR      
       rIISMOD=(0<<9)|(0<<8)|(2<<6)|(0<<5)|(0<<4)|(1<<3)|(1<<2)|(1<<0);//send status,16 bit ,384 fs,32fs
       rIISFCON=(1<<15)|(1<<13); //send FIFO mode 1 DMA;FIFO enable;
       rIISCON|=0x01;      
}