#include "watch_process.h"
#include <typedefine.h>
#include <file_def.h>
#include <mod_com.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <iniparser.h>
#include <commonlib.h>
#include <errno.h>
#include <sys/wait.h>

#include <devinfo.h>
#define USE_V1  					//wsy����Ϊ֧��1��ϵͳ�ķ�ʽ	
#define	TMP_FILE_GREP			"/var/tmp/watch_grep.tmp"		//����grep�����õ���ʱ�ļ���
#define   SAVE_DEBUG_INFO_DIR	"/log/debug"					//�洢�����߳���Ŀ����ʱ�ĵ���Ӧ�ó����߳�״����Ŀ¼


typedef struct{			//��������ض���Ľṹ
	int	 min_threads;	//��С�߳���//0��ʾ���ù�
	int	 ps_cnt;			//ͨ�����������жϳ����Ƿ��������еļ�����
	char  *prog_name;			//������(��·��)
	char  *lock_file;		//�����ļ���(��·��)
}watch_target_t;
static int	first_run_flag=1;	//�������б�־����������ʱ����Ӧ�ó����¼��־�ķ�ʽ��ͬ(����¼Ӧ�ó����쳣�˳���־)

//�����������Ҫ���м�ص�Ӧ�ó���ľ�̬�ṹ
watch_target_t 	watch_targets[]=
{//���������߳������ǻ����߳�,
	#ifdef USE_V1
		//lc to do ÿ�����̵��߳����������¶��壬�����ȶ����Ժ�ȷ��
		{17,0,"/ip1004/encbox","/lock/ipserver/encbox"},
		{3,0,"/ip1004/diskman",DISKMAN_LOCK_FILE},
		{30,0,"/ip1004/rtimage",RT_LOCK_FILE},
		{4,0,"/ip1004/hdmodule",HDMOD_LOCK_FILE},
		{14,0,"/ip1004/ipmain",IPMAIN_LOCK_FILE},
		{7,0,"/ip1004/playback",HDPLAYBACK_LOCK_FILE}
	#endif
};
int	total_watch=sizeof(watch_targets)/sizeof(watch_target_t);	//�ܹ���Ҫ��ص�Ӧ�ó�������

//lc 2014-2-25 ���ڲ�ͬ����ѡ�����ִ�в�ͬ��target���ϣ�ͨ�����úͰ汾����
watch_target_t 	watch_targets2[]=
{//���������߳������ǻ����߳�,
	#ifdef USE_V1
		//lc to do ÿ�����̵��߳����������¶��壬�����ȶ����Ժ�ȷ��
		{16,0,"/ip1004/encbox","/lock/ipserver/encbox"},
		{3,0,"/ip1004/diskman",DISKMAN_LOCK_FILE},
		{29,0,"/ip1004/rtimage",RT_LOCK_FILE},
		{10,0,"/ip1004/hdmodule",HDMOD_LOCK_FILE},
		{14,0,"/ip1004/ipmain",IPMAIN_LOCK_FILE},
		{7,0,"/ip1004/playback",HDPLAYBACK_LOCK_FILE}
	#endif
};
int	total_watch2=sizeof(watch_targets2)/sizeof(watch_target_t);	//�ܹ���Ҫ��ص�Ӧ�ó�������


static int g_playback_enable = 1;
static int g_multichannel_enable = 0;

//����Ӧ�ó������ҵ���Ӧ��Ӧ����Ϣ�ṹָ��
//NULL��ʾû���ҵ�
static watch_target_t *get_target_by_name(char *name)
{
	int i;
	watch_target_t *cur=NULL;
	int watchnum = 0;
	const watch_target_t *target = NULL;
	if(name==NULL)
		return NULL;
	
	if(g_multichannel_enable)
	{
		target = watch_targets2;
		watchnum = total_watch2;
	}
	else
	{	
		target = watch_targets;
		watchnum = total_watch;
	}
	
	for(i=0;i<watchnum;i++)
	{
		cur=&target[i];
		if(strcmp(cur->prog_name,name)==0)
		{
			return cur;	//�ҵ���ƥ�����Ϣ
		}
	}
	return NULL;			//û�ҵ�
}
//���ݲ�ͬ�ͺŵ���Ϣ���޸�Ӧ�ó���Ӧ�е��߳���
static void fix_watch_structs(void)
{
	watch_target_t *w=NULL;
	//ipmain
	w=get_target_by_name("ipmain");
	if(w!=NULL)
	{
		if(get_quad_flag())
		{
			if(w->min_threads!=0)					//=0��ʾ����Ҫ���
				w->min_threads+=1;
		}
	}

	//encbox
	w=get_target_by_name("encbox");
	if(w!=NULL)
	{
		if(w->min_threads!=0)						//=0��ʾ����Ҫ���
		{
			w->min_threads+=get_videoenc_num();	//һ����Ƶ��������Ҫһ���߳�
		}
	}
}


