/** @file	main.c
 *   @brief 	rtimage2ģ�������������
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <signal.h>


#include <commonlib.h>

#include "maincmdproc.h"
#include "net_avstream.h"
#include "avserver.h"
#include "net_aplay.h"
#include  "debug.h"
//zsk add
//#include <mcheck.h>
#include <stdlib.h>
#ifdef FOR_PC_MUTI_TEST
#include "pc_multi_test.c"
#endif
/** 
 *   @brief     ����־�ϼ�¼�˳�����
 *   @param  signo �ź������
 */
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:       ///<���ѹرյ�socket��д�����ź�
			printf("process_sig_pipe \n");	
			return ;
			break;
		case SIGTERM:      ///<��ͨ��kill�ź�
		case SIGKILL:       ///<kill -9�ź�
			gtloginfo("tcprtimg ��kill,�����˳�!!\n");
			close_all_res();
			exit(0);
			break;
		case SIGINT:         ///<ctrl-c�ź�
			gtloginfo("tcprtimg ���û���ֹ(ctrl-c)\n");
			close_all_res();
			exit(0);
			break;
		case SIGSEGV:       ///<�δ����ź�
			gtloginfo("tcprtimg �����δ���\n");
			printf("tcprtimg segmentation fault\n");
			close_all_res();
			exit(0);
			break;
	}
	return;
}

/** 
 *   @brief     tcprtimage2ģ����봦���߳�,������õĺ�����Ӧ��������
 *   @param  ��
 */
static void second_proc_thread(void)
{
	printf(" start second_proc_thread...\n"); 	
	gtloginfo(" start second_proc_thread...\n"); 	
	while(1)
	{
		sleep(1);
		avserver_second_proc();
	}
	return ;
}

int process_opt_h(void)
{
	printf("ʵʱ����Ƶ�������tcprtimg version:%s\n",version);
	printf("�÷�:tcprtimg [OPTION] [argument]\n");
	printf("OPTION ѡ��˵��\n");
	printf("-h:��ʾ������Ϣ\n");
	printf("-v:��ʾ�汾��Ϣ���˳�����\n");
	return 0;
}

/** 
 *   @brief     ����tcprtimage���������
 *   @param  argc ������Ŀ
 *   @param  argv ����ֵ����
 *   @return   0��ʾ�ɹ� ,��ֵ��ʾ����
 */
int process_argument(int argc,char **argv)
{
	int oc;
	if(argc<2)
	{
		return 0;
	}
	printf("*************************************************\n");
	while((oc=getopt(argc,argv,"hv"))>=0)
	{
		switch(oc)
		{
			case 'h':
				process_opt_h();
				exit(0);
				break;
			case 'v':
				printf("tcprtimage version:%s\n",version);
				create_lockfile_save_version(RT_LOCK_FILE,(char*)version);
				printf("*************************************************\n");
				exit(0);
				break;
			default:
				break;
		}
	}

	printf("*************************************************\n\n\n");
	return 0;
}

/** 
 *   @brief  tcprtimage2ģ�����ں���
 */
int main(int argc,char **argv)
{
	//setenv("MALLOC_TRACE", "/mnt/zsk/mtrace.log", 1);
	//mtrace();

	int ret;
	close_all_res();                            ///<�ر����м̳��������Ѵ���Դ
	setsid();                                      ///<����������Ϊ�׽���
	gtopenlog("rtimage");                  ///<����־

	process_argument(argc,argv);
	///�ж�ģ���Ƿ��Ѿ�����
	if(create_lockfile_save_version(RT_LOCK_FILE,(char*)version)<0)
	{
		printf("rtimage module are running!!\n");
		gtloginfo("rtimage module are running!!\n");
		exit(1);
	}

	///<��ʾ������Ϣ
	printf     ("[rtimage(ver:%s)] process run pid=%d \n",version,getpid());
	gtloginfo("[rtimage(ver:%s)] process run pid=%d \n",version,getpid());

	///ע���źŴ�����
	signal(SIGKILL,exit_log);		 ///<kill -9�ź�
	signal(SIGTERM,exit_log);		 ///<��ͨ��kill�ź�
	signal(SIGINT,exit_log);		        ///<ctrl-c�ź�
	signal(SIGSEGV,exit_log);		 ///<�δ����ź�
	signal(SIGPIPE,exit_log);		 ///<���ѹرյ�socket��д�����ź�
	signal(SIGUSR1,begin_debug);               //��ʼdebug��Ϣ���ź�
	signal(SIGUSR2,end_debug);						//����debug��Ϣ���ź�
	init_devinfo();                                ///<��ʼ���豸��Ϣ

	ret=init_server_para();               ///<ϵͳ������״̬��ʼ��

	if(ret<0)
	{
		printf    ("���ܷ�������ڴ�,�����˳�!\n");
		gtlogerr("���ܷ�������ڴ�,�����˳�!\n");
		exit(1);
	}

	read_server_para_file();           ///<�������ļ��л�ȡ����
	init_com_channel();			///<��ʼ����������ͨѶ������ͨ��


	creat_mod_cmdproc_thread(); ///<�������ղ����������̷�����������߳�
	create_av_server();                  ///<��������Ƶ���з����߳�
	
    create_rtnet_av_servers();      ///<������������Ƶ����socket���̳߳�
    create_rtnet_aplay_servers(); ///<������Ƶ���з���socket���̳߳�



    second_proc_thread();              ///<ת��Ϊ�봦���߳�   

    exit(0);
		       
}


