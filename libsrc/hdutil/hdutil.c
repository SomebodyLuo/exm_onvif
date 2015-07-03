/*
	����mpdisk��fileindex�⣬��hdmodule��diskman����
								--wsy Dec 2007
*/


#include "fileindex.h"
#include "mpdisk.h"
#include "hdutil.h"
#include <errno.h>
#include "file_def.h"
#include <sys/time.h>


//���ڲ�ѯ���ϵĿ�ɾ��������ʱ��
struct oldest_filetime_struct
{
	char oldest_partition[100];	//���ϵķ�������"/hqdata/sda2"
	unsigned int  oldest_file_time;//���ϵ��ļ�����ʱ�䣬time_t
};


//��ɵĹ���:��filename�ŵ���Ӧ��Ŀ¼
//mountpath: ��������Ŀ¼����"/hqdata/sda2"
//filename,����./#93889/HQ_C00_D20051024125934_L30_T00.AVI
//�ƶ��˾��ļ�����0�����򷵻ظ�ֵ
int relocate_avi_file(char * mountpath ,char *filename)
{
	char *p,*t;
	char temp[100];
	char filepath[200];
	char fix[100];
	int ret=0;
	char command[300];
	if((filename==NULL)||(mountpath == NULL))
		return -1;
	p=rindex(filename,'/');
	if (p==NULL)
		return -1;
	p++;
	t=strstr(p,IMG_FILE_EXT); 
	if(t==NULL)
		return -1;
	t=t+4;
	*t='\0';
	strcpy(temp,p);
	//printf("temp is %s\n",temp);
	if (hdutil_filename2dirs(mountpath,temp,filepath)!=0)
		return -1;
	//printf("path is %s\n",path);
	ret=hdutil_create_dirs(filepath);
	if(ret<0)
	{
		gtlogerr("�޷�����Ŀ¼%s,����%d: %s\n",filepath,-ret,strerror(-ret));	
		fix_disk(filepath,-ret);
		return -1;
	}
	
	sprintf(command,"mv %s %s",filename,filepath);
	system(command);	
	
	return 0;
	
}

#define	HDERR_RBT_FILE	"/log/hderr_reboot.txt"	//��¼���һ����ΪӲ�̹��������豸��ʱ��
#define HDERR_RBT_TIME	3600
 
/*************************************************************************
* 	������:hdutil_get_hderr_reboot_flag()
*	����:	�ж��Ƿ�HDERR_RBT_TIME������ΪӲ������������
*	����:	��
* 	����ֵ:	1��ʾӦ��������0��ʾ��Ҫ����
*************************************************************************/
int hdutil_get_hderr_reboot_flag(void)
{	
	FILE *fp = NULL;
	char buf[40];
	time_t time_lastrbt;
	time_t timenow;
	
	
	time(&timenow);
	if(access(HDERR_RBT_FILE,F_OK)==0)	//����
	{
		
		fp = fopen(HDERR_RBT_FILE,"r+");
		if(fp!= NULL)
		{
			fgets(buf,40,fp);
			fclose(fp);
			time_lastrbt = atoi(buf);
			if((time_lastrbt <= 0) || ((timenow - time_lastrbt) < HDERR_RBT_TIME)) //���һ��rbtʱ��̫��
				return 0;
		}	
	}
	
	//����д����ļ�������1
	fp = fopen(HDERR_RBT_FILE,"w+");
	if(fp!= NULL)
	{
		fprintf(fp,"%d",timenow);
		fclose(fp);
	}
	return 1;
}

//��������µ�lost,foundĿ¼,����ֵ1��ʾ�����˾��ļ���0��ʾû��
int init_lost_found( char *path)
{	
	char command[200];
	char filename[150];
	FILE *fp;
	int result = 0; //�����˾��ļ�����1
	
	if(path == NULL)
		return -EINVAL;
		
	mkdir("/var/tmp",0755);
  //here the variable path represents the mountpath.
	sprintf(command,"find %s/lost+found -name '*%s' > %s",path,IMG_FILE_EXT,RESULT_TXT);
	system(command);
	fp=fopen(RESULT_TXT,"r+");
	//���ж�����filename������
 	while (fgets(filename,150,fp)!=NULL)
	{
		printf("relocating %s",filename);
		if(relocate_avi_file(path, filename) == 0) //�ƶ��˾��ļ�
			result = 1;
	}
	fclose(fp);
	remove(RESULT_TXT);

	//ɾ��lost+found�ļ�Ŀ¼�µ�һ��
	sprintf(command,"rm -rf %s/lost+found/*",path);
	system(command);
	
	return result;

}




