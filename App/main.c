#include "config.h"
#include "app_cfg.h"
char a[16]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
char bfile[16]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
char cfile[32]={0};
OS_STK   MainTaskStk[MainTaskStkLengh];
OS_STK   BToothTaskStk[BToothTaskStkLengh];
OS_STK   Can2515TaskStk[Can2515TaskStkLengh];
OS_STK   Can2510TaskStk[Can2510TaskStkLengh];
OS_STK   CheckTaskStk[CheckTaskStkLengh];
OS_STK   SoundTaskStk[SoundTaskStkLengh];
OS_STK   BTSendTaskStk[BTSendTaskStkLengh];
OS_STK   BTReceiveTaskStk[BTReceiveTaskStkLengh];
OS_STK   ProcessTaskStk[ProcessTaskStkLengh];
OS_EVENT *RxD_Sem,*Sound_Sem,*Tx_Sem,*Rx_Sem;
extern CanConfig canconfig;            
U8 Index=0;                            //selet DMA channel
S16  SoundData[64];                    //data cache buffer
U32 FileDataLenth=0,WhFlag=0;          
short  m_Wavadata[820000]={0};
float m_Rpm,m_Speed,m_Throttle;       //����CAN��Ϣ
U8 m_RpmIndex=21,m_OldRpmIndex=21,m_ToChange=FALSE;       //ת��λ��;
U32 m_FreData[129][40];
U32 m_PhaseCnt[40];
int Main(void)
{  
    TargetInit();
    OSInit();
    OSTimeSet(0);   
    OSTaskCreate (MainTask,(void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);	
    OSStart();
    return 0;
}
void  MainTask(void *pdata)
{

    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    OS_ENTER_CRITICAL();
    Timer0Init();//initial timer0 for ucos time tick
    ISRInit();   //initial interrupt prio or enable or disable
    DMAIntSeverInit();
    OS_EXIT_CRITICAL();
    OSStatInit();
    OSTaskCreate(Can2515Task,(void *)0, &Can2515TaskStk[Can2515TaskStkLengh - 1], Can2515TaskPrio);   
    OSTaskCreate(BToothTask,(void *)0, &BToothTaskStk[BToothTaskStkLengh - 1], BToothTaskPrio); 
    OSTaskCreate(SoundTask,(void *)0, &SoundTaskStk[SoundTaskStkLengh - 1], SoundTaskPrio);    	    
    OSTaskCreate(ProcessTask,(void *)0, &ProcessTaskStk[ProcessTaskStkLengh - 1], ProcessTaskPrio); 
    OSTaskCreate(CheckTask,(void *)0, &CheckTaskStk[CheckTaskStkLengh - 1], CheckTaskPrio);     
    while(1)
    {     
       Led0_On();
       OSTimeDlyHMSM(0,0,1,0);
       Led0_Off();
       OSTimeDlyHMSM(0,0,1,0); 
    }
}
void ProcessTask(void *pdata)
{
    
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    while(1)
    {   
       ProcessBasicData(&m_FreData[0][0],&m_PhaseCnt[0]);
       OSTaskResume(SoundTaskPrio);      
       OSTaskSuspend(OS_PRIO_SELF); 
    }   
}
void BToothTask(void *pdata)
{
    U8 data=255,err;      
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif    
    OS_ENTER_CRITICAL(); 
    RxIRQStart();
    OS_EXIT_CRITICAL();
    Tx_Sem=OSSemCreate(0);
    Rx_Sem=OSSemCreate(1);
    RxD_Sem=OSSemCreate(0);
    while(1)
    {                  
        OSSemPend(Rx_Sem,0,&err);
        data=RdURXH2();
        Uart_Printf("%c\n",data);
        switch(data)
        {
             case 'a':Bluetooth_Putbyte(data);
                      Bluetooth_Putbyte('\n');
                      FileDataLenth=16;
                      Index=2;
                      OS_ENTER_CRITICAL();
                      OSTaskCreate(BTReceiveTask,bfile, &BTReceiveTaskStk[BTReceiveTaskStkLengh - 1], BTReceiveTaskPrio); 
                      OS_EXIT_CRITICAL();
                   break;
             case 'b':Bluetooth_Putbyte(data);
                      Bluetooth_Putbyte('\n');
                      FileDataLenth=32;
                      Index=2;
                      OS_ENTER_CRITICAL();
                      OSTaskCreate(BTReceiveTask,cfile, &BTReceiveTaskStk[BTReceiveTaskStkLengh - 1], BTReceiveTaskPrio); 
                      OS_EXIT_CRITICAL();
                   break;
             case 'c':Bluetooth_Putbyte(data);
                      Bluetooth_Putbyte('\n');
                      OS_ENTER_CRITICAL();                      
                      OSTaskCreate(BTSendTask,(void *)0, &BTSendTaskStk[BTSendTaskStkLengh - 1], BTSendTaskPrio);                       
                      OS_EXIT_CRITICAL();
                   break;
             case 'd':Change2IRQTxMode();
                      Bluetooth_Putbyte(data);
                      Bluetooth_Putbyte('\n');                      
                      OSTaskDel(BTSendTaskPrio);
                   break;
             default:
                   break;
         }
    }   
}
void CheckTask(void *pdata)
{
    char data;
    U32 iStartSize,j;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif 
    while(1)
    {
        data=Uart_GetKey();
        if(data=='+')
        {
          m_RpmIndex+=1;
          if(m_RpmIndex>=128)m_RpmIndex=128;
          Uart_Printf("%d\n",m_RpmIndex);
        }
        else if(data=='-')
        {                     
          m_RpmIndex-=1;
          if(m_RpmIndex<=1)m_RpmIndex=1;
          Uart_Printf("%d\n",m_RpmIndex);
        }
        OSTimeDlyHMSM(0,0,0,50);                     
     }
}
void BTReceiveTask(void *pdata)
{
    U8 err;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif    
    Change2DMARxMode(Index);
    OS_ENTER_CRITICAL(); 
    InitDMARxMode((char *)pdata,FileDataLenth,Index);
    OS_EXIT_CRITICAL();      
    OSSemPend(RxD_Sem,0,&err);  
    Uart_Printf("complete %d\n",FileDataLenth);  
    Change2IRQRxMode(Index);  
    OSTaskDel(OS_PRIO_SELF);    
}
void BTSendTask(void *pdata)
{
    U8 err;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    Change2DMATxMode();
    OS_ENTER_CRITICAL(); 
    InitDMATxMode(bfile,16);
    OS_EXIT_CRITICAL();     
    while(1)
    {    
         rDMASKTRIG0=(0<<2)|(1<<1)|0;                
         OSSemPend(Tx_Sem,0,&err);
         while(!(rUTRSTAT2 & (1 << 2)));
         Uart_Printf("abcdefghijklmnop\n");
         Change2IRQTxMode();          
         Bluetooth_Putbyte('\n');
         Change2DMATxMode();                                        
         OSTimeDlyHMSM(0,0,2,0);           
    }
}
void Can2515Task(void *pdata)
{
    
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif    
    pdata=pdata;
    GetCanConfigInfo();
    Init_MCP2515(BandRate_1Mbps,canconfig);
	Can_2515Setup();
    while(1)
    {
        CAN_2515_TX();   
        OSTimeDlyHMSM(0,0,1,0);       
    }
}
void SoundTask(void *pdata)
{
    unsigned char buffId=0,id;
    short music=0,Tamt=0;
    int imusic[16]; 
    U32 iphasecnt=0,PhaseOff=0;
    U8 err,iOrder;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    Wm8731AndIISPortInit();     
    Wm8731RegInit();
    OS_ENTER_CRITICAL(); 
    IIS_Init();
    DMA_Init(&SoundData[0],32);      
    OS_EXIT_CRITICAL();
    Sound_Sem=OSSemCreate(0);
    OSTaskSuspend(OS_PRIO_SELF); 
    rDMASKTRIG2=(0<<2)|(1<<1)|0;  
    while(1)
    {                   
        // OSSemPend(Sound_Sem,0,&err);  
         for(iOrder=0;iOrder<40;iOrder++) 
         {
             iphasecnt=m_PhaseCnt[iOrder];
             PhaseOff =m_FreData[38][iOrder];
             Tamt = m_RpmAmt[38][iOrder];
             for(buffId=0;buffId<16;buffId++)
             {
                 imusic[buffId]+=Tamt*rawDataSin[iphasecnt];            
                 iphasecnt+=PhaseOff; 
                 if(iphasecnt>=441000)
                 {
                    iphasecnt-=441000;
                 }                 
             }
             m_PhaseCnt[iOrder]= iphasecnt;
         }    
         if(WhFlag)
         {
             for(buffId=32;buffId<64;buffId+=2)
             {               
               music =imusic[(buffId&0xdf)>>1]>>14;
               imusic[(buffId&0xdf)>>1]=0;
               SoundData[buffId]  = music;  
               SoundData[buffId+1]= music; 
               Uart_Printf("%d\n",music);
             } 
             
             WhFlag=0;
         } 
         else
         {
             for(buffId=0;buffId<32;buffId+=2)
             {
               music =imusic[buffId>>1]>>14;
               imusic[buffId>>1]=0;
               SoundData[buffId]  = music;  
               SoundData[buffId+1]= music;
               Uart_Printf("%d\n",music); 
             }
             
             WhFlag=32;
         } 
    }
}