#include "config.h"
#include "def.h"
#define  TPI 6.2831853
#define  SAMPLERATE  4410000
U32 rpm_sizefromzero[129]={0};
U32 rpm_datasize[129]={0};
//====================================================
// �﷨��ʽ��CreatePerRpmDatasize(U32 start_rpm,U32 stop_rpm)
// ��������: ����ÿ��ת�������ݳ��ȣ�
// ��ڲ���: �״���Ϣ
// ���ڲ���: ���ݴ�С
//====================================================
void  CreatePerRpmDatasize(U32 *DataSize)
{
    S32 iorder=0,irpm=0;	                                       //�״�ת�ټ���;
	float orderfre=0.0;                                            //�״�Ƶ��;
	U32 Minorder=0,MarkOrder=0,flag;
	if(*OrderData<=0.0)                                            //��״���ϢΪ������������;
		return;
    for(irpm=0;irpm<129;irpm++)
	{
       flag=1;                                            
	   for(iorder=0;iorder<40;iorder++)
	   {
		   if(*(OrderData+iorder)<=0.0)
		        break;
		   if((*(OrderData+iorder))*BasicFre[irpm]>=200)
		   {
		      if(flag)
		      {
			    Minorder=(int)((*(OrderData+iorder))*10);
			    flag=0;
			  }
		   }
		   else
		   {
			  continue;
		   }
		   MarkOrder=(int)((*(OrderData+iorder))*10);
		   if(MarkOrder%Minorder)
		   {
		      Minorder-=5;
		      iorder=-1;   
		   }	     
	   }
	   if(Minorder>0)
	   {
	     orderfre=(float)Minorder*BasicFre[irpm];
         *(DataSize+irpm)=SAMPLERATE/orderfre; 
         Uart_Printf("irpm=%d,%d\n",irpm,*(DataSize+irpm));    
       }
	}	
}
//====================================================
// �﷨��ʽ��WavadataCreateWithSin(U32 *dataSize)
// ��������: ����һ��U32��ָ�룬��ȡ�����Ĵ�С���ϳ���������
// ��ڲ���: ��
// ���ڲ���: ���յ����ݴ�С
//====================================================
void WavadataCreateWithSin(U32 *DataSize,S16 *Wavadata)
{
     U32   icount=0,iorder=0,iRpm=0,j,iStartSize;
     float temp_val=0;
	 U32 frerpm;
	 U32 it_phase;
	 for(iRpm=20;iRpm<21;iRpm++)
	 {
		 iStartSize=rpm_sizefromzero[iRpm];
		 if(iStartSize<=0)
		 continue;
         for(iorder=0;iorder<40;iorder++)                  //�����״�
         {    
			 if(OrderData[iorder]<=0.0)
				 break;
             frerpm=BasicFre[iRpm]*OrderData[iorder];     //����Ƶ��
			 if(frerpm>200) 
			 {
                it_phase=m_RpmPhase[iorder];                 //��ȡ��ʼ��λ     
                for(j=0;j<DataSize[iRpm];j++)                        
                {
                    temp_val=m_RpmAmt[iRpm][iorder]*rawDataSin[it_phase];     //������������
                    if(temp_val>0)
                    {
                       temp_val+=0.5;
                    }
                    else 
                    {
                       temp_val-=0.5;
                    }
                    Wavadata[j]+=(short)temp_val; //#important              
                    it_phase+=frerpm;
                    if(it_phase>=441000)
                    {
                       it_phase-=441000;
                    }           //����������λ��
                }
             }            
         }         
	 }
}
void CaculateDataAddress(U32 *datasize, U32 *StartAddr)
{
     U32 temp_size=0,i=0;
	 for(i=0;i<129;i++)
	 {	   
       StartAddr[i] =  temp_size;
	   temp_size   +=  datasize[i];
	 }
}