//��������������hdutil_init_all_partitions����
//����:	����ϵͳ��ǰ���д��ڵķ���������lostfoundĿ¼����ʼ�������ļ�
int init_partitions_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	char cmd[200];
	int partition_total;
	int ret;
	
	if((devname ==NULL)||(mountpath == NULL))
		return -EINVAL;

	ret = init_lost_found(mountpath);
	if(ret == 1)//lost_foundĿ¼�������˾��ļ�		
		fileindex_create_index(mountpath,1); //ǿ�����´��������ļ�
	else
		fileindex_create_index(mountpath,0); //��û�������ļ��������´���				
	return 0;
}



int recreate_db_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	char cmd[200];
	int partition_total;
	
	if(mountpath == NULL)
		return -EINVAL;
	
	fileindex_create_index(mountpath,1); //ǿ�����´��������ļ�
	
	return 0;
}

 /*************************************************************************
 * 	������:	hdutil_create_dirs()
 *	����:	�����ļ������ж��Ƿ���ڸ�Ŀ¼�����û���򴴽�
 *	����:	file,�ļ���
 *	���:	
 * 	����ֵ:	0��ʾ�ɹ�����ֵ��ʾʧ��
 *************************************************************************/
int hdutil_create_dirs(char *file)
{
	char dir[256];
	char t,*p;
	if(file==NULL)
		return -1;
	p=strrchr(file,'/');
	if(p==NULL)	//��ǰĿ¼
		return 0;
	t=*p;
	*p='\0';
	if(access(file,F_OK)==0)
	{
		*p=t;	//�����һ��Ŀ¼
		return 0;
	}

	*p=t;
	strncpy(dir,file,256);

	p=dir;
	t=*p;
	while(p!=NULL)
	{
		*p=t;
		p++;	//��һ���ַ���������
		p=strchr(p,'/');//��һ�� /
		if(p==NULL)
			break;
		t=*p;
		*p='\0';
		
		if(access(dir,F_OK)==0)
		{
			continue;
		}
		if(mkdir(dir,0755)<0)
		{//���ܴ���Ŀ¼
			return -errno;
		}
		
	}
	return 0;
	
}


 /*************************************************************************
 * 	������:	hdutil_lock_filename()
 *	����:	���ļ�����������־
 *	����:	filename,�ļ���
 *	���:	tname,��������ļ���
 * 	����ֵ:	0��ʾ�ɹ�����ֵ��ʾʧ��
 *************************************************************************/
int hdutil_lock_filename(char *filename,char* tname)
{
	char *p;
	if((filename==NULL)||(tname==NULL))
		return -1;
		
	strncpy(tname,filename,strlen(filename)+1);

	p=index(tname,LOCK_FILE_FLAG);
	if(p!=NULL)
	{
		//�ļ��Ѿ�����		
		return 0;
	}
	p=strstr(tname,IMG_FILE_EXT);
	if(p==NULL)
	{
		//����¼����ļ����ܱ�����
		return 0;
	}
	
	p=index(tname,REMOTE_TRIG_FLAG);
	if(p!=NULL)
	{
		sprintf(p,"%c%c%s",REMOTE_TRIG_FLAG,LOCK_FILE_FLAG,IMG_FILE_EXT);
		return 0;
		//�Ѽӱ�־	
	}	
	sprintf(strstr(tname,IMG_FILE_EXT),"%c%s",LOCK_FILE_FLAG,IMG_FILE_EXT);
	
	return 0;	
	
}

 /*************************************************************************
 * 	������:	hdutil_unlock_filename()
 *	����:	����������־���ļ�ȥ��������־
 *	����:	filename,�ļ���
 *	���:	tname,��������ļ���
 * 	����ֵ:	0��ʾ�ɹ�����ֵ��ʾʧ��
 *************************************************************************/
int hdutil_unlock_filename(char *filename,char *tname)
{
	char *p;
	if((filename==NULL)||(tname==NULL))
		return -1;
	strncpy(tname,filename,strlen(filename)+1);
	p=index(tname,LOCK_FILE_FLAG);
	if(p==NULL)
	{//�ļ��Ѿ�����		
		return 0;
	}
	if(strstr(tname,RECORDING_FILE_EXT)!=NULL)//ing�ļ�  
		return 0;
	sprintf(p,"%s",IMG_FILE_EXT);
	return 0;
}


/*************************************************************************
 *	������	hdutil_init_all_partitions()
 *	����:	����ϵͳ��ǰ���д��ڵķ�����
 			����lostfoundĿ¼��
 			��ʼ�������ļ� 
 *	����:	
 *	���:	
 * 	����ֵ:	0
 *************************************************************************/
int hdutil_init_all_partitions(void)
{
	mpdisk_process_all_partitions(init_partitions_fn,NULL);
	return 0;
}


