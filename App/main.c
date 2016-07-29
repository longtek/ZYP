#include "config.h"
#include "app_cfg.h"
char bfile[16]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
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
float m_RpmVal=123,m_SpeedVal=1,m_ThrottleVal=55;       //三个CAN信息
U8 m_RpmIndex=0;       //转速位置;
U32 m_FreData[129][40];               //记录进行时各阶次相位增加
U32 m_PhaseCnt[40];                   //记录各阶次进行时相位
U16 m_AmtCnt[40];                     //记录各阶次进行时幅值
U8 m_CanFlash=0;                      //CAN运行标示
char SendBuffer[16];                  //发送数据缓冲区
float m_SpeedGain[256];               //速度增益
float m_ThrottleGain[128]; 
U32   m_SpeedIndex;
U32   m_ThrottleIndex;
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
    //OSTaskCreate(Can2515Task,(void *)0, &Can2515TaskStk[Can2515TaskStkLengh - 1], Can2515TaskPrio);   
    OSTaskCreate(BToothTask,(void *)0, &BToothTaskStk[BToothTaskStkLengh - 1], BToothTaskPrio); 
    OSTaskCreate(SoundTask,(void *)0, &SoundTaskStk[SoundTaskStkLengh - 1], SoundTaskPrio);    	    
    OSTaskCreate(ProcessTask,(void *)0, &ProcessTaskStk[ProcessTaskStkLengh - 1], ProcessTaskPrio); 
    OSTaskCreate(CheckTask,(void *)0, &CheckTaskStk[CheckTaskStkLengh - 1], CheckTaskPrio);     
    OSTaskCreate(BTSendTask,(void *)0, &BTSendTaskStk[BTSendTaskStkLengh - 1], BTSendTaskPrio);                       
    while(1)
    {     
       if(m_CanFlash)
          Led1_On();
          Led0_On();
       OSTimeDlyHMSM(0,0,1,0);
       if(m_CanFlash)
          Led1_Off();
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
       ProcessSpeedGain(SpeedData,&m_SpeedGain[0]);
       ProcessThrottleGain(ThrottleData,&m_ThrottleGain[0]);      
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
                      OSTaskCreate(BTReceiveTask,bfile, &BTReceiveTaskStk[BTReceiveTaskStkLengh - 1], BTReceiveTaskPrio); 
                      OS_EXIT_CRITICAL();
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
    int itVal[3],i,wch,ioff;
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif
    Change2DMATxMode();
    OS_ENTER_CRITICAL(); 
    InitDMATxMode(SendBuffer,16);
    OS_EXIT_CRITICAL();
    SendBuffer[0]='R';
    SendBuffer[5]='S';
    SendBuffer[10]='T'; 
    SendBuffer[15]='E'; 
    Tx_Sem=OSSemCreate(0);       
    while(1)
    {    
         itVal[0]=(int)m_RpmVal;
         itVal[1]=(int)m_SpeedVal;
         itVal[2]=(int)m_ThrottleVal;
         for(wch=0;wch<3;wch++)
         {
            ioff=5*wch;
            for(i=3;i>=0;i--)
            {
                SendBuffer[i+ioff]=itVal[wch]%10+'0';
                itVal[wch]/=10;
            }
            for(i=0;i<3;i++)
            {
                if(SendBuffer[i+ioff]=='0')
                   SendBuffer[i+ioff]-='0';
                else
                   break;
            }
         }          
         rDMASKTRIG0=(0<<2)|(1<<1)|0;             
         OSSemPend(Tx_Sem,0,&err);        
        //while(!(rUTRSTAT2 & (1 << 2)));                                     
         OSTimeDlyHMSM(0,0,0,500);           
    }
}
void Can2515Task(void *pdata)
{   
    #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
    #endif    
    pdata=pdata;
    GetCanConfigInfo();
    Init_MCP2515(BandRate_500kbps,canconfig);
	Can_2515Setup();
    while(1)
    {
        CAN_2515_RX();       
        OSTimeDlyHMSM(0,0,0,5);       
    }
}
void SoundTask(void *pdata)
{
    unsigned char buffId=0,id;
    short music=0,Oldamt=0,Newamt;
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
         OSSemPend(Sound_Sem,0,&err);  
         for(iOrder=0;iOrder<40;iOrder++) 
         {
             iphasecnt= m_PhaseCnt[iOrder];
             PhaseOff = m_FreData[m_RpmIndex][iOrder];
             Oldamt   = m_AmtCnt[iOrder];
             Newamt   = m_RpmAmt[m_RpmIndex][iOrder];
             Newamt   = Newamt*m_SpeedGain[m_SpeedIndex];   
             Newamt   = Newamt*m_ThrottleGain[m_ThrottleIndex];
             for(buffId=0;buffId<16;buffId++)
             {
                 imusic[buffId]+=Oldamt*rawDataSin[iphasecnt];            
                 iphasecnt+=PhaseOff;
                 if(Oldamt<Newamt)    //保证幅值变化的连续性
                 {
                    Oldamt+=1;
                 }
                 else if(Oldamt>Newamt)
                 {
                    Oldamt-=1;
                 } 
                 if(iphasecnt>=441000)
                 {
                    iphasecnt-=441000;
                 }                 
             }
             m_AmtCnt[iOrder]  = Oldamt;
             m_PhaseCnt[iOrder]= iphasecnt;
         }    
         if(WhFlag)
         {
             for(buffId=32;buffId<64;buffId+=2)
             {               
               music =imusic[(buffId&0xdf)>>1]>>15;            
               SoundData[buffId]  = music;  
               SoundData[buffId+1]= music; 
               imusic[(buffId&0xdf)>>1]=0;
             }              
             WhFlag=0;
         } 
         else
         {
             for(buffId=0;buffId<32;buffId+=2)
             {
               music =imusic[buffId>>1]>>15;            
               SoundData[buffId]  = music;  
               SoundData[buffId+1]= music;
               imusic[buffId>>1]=0;
             }             
             WhFlag = 32;
         } 
    }
}