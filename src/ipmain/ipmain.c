#include "ipmain.h"
#include "commonlib.h"
#include "ipmain_para.h"
#include "maincmdproc.h"
#include "mainnetproc.h"
#include "watch_board.h"
#include "watch_process.h"
#include "trans_com.h"
#include <gt_com_api.h>
#include <devinfo.h>
#include <signal.h>
#include <dirent.h>
//#include <hdctl.h>
//#include <ftw.h>
#include "netcmdproc.h"
#include "leds_api.h"
#ifdef ARCH_3520D
#include "audioout_api.h"
#endif
#include "infodump.h"
#include "devstat.h"
//#include <gate_cmd.h>
//#include "gtvs_io_api.h"
#include "gate_connect.h"
//#include "osd_api.h"
#include "video_para_api.h"
//#include "gpsupport.h"
#ifdef TEST_FROM_TERM
	#include "testfromterm.c"
#endif
#include <ctype.h>  
#ifdef ARCH_3520A
#include "exdrv_3520Ademo/hi_wtdg/watchdog.h"
#else 
#include "hi3520D/watchdog.h"
#endif

//#include "rand48.h"
/****************��ulibc������********************************/
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if (alignment % sizeof(void *) != 0)
		//|| !powerof2(alignment / sizeof(void *)) != 0
		//	|| alignment == 0
	return -EINVAL;

	*memptr = memalign(alignment, size);
	return (*memptr != NULL ? 0 : ENOMEM);
}

/*
__const unsigned short int *__ctype_b;
__const __int32_t *__ctype_tolower;
__const __int32_t *__ctype_toupper;
//Ϊ����һ���ļ��������²�

#define ctSetup()   {  \
                         __ctype_b = *(__ctype_b_loc());   \
                         __ctype_toupper = *(__ctype_toupper_loc());   \
                         __ctype_tolower = *(__ctype_tolower_loc());  \
}
*/
/*
extern unsigned short _rand48_seed[3];
extern unsigned short _rand48_mult[3];
extern unsigned short _rand48_add;

void
lcong48(unsigned short p[7])
{
	_rand48_seed[0] = p[0];
	_rand48_seed[1] = p[1];
	_rand48_seed[2] = p[2];
	_rand48_mult[0] = p[3];
	_rand48_mult[1] = p[4];
	_rand48_mult[2] = p[5];
	_rand48_add = p[6];
}
*/
/*************************3520D ���Ź����Ƴ��� ********************************/
int g_wtdg_fd = 0;
//hi3515  watchdog��ʼ��������timeoutʱ��
int init_wtdg_dev(int timeout)
{
	int timeOut;
	if(timeout > 0)
		timeOut = timeout/2;
	else
		timeOut = WTDG_TIMEOUT; 

	g_wtdg_fd = open("/dev/watchdog", O_RDWR);
	if (g_wtdg_fd < 0)
	{
#ifdef SHOW_WORK_INFO
		printf("open /dev/watchdog failed!");
#endif
		gtlogerr("open /dev/watchdog failed!");
		return -1;
	}
	if (ioctl(g_wtdg_fd, WDIOC_SETTIMEOUT, &timeOut) < 0)
	{
#ifdef SHOW_WORK_INFO
		printf("ioctl /dev/watchdog failed!");
#endif
		gtlogerr("ioctl /dev/watchdog failed!");
		return -1;
	}

#ifdef SHOW_WORK_INFO
	printf("init_wtdg_dev OK!\n");
#endif
	gtloginfo("��ʼ�����Ź��ɹ�!��ʱʱ��Ϊ%d��\n",timeOut);

	return 0;		
}

int feed_watch_dog()
{
	if (g_wtdg_fd >= 0)
	{
		if (ioctl(g_wtdg_fd, WDIOC_KEEPALIVE,0) < 0)
		{
#ifdef SHOW_WORK_INFO
			printf("feed dog failed!\n");
#endif
			gtlogerr("feed dog failed!\n");
			return -1;
		}
	}

#ifdef SHOW_WORK_INFO
	//printf("feed_watch_dog OK!\n");
#endif

	return 0;
}

/************************************************************************************/

