/* send state to vsmain/vmmain*/



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

static pthread_t recv_modsocket_id = -1;
 	

static int send_cmd_ch=-1,recv_cmd_ch=-1;//��������ͽ��������ͨ����(�ӱ�ģ��ĽǶȿ�)
static int	com_fd= -1; //���ͺͽ��������udp socket


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
		printf("init_modcom_channel error!!!\n");
		return -1;
	}
	
	com_fd	=	mod_socket_init(0,0);	
	if(com_fd <=0)
	{
		printf("init_modsocket_channel error!!!\n");
		return -1;
	}
	return 0;
}

//�������������
//len:mod_com_type�ṹ�� para�ֶε���Ч��Ϣ����
static int send_main_cmd(struct mod_com_type *send,int len)
{
	send->target=MAIN_PROCESS_ID;
	send->source=UPNPD_MOD_ID;
	send->cmdlen=len+2;
	return mod_com_send(send_cmd_ch,send,0);
}


/*
 * ����״̬�������� 
 * ����:channel: ��ΪMOD_SOCKET_CHANNEL��MOD_COM_CHANNEL

 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int send_upnpd_stat2main(int channel)
{
	DWORD *state;
	pid_t *pid;
	DWORD buffer[10];
	struct mod_com_type *send;
	int stat;
		

	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	
	stat = get_current_state();
	if((stat == UPNPD_STATE_INIT) ||(stat == UPNPD_STATE_GETROUTER))
		return 0;
	
	if(channel == MOD_COM_CHANNEL)
	{
		
		send=(struct mod_com_type *)buffer;
		pid=(pid_t*)send->para;
		state=(DWORD*)&send->para[sizeof(pid_t)];
		send->cmd=UPNPD_STATE_RETURN;
		*state=stat;
		*pid=getpid();
		send_main_cmd(send,sizeof(pid_t)+sizeof(DWORD));
	}

	if(channel == MOD_SOCKET_CHANNEL)
	{	
		
		cmd=(mod_socket_cmd_type *)socketbuf;
		cmd->cmd	=	UPNPD_STATE_RETURN;
		cmd->len	=	4+sizeof(pid_t);
		pid=(pid_t*)cmd->para;
		*pid=getpid();
		state=(DWORD*)&cmd->para[sizeof(pid_t)];
		*state=stat;
		mod_socket_send(com_fd,MAIN_PROCESS_ID,UPNPD_MOD_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
	}


	return 0;

}

/*����ǰ״̬���͸�modcom��modsocket����ͨ��*/
void send_state(void)
{
	send_upnpd_stat2main( MOD_COM_CHANNEL);
	send_upnpd_stat2main( MOD_SOCKET_CHANNEL);
	
}


static int process_modsocket_cmd(int sourceid, mod_socket_cmd_type *modsocket)
{
	printf("upnpd recved a module-cmd from id %d\n",sourceid);
	switch (sourceid)
	{
		case MAIN_PROCESS_ID:	
			if(modsocket->cmd == MAIN_QUERY_STATE) //��ѯ״̬
				send_upnpd_stat2main(MOD_SOCKET_CHANNEL);
		break;
		default:
		break;
	}
	return 0;
}



//��������vsmain�Ĳ�ѯ���������״̬
void *recv_modcom_thread (void *data)
{
	int len;
	struct mod_com_type recv;


	printf("start recv_modcom_thread!\n");
	gtloginfo("start recv_modcom_thread!\n");
	while(1)
	{
		len = mod_com_recv(recv_cmd_ch, UPNPD_MOD_ID, &recv,MAX_MOD_CMD_LEN, 0);
		{
			printf("upnpd recved a module-cmd from vsmain\n");
			if(recv.cmd == MAIN_QUERY_STATE) //��ѯ״̬
			{
				gtloginfo("recv MAIN_QUERY_STATE cmd\n");
				send_upnpd_stat2main( MOD_COM_CHANNEL);
			}
		}

	}
	return NULL;
}

int create_recv_modsocket_thread(void)
{
	return creat_modsocket_thread(&recv_modsocket_id,com_fd, UPNPD_MOD_ID,"upnpd",process_modsocket_cmd);
}


