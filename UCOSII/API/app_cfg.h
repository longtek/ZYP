#ifndef APP_CFG_H
#define APP_CFG_H


//Task Stk lengh

#define	MainTaskStkLengh	    1024*2   	// Define the MainTask stack length 
#define	Task_SoundStkLengh		1024     	// Define the Task0 stack length 
#define	Task_LedStkLengh		1024   	// Define the Task1 stack length 
#define	Task_CANStkLengh		1024   	// Define the Task2 stack length 
#define	TaskUartStkLengh	    1024    	// Define the TaskUart stack length 
#define	Task_NFStkLengh	        1024*10
#define N_TASKS            10       //Number of identical tasks                         

//Task function
void    MainTask(void *pdata);   		// MainTask task
void 	Task_Sound(void *pdata);			    // Task0 
void 	Task_Led(void *pdata);			    // Task1 
void 	Task_CAN(void *pdata);			// Task2
void    TaskUart(void *pdata);          // Task Uart
void    Task_NF(void *pdata);

//Task Prio
#define NormalTaskPrio        5
#define MainTaskPrio 	    NormalTaskPrio+1
#define Task_SoundPrio 	    NormalTaskPrio
#define Task_LedPrio  	    NormalTaskPrio+2
#define Task_CANPrio  		NormalTaskPrio+3
#define TaskUartPrio  	    NormalTaskPrio+4
#define Task_NFPrio         NormalTaskPrio+6
  

#endif