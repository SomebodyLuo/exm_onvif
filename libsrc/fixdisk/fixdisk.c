#include <time.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include "fixdisk.h"
#include <sys/stat.h>
#include "file_def.h"
#include "devinfo.h"
#include <errno.h>
#include <filelib.h>

/*
 * ������	:is_disk_error()
 * ����	:�жϴ�������errno�Ƿ���Ӧ��������̵�����
 * ����	:error:������
 * ����ֵ	:1��ʾӦ������̣�0��ʾ�����������
*/
int is_disk_error(int error)
{
	if((error==EIO)||(error==EACCES)||(error=EBUSY)||(error=EROFS))
		return 1;
	else
		return 0;
}


/*
	����:	fixdisk_log()
	����:   ��������̵������Ϣ��type,reason��д��fixlog�
			������ʱ��ʹ洢��������
	����:	fixlog,������־���ļ�����
			typeΪ����ѡ��,y,c,f֮һ��
			reasonΪ����ԭ���ַ���
	����ֵ:	��
*/
void fixdisk_log(char *fixlog,char type,char *reason)
{

	struct timeval tv;
	struct tm *ptime;
	time_t ctime;
	struct stat buf;
	FILE *dfp;
	

	dfp=fopen(fixlog,"a+");
	if(dfp==NULL)
		return ;

	//дʱ��
	if(gettimeofday(&tv,NULL)<0)
	{
		fprintf(dfp,"<��ȡϵͳʱ��ʱ���� >   ");
	}
	else
	{
		ctime=tv.tv_sec;
		ptime=localtime(&ctime);
		if(ptime!=NULL)
			fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d> ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);	
	}
	//д��Ϣ
	fprintf(dfp,"  *%c  ",type);
	switch(get_hd_type())
		{
			case(1): fprintf(dfp," HD ");break;
			case(0): fprintf(dfp," CF ");break;
			default: fprintf(dfp," -- ");break;
		}
	fprintf(dfp," #%ld  ",mktime(ptime));
	fprintf(dfp," %s ",reason);	
	fclose(dfp);

	
}
	
//�����־�����Ƿ�������ѱ������������ֵΪ�´�Ҫ�õ��޸�ѡ�y,f,c��q(�˳�)
//����ϴ�������FIXDISK_INTERVAL����,���ø���ʱ��ѡ���޸���������-y�޸�
//����fixlog, ����"/var/tmp/fixdisk-hda2.txt",��ʾ��Ҫ����ķ�����������־

char check_last_fixdisk(char * fixlog)
{
	FILE * fp;
	char temp[200];
	char log[200];
	struct timeval tv;
	struct tm *ptime;
	time_t ctime;
	char *lp;
	
	fp=fopen(fixlog,"r+"); 
	
	if (fp==NULL)
		return 'y'; //��û���޹�
	
	fseek(fp,-200,SEEK_END);//��ȡ���һ��
	while(fgets(temp,200,fp)!=NULL)
	{
		strcpy(log,temp);
	}

	//����log,<2006-05-29 14:02:06>  y  no CF avail��ʽ

	//�ȿ��Ƿ�������
	lp=strstr(log,"$DONE");
	if(lp==NULL)
		return 'q';
	//gtloginfo("test,��$done��־!!!!!!!!\n");

	//��ȡʱ�䣬���ж�
	lp=index(log,'#');
	if(lp==NULL)
		return 'y';
	lp++;

	if(gettimeofday(&tv,NULL)<0)
		return 'y';
	else
	{
		ctime=tv.tv_sec;
		ptime=localtime(&ctime);
		if((ptime!=NULL)&&(mktime(ptime)-atoi(lp)<FIXDISK_INTERVAL))
			{
				//�ڶ���֮�ڸ���ĳѡ���޸���
				lp=index(log,'*');
				if(lp!=NULL)
					{
						switch(*(lp+1))
							{
								case 'y': // wsydel if(use_harddisk!=0)//������cf����Ҫ��-f��
									//	  	return 'q';
									//	  else	
										  	return 'f';
										  break;
								case 'f':  if(get_hd_type()!=0 )//wsyadd, Ӳ�̶�����-f
										return 'q';
									 else
										 return 'c';
										  break;
								case 'c': 
								case 'q': return 'q';break;
								default:  return 'y';
							}
					}

			}
			
	}
	return 'y';
}
	
/*
	������: get_error_partition()
	����: �����������Ŀ¼���ļ������ķ����ַ���
	����:path,���������Ŀ¼���ļ�����
	���:partition,��Ҫ����ķ���������"sda3"
		 fixlog,��Ӧ��������־������"/var/tmp/fixdisk-sda2.txt"
	����ֵ:�ɹ�����0�����򷵻ظ�ֵ
*/

int get_error_partition(IN char *path, OUT char *partition, OUT char *fixlog)
{
	char *lp;
	if((path==NULL)||(partition ==NULL))
		return -EINVAL;
	lp = strstr(path,"/sd");
	if(lp==NULL)
	{
		sprintf(partition,"sda1");//����
	}
	else
		sprintf(partition,"%s",lp+1);
	lp = index(partition,'/');
	if(lp!=NULL)
		*lp = '\0';
	sprintf(fixlog,"/var/tmp/fixdisk-%s.txt",partition);
	return 0;	
}

/*
 * ������	:fix_disk()
 * ����	:���errnoӦ��������̣�������ط����������¼�������ѡ���������
 * ����	:path:����"/hqdata/hda2/xxx"���ַ�����ֻҪǰ׺��"/hqdata/hda2"�Ϳ���
 *	 	 errno:�����룬��ֵ
 * ����ֵ:��
*/
void fix_disk( char *path,int diskerrno)
{
	char type;
	char cmd[50];
	int usehd;
	char partition[100];
	char fixlog[100];
	int lock_file;
	int i;

	//set_cferr_flag(1);
#if EMBEDED==1
	
	if(get_ide_flag()==0) //û�洢�豸
		return;
	if(is_disk_error(diskerrno) == 0) //�����ļ�ϵͳ����
		return ;
		
	get_error_partition(path,partition,fixlog);
	
	set_cferr_flag(1);
	type=check_last_fixdisk(fixlog);
	if(type=='q')
	{
		gtloginfo("fix_disk���Ѿ������%s,ʧ���˻�û���꣬�˳�����ж�ط���\n",partition);
		//wsy�ģ��˳���������������ж�ص��������
		close_all_res(); 
		sprintf(cmd,"/ip1004/diskumount %s &",partition);
		system(cmd);
		exit(0);
		//return ;
	}
	gtloginfo("����fixdiskproblem,��%cѡ���������%s\n",type,partition);
	gtloginfo("����ԭ��:����%s:%s\n",path,strerror(diskerrno));
	fixdisk_log(fixlog,type,strerror(diskerrno));
	close_all_res(); 
	//ִ��fixdisk,������	
	sprintf(cmd,"/ip1004/fixdiskproblem -%c %s &",type,partition);
	system(cmd);
	exit(0);			
#endif
	return;
}
/*wsyadd, ���ڴ������ftw_sort����ʱ����Ҫ������̵����*/
int  fix_disk_ftw(const char *dirname, int err)
{
        if(dirname==NULL)
                return -EINVAL;

        gtlogerr("ftw ����, %s, %s\n",dirname, strerror(err));
        fix_disk(dirname,err);
        return FN_DISK_ERR;
}	