/*************************************************************************
 *	������	hdutil_recreat_db_for_all()
 *	����:	����ϵͳ��ǰ���д��ڵķ�����
 			ǿ�����´����ļ�����
 *	����:	
 *	���:	
 * 	����ֵ:	0
 *************************************************************************/
int hdutil_recreat_db_for_all(void)
{
	mpdisk_process_all_partitions(recreate_db_fn,NULL);
	return 0;
} 


//��hdutil+get_oldest_file_partition���ã�
//�������ķ��������ϵ��ļ���֮ǰ�ڱ�ķ����ҵ��Ļ��ϣ��ͽ���������Ϊ"Ŀǰ���ϵķ���"
int get_oldest_partition_fn(IN char * devname, IN char * mountpath, IO void * arg)
{
	int oldest_time_in_path ;

	struct oldest_filetime_struct * oldest;
	
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	oldest = (struct oldest_filetime_struct *)arg;
	oldest_time_in_path = fileindex_get_oldest_file_time(mountpath);
	
	if((oldest_time_in_path > 0)&&(oldest_time_in_path < oldest->oldest_file_time))	
	{	
		oldest->oldest_file_time = oldest_time_in_path;
		strncpy(oldest->oldest_partition,mountpath,strlen(mountpath)+1);
		printf("%s:  oldest_partition %s\n",mountpath,oldest->oldest_partition);
		
	}
	else
	{
		//printf("oldest_time_in_path %s is %d,compare to %d\n",mountpath,oldest_time_in_path,*(oldest->oldest_file_time));
	}	
	return 0;

}
/*************************************************************************
 * 	������:	hdutil_get_oldest_file_partition()
 *	����:	�ҵ���ǰ����Ŀ�ɾ���ļ����ڵķ���
 *	����:	
 *	���:	oldest_partition:����Ŀ�ɾ���ļ����ڵķ���
 * 	����ֵ:	0��ʾ�ɹ�,��ֵ��ʾû���ҵ�
 *************************************************************************/
int hdutil_get_oldest_file_partition(OUT char* oldest_partition)
{
	int i,j;
	struct oldest_filetime_struct oldest;

	if(oldest_partition == NULL)
		return -EINVAL;
	
	oldest.oldest_file_time = 0xffffffff;

  //��ÿ������������һ��get_oldest_partition_fn().
	mpdisk_process_all_partitions(get_oldest_partition_fn,&oldest);
	//printf("oldest_file_time is %d\n",oldest.oldest_file_time);		

  //ִ�е�����Ѿ������ϵ��Ǹ��ļ���������ļ����ڵķ������µ���oldest�ṹ���ˡ�		
	if((oldest.oldest_file_time > 0)&&(oldest.oldest_file_time != 0xffffffff))
	{
    //���������ļ����ڵķ���·��
		strncpy(oldest_partition, oldest.oldest_partition, strlen(oldest.oldest_partition)+1);
		return 0;
	}
	else
		return -ENOENT;

}




//������������ǰĿ¼�����е�¼���ļ�
int lock_all_fn(IN char * devname, IN  char * mountpath, IO void *arg)
{
	int result = 0;
	char newname[200];
	int *mode;
	
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	
	mode = (int *)arg;
	
	
	result = fileindex_lock_by_time(mountpath,*mode, -1,-1,-1,-1);

	return result;
}



/*************************************************************************
 * 	������:	hdutil_lock_all_files()
 *	����:	�ӽ������е�¼���ļ�
 *	����:	mode, 1Ϊ������0Ϊ����
 *	���:	
 * 	����ֵ:	0��ʾ�ɹ�,��ֵ��ʾʧ��
 *************************************************************************/
int hdutil_lock_all_files(int mode)
{
	return mpdisk_process_all_partitions(lock_all_fn,&mode);

}



/*************************************************************************
 * 	������:	hdutil_filename2dirs()
 *	����:	Ϊ�ļ�����������Ӧ�ô�����Ŀ¼����
 *	����:	partition,����"/hqdata/sda1"
 *			filename������HQ_C00_D20051024125934_L30_T00.AVI
 *  ���:	dir,����/hqdata/sda1/2005/10/24/12/HQ_C00_D20051024125934_L30_T00.AVI
 * 	����ֵ:	0��ʾ�ɹ�,��ֵ��ʾʧ��
 *************************************************************************/
