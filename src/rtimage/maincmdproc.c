/** @file	       maincmdproc.c
 *   @brief 	���ղ�����������̷���������ĺ�������
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <gt_com_api.h>
#include <gate_cmd.h>
#include <gtthread.h>
#include <mod_com.h>
#include <mod_cmd.h>
#include <gt_errlist.h>
#include "mod_socket.h"
#include <gtifstat.h>
#include "avserver.h"
#include "net_aplay.h"
#include "play_back.h"
#define NET_STA_FILE        ("/tmp/net_sta_file")
#define SCR_FULL 0           //zsk add
#define SCR_QUAD 1         //zsk add
#define VSMAIN_PARA_FILE		"/conf/gt1000.ini"	//zsk add
#define MUST_PLAY               1
#define FIRST_PLAY				0 
///��������

static int 	            send_cmd_ch=-1;				///<���������ͨ����ʶ��
static int	            recv_cmd_ch=-1;				///<���������ͨ����(�ӱ�ģ��ĽǶȿ�)

static int	com_fd= -1; //���ͺͽ��������udp socket
static pthread_t modsocket_thread_id=-1;
static  struct rtimage_state_struct rt_stat={           ///<rtimageģ�鹤��״̬
    .mod_state=0,
    .net_enc_err=0,
    .net_enc_busy=0,
    .test_d1_flag=0,
    .reserve=0,
};              


unsigned long 	net_pkts_sta;			//���緢�͵����ݰ����



/** 
 *   @brief     ��������ʼ����������ͨѶ������ͨ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int init_com_channel(void)
{
	int devtype=get_devtype();
	if((devtype<T_GTVS3021))
  	{
		//��������ʹ��modcom�ӿ�
		send_cmd_ch=mod_com_init(MAIN_RECV_CMD_CHANNEL,MSG_INIT_ATTRIB);
		recv_cmd_ch=mod_com_init(MAIN_SEND_CMD_CHANNEL,MSG_INIT_ATTRIB);
		if((send_cmd_ch<0)||(recv_cmd_ch<0))
		{
			printf("init_com_channel error!!!\n");
			gtlogerr("init_com_channel error!!!\n");
			#if EMBEDED==0		
					exit(1);
			#endif			
			return -1;
		}
	}
	else
	{
		//��������ʹ��modsocket�ӿ�
		com_fd	=	mod_socket_init(0,0);
	}

	return 0;
}

/** 
 *   @brief     �������������
 *   @param  send Ҫ���͵������
 *   @param  len    mod_com_type�ṹ�� para�ֶε���Ч��Ϣ����
 *   @return   0��ʾ�ɹ�����ֵ��ʾʧ��
 */ 
static int send_main_cmd(struct mod_com_type *send,int len)
{
	send->target=MAIN_PROCESS_ID;
	send->source=RTIMAGE_PROCESS_ID;
	send->cmdlen=len+2;
	return mod_com_send(send_cmd_ch,send,0);
}