/***************����־�ϼ�¼�����˳�״̬*********************/
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("ipmain process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
			gtloginfo("ipmain ��kill,�����˳�!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGKILL:
			gtloginfo("ipmain SIGKILL,�����˳�!!\n");
			//lc do ����ʼ��mpp���ͨ��
			uninit_video_vadc();
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("ipmain ���û���ֹ(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1:
			//���ϵͳ��Ϣ��ָ���ļ�
			//lc do 
			dump_sysinfo();
		break;
		case SIGSEGV:
			gtloginfo("ipmain �����δ���\n");
			close_all_res();
			printf("ipmain segmentation fault\n");
			exit(0);
		break;
	}
	return;
}


int set_io_delay(int valid_delay,int invalid_delay)
{
	int i;
	int ret = 0;

	ret = set_trigin_delay(valid_delay,invalid_delay);
	
	return ret;
}



/**********************************************************************************************
 * ������	:get_gtthread_attr()
 * ����	:��ȡһ��Ĭ���߳����Խṹ
 * ���	:attr:����ʱ�������Խṹ
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 * ע		:�����Ӧ�õ���pthread_attr_destroy�����ͷ�
 **********************************************************************************************/
int get_gtthread_attr(pthread_attr_t *attr)
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
 * ������	:init_check_cert()
 * ����	:��ʼ��������豸��֤���Ƿ������������б��ݼ��ָ�
 * ����ֵ	:��
 **********************************************************************************************/
static void init_check_cert(void)
{
	int	ret,ret1;
	char pbuf[200];
	struct ipmain_para_struct *ipmain_para=get_mainpara();
	if(ipmain_para->rmt_env_mode==MSG_AUTH_SSL)
	{
		//lc do ��ʼ��֤����֤
#ifdef USE_SSL		
		ret=env_init(CERT_FILE,KEY_FILE);
		printf(" ��ʼ��֤����֤!\n");  
		//ret = 0;
		if(ret!=0)
		{
			printf("env_init file %s and %s ret=%d\n",CERT_FILE,KEY_FILE,ret);
			gtlogfault("������֤�����ret=%d\n",ret);	
			ipmain_para->valid_cert=0;

			sprintf(pbuf,"cmp %s %s\n",CERT_FILE,CERT_BAK_FILE);
			ret=system(pbuf);
			sprintf(pbuf,"cmp %s %s\n",KEY_FILE,KEY_BAK_FILE);
			ret1=system(pbuf);
			if((ret!=0)||(ret1!=0))
			{//�����ļ���Դ�ļ���һ��
				ret=env_init(CERT_BAK_FILE,KEY_BAK_FILE);
				printf("bak��ʼ��֤����֤!\n");  ret = 0;
				if(ret==0)
				{
					gtloginfo("�ָ�֤���ļ�%s��%s\n",CERT_BAK_FILE,CERT_FILE);
					gtloginfo("�ָ�˽Կ�ļ�%s��%s\n",KEY_BAK_FILE,KEY_FILE);
					sprintf(pbuf,"cp %s %s\n",CERT_BAK_FILE,CERT_FILE);
					system(pbuf);	
		
					sprintf(pbuf,"cp %s %s\n",KEY_BAK_FILE,KEY_FILE);
					system(pbuf);	
					
					ipmain_para->valid_cert=1;
				}
				else
				{
					gtloginfo("û�п��õı���֤���ļ�\n");
				}
			}

			if(ipmain_para->valid_cert==0)
			{
				ipmain_para->rmt_env_mode=GT_CMD_NO_AUTH;
				ipmain_para->rmt_enc_mode=GT_CMD_NO_ENCRYPT;
			}
		}
		else
		{
#ifdef SHOW_WORK_INFO
			printf("env_init file %s and %s success!\n",CERT_FILE,KEY_FILE);
#endif
			ipmain_para->valid_cert=1;

			sprintf(pbuf,"cmp %s %s\n",CERT_FILE,CERT_BAK_FILE);
			ret=system(pbuf);
			sprintf(pbuf,"cmp %s %s\n",KEY_FILE,KEY_BAK_FILE);
			ret1=system(pbuf);
			if((ret!=0)||(ret1!=0))
			{
				sprintf(pbuf,"cp %s %s\n",CERT_FILE,CERT_BAK_FILE);
				system(pbuf);
				sprintf(pbuf,"cp %s %s\n",KEY_FILE,KEY_BAK_FILE);
				system(pbuf);				
				gtloginfo("����֤�鱸���ļ�%s %s\n",CERT_BAK_FILE,KEY_BAK_FILE);
				
			}


		}
#endif		
	}

}

#ifdef FOR_PC_MUTI_TEST
int test_alarm_interval=-1;		//���Ա�����ʱ����
int test_alarm_num=-1;			//���Ա����õı�������
int test_alarm_need_inc=0;		//�����������Ƿ���Ҫÿ�α�����Ϣ�仯
#include "pc_multi_test.c"
int process_opt_h(void)
{
	printf("ip1004 vsmainģ��ģ�����\n");
	printf("�÷�:vsmain [OPTION] [argument]\n");
	printf("OPTION ѡ��˵��\n");
	printf("-h:��ʾ������Ϣ\n");
	printf("-i:��ʾ��ͨ���غ�ı������ʱ��(��) Ĭ��Ϊ����\n");
	printf("-n:��ʾ�ﵽ���ʱ���ı�������(����С��100)\n");
	printf("-v:��ʾÿ�α�������Ϣ��Ҫ�仯\n");
	return 0;
}
int process_opt_i(char *argument)
{
	if(argument==NULL)
	{
		printf("process_opt_i argument=NULL!!!\n");
		return -1;
	}
	test_alarm_interval=atoi(argument);
	printf("�Զ��������:%d(��)\n",test_alarm_interval);
	return 0;
}
int process_opt_n(char *argument)
{
	if(argument==NULL)
	{
		printf("process_opt_n argument=NULL!!!\n");
		return -1;
	}
	test_alarm_num=atoi(argument);
	if(test_alarm_num>100)
	{
		printf("��������̫��,ǿ��������ֵ!\n");
	}
	printf("�Զ���������:%d (��/��)\n",test_alarm_num);
	return 0;
}
int process_opt_v(void)
{

	test_alarm_need_inc=1;
	printf("ÿ�������ı�����Ϣ�Զ��仯\n");
	return 0;
}
int process_argument(int argc,char **argv)
{
	int oc;
	if(argc <= 1)
		return 0;
        while((oc=getopt(argc,argv,"hvi:n:"))>=0)
        {
                switch(oc)
                {
			case 'h':
				process_opt_h();
				exit(0);
			break;
			case 'i':
				process_opt_i(optarg);
			break;
			case 'n':
				process_opt_n(optarg);
			break;
			case 'v':
				process_opt_v();
			break;
			default:
			break;
                }
        }
	return 0;
}
#endif
#if 0
//�Ե�ǰϵͳʱ�������������
long int set_lrandomseed(void)
{
	unsigned short int param[7];
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	param[0]	=	p->tm_year;
	param[1]	=	p->tm_mon;
	param[2]	=	p->tm_mday;
	param[3]	=	p->tm_wday;
	param[4]	=	p->tm_hour;
	param[5]	=	p->tm_min;
	param[6]	=	p->tm_sec;

	//lc do
	lcong48(param);
	return 0;
}
#endif

int main(int argc,char **argv)
{
	//int ret,ret1;
	pthread_attr_t  thread_attr,*attr;				//�߳����Խṹ
#ifdef TEST_FROM_TERM
	pthread_t testfromterm_thread_id;
#endif
	struct ipmain_para_struct *ipmain_para;		//����ָ��
	int lock_file;									//���ļ���������
	char pbuf[100];								//��ʱ������
	char devid[32];								//�豸guid��ʱ������
	unsigned short serv_port;						//�������˿�
	close_all_res();								//�ر������Ѿ��򿪵���Դ
#ifdef FOR_PC_MUTI_TEST
	process_argument(argc,argv);
	gen_multi_devices();
#endif

	gtopenlog("ipmain");							//����־
	setsid();										//���ý���Ϊ��ͷ����
	show_time(NULL);								//��ʾϵͳ��ǰʱ�� ��.����
#ifdef DEAMON_MODE
	if(deamon_init()!=0)
	{
		printf("deamon_init error!\n");
	}
#endif
	/***************�ж�ģ���Ƿ��Ѿ�����***************************/
	lock_file=create_and_lockfile(IPMAIN_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("ipmain module are running!!\n");
		gtlogerr("ipmainģ�������У���������Ч�˳�\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);				//�����̵�id�Ŵ������ļ���

#ifdef FOR_PC_MUTI_TEST
	SetOutputTty(lock_file);						// �����Ӱ�䵽��һ��������
#endif

	//ctSetup();
	gtloginfo("����ipmain (ver:%s).......\n",VERSION);

	/***************ע���źŴ�����**********************************/
	signal(SIGKILL,exit_log);		//kill -9�ź�
	signal(SIGTERM,exit_log);		//��ͨ��kill�ź�
	signal(SIGINT,exit_log);		//ctrl-c�ź�
	signal(SIGSEGV,exit_log);		//�δ����ź�
	signal(SIGPIPE,exit_log);		//���ѹرյ�socket��д�����ź�


	init_devinfo();					//�������ļ���ʼ���豸��Ϣ(guid��)
	init_devstat();				//��ʼ���豸״̬
	
	get_devid(devid);
	printf("devid=%02x%02x%02x%02x%02x%02x%02x%02x\n",devid[7],devid[6],devid[5],devid[4],devid[3],devid[2],devid[1],devid[0]);
	gtloginfo("devid=%02x%02x%02x%02x%02x%02x%02x%02x\n",devid[7],devid[6],devid[5],devid[4],devid[3],devid[2],devid[1],devid[0]);
	
	init_para();					//������������Ϊ��ʼֵ
	readmain_para_file(IPMAIN_PARA_FILE,get_mainpara());	//�������ļ���ȡ������
	read_config_ini_file(CONFIG_FILE, get_mainpara());			//�������ļ��ж�ȡ�����豸����ֵ
	printf("���豸������ʽΪ: %s\n",get_internet_mode_str());
	gtloginfo("���豸������ʽΪ: %s\n",get_internet_mode_str()); 
	ipmain_para=get_mainpara();
	ipmain_para->total_com=get_total_com();
	if(ipmain_para->total_com>get_total_com())
		ipmain_para->total_com=1;
	printf("[ipmain(ver:%s)] module run pid=%d waiting for remote gateway at port:%d\n",VERSION,getpid(),serv_port);

   // set_lrandomseed();//����������� 
	init_check_cert();					//��ʼ�������֤��
#ifdef USE_VDA
	init_video_vadc(&ipmain_para->vadc);
#endif
#ifdef ARCH_3520D
	if(open_rtc_dev() < 0)
	{
		printf("init_rtc_dev failed!\n");
	}
	if(init_audioout() < 0)
	{
		printf("init_audioout failed!\n");
	}
#endif

	//lc do ��ʼ��gpio,����valid/invalidʱ�䣬Ĭ��Ϊ0/3��
#ifdef USE_IO
	init_gpiodrv();	//��ʼ��gpio�ܽ�������
	//lc do 
	set_io_delay(GTIP_IO_VALID_DELAY,GTIP_IO_INVALID_DELAY); 
#endif	
	//lc do ��ʼ��led
#ifdef USE_LED
	init_leds();								//��ʼ��ָʾ��״̬
#endif	
	
	init_netcmd_var();						//��ʼ��������ͨѶ��״̬
	//lc do ���ڷ����ʼ��
	init_trans_com_var();					//��ʼ��͸��������Ҫ�õ������ݽṹ
		
	init_com_channel();						//��ʼ������������������ͨѶ������ͨ��
#ifdef DISPLAY_THREAD_INFO
	printf("prepare to start threads...\n");			
#endif
	if(get_gtthread_attr(&thread_attr)==0)
		attr=&thread_attr;
	else
		attr=NULL;
		
	creat_connect_thread(attr);				//�������������߳�

	//lc do ����Ƶ��ʧ���ڵ����ƶ����м��,�˲��ֹ�������videoenc��
	creat_vda_proc_thread(attr,NULL);

#if EMBEDED==1
	//lc do ��ض���״̬�仯��������Ӧ����
	init_ipalarm_fd();
	init_ipalarm_fdset();
	creat_watch_board_thread(attr);		//��������������ӵ��߳�
#endif
	creat_cmdproc_thread(attr);				//��������������������߳�
	
	init_mainnetcmd_threads(attr,ipmain_para->devpara[0].cmd_port,0);

	creat_trans_com_thread(attr,get_trans_com_info(0));		//����͸�����ڷ����߳�
	if(get_total_com()>1)
		creat_trans_com_thread(attr,get_trans_com_info(1));	//����͸�����ڷ����߳�		
#ifdef TEST_FROM_TERM
	pthread_create(&testfromterm_thread_id,attr,test_from_term_thread,NULL);//����͸�����ڴ����߳�
#endif

	
	if(attr!=NULL)
	{
		pthread_attr_destroy(attr);
	}

	sleep(1);
	show_time(NULL);							//��ʾϵͳ��ǰʱ�� ��.����
	
	signal(SIGUSR1,exit_log);
	watch_process_thread();					//ת��Ϊ����߳�
	//Ӧ����Զ����ִ�е�����
	closelog();
	exit(-1);
}

