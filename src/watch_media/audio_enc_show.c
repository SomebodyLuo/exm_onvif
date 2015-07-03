#include "watch_media.h"
#include <mshmpool.h>
#include <calc_bitrate.h>
#include <devinfo.h>
#include <media_api.h>
#include <time.h>

MSHM_POOL	audio_encoder[MAX_AUDIO_ENCODER*2];		//��Ƶ����������ؽṹ
BIT_RATE_T	audio_bitrate[MAX_AUDIO_ENCODER*2];		//��Ƶ�����ṹ
MSHM_POOL	audio_decoder[MAX_AUDIO_DECODER];		//��Ƶ����������ؽṹ
BIT_RATE_T	dec_bitrate[MAX_AUDIO_DECODER];		//��Ƶ�����ṹ

void init_audio_enc_state(void)
{
	int i,j,total;
    int ret;
	memset(audio_encoder,0,sizeof(audio_encoder));
	memset(audio_bitrate,0,sizeof(audio_bitrate));
	total=4;//Ŀǰֻ��һ����Ƶ������get_audioenc_num();
	for(i=0;i<total;i++)
	{
		j=i*2;

		ret=MSHmPoolAttach(&audio_encoder[j],get_onvif_pool_key(i,0)+0x20000);
		if(ret!=0)
		{
			printf("MSHmPoolAttach audio encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		audio_bitrate[j].AvgInterval=2;
		audio_bitrate[j].LastCheckBytes=0;
	}
	for(i=0;i<total;i++)
	{
		j=i*2+1;
		ret=MSHmPoolAttach(&audio_encoder[j],get_onvif_pool_key(i,1)+0x20000);
		if(ret!=0)
        {
            printf("MSHmPoolAttach audio encoder%d ret=%d!!\n",j,ret);
            sleep(1);
        }
		audio_bitrate[j].AvgInterval=2;
		audio_bitrate[j].LastCheckBytes=0;
	}

}
void init_audio_dec_state(void)
{
	int i,total;
       int ret;
	memset(audio_decoder,0,sizeof(audio_decoder));
	memset(dec_bitrate,0,sizeof(dec_bitrate));
	total=4;
	for(i=0;i<total;i++)
	{
		ret=MSHmPoolAttach(&audio_decoder[i],get_audio_dec_key(i));
	  if(ret!=0)
	  {
		  printf("MSHmPoolAttach audio decoder%d ret=%d!!\n",i,ret);
		  sleep(1);
	  }
		dec_bitrate[i].AvgInterval=2;
		dec_bitrate[i].LastCheckBytes=0;
	}

}

//��ʾָ���������ŵ���Ϣ
//����1��ʾ����������
static int	show_audio_enc_state(int ano,int no,MSHM_POOL *a_enc)
{
	//int i;
	//int Val;
	media_attrib_t 		*e_attr;
	SHPOOL_HEAD		*ph;
	BIT_RATE_T			*br=&audio_bitrate[no];

	if(a_enc->mc==NULL)
	{//û�б���ʼ��
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	ph=a_enc->ph;
	e_attr=MShmPoolGetInfo(a_enc);


	CalcBitRate(br,ph->send_bytes-br->LastCheckBytes);	//��������
	br->LastCheckBytes=ph->send_bytes;				//�����ֽ���Ϣ

	
	WriteTermStr(C_WHITE,0,"%02d  ",ano);				//��Ƶ���������

	if(e_attr->stat!=1)								//�豸״̬
		WriteTermStr(C_RED,0,"  %02d",e_attr->stat);		//������������
	else
		WriteTermStr(C_WHITE,0,"  %02d",e_attr->stat);	//����������
	WriteTermStr(C_WHITE,0,"%10d ",ph->num);			//��ǰ���
	WriteTermStr(C_WHITE,0,"%4d ",(ph->tail));			//���µ�Ԫ��
	WriteTermStr(C_WHITE,0,"      %4.1f ",br->PeakBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",br->AvgBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
static int	show_audio_dec_state(int no,MSHM_POOL *a_dec)
{
	//int i;
	//int Val;
	media_attrib_t 		*e_attr;
	SHPOOL_HEAD		*ph;
	BIT_RATE_T			*br=&dec_bitrate[no];

	if(a_dec->mc==NULL)
	{//û�б���ʼ��
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	ph=a_dec->ph;
	e_attr=MShmPoolGetInfo(a_dec);


	CalcBitRate(br,ph->send_bytes-br->LastCheckBytes);	//��������
	br->LastCheckBytes=ph->send_bytes;				//�����ֽ���Ϣ

	
	WriteTermStr(C_WHITE,0,"%02d  ",no);				//��Ƶ���������

	if(e_attr->stat!=1)								//�豸״̬
		WriteTermStr(C_RED,0,"  %02d",e_attr->stat);		//������������
	else
		WriteTermStr(C_WHITE,0,"  %02d",e_attr->stat);	//����������
	WriteTermStr(C_WHITE,0,"%10d ",ph->num);			//��ǰ���
	WriteTermStr(C_WHITE,0,"%4d ",(ph->tail));			//���µ�Ԫ��
	WriteTermStr(C_WHITE,0,"      %4.1f ",br->PeakBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",br->AvgBitrate/1024);	//��ֵ����
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
//��ʾ���ӵ�ָ�����������û���Ϣ
static int	show_audio_enc_usr_state(int no,MSHM_POOL *a_enc)
{
	SHPOOL_USR   		*usr;
	SHPOOL_HEAD		*ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//��ǰʱ��

	if(a_enc->mc==NULL)
	{//û�г�ʼ��			
		return 0;
	}	
	ph=a_enc->ph;

///�û���Ϣ
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		usr=&ph->users[i];
		if(usr->valid)
		{	//��Ч�û�
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",no);		//��������
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//�û���
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",usr->curele);	//��ǰ��ȡ��Ԫ�����
			WriteTermStr(C_WHITE,0,"%12s",usr->name);//�û���
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
static int	show_audio_dec_usr_state(int no,MSHM_POOL *a_dec)
{
	SHPOOL_USR   		*usr;
	SHPOOL_HEAD		*ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//��ǰʱ��

	if(a_dec->mc==NULL)
	{//û�г�ʼ��			
		return 0;
	}	
	ph=a_dec->ph;

///�û���Ϣ
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		usr=&ph->users[i];
		if(usr->valid)
		{	//��Ч�û�
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",no);		//��������
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//�û���
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",usr->curele);	//��ǰ��ȡ��Ԫ�����
			WriteTermStr(C_WHITE,0,"%12s",usr->name);//�û���
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}

void display_audio_enc_state(void)
{//��ʾ��Ƶ��������״̬
	int i;
	//WriteTermStr(C_HWHITE,1,"GT1000 media resource usage display\n");
	WriteTermStr(C_WHITE,0,"\t\t   ��Ƶ������(������)\n");
	WriteTermStr(C_WHITE,1,"ANo  ״̬   LastSeq NewEle  ��ֵ(kbps)  ƽ��ֵ(kbps)\t\t\t\n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_state(i,i*2,&audio_encoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t   ��Ƶ������(������)\n");
	WriteTermStr(C_WHITE,1,"ANo  ״̬   LastSeq NewEle  ��ֵ(kbps)  ƽ��ֵ(kbps)\t\t\t\n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_state(i,i*2+1,&audio_encoder[i*2+1]);
	}

	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ�������û���Ϣ(������)\n");
	WriteTermStr(C_WHITE,1,"ANo �û���          NextEle     �û���  ����ʱ��(s) \n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_usr_state(i,&audio_encoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ�������û���Ϣ(������)\n");
	WriteTermStr(C_WHITE,1,"ANo �û���          NextEle     �û���  ����ʱ��(s) \n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_usr_state(i,&audio_encoder[i*2+1]);
	}
}
void display_audio_dec_state(void)
{//��ʾ��Ƶ��������״̬
	int i;
	WriteTermStr(C_WHITE,0,"\t\t   ��Ƶ������\n");
	WriteTermStr(C_WHITE,1,"ANo  ״̬   LastSeq NewEle  ��ֵ(kbps)  ƽ��ֵ(kbps)\t\t\t\n");
	for(i=0;i<MAX_AUDIO_DECODER;i++)
	{
		show_audio_dec_state(i,&audio_decoder[i]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    ��Ƶ�������û���Ϣ\n");
	WriteTermStr(C_WHITE,1,"ANo �û���          NextEle     �û���  ����ʱ��(s) \n");
	for(i=0;i<MAX_AUDIO_DECODER;i++)
	{
		show_audio_dec_usr_state(i,&audio_decoder[i]);
	}
}





