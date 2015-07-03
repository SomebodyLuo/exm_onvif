#include <stdio.h>
#include "play_back.h"
//zsk Ϊ¼��ط���ӵģ���Ϊ�߳��������Ż�Ϊpthread_rwlock?�ദ����ʱ
static playback_t pb;
int default_screen=4;
extern int debug;
playback_t * get_playback_parm(void)
{

	return &pb;


}

void init_playback_parm(void)
{
	int i;
	playback_t * pb=get_playback_parm();
	memset(pb->pb_venc,0,sizeof(pb->pb_venc));//�طŻ��������״̬
	memset(pb->pb_aenc,0,sizeof(pb->pb_aenc));
	memset(pb->pb_vct,0,sizeof(pb->pb_vct));
	memset(pb->pb_act,0,sizeof(pb->pb_act));
	
	memset(pb->pb_ct,0,sizeof(pb->pb_ct));//����ʱ��
	memset(pb->frame_adjust,0,sizeof(pb->frame_adjust));//�������ط�ָ��ƫ�ƣ�>0
	memset(pb->aframe_adjust,0,sizeof(pb->aframe_adjust));//�������ط�ָ��ƫ�ƣ�>0
	
	
	
	for(i=0;i<MAX_VIDEO_ENCODER;i++)
	{
	
			pb->pb_first_fg[i]=-1;//������������״̬��>0���������ӣ�=0��������

			pb->playback_flag[i]=-1; //-2 �ص�ʵʱ���浫�ط�û����;-1 ����״̬;>0ͨ���ط�

	
	}
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
		pb->current_net_ch[i]=4;

	pb->default_screen=4;
	pthread_mutex_init(&pb->mutex,NULL);
}


void set_playback_stat(int no,int stat);
void mutichannel_set_playback_en(int no)
{
	//ʹ��¼��ط�
	int stat;
	int i;
	stat=get_playback_stat(no);
	playback_t * pb=get_playback_parm();
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t	*send_info =NULL;
	//��û��ʼ

	if (stat<0)
	{
		if(pb->pb_venc[no]==-1)//�жϻطŻ�����Ƿ�������
		{
			//pthread_mutex_lock(&pb->mutex);
			set_playback_stat(no,no);
			//pthread_mutex_unlock(&pb->mutex);
			gtloginfo("��ʼ¼��طţ�ͨ��[%d]\n",no);
		}
		else
		{


			gtloginfo("��û�д洢��ô�����ݣ����ܻط�\n");
			return;

		}
		
	}
	//�ڻطŹ������ظ�����ͬһͨ��
	else if(get_playback_stat(no)!=-1)
	{
		gtloginfo("ͨ��[%d]�ط��У��ٴλط�[%d]ͨ��\n",stat,no);
		return;
		

	}
	

	//��ʼ����
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{

		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==no)
		{
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			pthread_mutex_lock(&pb->mutex);
			pb->pb_ct[no]=p->frame_rate*(p->playback_pre+p->playback_dly);
			pthread_mutex_unlock(&pb->mutex);
			break;
		}

	}



	gtloginfo("playback_flag==%d\n",pb->playback_flag[no]);
	return;
}


void mutichannel_set_playback_cancel(int no)
{
	//ȡ��¼��ط�
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	memset(pb->playback_flag,-1,sizeof(pb->playback_flag));//-2 �ص�ʵʱ���浫�ط�û����;-1 ����״̬;>0ͨ���ط�
	memset(pb->pb_ct,0,sizeof(pb->pb_ct));//����ʱ��

	pthread_mutex_unlock(&pb->mutex);
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t  *send_info =NULL;
	int i;
	int ret;
	for(i=0;i<MAX_VIDEO_ENCODER;i++)
	{
		ret=get_playback_frame_adjust(i);
		if(ret>0)
		{
			recover_playback_iframe2normal(i,ret);
			set_playback_frame_adjust(i,0);
		}

	}
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{
		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==no)
		{
			usr=&p->av_server.av_usr_list[i];
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			break;
		}

	}
	return ;
}

void mutichannel_set_playback_to_live(int enc_no)
{
	playback_t * pb=get_playback_parm();

	pthread_mutex_lock(&pb->mutex);
	pb->playback_flag[enc_no]=-2;
	pthread_mutex_unlock(&pb->mutex);
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t  *send_info =NULL;
	int i;
	//�ҵ��Ǹ��û����������enc
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{

		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==enc_no)
		{
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			break;
		}
	}

}

/****************************************************
 *������:get_playback_stat()
 *����  : ���ص�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
int get_playback_stat(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	if(no<0||no>=MAX_VIDEO_ENCODER)
	  no=0;
		
	ret = pb->playback_flag[no];
	return ret;
}
/****************************************************
 *������:set_playback_stat()
 *����  : ���õ�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
void set_playback_stat(int no,int stat)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->playback_flag[no]=stat;
	pthread_mutex_unlock(&pb->mutex);

}
/****************************************************
 *������:get_playback_frame_adjust()
 *����  : ���ص�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
int get_playback_frame_adjust(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	ret = pb->frame_adjust[no];
	//printf("get_playback_frame_adjust[%d][%d]\n",no,ret);
	return ret;
}
/****************************************************
 *������:set_playback_frame_adjust()
 *����  : ���õ�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
void set_playback_frame_adjust(int no,int frame_ct)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->frame_adjust[no]=frame_ct;
	pthread_mutex_unlock(&pb->mutex);

	gtloginfo("set_playback_frame_adjust[%d][%d]\n",no,pb->frame_adjust[no]);
	printf("set_playback_frame_adjust[%d][%d]\n",no,pb->frame_adjust[no]);

}
/****************************************************
 *������:get_playback_aframe_adjust()
 *����  : ���ص�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
int get_playback_aframe_adjust(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	ret = pb->aframe_adjust[no];


	return ret;
}
/****************************************************
 *������:set_playback_aframe_adjust()
 *����  : ���õ�ǰ¼��طŵı�־λ
 *����  : ��
 *����ֵ:��ֱ��ʾ����¼��طŵ�ͨ����-1ֹͣ¼��ط�
 * *************************************************/
void set_playback_aframe_adjust(int no,int frame_ct)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->aframe_adjust[no]=frame_ct;
	pthread_mutex_unlock(&pb->mutex);
	printf("set_playback_frame_adjust[%d][%d]\n",no,pb->aframe_adjust[no]);

}