/**************************************************************************
  *������	:find_str_in_file
  *����	:���ļ�����ָ�������ַ������ֵĴ���
  *����	: FileName:���������ļ�����StrҪ���ҵ��ַ���
  *����ֵ	:��ֵ��ʾָ���ַ������ֵĴ�������ֵ��ʾ����
  *************************************************************************/
int find_str_in_file(char *FileName,char *Str)
{
	int Cnt=0;
	FILE *Fp=NULL;
	char  ReadBuf[256];
	char  *PR;
	char *p;
	if(FileName==NULL)
		return -EINVAL;
	Fp=fopen(FileName,"r");
	if(Fp==NULL)
		return -errno;
	while(1)
	{
		PR=fgets(ReadBuf,sizeof(ReadBuf),Fp);
		if(PR==NULL)
		{
			//printf("errno=%d:%s\n",errno,strerror(errno));
			break;
		}
		p=strstr(PR,Str);
		if(p!=NULL)
			Cnt++;
	}
	fclose(Fp);
	//printf("%d lines!!\n",Lines);
	return Cnt;		
}

/**************************************************************************
  *������	:grep_process2file
  *����	:��ָ������������Ϣ�����ָ���ļ���
  *����	: prog_name:��������FileName:����ļ���
  *����ֵ	:0��ʾ�ɹ�����ֵ��ʾ����
  *************************************************************************/
int grep_process2file(char *prog_name,char *FileNmae)
{
	int ret;
	char PBuf[256];
	if((prog_name==NULL)||(FileNmae==NULL))
	{
		return -EINVAL;
	}
	sprintf(PBuf,"ps | grep %s>%s",prog_name,FileNmae);
	ret=system(PBuf);
	printf("test grep_process2file PBuf=%s ret=%x errno=%d:%s!!!\n",PBuf,ret,errno,strerror(errno));
/*
	if(ret!=0)
		return -errno;
		*/
	return 0;	
}

/**************************************************************************
  *������	:log_ps_info2file
  *����	:��ָ���Ľ��̵���Ϣ��¼����������־�ļ���
  *			 /log/debug/prog_name ,����׷�ӵķ�ʽ������ļ�����
  *			����һ��ֵ(8192 byte)�Ժ�����0
  *����	: prog_name:������,����������Ӧ����־�ļ���
  *			  min_threads:�������õ���С�߳���
  *		         CurThreads:���̵�ǰ�����߳���
  *����ֵ	:��
  *************************************************************************/
void log_ps_info2file(char *prog_name,int min_threads,int CurThreads)
{
	char LogFile[256];
	char *SaveFlag=">>";
	char SaveBuf[256];
	struct stat FStat;
	//int ret;
	sprintf(LogFile,"%s/%s",SAVE_DEBUG_INFO_DIR,prog_name);
	if(stat(LogFile,&FStat)>=0)
	{
		if(FStat.st_size>8192)
			SaveFlag=">";
	}
	sprintf(SaveBuf,"date %s %s",SaveFlag,LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"echo %s min_threads=%d CurThreads=%d >>%s",prog_name,min_threads,CurThreads,LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"ps |grep %s >>%s",prog_name,LogFile);
	system(SaveBuf);
	
	sprintf(SaveBuf,"echo ====================================================>>%s",LogFile);
	system(SaveBuf);
#if 0
	sprintf(SaveBuf,"echo \"old file info:\n\">>%s",LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"cat %s>>%s",TMP_FILE_GREP,LogFile);
	system(SaveBuf);	
#endif




	
}

/**************************************************************************
  *������	:run_target
  *����	:�ж��Ƿ�Ӧ���������Ŀ�겢����
  *����	: target:��������ض�������ݽṹָ��
  *����ֵ	:0��ʾ�ɹ�����ֵ��ʾ����
  *************************************************************************/
