#include "vs3_videoenc.h"
#include <commonlib.h>
#include <signal.h>

#include "gtthread.h"
#include "videoencoder.h"
#include "watch_process.h"
#include "device_state.h"
#include <devinfo.h>
#include <devres.h>
#include "process_modcmd.h"
#include "onviflib.h"
#include "onvif_system.h"
//#include "onvifcb.h"


//����־�ϼ�¼�˳�����
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
		case SIGKILL:
			gtloginfo("onvifbox ��kill,�����˳�!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("onvifbox ���û���ֹ(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1:
			//���ϵͳ��Ϣ��ָ���ļ�
			//dump_sysinfo();
		break;
		case SIGSEGV:
			gtloginfo("onvifbox �����δ���\n");
			printf("onvifbox segmentation fault\n");
			close_all_res();
			exit(0);
		break;
	}
	return;
}

void regist_signals(void)
{
	signal(SIGTERM,exit_log);	//kill�ź�
	signal(SIGKILL,exit_log);	//kill -9 �ź�
	signal(SIGSEGV,exit_log);	//�δ����ź�
	signal(SIGPIPE, exit_log);	//���Ѿ��رյ�������д����
	signal(SIGUSR1,SIG_IGN);	
}



int main(int argc,char *argv[])
{
	int lock_fd;
    
	close_all_res();
	setsid();
	gtopenlog("onvifbox");							//����־��¼
	lock_fd=create_lockfile_save_version(ENC_LOCK_FILE,VERSION);	//�����ļ�
	if(lock_fd<0)
	{
		printf("encbox module are running!!\n");
		exit(1);
	}



	printf("����onvifbox(ver:%s).......\n",VERSION);
	gtloginfo("����onvifbox(ver:%s).......\n",VERSION);
	



       
	regist_signals();						 //ע���źŴ�����
	if(GT_SUCCESS!=init_devinfo())	                                    //��ʼ���豸��Ϣ
	{
		gtloginfo("init_devinfo err!\n");
		exit(1);
	}
	init_com_channel();                               //��ʼ��ͨѶ����ͨ��
	if(GT_SUCCESS!=creat_modcmdproc_thread())          //����ͨѶ�߳�*/
	{   
		gtloginfo("create_modecmdporc_thread err!\n");
		exit(1);
	}


	if(GT_SUCCESS!=onvif_lib_init())						//��ʼ��onviflib
	{

		gtloginfo("onvif_lib_init err!\n");
		exit(1);
	
	}
	if(GT_SUCCESS!=init_media_system())					//��ʼ��ý����Ϣ
	{

		gtloginfo("init_media_system err!\n");
		exit(1);

	}
	
	if(GT_SUCCESS!=init_onvif_system())		//���������ļ���ʼ���������
	{

		gtloginfo("init_onvif_system failed!\n");
		exit(1);
	
	}
	create_onvif_device_thread();
	create_ip_device_thread();


	second_proc();
	exit(0);
}

