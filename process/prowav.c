#include "config.h"
#include "def.h"
#define  TPI 6.2831853
#define  SAMPLERATE  4410000
U32 rpm_sizefromzero[129]={0};
U32 rpm_datasize[129]={0};
//====================================================
// 语法格式：CreatePerRpmDatasize(U32 start_rpm,U32 stop_rpm)
// 功能描述: 计算每个转速下数据长度；
// 入口参数: 阶次信息
// 出口参数: 数据大小
//====================================================
void  CreatePerRpmDatasize(U32 *DataSize)
{
    S32 iorder=0,irpm=0;	                                       //阶次转速计数;
	U32 orderfre=0;                                                //阶次频率;
	U32 Minorder=0,MarkOrder=0,flag;
	if(*OrderData<=0.0)                                            //如阶次信息为零则跳出函数;
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
	     orderfre=Minorder*BasicFre[irpm];
         *(DataSize+irpm)=(float)SAMPLERATE/orderfre+0.5; 
         Uart_Printf("irpm=%d,%d\n",irpm,*(DataSize+irpm));    
       }
	}	
}
//====================================================
// 语法格式：WavadataCreateWithSin(U32 *dataSize)
// 功能描述: 接收一个U32型指针，获取样本的大小，合成声音数据
// 入口参数: 无
// 出口参数: 接收的数据大小
//====================================================
void WavadataCreateWithSin(U32 *DataSize,S16 *Wavadata)
{
     U32   icount=0,iorder=0,iRpm=0,j,iStartSize;
     float temp_val=0.0;
	 U32 frerpm;
	 U32 it_phase[40];
	 for(iorder=0;iorder<40;iorder++)
	 {
	    it_phase[iorder]=m_RpmPhase[iorder]; //获取初始相位 
	 }
	 for(iRpm=20;iRpm<80;iRpm++)
	 {
		 iStartSize=rpm_sizefromzero[iRpm];
		 if(iStartSize<0)
		 continue;
         for(iorder=0;iorder<40;iorder++)                  //声音阶次
         {    
			 if(OrderData[iorder]<=0.0)
				 break;
             frerpm=BasicFre[iRpm]*OrderData[iorder];     //计算频率
			 if(frerpm>200) 
			 {                   
                for(j=0;j<DataSize[iRpm];j++)                        
                {
                    temp_val=m_RpmAmt[iRpm][iorder]*rawDataSin[it_phase[iorder]];     //计算声音数据
                    if(temp_val>0)
                    {
                       temp_val+=0.5;
                    }
                    else 
                    {
                       temp_val-=0.5;
                    }
                    Wavadata[j+iStartSize]+=(short)temp_val; //#important              
                    it_phase[iorder]+=frerpm;                    
                    if(it_phase[iorder]>=441000)
                    {
                       it_phase[iorder]-=441000;
                    }           //计算声音相位；
                }
             }            
         } 
	 }
}
void CaculateDataAddress(U32 *datasize, U32 *StartAddr)
{
     U32 temp_size=0,i=0;
	 for(i=20;i<80;i++)
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