int send_rtimg_stop_playback(void)
{
  int devtype;
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;


  devtype=get_devtype();
  if((devtype<T_GTVS3021))
  {
    //��Ҫ��gtvs1000ʹ��
	  struct mod_com_type send;
	  send.cmd=RTIMG_PLAYBACK_STOP_CMD;
	  gtloginfo("��vsmain������[RTIMG_PLAYBACK_STOP_CMD]\n");
	  return send_main_cmd(&send,0);
  }
  else
  {
    //��gtvs3000��������
  	cmd=(mod_socket_cmd_type *)socketbuf;
	  cmd->cmd	=	RTIMG_PLAYBACK_STOP_CMD;
	  cmd->len	=	0;
	  gtloginfo("��vsmain����RTIMG_PLAYBACK_STOP_CMD\n");
	  return mod_socket_send(com_fd,MAIN_PROCESS_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
  }

}

void require_videoenc_iframe(int venc_no)
{
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	cmd=(mod_socket_cmd_type *)socketbuf;
 	cmd->cmd	=	REQUIRE_IFRAME;
  	cmd->len	=	sizeof(venc_no);
	memcpy((int *)&cmd->para,(int *)(&venc_no),sizeof(int));	
	
  	mod_socket_send(com_fd,VIDEOENC_MOD_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	gtloginfo("��videoenc����REQUIRE_IFRAME\n");
	printf("��videoenc����REQUIRE_IFRAME\n");


}

void require_up_audio2enc(int aenc_no)
{
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	cmd=(mod_socket_cmd_type *)socketbuf;
 	cmd->cmd	=	REQUIRE_UP_AUDIO;
  	cmd->len	=	sizeof(aenc_no);
	memcpy((int *)&cmd->para,(int *)(&aenc_no),sizeof(int));	
	
  	mod_socket_send(com_fd,VIDEOENC_MOD_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	gtloginfo("��videoenc����REQUIRE_UP_AUDIO  [%d]\n",aenc_no);
	printf("��videoenc����REQUIRE_UP_AUDIO [%d]\n",aenc_no);


}

void stop_up_audio2enc(int aenc_no)
{
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	cmd=(mod_socket_cmd_type *)socketbuf;
 	cmd->cmd	=	STOP_UP_AUDIO;
  	cmd->len	=	sizeof(aenc_no);
	memcpy((int *)&cmd->para,(int *)(&aenc_no),sizeof(int));	
	
  	mod_socket_send(com_fd,VIDEOENC_MOD_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	gtloginfo("��videoenc����STOP_UP_AUDIO\n");
	printf("��videoenc����STOP_UP_AUDIO\n");


}

recv_send_down_audio_ack[MAX_AUDIO_ENCODER]={0};
int  send_down_audio2enc(int aenc_no,int mode)
{
	recv_send_down_audio_ack[aenc_no]=1;
	int ret=0;
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	cmd=(mod_socket_cmd_type *)socketbuf;
 	cmd->cmd	=	SEND_DOWN_AUDIO;
  	cmd->len	=	sizeof(aenc_no);
	cmd->para[0]=(char)aenc_no;
	cmd->para[1]=(char)mode;
	
  	mod_socket_send(com_fd,VIDEOENC_MOD_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	gtloginfo("��videoenc����SEND_DOWN_AUDIO [%d] mode[%d]\n",aenc_no,mode);
	printf("��videoenc����SEND_DOWN_AUDIO [%d] mode[%d]\n",aenc_no,mode);
	while((ret=recv_send_down_audio_ack[aenc_no])==1) //�Ѿ�����λ0(normal return)/3(busy)/2(not support)
	{
		sleep(1);
	}
	return ret;





}

void stop_down_audio2enc(int aenc_no)
{
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	cmd=(mod_socket_cmd_type *)socketbuf;
 	cmd->cmd	=	STOP_DOWN_AUDIO;
  	cmd->len	=	sizeof(aenc_no);
	memcpy((int *)&cmd->para,(int *)(&aenc_no),sizeof(int));	
	
  	mod_socket_send(com_fd,VIDEOENC_MOD_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	gtloginfo("��videoenc����STOP_DOWN_AUDIO\n");
	printf("��videoenc����STOP_DOWN_AUDIO\n");


}

/** 
 *   @brief     ����rtimageģ��ĵ�ǰ״̬��������
 *   @return   0��ʾ�ɹ���ֵ��ʾ����
 */ 
int send_state2main(void)
{
	int devtype=get_devtype();
	pid_t *pid;
	DWORD *state;
	DWORD tmp;
	memcpy((void*)&tmp,(void*)&rt_stat,sizeof(DWORD));
	if((devtype<T_GTVS3021))
	{
	
		
		DWORD buffer[10];
		struct mod_com_type *send;
		send=(struct mod_com_type *)buffer;
		pid=(pid_t*)send->para;
		state=(DWORD*)&send->para[sizeof(pid_t)];
		send->cmd=RTSTREAM_STATE_RETURN;
		*state=tmp;
		*pid=getpid();
		return send_main_cmd(send,sizeof(pid_t)+sizeof(DWORD));

	}
	else
	{
			
		DWORD socketbuf[20];
		mod_socket_cmd_type *cmd;
		cmd=(mod_socket_cmd_type *)socketbuf;
		cmd->cmd    =   RTSTREAM_STATE_RETURN;
		cmd->len    =   4+sizeof(pid_t);
		pid=(pid_t*)cmd->para;
		state=(DWORD*)&cmd->para[sizeof(pid_t)];
		*state=tmp;
		*pid=getpid();
		return mod_socket_send(com_fd,MAIN_PROCESS_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);

		
	}
	

}

/** 
 *   @brief    rtimage������Ҫ�л�����Ƶ����ͨ���Ÿ�ipmain,mutichannelʱ��ʹ��
 *   @return   0��ʾ�ɹ���ֵ��ʾ����
 */ 

int send_audio_channel2main(int channel)
{
	int * tmp;
	pid_t *pid;
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;

	cmd=(mod_socket_cmd_type *)socketbuf;
	cmd->cmd	=	RTIMG_AUDIODOWN_CMD;
	cmd->len	=	4+sizeof(pid_t);
	pid=(pid_t*)cmd->para;
	tmp=(int *)&cmd->para[sizeof(pid_t)];
	*tmp=channel;
	*pid=getpid();
	return mod_socket_send(com_fd,MAIN_PROCESS_ID,RTIMAGE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);

}

/** 
 *   @brief     ������������Ƶ����æ״̬
 *   @param  busy 0��ʾû���û����� 1��ʾ������һ���û���������Ƶ
 */ 
void set_net_enc_busy(int busy)
{
    static int old_busy=0;
    rt_stat.net_enc_busy=busy;
    if(old_busy!=busy)
    {
        old_busy=busy;
        send_state2main();
    }
}



static int process_gate_req_rtimg_modsocket(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{

	struct usr_req_rt_img_struct *req=(struct usr_req_rt_img_struct*)cmd->para;
	mod_socket_cmd_type *modsocket=NULL;
	struct gt_usr_cmd_struct *gtcmd=NULL;
	struct usr_req_rt_img_answer_struct *answer=NULL;
	DWORD   send_buf[200];
	DWORD 	result=0;
	DWORD   id;
	int ret;
	if((gate->gatefd<0)||(cmd->cmd!=USR_REQUIRE_RT_IMAGE))
		return -1;
	printf("recv USR_REQUIRE_RT_IMAGE cmd!!\n");
	gtloginfo("recv USR_REQUIRE_RT_IMAGE cmd!!\n");
	modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    memcpy(&modsocket->gate,gate,sizeof(gateinfo));
    gtcmd = (struct gt_usr_cmd_struct *)modsocket->para;
    gtcmd->cmd=USR_REQUIRE_RT_IMAGE_ANSWER;
    gtcmd->en_ack=0;
    gtcmd->reserve0=0;
    gtcmd->reserve1=0;
    answer=(struct usr_req_rt_img_answer_struct *)((char *)gtcmd->para);
    result = rtnet_av_connect2gate(req,&id);

    answer->status = result;
    answer->query_usr_id = id;    
    printf("answer return ,channel :%ld,status:%d\n",answer->query_usr_id, answer->status);
    
    gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)+sizeof(viewer_subsrcibe_answer_record_struct);
    modsocket->len = gtcmd->len +sizeof(gtcmd->len);
    ret = mod_socket_send(com_fd,MAIN_PROCESS_ID,PLAYBACK_PROCESS_ID,modsocket,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);

	return 0;
}

static int process_gate_stop_rtimg_modesocket(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
	struct usr_stop_rt_img_struct *req=(struct usr_stop_rt_img_struct*)cmd->para;
	if((gate->gatefd<0)||(cmd->cmd!=USR_STOP_RT_IMAGE))
		return -1;
	printf("recv USR_STOP_RT_IMAGE cmd!!\n");
	gtloginfo("recv USR_STOP_RT_IMAGE cmd!!\n");
	rtnet_av_close_connect(req->query_usr_id);
	return 0;
	


}


static int process_gate_req_rtsnd_modsocket(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
	struct usr_req_rt_img_struct *req;
//	struct sockaddr_in newaddr;
//	DWORD newip;
	WORD result=0;
	if((gate->gatefd<0)||(cmd->cmd!=USR_REQUIRE_SPEAK))
		return -EINVAL;
       gtloginfo("recv USR_REQUIRE_SPEAK cmd!!\n");
	req=(struct usr_req_rt_img_struct *)cmd->para;
#if 0
    TODO:����ʵ��
	if(req->mode==1)
	{// 1��ʾ��������
		newip=htonl(req->remoteip);
		memcpy(&newaddr.sin_addr,&newip,sizeof(DWORD));		
		result=set_valid_service_guest(get_rtsnd_struct(),&newaddr);	
	}
	else
	{//0��ʾ�Ͽ�����
		clear_valid_media_guest(get_rtsnd_struct());
		result=RESULT_SUCCESS;
	}
#endif
	
	if((cmd->en_ack!=0)||(result!=RESULT_SUCCESS))
	{
		send_ack_to_main(com_fd,RTIMAGE_PROCESS_ID,cmd->cmd,result,gate);
	}
	
	return 0;
}



void process_playback(int enc_no)
{
	if(enc_no>4||enc_no<0)
	{
		gtloginfo("Are you kidding me? channel is [%d]\n",enc_no);
		return;
	}

	tcprtimg_svr_t    *p=get_rtimg_para();

	mutichannel_set_playback_en(enc_no);
	//send_down_audio2enc(enc_no,MUST_PLAY);
	
	
}

void process_playback_cancel(int enc_no)
{
	tcprtimg_svr_t    *p=get_rtimg_para();
	mutichannel_set_playback_cancel(enc_no);
	//send_down_audio2enc(enc_no,MUST_PLAY);
}



int process_modsocket_gate_cmd(gateinfo *gate, struct gt_usr_cmd_struct *cmd)
{
	switch(cmd->cmd)
	{
		case USR_REQUIRE_RT_IMAGE:
        	printf("rtimage recv a bypass cmd:USR_REQUIRE_RT_IMAGE netfd=%ld\n",gate->gatefd);
			process_gate_req_rtimg_modsocket(gate,cmd);
		break;
		case USR_STOP_RT_IMAGE:
			printf("rtimage recv a bypass cmd:USR_STOP_RT_IMAGE netfd=%ld\n",gate->gatefd);
			process_gate_stop_rtimg_modesocket(gate,cmd);
		break;
		case USR_REQUIRE_SPEAK:
			printf("rtimage recv a bypass cmd:USR_REQUIRE_SPEAK netfd=%ld\n",gate->gatefd);
			process_gate_req_rtsnd_modsocket(gate,cmd);			
		break;
		
		default:
			printf("rtimage recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
			send_ack_to_main(com_fd,RTIMAGE_PROCESS_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT,gate);
		break;
	}
	return 0;
}




static int process_modsocket_cmd(int sourceid,mod_socket_cmd_type *modsocket)
{
	int rc;
    char no;
	char  status;
	rc=0;
	tcprtimg_svr_t    *p=get_rtimg_para();
	//if(sourceid !=MAIN_PROCESS_ID)
	//	return -EINVAL;
	switch(modsocket->cmd)
	{
		case GATE_BYPASSTO_MOD_CMD://��������ת��������������
			rc= process_modsocket_gate_cmd(&modsocket->gate,(struct gt_usr_cmd_struct *)&modsocket->para);
			//para�Ŀ�ͷ4���ֽڴ�ŵ��������̶����ص�������
			break;
			
		case MAIN_QUERY_STATE:
			gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
			send_state2main();
			break;

		case MAIN_PLAYBACK_IMG_CMD:
			gtloginfo("recv MAIN_PLAYBACK_IMG_CMD !\n");
			process_playback(*(int *)&modsocket->para);	//�˴�Ϊ0����������Ƕ�gtvs1000ʹ�õģ�����3000���豸û������
			break;

		case RTIMG_PLAYBACK_STOP_CMD:
			gtloginfo("recv RTIMG_PLAYBACK_STOP_CMD !\n");
			process_playback_cancel(*(int *)&modsocket->para);
			break;
		case SEND_DOWN_AUDIO_ACK:
			gtloginfo("recv SEND_DOWN_AUDIO_ACK !\n");
			no=modsocket->para[0];
			status=modsocket->para[1];
			printf("recv SEND_DOWN_AUDIO no=%d status=%d\n",no,status);
			recv_send_down_audio_ack[no]=status; 
			break;

		default:
			printf("tcprtimg recv a unknown cmd %x:",modsocket->cmd);
			gtloginfo("tcprtimg recv a unknown cmd %x:",modsocket->cmd);
		break;
	}
	return rc;
	
}


/** 
 *   @brief     �������ղ����������̷�����������߳�
 *   @param  ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int creat_mod_cmdproc_thread(void)
{
	//gt_create_thread(&cmdproc_thread_id, proc_mod_cmd_thread, NULL);
	creat_modsocket_thread(&modsocket_thread_id,com_fd,RTIMAGE_PROCESS_ID, "tcprtimg", process_modsocket_cmd);
	return 0;
}





static void *proc_snd_pkts_thread(void *para)
{
        int cnt;
        unsigned long tmp_snd_pkts=0;
        unsigned long old_snd_pkts=0;
        unsigned long snd_pkts;
        unsigned long snd_pkts_total=0;
        unsigned long file_cnt=0;
        FILE *fp=NULL;
        char buff[32];
        
        fp=fopen(NET_STA_FILE,"w+");
        if(fp==NULL)
        {
            gtlogerr("create net_sta_file error \n");
            return NULL;    
        }
        
        while(1)
        {
                tmp_snd_pkts= get_send_pkts();
                snd_pkts=tmp_snd_pkts - old_snd_pkts;
                old_snd_pkts=tmp_snd_pkts;
                //net_pkts_sta=snd_pkts;
               // printf("========= ================================snd_pkts=[%ld]..............\n",snd_pkts);
                usleep(100000);

                snd_pkts_total+=snd_pkts;
		
                if(cnt++>=20)
                {
                    cnt=0;
                    snd_pkts_total=snd_pkts_total/20;
                    //zsk del printf("************************=====================================**snd_pkts_total=%ld\n",snd_pkts_total);
                    net_pkts_sta=snd_pkts_total;
                    snd_pkts_total=0;
                    memset(buff,0,sizeof(buff));
                    sprintf(buff,"%ld------%ld\n",file_cnt++,net_pkts_sta);
                    fputs(buff,fp);
                    fflush(fp);
                }
        }
        fclose(fp);
	
        return NULL;
}



/**
*	@brief 	����������緢�͵����ݰ�����
*	@param	��
*	@return	0
*/
int creat_snd_pkts_thread(void)
{
	pthread_t snd_pkts_thread_id;

	////zw-add 2011-05-25
	init_gtifstat();
	////zw-add 2011-05-25
	gt_create_thread(&snd_pkts_thread_id,proc_snd_pkts_thread,NULL);
    
	return 0;
}



