#include "watch_media.h"
#include <mshmpool.h>
#include <VEncApi.h>
#include <calc_bitrate.h>
#include <devinfo.h>

#define MAX_ONVIF_DEVICE    4
MSHM_POOL	VideoEncoder[MAX_ONVIF_DEVICE*2];	//��Ƶ����������ؽṹ
BIT_RATE_T	VBitRate[MAX_ONVIF_DEVICE*2];		//��Ƶ�����ṹ
BIT_RATE_T	frame_bitrate[MAX_ONVIF_DEVICE*2];	//֡��
int	old_frame_seq[MAX_ONVIF_DEVICE*2];

/*yk add */
inline int get_videoenc_num()
{
	return 4;
}
void InitVEncState(void)
{
	int i,j,total;
    int ret;
	memset(VideoEncoder,0,sizeof(VideoEncoder));
	memset(VBitRate,0,sizeof(VBitRate));
	total=get_video_num();
	for(i=0;i<total;i++)
	{
		j=i*2;
		ret=MSHmPoolAttach(&VideoEncoder[j], get_onvif_pool_key(i,0));
		if(ret!=0)
		{
			printf("connect video encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		VBitRate[j].AvgInterval=2;
		VBitRate[j].LastCheckBytes=0;

		frame_bitrate[j].AvgInterval=2;
		frame_bitrate[j].LastCheckBytes=0;
		old_frame_seq[j]=-1;
	}
	for(i=0;i<total;i++)
	{
		j=i*2+1;
		ret=MSHmPoolAttach(&VideoEncoder[j], get_onvif_pool_key(i,1));
		if(ret!=0)
		{
			printf("connect video encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		VBitRate[j].AvgInterval=2;
		VBitRate[j].LastCheckBytes=0;

		frame_bitrate[j].AvgInterval=2;
		frame_bitrate[j].LastCheckBytes=0;
		old_frame_seq[j]=-1;
	}

}
//��ʾָ���������ŵ���Ϣ
//����1��ʾ����������
static int	ShowEncState(int vno,int No,MSHM_POOL *VEnc)
{
	//int i;
	//int Val;
	ENC_ATTRIB 			*EAttr;
	SHPOOL_HEAD		*Ph;
	BIT_RATE_T			*BR=&VBitRate[No];
	BIT_RATE_T			*FR=&frame_bitrate[No];
	int					*OldSeq=&old_frame_seq[No];
	

	if(VEnc->mc==NULL)
	{//û�б���ʼ��
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	Ph=VEnc->ph;
	EAttr=MShmPoolGetInfo(VEnc);


	CalcBitRate(BR,Ph->send_bytes-BR->LastCheckBytes);	//��������
	BR->LastCheckBytes=Ph->send_bytes;				//�����ֽ���Ϣ

	if(*OldSeq>=0)
	{
		CalcBitRate(FR,Ph->num-*OldSeq);		
	}
	else
	{
		CalcBitRate(FR,0);		
	}
	*OldSeq=Ph->num;

	
	WriteTermStr(C_WHITE,0,"%02d  ",vno);				//��Ƶ���������

	if(EAttr->State!=1)								//�豸״̬
		WriteTermStr(C_RED,0,"  %02d",EAttr->State);		//������������
	else
		WriteTermStr(C_WHITE,0,"  %02d",EAttr->State);	//����������
	WriteTermStr(C_WHITE,0,"%10d ",Ph->num);			//��ǰ���
	WriteTermStr(C_WHITE,0,"%4d ",(Ph->tail));			//���µ�Ԫ��
	WriteTermStr(C_WHITE,0,"      %4.1f ",BR->PeakBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",BR->AvgBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"    \t%4.1f ",FR->AvgBitrate/8);	//��ֵ����
	
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
//��ʾ���ӵ�ָ�����������û���Ϣ
static int	ShowEncUsrState(int No,MSHM_POOL *VEnc)
{
	SHPOOL_USR   		*Usr;
	SHPOOL_HEAD		*Ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//��ǰʱ��

	if(VEnc->mc==NULL)
	{//û�г�ʼ��			
		return 0;
	}	
	Ph=VEnc->ph;

///�û���Ϣ
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		Usr=&Ph->users[i];
		if(Usr->valid)
		{	//��Ч�û�
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",No);		//��������
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//�û���
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",Usr->curele);	//��ǰ��ȡ��Ԫ�����
			WriteTermStr(C_WHITE,0,"%12s",Usr->name);//�û���
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-Usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
		Usr++;
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}

void DisplayVideoEncState(void)
{//��ʾ��Ƶ��������״̬
	int i;
	//WriteTermStr(C_HWHITE,1,"GT1000 media resource usage display\n");
	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ������(������)\n");
	WriteTermStr(C_WHITE,1,"VNo  ״̬   LastSeq NewEle  ��ֵ(kbps) ƽ��ֵ(kbps) ֡��(fps)\t\t\n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncState(i,i*2,&VideoEncoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ������(������)\n");
	WriteTermStr(C_WHITE,1,"VNo  ״̬   LastSeq NewEle  ��ֵ(kbps) ƽ��ֵ(kbps) ֡��(fps)\t\t\n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncState(i,i*2+1,&VideoEncoder[i*2+1]);
	}

	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ�û���Ϣ(������)\n");
	WriteTermStr(C_WHITE,1,"VNo �û���          NextEle     �û���  ����ʱ��(s) \n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncUsrState(i,&VideoEncoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ�û���Ϣ(������)\n");
	WriteTermStr(C_WHITE,1,"VNo �û���          NextEle     �û���  ����ʱ��(s) \n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncUsrState(i,&VideoEncoder[i*2+1]);
	}
}





