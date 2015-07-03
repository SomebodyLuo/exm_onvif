/**********************************************************************************************
 * file:calc_bitrate.h
 * ���������Ĺ��ܺ�������
 * 
 **********************************************************************************************/
#ifndef	CALC_BITRATE_H
#define	CALC_BITRATE_H
//��������
//#include <types.h>
 #include <sys/time.h>


typedef	struct{					//���������õ��Ľṹ����
	struct timeval	PeakTime;		//�ϴμ����ֵ�õ�ʱ��
	struct timeval 	AvgTime;		//�ϴμ���ƽ��ֵ�õ�ʱ��
	unsigned long		LastCheckBytes;	//�ϴμ�������ʱ���ѷ����ֽ���
	unsigned long		PeakBytes;	//�ϴμ����ֵʱ���ֽ���
	unsigned long		AvgBytes;	//�ϴμ���ƽ��ֵʱ���ֽ���
	double			PeakBitrate;	//�ϴμ�����ķ�ֵ	bit per second
	double			AvgBitrate;	//�ϴμ������ƽ��ֵ	bit per second
	int			AvgInterval;	//����ƽ��ֵ��ʱ����(��)
}BIT_RATE_T;
 
 /**********************************************************************************************
 * ������:GetPeakBitRate()
 * ����:��ȡBR�ṹ�еķ�ֵ����
 * ����: BR:ָ���������������ṹ��ָ��
 * ����ֵ:�����еķ�ֵ����
 **********************************************************************************************/
 static __inline__ double GetPeakBitRate(BIT_RATE_T *BR)
 {
 	return BR->PeakBitrate;
 }
 
 /**********************************************************************************************
 * ������:GetPeakBitRate()
 * ����:��ȡBR�ṹ�е�����ƽ��ֵ
 * ����: BR:ָ���������������ṹ��ָ��
 * ����ֵ:�����е�����ƽ��ֵ
 **********************************************************************************************/
 static __inline__ double GetAvgBitRate(BIT_RATE_T *BR)
 {
 	return BR->AvgBitrate;
 }

 /**********************************************************************************************
 * ������:SetCalcAvgTime()
 * ����:���ü�������ƽ��ֵ��ʱ����
 * ����: BR:ָ���������������ṹ��ָ��
 *		   second:���ʱ��(��)
 * ����ֵ:��
 **********************************************************************************************/
 static __inline__ void SetCalcAvgTime(BIT_RATE_T *BR,int second)
 {
 	if(second<=0)
		second=1;
 	BR->AvgInterval=second;
 }

/**********************************************************************************************
 * ������:DiffTv()
 * ����:��������ʱ��ļ��������tv1-tv2�ĸ�����ֵ
 * ����: tv1:��һ��ʱ��
 * 	 	   tv2:�ڶ���ʱ��
 * ����ֵ:(tv1-tv2)�ĸ�����ֵ
 **********************************************************************************************/
static __inline__ double	DiffTv(struct timeval *tv1,struct timeval *tv2)
{
	double res;
	struct timeval temp;
	temp.tv_sec=tv1->tv_sec-tv2->tv_sec;
	if(tv1->tv_usec<tv2->tv_usec)
	{
		temp.tv_sec--;
		temp.tv_usec=1000000+tv1->tv_usec-tv2->tv_usec;	
	}
	else
		temp.tv_usec=tv1->tv_usec-tv2->tv_usec;
	res=(double)((double)temp.tv_sec*1000000+(double)temp.tv_usec)/(double)1000000;
	return res;
}

/**********************************************************************************************
 * ������:CalcBitRate()
 * ����:��������ʣ���NewBytes���ֽ���ˢ��BR�ṹ
 * ����: BR:ָ�����������õ����ݽṹָ��
 * 	 NewBytes:�����ӵ��ֽ���
 * ����ֵ:��
 **********************************************************************************************/
#if 0
static __inline__ void CalcBitRate(BIT_RATE_T *BR,unsigned long NewBytes)
{
	struct timeval	CurTv;
	double	PDiff;		//��ֵʱ���
	double	ADiff;		//ƽ��ֵʱ���
	long		PBytes;		//��ֵ�ֽ���
	long		ABytes;		//ƽ��ֵ�ֽ���
	if(BR==NULL)
		return;
	if(gettimeofday(&CurTv,NULL)<0)
		return;
	PDiff=DiffTv(&CurTv,&BR->PeakTime);
	PBytes=NewBytes-BR->PeakBytes;

	ADiff=DiffTv(&CurTv,&BR->AvgTime);

	if(PDiff>0.1)//����2���ʱ��ż���
	{
		BR->PeakBitrate=(double)(((double)PBytes/(double)PDiff)*8)/(double)1000;
		BR->PeakBytes=NewBytes;
		*(double*)&BR->PeakTime=	*(double*)&CurTv;			
	}
	//printf("%f-%f-%d-%d!!\n",PDiff,ADiff,PBytes,ABytes);
	if((ADiff+0.2)>=BR->AvgInterval)
	{
		ABytes=NewBytes-BR->AvgBytes;
		BR->AvgBitrate=(((double)ABytes/(double)ADiff)*8)/(double)1000;
		BR->AvgBytes=NewBytes;
		*(double*)&BR->AvgTime=	*(double*)&CurTv;
	}	
	
	
}
#endif

static __inline__ void CalcBitRate(BIT_RATE_T *BR,unsigned long NewBytes)
{
	struct timeval	CurTv;
	double	PDiff;		//��ֵʱ���
	double	ADiff;		//ƽ��ֵʱ���
//	long		PBytes;		//��ֵ�ֽ���
//	long		ABytes;		//ƽ��ֵ�ֽ���
	if(BR==NULL)
		return;
	if(gettimeofday(&CurTv,NULL)<0)
		return;
	PDiff=DiffTv(&CurTv,&BR->PeakTime);
//	PBytes=NewBytes-BR->PeakBytes;

	ADiff=DiffTv(&CurTv,&BR->AvgTime);
	BR->PeakBytes+=NewBytes;
	
	if((double)PDiff>(double)0.2)//����2���ʱ��ż���
	{
		BR->PeakBitrate=(double)(((double)BR->PeakBytes/(double)PDiff)*8);
		BR->PeakBytes=0;
		*(double*)&BR->PeakTime=	*(double*)&CurTv;			
	}
	//printf("%f-%f-%d-%d!!\n",PDiff,ADiff,PBytes,ABytes);
	BR->AvgBytes+=NewBytes;	
	if((ADiff+0.2)>=(double)BR->AvgInterval)
	{
//		ABytes=NewBytes-BR->AvgBytes;
		BR->AvgBitrate=(((double)BR->AvgBytes/(double)ADiff)*8);///(double)1000;
		BR->AvgBytes=0;
		*(double*)&BR->AvgTime=	*(double*)&CurTv;
	}	
	
	
}

#endif
