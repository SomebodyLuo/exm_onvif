#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <file_def.h>
#include <time.h>
#include <mod_cmd.h>
#include "commonlib.h"
#include "gtlog.h"
#include "send_state.h"
#include "gtthread.h"

#define VERSION "2.03"	//�汾��

// 2.03 lc  ��ֲ��ip1004�豸��
// 2.02	wsy	ͬʱ֧��ģ���ͨ�Ż���Ϣ���е����ֻ���
// 2.01 wsy �ָ�USR_LOGINED,���Ӷ�����Ϣ�����֮��3�еĻ���
// 2.00	wsy	��д��������,�Ծ�����־�ļ�����ʱ������������״̬����,��֧��vsmain�Ĳ�ѯ״̬����
//
// 1.05 wsy �����е��û��ʺ�������ص���֤ʧ�ܶ��ܽ�Ϊpppoe_pap_failed
// 1.04	wsy	����һ���߳�����ģ��ͨѶ��Ϣ����vmmain��ѯʱ�����Լ�״̬
// 1.03 wsy	����PAP authentication failed�ؼ���
// 1.02	wsy	��socketȡ����Ϣ����
// 1.01  ��00236	�������� fgets( log_buf, sizeof(log_buf)��Ϊfread(log_buf,1,sizeof(log_buf),f_log);
// ��Ϊfgets()��ȡ��־gtlog.txtʱÿ��ֻ��ȡһ���ַ�������Ϣ��ʱ���ӳ�ʱfgets()��
// ��ȡ���Ȼ��������־gtlog.txt�ļ�¼�����Ի�����ʱ������fread()����־��
// �����ӵ������ַ�����������log_buf�У�Ȼ���ٷ����ַ���������ݡ�



#define OK					0x00
#define ERR_OPENFILE		0x01	
#define ERR_READFILE		0x02
#define ERR_WRITFILE		0x03
#define ERR_STATFILE		0x04
#define ERR_NOFILEIP		0x05
#define ERR_SIGNAL			0x06
#define ERR_OVERTIME		0x07	
#define ERR_CANNOTINITCOM	0x08
#define ERR_OTHER			0x99

#define	BYTE	unsigned char
#define	WORD	unsigned short
#define	DWORD	unsigned long

#define PPPOEWATCH_LOCKFILE 	"/lock/ipserver/pppoe_watch"

typedef  struct{
	int retcode;
	char showmsg[32];
	char chkmsg[64];
}PPPOE_SEARCH_STR;

const PPPOE_SEARCH_STR PPPOE_SEARCH_STRING[] = {
	{PPPOE_SUCCESS,		"PPPOE_SUCCESS", 	"remote IP address"},
	{PPPOE_NO_MODEM,	"PPPOE_NO_MODEM",	"LCP: timeout sending Config-Requests"},
/*wsymod,�����е���֤ʧ�ܶ�����һ��״̬.PPPOE_PAP_FAILED.
	{PPPOE_PASSWD_ERR,	"PPPOE_PASSWD_ERR",	"ai-Service-Password"},
*/	
	{PPPOE_USR_TWICE,	"PPPOE_USR_TWICE",	"login limit 1 exceeded"},
/*	{PPPOE_USR_INVALID, "PPPOE_USR_INVALID","return Illegal Account"},*/
	{PPPOE_PAP_FAILED,  "PPPOE_PAP_FAILED", "PAP authentication failed"},
//�ڴ˴������״̬!!!		
//	{0xffff,			"",					"*********"}
};


//��һ���ļ�ֱ���ɹ�
static FILE * OpenFileTillSucc( char *filename, char *openmode )
{
	FILE * fp;
	
	while ( 1 )
	{
		fp = fopen( filename, openmode );
		if( fp != NULL )
			return fp;
		else
		{
			printf( "pppoe_watch Open the %s error!\n",filename);
			sleep( 1 );
		}
	}
	
	return NULL;
}


//������־�ļ���С����λbyte
int get_logsize(void)
{
	int getstat = 0;
	struct stat statbuf;
	
	while(1)
		{
			getstat = stat("/log/gtlog.txt",&statbuf);
			if(getstat == 0)
				break;
			else
				sleep(1);
		}
	return statbuf.st_size;
}


