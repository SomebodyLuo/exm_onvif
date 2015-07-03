#include "ipmain.h"
#include "maincmdproc.h"
#include "process_rtimg.h"
#include "process_hdmod.h"
#include <gt_com_api.h>
#include "gate_connect.h"
#include "process_pppoe_watch_cmd.h"
#include "process_diskman.h"
#include "process_kbd.h"
#include "netcmdproc.h"
#include "process_videoenc.h"
#include "process_hw_diag.h"
#include "process_upnpd_cmd.h"
#include "process_hdplayback.h"
#include "mod_socket.h"
#include "devinfo.h"

static int	com_fd= -1; //���ͺͽ��������udp socket
static pthread_t modsocket_thread_id=-1;
/**********************************************************************************************
 * ������	:init_com_channel()
 * ����	:��ʼ����������ͨѶ������ͨ��
 * ����	:l��
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
int init_com_channel(void)
{
	com_fd	=	mod_socket_init(0,0);	
	if(com_fd <=0)
	{
		printf("ipmain init_modsocket error ret %d!!!\n",com_fd);
		gtlogerr("init_modsocket error ret %d!!!\n",com_fd);
	}
	return 0;
}

/**********************************************************************************************
 * ������	:main_send_cmd()
 * ����	:vsmain��ָ����ģ����ģ���socketͨѶ�������Ϣ
 * ����	:send:ָ��Ҫ���͵�����(mod_socket_cmd_type����)�Ļ�������ָ��(�Ѿ����������Ϣ)
 *		 target:Ŀ��ģ���id����Ϊ0���ʾ���͸�����ģ��
 *		 len:����������Ч���ݵĳ���
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int main_send_cmd(mod_socket_cmd_type * send, long int target, int len)
{
	int rc;
	if(send== NULL)
		return -EINVAL;

	rc = mod_socket_send(com_fd,target,MAIN_PROCESS_ID,send,len);
	return rc;
}

int bypass2gate(mod_socket_cmd_type *cmd)
{	//
//
	struct mod_com_type *modcom;
	DWORD buf[250];
	struct gt_usr_cmd_struct *cmdpkt;
	int rc,len;
	int net_fd;
	if(cmd->cmd!=MOD_BYPASSTO_GATE_CMD)
		return -1;
	

	gtloginfo("bypass2gate gatefd=%d env %d,enc %d \n",cmd->gate.gatefd,cmd->gate.env,cmd->gate.enc);
	modcom= (struct mod_com_type *)buf;
	
	cmdpkt=(struct gt_usr_cmd_struct*)(cmd->para);
	len=cmdpkt->len+2;
	net_fd = cmd->gate.gatefd;
	modcom->env = cmd->gate.env;
	modcom->enc = cmd->gate.enc;
	virdev_get_devid(cmd->gate.dev_no, cmdpkt->para);
	memcpy(&(modcom->para),cmd->para,len);
	rc=send_gate_pkt(net_fd,modcom,len,cmd->gate.dev_no);	

	if(rc<0)
	{
		printf("ipmain bypass2gate send pkt err!\n");
		gtlogerr("ipmain bypass2gate���Ͱ�����\n");
	}
	return rc;
}



//��ģ��������ģ�鷢�Ͳ�ѯ״̬����
int send_query_state_cmd(long int target)
{
	DWORD buffer[40];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)buffer;
	send->cmd=MAIN_QUERY_STATE;
	send->len = 0;
	return main_send_cmd(send,target,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

int process_gate_cmd_ack(mod_socket_cmd_type *cmd)
{

	struct usr_cmd_ack_struct *ack;
	
	if(cmd==NULL)
		return -EINVAL;

	if(cmd->gate.gatefd<=0)	
		return -EINVAL;
	ack = (struct usr_cmd_ack_struct *)cmd->para;
		
	
	return send_gate_ack(cmd->gate.gatefd, ack->rec_cmd,ack->result,cmd->gate.env,cmd->gate.enc,cmd->gate.dev_no);
}

/**********************************************************************************************
 * ������	:process_cmd()
 * ����	:��������ģ�鷢��������
 * ����	:cmd:����ģ�鷢��������Ļ�����
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
static int process_cmd(int sourceid,mod_socket_cmd_type *cmd)
{
	switch(sourceid)
	{
		case RTIMAGE_PROCESS_ID:
			process_rtimg_cmd(cmd);
		break;
		//lc do 2013.12.19
		case PLAYBACK_PROCESS_ID:
			process_hdplayback_cmd(cmd);
		break;
		//case RTAUDIO_PROCESS_ID:			
		//	process_rtaudio_cmd(cmd);
		//break;
		case HQSAVE_PROCESS_ID:
			process_hdmod_cmd(cmd);
		break;
		//lc do PPPOE_WATCH����
		case PPPOE_WATCH_ID:
			process_pppoe_watch_cmd(cmd);
		break;
		case DISKMAN_ID:
			process_diskman_cmd(cmd);
		break;

		//lc to do KEYBOARD������̺������
		/*
		case KEYBOARD_ID:
			process_keyboard_cmd(cmd);
		break;
		*/
		case VIDEOENC_MOD_ID:
			process_videoenc_cmd(cmd);
		break;
		//case HW_DIAG_MOD_ID:
		//	process_hw_diag_cmd(cmd);
		//break;

		//lc to do KEYBOARD������̺������
		/*
		case UPNPD_MOD_ID:
			process_upnpd_cmd(cmd);
		break;
		*/
		default:
			printf("ipmain recv a unknown mod cmd from mod id %x\n",sourceid);
            gtloginfo("ipmain recv a unknown mod cmd from mod id %x\n",sourceid);
			//process_upnpd_cmd(cmd);			//// lsk 2010-8-17 testing!!!!!!!!!!
		break;
	}
	
	return 0;
}




void cmd_proc_init_var(void)
{
	
}


/**********************************************************************************************
 * ������	:creat_cmdproc_thread()
 * ����	:�������ղ���������ģ�鷢����������߳�
 * ����	:attr:�߳�����ָ��
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
int creat_cmdproc_thread(pthread_attr_t *attr)
{
	return creat_modsocket_thread(&modsocket_thread_id,com_fd,MAIN_PROCESS_ID,"ipmain", process_cmd);
}




