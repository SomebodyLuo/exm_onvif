#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "version.h"

//�����������ļ�
//���ش򿪵��ļ�������
int create_and_lockfile_cgi(char *lockfile)
{
	int lf;
	char fbuf[200];
	int ret;
	
	if(lockfile==NULL)
		return -1;
	lf=open(lockfile,O_RDWR|O_CREAT,0640);//�����ļ�
	if(lf<0)
	{
		mkdir("/lock",0770);
		mkdir("/lock/ipserver",0770);
		lf=open(lockfile,O_RDWR|O_CREAT,0640);
		if(lf<0)
		{
			printf("create lock file:%s error!\n",lockfile);
			return -2;
		}
	}
	if(flock(lf,LOCK_EX|LOCK_NB)!=0)//�����̱�־�ļ��������Է�ֹ�������ͬһ����Ķ������
	{
		return -1;
	}	

	if(lf<0)
		return lf;
	else
	{
		sprintf(fbuf,"%d\nversion:%s\n",getpid(),VERSION);
		ret=write(lf,fbuf,strlen(fbuf));
		if(ret < 0)
		{
			printf("write error!\n");
		}
		close(lf);
	}
	
	return lf;
}
//
int get_prog_ver(char *lockfile,char *vbuf,int len)
{

	int fd;
		
	if((lockfile==NULL)||(vbuf==NULL))
	{
		printf("lockfile/vbuf == NULL!\n");
		return -1;
	}
	fd=open(lockfile,O_RDONLY);//�����ļ�
	if(fd <= 0)
	{
		printf("open lockfile: %s error!\n",lockfile);
		return -1;
	}
	else 
	{
		read(fd,vbuf,len);
		//printf("vsmain:%s",vbuf);
		close(fd);
		return 0;
	}
	return 0;
	
}

