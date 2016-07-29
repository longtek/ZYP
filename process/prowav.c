#include "config.h"
#include "def.h"
#define  TPI 6.2831853
#define  SAMPLERATE  4410000
/*====================================================
// 语法格式：CreatePerRpmDatasize(U32 start_rpm,U32 stop_rpm)
   实现目标：求最小公倍数
        声音采样率44.1k,每秒的声音样本就是44.1,每个转速的下声音数据具有周期性
   多少个数据样本能够完整反映当前转速的声音信息，由44.1/转速的下的声音频率；
   而当前声音频率由各个阶次的频率组合相加;
   
// 功能描述: 计算每个转速下数据长度；生意数据的长度根据具体阶次的不同而变化；
// 入口参数: 阶次信息
// 出口参数: 数据大小
====================================================*/
void  ProcessBasicData(U32 *Fredata,U32 *Phase)
{
   U32 iOrder,iRpm;
   for(iOrder=0;iOrder<40;iOrder++)
   {
	   Phase[iOrder]=m_RpmPhase[iOrder]; //获取初始相位 
   }
   for(iRpm=0;iRpm<129;iRpm++)
   {
       for(iOrder=0;iOrder<40;iOrder++)
       {
         *(Fredata+iOrder+iRpm*40)=(int)(BasicFre[iRpm]*OrderData[iOrder]+0.5);//计算每个转速下各个阶次相位的增加
       }
   }   
}
void ProcessSpeedGain(U16 *Speeddata,float *SpeedGain)
{
   U32 i;
   for(i=0;i<256;i++)
   {
       Speeddata[i]=65535;
       SpeedGain[i]=(float)Speeddata[i]/65535;
   }
}
void ProcessThrottleGain(U16 *Throttledata,float *ThrottleGain)
{
   U32 i;
   for(i=0;i<128;i++)
   {
        ThrottleGain[i]=(float)Throttledata[i]/65535;
   }
}