int run_target(watch_target_t *target)
{
	int l_fd=-1;
	int ret;
	pid_t pid;
	char *prog_name=NULL;
	char k_buf[100];
	int thread_num;
	l_fd=create_and_lockfile(target->lock_file);		//��ͼ��ס�ļ�
	do
	{
		if(l_fd<0)
		{	//Ŀ������Ѿ�����
			if(target->min_threads<=0)			//����Ҫ�жϽ�������
				return 0;						//������������
			else
			{//�ж�Ŀ����̵��߳����ǲ���������Сֵ�Լ��Ƿ��н�ʬ����
				if(++target->ps_cnt>2)
				{//	20��(������ʱ���������10��)�ж�һ���߳���
					target->ps_cnt=0;
					//��ȡ������
					prog_name=rindex(target->prog_name,'/');
					if(prog_name!=NULL)
						prog_name++;
					else
						prog_name=target->prog_name;	

					
					ret=grep_process2file(prog_name,TMP_FILE_GREP);
					//�����ȡ�߳���ʧ����ֱ�ӷ���
					if(ret!=0)	
						return 0;
					//lc do �µķ�ʽ��ȡ�߳���
					thread_num=get_ps_threadnum(prog_name,TMP_FILE_GREP);
										
					if(thread_num>=target->min_threads)
					{
						ret=find_str_in_file(TMP_FILE_GREP," Z ");//�ж��Ƿ��н�ʬ����
						if(ret>0)
						{	//�����н�ʬ����
							gtloginfo("find %s have %d Z threads!!!\n",prog_name,ret);
							log_ps_info2file(prog_name,target->min_threads,thread_num);
							break;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						sleep(1);
						thread_num=get_ps_threadnum(prog_name,TMP_FILE_GREP);
						if((thread_num>0)&&(thread_num<target->min_threads))
						{
							//���̵�ʵ�������߳���С��Ԥ�����Сֵ
							if(!first_run_flag)
							{
								gtloginfo("%s min_threads=%d Current=%d!!\n",prog_name,target->min_threads,thread_num);
								log_ps_info2file(prog_name,target->min_threads,thread_num);
							}
							break;
						}
						else
						{
							//��ȡ�ļ���������������������
							return 0;
						}
						
					}
					
				}
				else
					return 0;
			}
		}
		else
		{
			// ���̻�û���������쳣�˳�
			if(!first_run_flag)
				gtloginfo("%s maybe exited ,start it!\n",target->prog_name);
			flock(l_fd,LOCK_UN);
			fsync(l_fd);
			close(l_fd);	//�ر��Ѿ��򿪵��ļ�
			break;
		}
	}while(0);

	pid=fork();
	if(pid==0)
	{
		gtlogerr("watch_process fork child pid is %d,last errno is %d\n",pid,errno);
		setsid();
		//�������̵���������	
		prog_name=rindex(target->prog_name,'/');
		if(prog_name!=NULL)
			prog_name++;
		else
			prog_name=target->prog_name;	
		
		chdir("/");
		umask(0);

		//���������Ѿ������ĸ���
		printf("watch_process2 kill %s!!\n",prog_name);
		sprintf(k_buf,"killall -15 %s",prog_name);
		ret=system(k_buf);		

		//�ر��Ѿ��򿪵�������Դ
		close_all_res();
		sleep(1);
		printf("watch_process2 start %s !!\n",target->prog_name);
		gtloginfo("watch_process2 start %s !!\n",target->prog_name);
		sprintf(k_buf,"%s &",target->prog_name);
		
		//�ں�̨����Ŀ�����
		ret=system(k_buf);
		if(ret==0)
			exit(0);
		else
			exit(1);	

	}
	else
	{
		printf("%s fork=%d !!\n",target->prog_name,(int)pid);
	}
	
	return 0;
}
#if 0
/**************************************************************************
  *������	:CheckFileSize
  *����	:���ָ�����ļ��Ƿ񳬹��涨�Ĵ�С
  *			 ��������������Ϊ.0 .0��Ϊ.1...
  *			 ����¼MaxNum���ļ�
  *����	: target:��������ض�������ݽṹָ��
  *����ֵ	:0��ʾ�ɹ�����ֵ��ʾ����
  *************************************************************************/
void CheckFileSize(char *FileName,int Size,int MaxNum)
{
	int i;
	int ret;	

	struct stat Stat;
	if((FileName==NULL)||(Size<0)||(MaxNum<0))
	{
		return;
	}
	ret=stat(FileName,&Stat);
	if(ret<0)
		return ;
	if(Stat.st_size<Size)
		return;
	char TmpFile[100];
	char OldFile[100];
	char NewFile[100];
	char SBuf[256];
	for(i=(MaxNum-1);i>0;i--)
	{
		 sprintf(OldFile, "%s.%d", FileName, i-1);
               sprintf(NewFile, "%s.%d", FileName, i);
               rename(OldFile, NewFile);
	}
	sprintf(NewFile,"%s.0",FileName);
	sprintf(TmpFile,"%s.tmp",FileName);

	rename(FileName,TmpFile);
	
	sprintf(SBuf,"cp %s %s -f",TmpFile,NewFile);
	system(SBuf);


	remove(TmpFile);
	
	return;
	
}
#endif

void OnSIGSEGV(int n,struct siginfo *siginfo,void *myact)  
{  
        int i, num;  
        char **calls;  
        gtlogerr("Fault address:%X\n",siginfo->si_addr);     
        //num = backtrace(buffer, SIZE);  
        //calls = backtrace_symbols(buffer, num);  
        //for (i = 0; i < num; i++)  
        //        printf("%s\n", calls[i]);  

		gtlogerr("pid is %d\n sigment fault!\n");
		
        exit(1);  
}    

/**************************************************************************
  *������	:main
  *����	:watch_proc��������
  *����	: ��
  *����ֵ	:Ӧ���ò�����
  *************************************************************************/
int main(void)
{
	int timeout;
	dictionary      *ini;	
	int lock_file;
	char pbuf[100];
	int pid;
	int Status;
	int i;
	int watchnum = 0;
	const watch_target_t *target = NULL;
#ifdef USE_V1
	#warning "this program will compile for v1 system!"
#else
	#warning "this program will compile for v2 system!"
#endif
	close_all_res();		//�ر��Ѿ��򿪵�������Դ
	gtopenlog("watch_proc");

	//�ж�ģ���Ƿ��Ѿ�ִ��
	lock_file=create_and_lockfile(WATCH_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("watch_process module are running!!\n");
		gtlogerr("watch_process ģ�������У���������Ч�˳�\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���
	
	printf("watch_proc(%s) running...\n",VERSION);
	gtloginfo("watch_proc(%s) running...\n",VERSION);

	 struct sigaction act;
     int sig = SIGSEGV;
     sigemptyset(&act.sa_mask);
     act.sa_sigaction = OnSIGSEGV;
     act.sa_flags = SA_SIGINFO;
     if(sigaction(sig, &act, NULL)<0)
     {
        printf("sigaction error!\n");
		exit(1);
     }
	

	init_devinfo();
	fix_watch_structs();
	//�������ļ��ж�ȡ���ʱ����������Ĭ��ֵ10��
	ini=iniparser_load(WATCH_PARA_FILE);
	if (ini==NULL) 
	{
            	printf("watch_proc  cannot parse ini file file [%s]", WATCH_PARA_FILE);
		gtlogerr("watch_proc �޷�����ini�ļ�[%s]",WATCH_PARA_FILE);
            	timeout=10;
        }
	else
	{
		timeout=iniparser_getint(ini,"product:watch_interval",10);
		if(timeout>1800)//���ܳ������Сʱ
			timeout=10;
		if(timeout<1)
			timeout=1;

		//lc 2014-2-25
		g_playback_enable = iniparser_getint(ini,"netencoder:playback_enable",1);  //default 1
		g_multichannel_enable = iniparser_getint(ini,"multichannel:enable",0);     //default 0
		
		iniparser_freedict(ini);
	}
	//�������ڼ�¼������־��Ŀ¼
	if(access(SAVE_DEBUG_INFO_DIR,F_OK)!=0)
	{
		if(mkdir(SAVE_DEBUG_INFO_DIR,0755)<0)
		{//���ܴ���Ŀ¼
			printf("can't create dir:%s!!\n",SAVE_DEBUG_INFO_DIR);
			gtlogerr("can't create dir:%s!!\n",SAVE_DEBUG_INFO_DIR);
		}
	}

	if(g_multichannel_enable)
	{
		target = watch_targets2;
		watchnum = total_watch2;
	}
	else
	{	
		target = watch_targets;
		watchnum = total_watch;
	}

	while(1)
	{
		//�����еĶ������̽�Ⲣִ��
		for(i=0;i<watchnum;i++)
		{
			run_target(&target[i]);
		}
		
		for(i=0;i<10;i++)
		{
			//�ռ��Ѿ��˳����ӽ�����Դ
			pid=waitpid(-1,&Status,WNOHANG);
			if((int)pid>0)
                	{
                        continue;
                	}
			else
				break;
			
		}

		sleep(timeout);	//
		first_run_flag=0;	//�����Ѿ�����������״����б�־
	}

	return 0;
}


