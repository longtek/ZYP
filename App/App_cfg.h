#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#define MainTaskStkLengh       1024
#define BToothTaskStkLengh     1024
#define Can2515TaskStkLengh    1024
#define Can2510TaskStkLengh    1024
#define CheckTaskStkLengh      1024
#define SoundTaskStkLengh      4048
#define BTSendTaskStkLengh     1024
#define BTReceiveTaskStkLengh  1024
#define ProcessTaskStkLengh    1024

#define N_MESSAGES 128
#define NormalTaskPrio    5
#define MainTaskPrio      NormalTaskPrio+6
#define BToothTaskPrio    NormalTaskPrio+7
#define BTSendTaskPrio    NormalTaskPrio+5
#define Can2515TaskPrio   NormalTaskPrio+1
#define CheckTaskPrio     NormalTaskPrio+3
#define SoundTaskPrio     NormalTaskPrio
#define BTReceiveTaskPrio NormalTaskPrio+2
#define ProcessTaskPrio   NormalTaskPrio+4


void MainTask(void *pdata);
void BToothTask(void *pdata);
void Can2515Task(void *pdata);
void BTReceiveTask(void *pdata);
void SoundTask(void *pdata);
void BTSendTask(void *pdata);
void ProcessTask(void *pdata);
void CheckTask(void *pdata);
#endif