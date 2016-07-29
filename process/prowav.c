#include "config.h"
#include "def.h"
#define  TPI 6.2831853
#define  SAMPLERATE  4410000
/*====================================================
// �﷨��ʽ��CreatePerRpmDatasize(U32 start_rpm,U32 stop_rpm)
   ʵ��Ŀ�꣺����С������
        ����������44.1k,ÿ���������������44.1,ÿ��ת�ٵ����������ݾ���������
   ���ٸ����������ܹ�������ӳ��ǰת�ٵ�������Ϣ����44.1/ת�ٵ��µ�����Ƶ�ʣ�
   ����ǰ����Ƶ���ɸ����״ε�Ƶ��������;
   
// ��������: ����ÿ��ת�������ݳ��ȣ��������ݵĳ��ȸ��ݾ���״εĲ�ͬ���仯��
// ��ڲ���: �״���Ϣ
// ���ڲ���: ���ݴ�С
====================================================*/
void  ProcessBasicData(U32 *Fredata,U32 *Phase)
{
   U32 iOrder,iRpm;
   for(iOrder=0;iOrder<40;iOrder++)
   {
	   Phase[iOrder]=m_RpmPhase[iOrder]; //��ȡ��ʼ��λ 
   }
   for(iRpm=0;iRpm<129;iRpm++)
   {
       for(iOrder=0;iOrder<40;iOrder++)
       {
         *(Fredata+iOrder+iRpm*40)=(int)(BasicFre[iRpm]*OrderData[iOrder]+0.5);//����ÿ��ת���¸����״���λ������
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







