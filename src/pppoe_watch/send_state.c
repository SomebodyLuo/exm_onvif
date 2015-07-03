
//wsyע:���ñ��뿪�صķ�ʽѡ��������ǰ����Ϣ���л�����udp-socketʵ��
//�������úͽӿڱ��ֲ��䡣

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <mod_com.h>
#include <mod_cmd.h>
#include <mod_socket.h>
#include <string.h>
#include "send_state.h"
#include "gtlog.h"




#ifndef DWORD
	#define DWORD unsigned long
#endif

static int send_cmd_ch=-1,recv_cmd_ch=-1;//��������ͽ��������ͨ����(�ӱ�ģ��ĽǶȿ�)
static int	com_fd= -1; //���ͺͽ��������udp socket

static pthread_t recv_modsocket_thread_id = -1;
	

static int current_state = PPPOE_SUCCESS;  //����״̬

/*
 *��ʼ����������ͨѶ������ͨ��
 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/

int init_com_channel(void)
{
	send_cmd_ch=mod_com_init(MAIN_RECV_CMD_CHANNEL,MSG_INIT_ATTRIB);
	recv_cmd_ch=mod_com_init(MAIN_SEND_CMD_CHANNEL,MSG_INIT_ATTRIB);
	if((send_cmd_ch<0)||(recv_cmd_ch<0))
	{
		return -1;
	}
	
	com_fd	=	mod_socket_init(0,0);	
	return 0; 
}

//�������������
//len:mod_com_type�ṹ�� para�ֶε���Ч��Ϣ����
static int send_main_cmd(struct mod_com_type *send,int len)
{
	send->target=MAIN_PROCESS_ID;
	send->source=PPPOE_WATCH_ID;
	send->cmdlen=len+2;
	return mod_com_send(send_cmd_ch,send,0);
}

/*
 * ����״̬�������� 
 * ����:stat:Ҫ���͵�״̬
 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int send_pppoe_stat2main(int stat)
{
	
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	int sendstat;
	DWORD *state;
	pid_t *pid;
	DWORD buffer[10];
	struct mod_com_type *send;
	
	current_state = stat;
	sendstat=stat;
	send=(struct mod_com_type *)buffer;
	pid=(pid_t*)send->para;
	state=(DWORD*)&send->para[sizeof(pid_t)];
	send->cmd=PPPOE_STATE_RETURN;
	*state=sendstat;
	*pid=getpid();
	send_main_cmd(send,sizeof(pid_t)+sizeof(DWORD));
	
	cmd=(mod_socket_cmd_type *)socketbuf;
	cmd->cmd	=	PPPOE_STATE_RETURN;
	cmd->len	=	4+sizeof(pid_t);
	pid=(pid_t*)cmd->para;
	*pid=getpid();
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	*state=sendstat;
	mod_socket_send(com_fd,MAIN_PROCESS_ID,PPPOE_WATCH_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);

	return 0;
	
}

//����������ģ��Ĳ�ѯ���������״̬
static int process_modsocket_cmd(int sourceid , mod_socket_cmd_type *modsocket)
{	
	printf("pppoe_watch recved a module-cmd from id %d\n",sourceid);
	switch (sourceid)
	{
		case MAIN_PROCESS_ID:	
			if(modsocket->cmd == MAIN_QUERY_STATE) //��ѯ״̬
				send_pppoe_stat2main(current_state);
		break;
		default:
		break;
	}
	return 0;
}


void *recv_modcom_thread (void *data)
{
	
	int ret;
	char buf[MAX_MOD_CMD_LEN];
	struct mod_com_type *recv;

	printf("pppoe_watch start recv_modcom_thread!\n");
	gtloginfo("pppoe_watch start recv_modcom_thread!\n");
	
	recv=(struct mod_com_type*)(buf);	
	while(1)
	{
		
		ret=mod_com_recv(recv_cmd_ch,PPPOE_WATCH_ID,recv,MAX_MOD_CMD_LEN,0);
		if(ret>0)
		{
			if(recv->cmd == MAIN_QUERY_STATE) //��ѯ״̬
			{
				gtloginfo("recv MAIN_QUERY_STATE cmd!\n");
				send_pppoe_stat2main(current_state);	
			}
		}
	}
	return NULL;
}

int creat_recv_modsocket_thread(void)
{
	return creat_modsocket_thread(&recv_modsocket_thread_id,com_fd,PPPOE_WATCH_ID,"pppoe_watch", process_modsocket_cmd);
}	


