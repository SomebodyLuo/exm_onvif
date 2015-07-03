/*������ʵʱͼ����ģ��ͨѶ������

*/
#include "ipmain.h"
#include "gate_cmd.h"
#include "maincmdproc.h"
#include "netcmdproc.h"
#include "process_rtimg.h"
#include "ipmain_para.h"
//#include "hdmodule.h"
#include "devstat.h"
#include "hdmodapi.h"
#ifdef ARCH_3520D
#include "audioout_api.h"
#endif

static DWORD	old_rtimg_state=0;

#if 0
//��ȡʵʱͼ���ͽ��̵�״̬
//��������0 �й��Ϸ���1
int get_netenc_state(void)
{
	struct ip1004_state_struct * gtstate;
	int state;
	gtstate=get_ip1004_state();
	pthread_mutex_lock(&gtstate->mutex);
	state=gtstate->reg_dev_state.video_enc0_err;
	pthread_mutex_unlock(&gtstate->mutex);	
	return state;
}
#endif

int process_rtimg_state(mod_socket_cmd_type *cmd)
{//
	pid_t *pid;
	DWORD *state;
	struct rtimage_state_struct *newstate,*change_state;
	struct ip1004_state_struct * gtstate;
	struct ipmain_para_struct *main_para;
	//struct hd_enc_struct	*hd_enc=NULL;
	DWORD	change;
	if(cmd->cmd!=RTSTREAM_STATE_RETURN)
		return -1;
	pid=(pid_t *)cmd->para;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	printf("recv rtimg state:pid=%d state=0x%08x\n",(int)*pid,(int)*state);
	gtloginfo("�յ�rtimage״̬:pid=%d state=0x%08x\n",(int)*pid,(int)*state);
	*state&=(DWORD)(~1);
	//printf("process_rtimg_state state is %x\n",*state);
	if(old_rtimg_state!=*state)
	{
		change=old_rtimg_state^*state;
		change&=0x3f;			//
		//printf("process_rtimg_state change is %x\n",change);
		if(change!=0)
		{
			change_state=(struct rtimage_state_struct*)&change;			
			newstate=(struct rtimage_state_struct *)state;	
			if(change_state->net_enc_err)
			{
				gtstate=get_ip1004_state(0);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_dev_state.video_enc0_err=newstate->net_enc_err;
				pthread_mutex_unlock(&gtstate->mutex);
				//gtloginfo("��Ƶ��ʧ��оƬ���ϣ�����״̬\n");
#ifdef SHOW_WORK_INFO
				printf("��Ƶ��ʧ��оƬ���ϣ�����״̬\n");
#endif
				send_dev_state(-1,1,0,0,0,0);
			}
			if(change_state->net_enc_busy)
			{
				if(!newstate->net_enc_busy)
				{//����
#if 0				
					if(get_quad_flag()==1)
						{
							//lc do ������������(30 sec)ʱ���л���Ĭ��ͨ��
							
							main_para=get_mainpara();
							if(main_para->net_ch<4)
								set_net_scr_full(main_para->net_ch);
							else
								set_net_scr_quad();
							
#ifdef SHOW_WORK_INFO
							printf("ͼ�����ӿ��� �л���Ĭ��ͨ��:%d\n",main_para->net_ch);
#endif							
							gtloginfo("ͼ�����ӿ��� �л���Ĭ��ͨ��:%d\n",main_para->net_ch);
						}
#endif
				}

			}
		}		
		old_rtimg_state=*state;
	}
	return 0;
}


void process_playback_stop_cmd()
{
	gtloginfo("��videoenc����ֹͣ¼��ط�����RTIMG_PLAYBACK_STOP_CMD\n");
	alarm_cancel_playback();
}

extern pthread_mutex_t g_audiodown_channel_mutex;

void process_audiodown_cmd(mod_socket_cmd_type *cmd)
{
	pid_t *pid;
	int   *p_audio_down_chan;
	struct ipmain_para_struct * para;
	if(cmd->cmd!=RTIMG_AUDIODOWN_CMD)
		return -1;

	pid=(pid_t *)cmd->para;
	p_audio_down_chan=(int*)&cmd->para[sizeof(pid_t)];
	printf("recv rtimg audio down channel:pid=%d chn=%d\n",(int)*pid,(int)*p_audio_down_chan);
	gtloginfo("�յ�rtimage����ͨ����:chn=%d\n",(int)*p_audio_down_chan);

	para=get_mainpara();

	pthread_mutex_lock(&g_audiodown_channel_mutex);
	para->current_audio_down_channel = (int)*p_audio_down_chan;
	pthread_mutex_unlock(&g_audiodown_channel_mutex);

	return;
	
}


/**********************************************************************************************
 * ������	:process_rtimg_cmd()
 * ����	:����tcprtimgģ�鷢��������
 * ����	:cmd:���յ���tcprtimgģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_rtimg_cmd(mod_socket_cmd_type *cmd)
{
	switch(cmd->cmd)
	{
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv tcprtimg MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv tcprtimg MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
			break;

		case RTSTREAM_STATE_RETURN:
			process_rtimg_state(cmd);
			break;

		case RTIMG_PLAYBACK_STOP_CMD:
			printf("���յ�RTIMG_PLAYBACK_STOP_CMD\n");
			gtloginfo("���յ�RTIMG_PLAYBACK_STOP_CMD\n");
			process_playback_stop_cmd();
			break;
		case RTIMG_AUDIODOWN_CMD:
			printf("���յ�RTIMG_AUDIODOWN_CMD\n");
			gtloginfo("���յ�RTIMG_AUDIODOWN_CMD\n");
			process_audiodown_cmd(cmd);
			break;
					
		default:
			printf("vsmain recv a unknow cmd:%04x from rtimg\n",cmd->cmd);
			gtloginfo("recv a unknow cmd:%04x from rtimg\n",cmd->cmd);
			break;
	}
	return 0;
}




