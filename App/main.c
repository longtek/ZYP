#include "config.h"
#include "app_cfg.h"
#include "wav.h"
char a[16]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
char bfile[16]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
char cfile[32]={0};
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK  BToothTaskStk[BToothTaskStkLengh];
OS_STK  Can2515TaskStk[Can2515TaskStkLengh];
OS_STK  Can2510TaskStk[Can2510TaskStkLengh];
OS_STK  CheckTaskStk[CheckTaskStkLengh];
OS_STK  SoundTaskStk[SoundTaskStkLengh];
OS_STK  BTSendTaskStk[BTSendTaskStkLengh];
OS_STK  BTReceiveTaskStk[BTReceiveTaskStkLengh];

OS_EVENT *RxD_Sem,*Sound_Sem,*Tx_Sem,*Rx_Sem;
extern CanConfig canconfig;
U8 state=RxCmdState,Index=0;
U8  SoundData[128];
U32 FileDataLenth=0,WhFlag=0,Change=1;
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
    U32 i=0;
    U8 flag=1;
    U32 count=0;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    OS_ENTER_CRITICAL();
    Timer0Init();//initial timer0 for ucos time tick
    ISRInit();   //initial interrupt prio or enable or disable 
    OS_EXIT_CRITICAL();
    OSStatInit();
    OSTaskCreate(Can2515Task,(void *)0, &Can2515TaskStk[Can2515TaskStkLengh - 1], Can2515TaskPrio);   
    OSTaskCreate(BToothTask,(void *)0, &BToothTaskStk[BToothTaskStkLengh - 1], BToothTaskPrio);   	
    OSTaskCreate(SoundTask,(void *)0, &SoundTaskStk[SoundTaskStkLengh - 1], SoundTaskPrio);  
    while(1)
    {     
       Led0_On();Led1_Off();
       OSTimeDlyHMSM(0,0,1,0);
       Led0_Off();Led1_On();
       OSTimeDlyHMSM(0,0,1,0); 
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
    OSTaskSuspend(BTSendTaskPrio); 
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
void BTReceiveTask(void *pdata)
{
    U8 i,err;
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
    U8 err,i=0;
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
    Init_MCP2515(BandRate_250kbps,canconfig);
	Can_2515Setup();		
    while(1)
    {
       //CAN_2515_RX();      
       OSTimeDlyHMSM(0,0,1,0);       
    }
}
void SoundTask(void *pdata)
{
    unsigned short music=0,i=0;
    U32 iphasecnt=0;
    U8 err;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    Wm8731AndIISPortInit();     
    Wm8731RegInit();
    OS_ENTER_CRITICAL(); 
    IIS_Init();
    DMA_Init(&SoundData[0],64);      
    OS_EXIT_CRITICAL();
    Sound_Sem=OSSemCreate(0); 
    rDMASKTRIG2=(0<<2)|(1<<1)|0;  
    while(1)
    {  
         OSSemPend(Sound_Sem,0,&err);                
         for(i=0;i<64;i++)
         {
             SoundData[i+WhFlag]=rawData[iphasecnt++];
             if(iphasecnt>=1033336)
             {
                iphasecnt=0;
             }
         }
         if(WhFlag)
         {
            WhFlag=0;
         } 
         else
         {
            WhFlag=64;
         } 
    }
}