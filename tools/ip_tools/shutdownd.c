/*
	���ڽ�ϵͳ�ر�ʱ���¼��־�ĳ����ڿ���ʱ���ã�Ȼ��һֱ����
*/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <commonlib.h>
#define		VERSION		"0.53"

//0.53 ip1004��ʹ��
//0.52 �ϴιػ�ʱ���޷�ȷ������־����¼00-00-00
//0.51 add check file when write
#define		LOG_FILE	"/log/gtlog.txt"
#define		RECORD_FILE	"/log/shutdown.txt"

/**********************************************************************************
 * ���ػ�ʱ��д����־
 **********************************************************************************/

int SaveShutdownTime2Log(time_t STime)
{
	FILE	  *Fp=NULL;
	time_t	  MTime;
	struct tm MinTime;
	struct tm *STm=NULL;
	char	  LogStr[256];//Ҫ������־���ִ�
	MinTime.tm_year=2006-1900;
	MinTime.tm_mon=1-1;
	MinTime.tm_mday=1;
	MinTime.tm_hour=0;
	MinTime.tm_min=0;
	MinTime.tm_sec=0;
	MTime=mktime(&MinTime);
/*	if((int)STime<(int)MTime)
	{
		sprintf(LogStr,"<0000-00-00 00:00:00> �ϴιػ�ʱ���޷�ȷ��(device power off)\n");
	}
	else
*/
	{
		STm=localtime(&STime);
		if(STm!=NULL)
		{
			sprintf(LogStr,"<%04d-%02d-%02d %02d:%02d:%02d> �豸�ر�(device power off)\n",STm->tm_year+1900,STm->tm_mon+1,STm->tm_mday,STm->tm_hour,STm->tm_min,STm->tm_sec);
		}
		else
		{
			sprintf(LogStr,"<0000-00-00 00:00:01> �ϴιػ�ʱ���޷�ȷ��(device power off)\n");
		}
	}
	
	Fp=fopen(LOG_FILE,"a");
	if(Fp==NULL)
		return -errno;
	fprintf(Fp,"%s\n",LogStr);
	fclose(Fp);
	return 0;
}
int main(void)
{
	time_t	ShutdownTime=0;
	FILE *Fp=NULL;
	char *Line=NULL;
	char ReadBuf[512];
	int  Ret;
	printf("start shutdownd %s...\n",VERSION);
	Fp=fopen(RECORD_FILE,"r");
	if(Fp!=NULL)
	{//��ȡ�ϴιػ�ʱ��
		Line=fgets(ReadBuf,sizeof(ReadBuf),Fp);
		if(Line!=NULL)
		{
			ShutdownTime=atoi(ReadBuf);
		}
		fclose(Fp);
		Fp=NULL;
	}
	ShutdownTime+=1;			//�ػ�ʱ���2
	SaveShutdownTime2Log(ShutdownTime);
	
	Fp=fopen(RECORD_FILE,"w");
	if(Fp==NULL)
	{
		printf("can't open %s:%s!!!\n",RECORD_FILE,strerror(errno));
		exit(1);
	}
	while(1)
	{
		//sleep(1);
		ShutdownTime=time(NULL);
		if(ShutdownTime!=(time_t)-1)
		{
			Ret=fseek(Fp,0,SEEK_SET);			
			if(Ret==0)
			{
				if(!check_file(RECORD_FILE))
				{
					printf("file:%s deleted!!!\n",RECORD_FILE);
					if(Fp!=NULL)
						fclose(Fp);
					Fp=fopen(RECORD_FILE,"w");
        				if(Fp==NULL)
        				{
                				printf("can't open %s:%s!!!\n",RECORD_FILE,strerror(errno));
						exit(1);
        				}

				}
				Ret=fprintf(Fp,"%d\n",(int)ShutdownTime);
				fflush(Fp);
			}
			else
			{
				printf("fseek Ret=%d:%s!\n",Ret,strerror(errno));
			}
		}	
		sleep(1);	
	}	
	
	
	exit(0);
	
	
	
}


