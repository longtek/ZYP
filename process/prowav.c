#include "config.h"
#include "def.h"
#define  TPI 6.2831853
#define  SAMPLERATE  4410000
U32 rpm_sizefromzero[129]={0};
U32 rpm_datasize[129]={0};
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
void  CreatePerRpmDatasize(U32 *DataSize)
{
    S32 iorder=0,irpm=0;	                                       //�״�ת�ټ���;
	U32 orderfre=0;                                                //�״�Ƶ��;
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
			  continue;    //����Ƶ��С��20HZ�Ľ״μ���
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
	     orderfre=Minorder*BasicFre[irpm];
         *(DataSize+irpm)=(float)SAMPLERATE/orderfre+0.5; 
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
     float temp_val=0.0;
	 U32 frerpm;
	 U32 it_phase[40];
	 for(iorder=0;iorder<40;iorder++)
	 {
	    it_phase[iorder]=m_RpmPhase[iorder]; //��ȡ��ʼ��λ 
	 }
	 for(iRpm=0;iRpm<129;iRpm++)
	 {
		 iStartSize=rpm_sizefromzero[iRpm];
		 if(iStartSize<0)
		 continue;
         for(iorder=0;iorder<40;iorder++)                  //�����״�
         {    
			 if(OrderData[iorder]<=0.0)                    //�����״�Ϊ0ʱ����������һת��
				 break;
             frerpm=BasicFre[iRpm]*OrderData[iorder];     //����״ζ�Ӧ��Ƶ��
			 if(frerpm>200)                               //����Ƶ��С��20hz������ 
			 {                   
                for(j=0;j<DataSize[iRpm];j++)             //�����������ݴ�С��������           
                {
                    temp_val=m_RpmAmt[iRpm][iorder]*rawDataSin[it_phase[iorder]];     //������������val=amt*sin��wt��
                    if(temp_val>0)
                    {
                       temp_val+=0.5;
                    }
                    else 
                    {
                       temp_val-=0.5;
                    }
                    Wavadata[j+iStartSize]+=(short)temp_val; //#important              
                    it_phase[iorder]+=frerpm;                //��λ����                         
                    if(it_phase[iorder]>=441000)
                    {
                       it_phase[iorder]-=441000;
                    }                                       //����������λ��
                }
             }            
         } 
	 }
}
void CaculateDataAddress(U32 *datasize, U32 *StartAddr)
{
    //�ɸ�ת���������ݳ��ȼ�¼�������ݵĴ�ŵ���ʼλ��
    //��ǰת���������ݵĴ�С������һת���������ݴ�ŵ���ʼλ��
     U32 temp_size=0,i=0;
	 for(i=0;i<129;i++) 
	 {	   
        StartAddr[i] =  temp_size;
	    temp_size   +=  datasize[i];
	    Uart_Printf("irpm=%d,%d\n",i, StartAddr[i]); 
	 }
}
void  CaculateInsertData(short *firt,short *tail, U8 InsertN,short *InsertData)
{
	 S16 FortOff,TailOff,ValOffSub,MaxOffSet;
     FortOff=*(firt+1)-*firt;
	 TailOff=*(tail)-*(tail-1);
	 ValOffSub=*firt-*(tail);	 
	 if((FortOff>0)&&(TailOff>0))
	 {
		MaxOffSet=(FortOff>TailOff)?FortOff:TailOff;
		if(ValOffSub>0)
		{
           if(ValOffSub>MaxOffSet)
		   {
               
		   }
		   else
		   {

		   }
		}
		else
		{
			 
		}
	 }
	 else if((FortOff<0)&&(TailOff<0))
	 {
		if(ValOffSub<0)
		{
              
		}
		else
		{
			 
		}
	 }
	 else if((FortOff>0)&&(TailOff<0))
     {

	 }
	 else if((FortOff<0)&&(TailOff>0))
     {

	 }
}