int hdutil_filename2dirs(char *partition, char *filename, char *dir)
{
	char *p;
	char path[200];
	char year[5];
	char month[3];
	char date[3];
	char hour[3];
	char part_name[20]; //����"sda1"
	//printf("filename is %s\n",filename);
	if((partition == NULL)||(filename==NULL)||(dir==NULL))
		return -1;
		
	p=strstr(partition,"/sd");
	if(p==NULL)
		return -1;
	
	sprintf(part_name,p+1);
	
	p=strstr(filename,"_D");
	if(p==NULL)
		return -1;	
	p=p+2;
	strncpy(year,p,4);
	year[4]='\0';
	p=p+4;
	strncpy(month,p,2);
	month[2]='\0';
	p=p+2;
	strncpy(date,p,2);
	date[2]='\0';
	p=p+2;
	strncpy(hour,p,2);
	hour[2]='\0';
	sprintf(path,"%s/%s/%s/%s/%s/%s/%s",HDMOUNT_PATH,part_name,year,month,date,hour,filename);
	strncpy(dir,path,200);
	//printf("path is %s\n",path);
	return 0;
}



int hex2ascii(char s)
{
	int value;
	value=toupper(s);
	switch(s)
		{
			case('F'):  value=63;break;
			case('E'):  value=62;break;
			case('D'):  value=61;break;
			case('C'):  value=60;break;
			case('B'):  value=59;break;
			case('A'):  value=58;break;			
		}
	return (value);	
}

//��ȡָ���ַ���β��ָ��
//��ĸ����������Ч�ַ�������������Ч�ַ�

char *get_strtail(char *pstr)
{
	char *p;
	if(pstr==NULL)
		return NULL;
	p=pstr;
	if(strlen(p)>100)
		return NULL;
	while(*p!=0)
	{
		if(isalnum(*p)==0)
			break; //������ĸ������
		p++;
	}
	return p;
}


/*************************************************************************
 * 	������:	hdutil_filename2finfo()
 *	����:	���ļ���ת���ɶ�Ӧ���ļ���Ϣ
 *	����:	filename���ļ���
 *  ���:	info,�ļ���Ϣ������ݽṹ
 * 	����ֵ:	0��ʾ�ɹ�,��ֵ��ʾʧ��
 *************************************************************************/
int hdutil_filename2finfo(char *filename,struct file_info_struct *info)
{
	char name[101],ctemp;
	char temp[5];
	struct tm ftime;
	int namelen,i,plen;
	int trig;
	//BYTE val;
	int state=0; //0��ʾ���ڲ��ұ��'-'
			    // 1��ʾ�ղ鵽һ�����'_'
	char *p,*t;
	
	if((filename==NULL)||(info==NULL))
		return -1;
	//memset((void*)info,0,sizeof(struct file_info_struct ));
	namelen=strlen(filename);
	if(namelen>100)
		return -1;
	memcpy(name,filename,namelen+1);
	p=strstr(filename,"sd");
	if(p!= NULL)
	{
		p+=4;  //p = "/2007"
		strncpy(info->partition,filename,p-filename);
		info->partition[12]='\0';
	}
	else
	{
		sprintf(info->partition,"%s","/hqdata/sda1");
		info->partition[12]='\0';
	}
	p=name;
	state=0;
	if(index(name,REMOTE_TRIG_FLAG)==NULL)
		info->remote=0;
	else
		info->remote=1;
	if(index(name,LOCK_FILE_FLAG)==NULL)
		info->lock=0;
	else
		info->lock=1;
	if(strstr(name,RECORDING_FILE_EXT)==NULL)
		info->ing_flag=0;
	else
		info->ing_flag=1;
	while((*p!='\n')&&(*p!='\0'))
	{

		if(state==0)
		{
			if(*p=='_')
			{
				state=1;
			}
			p++;
		}
		else
		{
			t=get_strtail(p);
			if(t==NULL)
			{
				//error
				return -1;
			}
			ctemp=*t;
			*t='\0';
			switch(*p)
			{
				case 'c':
				case 'C'://channel
					p++;
					info->channel=atoi(p);						
				break;
				case 'd'://���ڣ�ʱ��
				case 'D':
					p++;
					memcpy(temp,p,4);
					temp[4]='\0';
					p+=4;
					ftime.tm_year=atoi(temp)-1900;

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_mon=atoi(temp)-1;

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_mday=atoi(temp);
					
					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_hour=atoi(temp);
					
					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_min=atoi(temp);

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_sec=atoi(temp);

					info->stime=mktime(&ftime);
					
				break;
				case 'l':
				case 'L'://����
					p++;
					info->len=atoi(p); 						
				break;
				case 't': //����״̬
				case 'T':
					p++;
					trig=0;
					plen=strlen(p);
					if(plen>4)
					{
						p+=(plen-4);
						plen=4;
					}
					
					//p+=plen;
					for(i=0;i<plen;i++)
					{
						trig<<=4;
						trig=trig+hex2ascii(*p)-'0';
						p++;
					}
					info->trig=trig;

				break;
				default:
				break;
			}
			*t=ctemp;
			p=t;		
			state=0;
		}
	
	}
	return 0;
}