//������־������pppoe״̬���߳�
void *  send_pppoe_status_thread( void * data )
{
	BYTE log_buf[1024];
	FILE * f_log; //��־���ļ�ָ��
	int oldlogsize = 0;//ǰһ�ε���־�ļ���С,��λbyte
	int newlogsize=0; //��ǰ����־�ļ���С,��λbyte
	int state;
	int i;
	int ret;
	int state_no; //��Ҫ���յ�״̬��Ŀ���Լ�����
	int dropline = 0;	//�����ڿ�ʼҪdrop������
	
	printf("send_pppoe_status_thread start running!\n");
	gtloginfo("send_pppoe_status_thread start running!\n");

	//Open the log file
	f_log = OpenFileTillSucc( "/log/gtlog.txt", "r" );
	fseek( f_log, 0, SEEK_END );
	oldlogsize = get_logsize();

	while( 1 )
	{
		
		if(fgets(log_buf,1000,f_log)== NULL) //û�ж���
		{
			//����Ƿ�������ļ�
			newlogsize = get_logsize();
			if(newlogsize < oldlogsize) //������
			{
				fclose(f_log);
				f_log = OpenFileTillSucc( "/log/gtlog.txt", "r" );
				oldlogsize = get_logsize();
			}
			else	//ֻ��û���¼�¼����
			{
				oldlogsize = newlogsize;
				sleep(1);
			}
		
		}
		else //������
		{
			if(dropline!=0)
			{
				dropline--;
				continue;
			}
			state_no = sizeof(PPPOE_SEARCH_STRING)/sizeof(PPPOE_SEARCH_STR);
			//�����������һ��
			for(i=0; i< state_no; i++)
			{
				if(strstr(log_buf, PPPOE_SEARCH_STRING[i].chkmsg)!=NULL) //����һ����Ϣ,����
				{
					state = PPPOE_SEARCH_STRING[i].retcode;
					ret = send_pppoe_stat2main(state);
					printf("pppoe_watch->>Sending the pppoe message %s![%s]\n",ret<0?"FAILED":"OK",PPPOE_SEARCH_STRING[i].showmsg);
					gtloginfo("->> Sending the pppoe message %s![%s]\n",ret<0?"FAILED":"OK",PPPOE_SEARCH_STRING[i].showmsg);
					dropline = 3;
				}
			}
			
		}
			
	}
	
	return NULL;	
}


/* 
 *  ͨ��ʵʱ���� /log/gtlog.txt�ļ��ﵽ���adsl���ӽ���״̬�Ĺ���
 *	״̬��Ϊ:
 *		Adsl ������ͨ��
 *		�Ҳ���adsl modem��
 *		Adsl �ʺ��������
 *		�ȵ�..
*/
int main(int argc, char *argv[])
{ 	
	pthread_t send_pppoe_status_thread_id;
	pthread_t recv_modcom_thread_id;
	int ret;

		
	gtopenlog("pppoe_watch");
    if(create_lockfile_save_version(PPPOEWATCH_LOCKFILE,VERSION)<0)
    {
        printf("trying to start pppoe_watch but it's already running, exit!..\n");
        gtloginfo("trying to start pppoe_watch but it's already running,exit!..\n");
        exit(1);
    }
    printf("pppoe_watch [v%s] start running..\n",VERSION);
    gtloginfo("pppoe_watch [v%s] start running..\n",VERSION);

	//Init com channel
	ret = init_com_channel();
	if( ret < 0 )
	{
		printf("pppoe_watch init_com_channel error ret = %d, exit!!!\n",ret);
		gtloginfo("��ʼ�����̼�ͨѶʧ��ret = %d���˳�\n",ret);
		exit(1);
	}	
	
	//�����߳�send_pppoe_status_thread
	gt_create_thread(&send_pppoe_status_thread_id, send_pppoe_status_thread,NULL);

	//�����������������߳�
	creat_recv_modsocket_thread();
	gt_create_thread(&recv_modcom_thread_id, recv_modcom_thread,NULL);
	
	while(1)
		sleep(1);
		
	//should never reach here
	return 0;
}
