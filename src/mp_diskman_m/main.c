#include "mp_diskman.h"
#include "diskmanager.h"
#include <commonlib.h>
#include "process_modcmd.h"
#include <devinfo.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "process_modcmd.h"
#include "mpdisk.h"
#include "diskinfo.h"
#include "hdutil.h"
#include "sqlite3.h"
#include "fileindex.h"
#include <errno.h>

#if 0
//�ռ��ӽ���
void collect_child(void)
{
	pid_t Pid;
	int Status;
	int i;
	for(i=0;i<5;i++)
	{
		Pid=waitpid(-1,&Status,WNOHANG);
		if((int)Pid>0)
          	{
				  gtloginfo("�ռ���һ���ӽ���\n");		
                  continue;
          	}
		else
			break;
	}
}
#endif



struct partition_info_struct part_info[MAX_DISKNO][PART_PER_DISK];

struct partition_info_struct * get_part_info(int diskno, int partno)
{
	if((diskno >= get_disk_no())||(partno >= PART_PER_DISK))
		return NULL;
	else	
		return &part_info[diskno][partno];
}

//����־�ϼ�¼�˳�����
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("diskman process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
			gtloginfo("diskman ��kill,�����˳�!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGKILL:
			gtloginfo("diskman SIGKILL,�����˳�!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("diskman ���û���ֹ(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1://���ϵͳ��Ϣ��ָ���ļ�
			dump_sysinfo();
		break;
		//case SIGCHLD:
			// collect_child();
		//break;
		case SIGSEGV:
			gtloginfo("diskman �����δ���\n");
			close_all_res();
			printf("diskman segmentation fault\n");
			exit(0);
		break;
	}
	return;
}

void second_proc(void) 
{
	while(1)
	{	
		//remove_oldest_file();
		sleep(10);
	}
}

//��ȡһ��Ĭ�����Խṹ
//������Ӧ���ͷ�
static int get_gtthread_attr(pthread_attr_t *attr)
{
	int rc;
	if(attr==NULL)
		return -1;
	memset((void*)attr,0,sizeof(pthread_attr_t));
	rc=pthread_attr_init(attr);
	if(rc<0)
		return -1;
	rc=pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);//����״̬
	rc=pthread_attr_setschedpolicy(attr,SCHED_OTHER);
	return 0;
}


/**********************************************************************************************
 * ������	:set_hd_capacity_to_parafile()
 * ����	:�����豸�Ĵ�������ֵ
 * ����	:value:�����⵽�Ĵ�������ֵMBΪ��λ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾ����
 **********************************************************************************************/
int set_hd_capacity_to_parafile(int value)
{
	dictionary *ini=NULL;
	FILE *fp=NULL;
	
	ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
	if(ini==NULL)
		return -EINVAL;
		
	if(value>=0)
	{
		iniparser_setint(ini,"resource:disk_capacity",value);
	}
	save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	
	if(virdev_get_virdev_number()==2)//�����豸
	{
		system("/ip1004/ini_conv -s");
	}
	return 0;
}

int main(void)
{
	pthread_attr_t  thread_attr,*attr;
	int lock_file;
	char pbuf[100];
	int i;
	int parttotal=0;//����Ӳ�̵ķ�������֮��
	long disktotal = 0; //����Ӳ�̵�������
#ifdef FOR_PC_MUTI_TEST
	gen_multi_devices();
#endif
	signal(SIGKILL,exit_log);
	signal(SIGTERM,exit_log);
	signal(SIGINT, exit_log);
	signal(SIGSEGV,exit_log);
	signal(SIGPIPE,exit_log);
	signal(SIGUSR1,exit_log);
	signal(SIGCHLD,exit_log);
	setsid();
	gtopenlog("diskman");

	lock_file=create_and_lockfile(DISKMAN_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("diskman module are running!!\n");
		gtlogerr("diskmanģ�������У���������Ч�˳�\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���
	//��������ж�ģ���Ƿ��Ѿ�ִ�еĹ���
#ifdef FOR_PC_MUTI_TEST
	SetOutputTty(lock_file);// �����Ӱ�䵽��һ��������
#endif
	printf("����diskman(ver:%s).......\n",VERSION);
	gtloginfo("����diskman(ver:%s).......\n",VERSION);

	////zw-test		
	//fileindex_init_filelock();
	////zw-test
	InitAllDabase();



	init_devinfo();//��ʼ���豸��Ϣ
	init_com_channel();//��ʼ��ͨѶ����ͨ��
	init_diskman();// ����Ŀ¼�ͱ���
	read_diskman_para_file(IPMAIN_PARA_FILE);//�������ļ�������

	if(get_gtthread_attr(&thread_attr)==0)
		attr=&thread_attr;
	else
		attr=NULL;
	creat_diskman_modsocket_thread();
	if(get_ide_flag()!=0) //�д洢��
	{
	//����Ϊ�����������,��Ӳ��Ӧ�е�ÿ�������������͹���
#if 0//wsy,�Ƶ�init_all_ide_drv��ִ�С�EMBEDED==1 	
		mpdisk_check_disknode();
		mpdisk_creat_partitions(); //��rc.conf��Ӧ���Ѿ����ˣ����Ǳ������
	
		if(access(REBOOT_FILE,F_OK)!=0) //�ո�����
		{
			
			hdutil_e2fsck_and_mount_all_partitions();
			close(creat(REBOOT_FILE,F_OK|W_OK|R_OK));
		}
#endif
		//hdutil_init_all_partitions();
		//disktotal=mpdisk_get_sys_disktotal();
		for(i=0;i<get_sys_disk_num();i++)
		{
	               //��ȡ����������������Ȼ�󱣴浽�����ļ���ȥ	
			disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));  //MB
			parttotal+= get_sys_partition_num(get_sys_disk_name(i));
		}
		set_hd_capacity_to_parafile(disktotal); //setting the capacity to the file /conf/gt1000.ini
		set_hd_capacity(disktotal);             //setting the capacity to the file  /conf/devinfo.ini
		
		if(disktotal>200)//
		{
			if(disktotal<20*1024)//С��20G����Ϊ��CF�� 
				gtloginfo("��⵽���õ�CF��,����:%dMB,������%d\n",disktotal,parttotal);
			else
				gtloginfo("��⵽���õ�Ӳ��,����:%dMB,������%d\n",disktotal,parttotal);
		}	
		else
		{//û��mount��		
			gtlogerr("û�м�⵽���õ�Ӳ��/CF��\n");//add string by shixin
		}
	}
	
	
	creat_diskman_thread(attr,NULL);
	if(attr!=NULL)
	{
		pthread_attr_destroy(attr);
	}
	second_proc();
	exit(0);
}


