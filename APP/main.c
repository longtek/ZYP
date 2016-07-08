#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "app_cfg.h"
#include "Printf.h"
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task_SoundStk [Task_SoundStkLengh]; 
OS_STK	Task_LedStk [Task_LedStkLengh]; 
OS_STK	Task_CANStk [Task_CANStkLengh];
OS_STK	Task_NFStk [Task_NFStkLengh];
OS_EVENT *Sem_DMA;
U32 WhFlag;
void *MsgGrp[];
U8  SoundData[128];
U8  g_NowRpmInd;
extern CanConfig canconfig;  
extern float m_Rpm,m_Speed,m_Throttle;
int Main(void)
{    
   //初始化目标板 包含时钟 MMU 端口 串口等
	TargetInit(); 
	//初始化uC/OS 	  
   	OSInit ();	
   	//初始化系统时基
   	OSTimeSet(0);
   	//RandomSem  = OSSemCreate(1);   /* Random number semaphore                  */
    OSTaskCreate (MainTask,(void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);									    
    OSStart();
    return 0;
}
void MainTask(void *pdata)
{ 
     #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
     OS_CPU_SR  cpu_sr;
     #endif
     OS_ENTER_CRITICAL();
     Timer0Init();//initial timer0 for ucos time tick
     ISRInit();   //initial interrupt prio or enable or disable    
     OS_EXIT_CRITICAL();
     OSStatInit(); 
     OSTaskCreate(Task_Led,(void *)0, &Task_LedStk[Task_LedStkLengh - 1], Task_LedPrio);
     OSTaskCreate(Task_CAN,(void *)0, &Task_CANStk[Task_CANStkLengh - 1], Task_CANPrio);
     while(1)               
     {  
                        
        OSTaskSuspend(OS_PRIO_SELF);
     }
}
void Task_Led(void *pdata) //task for test
{ 
     #if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
     OS_CPU_SR  cpu_sr;
     #endif
     pdata=pdata;
     while(1)
     {
        Led0_On();Led2_Off();
        OSTimeDlyHMSM(0,0,1,0);
        Led0_Off();Led1_On();
        OSTimeDlyHMSM(0,0,1,0);
        Led1_Off();Led2_On();
        OSTimeDlyHMSM(0,0,1,0);
     }    
}
void Task_CAN(void *pdata) //task for CAN
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
       OSTimeDlyHMSM(0,0,1,0);   
    }
  
}

