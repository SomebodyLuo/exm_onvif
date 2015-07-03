/**
	@file 	ftw_sort.c
	@brief	�ݹ����ָ��Ŀ¼�µ�����Ŀ¼
*/
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <ftw.h>
#include<errno.h>
#include"filelib.h"
#include<fixdisk.h>
//#include"./test_ftwsort/memwatch.h"



/*
ver:2.1
2007-12-07
a) ���Ӷ�fn�ļ��,���ΪNULL��ʲôҲ����;
b) ��ftw_sort()�д���ʱ�����err_fn��ΪNULL���͵�����;���ش�����
c) ������Ŀ¼��ɾ��


ver:2.0:
2007-06-20

�ڵ���scandir()������������ͷ��ڴ�ĺ���
-----------------------------------------------------------------------
ver:1.0:
2007-05-21

���ط�FN_RTURN_CONTINUE��رյ�ǰĿ¼���������Ŀ¼
�µ����Ŀ¼��û�������ر��ˣ����ᵼ����
ls /proc/pid/fd -l ʱ�������Ŀ¼���ڴ��ţ��Ͳ����ˣ�
�ͻ��д���.
-----------------------------------------------------------------------
*/

///ftw_sort.c����ʾ������Ϣ�Ŀ���
//#define DEG_FTW_NO_CHDIR					(0)	///<��ʾftw_sort������Ϣ����
//#define DEG_FTW_DIS_DIR					(1)	///<��ʾftw_sort������Ŀ¼
//#define DEG_FTW_NO_CHDIR_ERROR			(1)  ///<���ڲ��Ե�scandir����ʱ�Ŀ���

/**
	@brief  rm_file_select.c�м�¼Ŀ¼����Ϣ
*/
struct DIR_INFO
{
//int while_loop;					//whileִ�еļ���
//int check_loop;					//checkִ�еļ���
const char *rm_dir;				///<��Ҫ������Ŀ¼·��
const char *rm_file_path;			///<��Ҫ�������ļ�·��
char retu_flag;					///<���ر�־
char rm_empty_flag;				///<ɾ����Ŀ¼�ı�־
char fn_continue;					///<fn����������һƽ�нڵ��־
int retu_val;						///<����ֵ����
long ftw_dir_no;					///<��¼Ŀ¼����
DIR *pre_dp;					///<�ϲ�Ŀ¼��Ŀ¼��
};

/**
	@brief  	�˺����ݹ����ָ����Ŀ¼����ftw_sort()����
	@param  *dir    Ҫ������Ŀ¼��
	@param  (*fn)      �����ļ���Ŀ¼ʱ���õĺ���,fn���ط�0ֵʱ�˳�����,  ftw_sort��
			��fn  ���ص�ֵ
//	@param  dir_depth ������Ŀ¼��ȣ��������������ftw_sort����������һ��
//			����
	@param  sort_mode    ����ʱ������ģʽ,ȡֵΪFTW_SORT_ALPHA...
	@param  rm_empty_dir  1:������ֿ�Ŀ¼��ɾ��0:��ɾ����Ŀ¼
	@param  (*err_fn)  ��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
			�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
			�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
	@param  *dir_info  ��¼Ŀ¼��Ϣ�Ľṹ
	@return	�����ж��򷵻�fn()�����ķ���ֵ,ȫ���������򷵻� 0.����
			 �������򷵻�-1(��scandir����),�������errno
*/

int check_dir_user(const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data_tmp),
					void *user_data_tmp,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum),
					struct  DIR_INFO  *dir_info
			)

{	
	DIR *dp;						//����һ����Ŀ¼��
	struct stat statbuf;			//�ļ���Ϣ
	struct dirent *entry;			//ָ��Ŀ¼�������ָ��
	struct dirent **namelist;
	int strr_length = 0;			//����strrchr�󷵻ص��ַ�����
	int work_dir_length = 0;		//work_dirĿ¼�ĳ���
	int remove_res = 0;
	int dir_total = 0;				//��ǰĿ¼����scandir��ȡ��������Ŀ¼��
	int dir_total_bak=0;
	int file_order = 0;				//��¼namelist���±�
	int dir_no = 0;				//��¼ÿ��Ŀ¼�µ��ļ���
	int fn_ret_val = 0;
	int check_dir_val = 0;
	
	//const char *work_dir;				//ָ��ǰĿ¼��ָ��
	char work_dir[256];
	char temp_work_dir[256];	//�ļ�������������1024���ַ��ڣ��ɾ������
	//char *mem_dir;				//ÿ�εݹ�����ڵ�Ŀ¼��¼
	char mem_dir[256];			//ÿ�εݹ�����ڵ�Ŀ¼��¼

	//��ֹ����������
	remove_res = remove_res;
//	dir_depth = dir_depth;
	sort_mode = sort_mode;
	rm_empty_dir = rm_empty_dir;
	file_order = file_order;
	dir_no = dir_no;
	
	if(dir_info->retu_flag == 1)
	{
		return (dir_info->retu_val);
	}

	strcpy(mem_dir,dir);
	
#ifdef DEG_FTW_NO_CHDIR
	printf("mem_dir==%s\n",mem_dir);
#endif	
	strcpy(work_dir,dir);

	//dir_info->check_loop ++;			//����fn����ֵʱʹ��
	
#ifdef DEG_FTW_NO_CHDIR
	//printf("entry----check_loop==%d\n",check_loop);

	printf("entry---dir==%s\n",dir);
#endif	

#if 0
	if(dir_info->rm_file_path != NULL)
	{
		if(strcmp(dir_info->rm_file_path,mem_dir) == 0)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("i can not delet the important dir\n");
#endif
			return 0;
		}
	}
#endif

	//ȡ��Ŀ¼��Ϣ
	if ((lstat(dir, &statbuf)) != 0) 
	{
#ifdef DEG_FTW_NO_CHDIR
		printf("%s==error\n", dir);		
#endif
		//��ȡstat����;
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_NS,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				dir_info->retu_val = fn_ret_val; 	
				return (dir_info->retu_val);
			}
		}	
		//zw-mod-20071297<----

		//dir_info->retu_val = -1;
		//return (dir_info->retu_val);
	}
	
	//���ж��Ƿ�ΪĿ¼����Ŀ¼�Ļ����п���
	//�ݹ���ú���check_dir()
	if (S_ISDIR(statbuf.st_mode)) 
	{
#ifdef DEG_FTW_DIS_DIR
		printf("DIR:\t\t%s\n",dir);		
#endif

		dir_info->ftw_dir_no++;
 #ifdef DEG_FTW_NO_CHDIR
		printf("entry--->dir_info->ftw_dir_no=[%ld]\n",dir_info->ftw_dir_no);
 #endif
 
		//��������Ŀ¼�Ƿ񳬹�������
//		if(dir_info->ftw_dir_no > dir_depth)
		if(dir_info->ftw_dir_no > DIR_DEPTH)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("over the limit\n");
 #endif
			dir_info->ftw_dir_no--;
			return (dir_info->retu_val);
		}

		//if(dir_info->check_loop > 40)		
		//fn_ret_val = fn(dir,&statbuf,23);
		
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_D,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				//closedir(dir_info->pre_dp);
				dir_info->retu_val = fn_ret_val;
				return (dir_info->retu_val);
			}
		}
		//zw-mod-20071207<----

		
		//��Ŀ¼
		if(!(dp = opendir(dir))) 
		{	
			//���ɶ�ȡ��Ŀ¼
			//zw-mod-20071207---->
			if(fn!=NULL)
			{
				fn_ret_val = fn(dir,&statbuf,FTW_DNR,user_data_tmp);	
				//printf("fn--%d\n",__LINE__);
				if(fn_ret_val !=0)
				{	
					printf("opendir error\n");
					dir_info->retu_val = fn_ret_val;
					return (dir_info->retu_val);
				}				
			}
			//zw-mod-20071207<----
			
			//dir_info->retu_val = -1;
			//return (dir_info->retu_val);
		}

		dir_info->pre_dp = dp;

		//�Ե�ǰĿ¼�µ���Ŀ��������
		if(sort_mode==1)
		{
#ifdef DEG_FTW_NO_CHDIR_ERROR
			if(dir_info->ftw_dir_no == 3) dir = "test.";
#endif
			errno = 0;
			//����scandir��������
			if((dir_total = scandir(dir,&namelist,0,alphasort))<0)
			{				
				dir_total_bak = dir_total;
				printf("scandir <0\n");
				
				//��λ���ر�־
				dir_info->retu_flag = 1;
				
				//���ô�������
				//dir_info->retu_val = err_fn(dir,dir_total);

				if(err_fn!=NULL)
				{
					err_fn(dir,errno);
				}
				//���ش���ֵ
				dir_info->retu_val=-errno;
				return (dir_info->retu_val);
			}
			dir_total_bak=dir_total;
		}	

		//����scandir���صĵ�ǰĿ¼�µ��ļ�����
		//��ֵ���ֲ���������
		dir_no = dir_total;		

#ifdef DEG_FTW_NO_CHDIR
		//��ʾÿ��Ŀ¼�µ�Ŀ¼,�ж��Ƿ�Ϊ��Ŀ¼
		printf("dir_total = %d\n",dir_total);	
		for(file_order=0;file_order<dir_total;file_order++)
		{
			printf("dir_name==\t%s\n",namelist[file_order]->d_name);
		}
#endif		
		//��ȡĿ¼��������һ����ȡ��Ŀ¼�����
		//while ((entry = readdir(dp))!=NULL) 
		for(file_order=0;file_order<dir_total;file_order++)
		{			
			//printf("DIR-NAME==%s\n",entry->d_name);
			entry = namelist[file_order];
			//while_loop++;
#ifdef DEG_FTW_NO_CHDIR
			//printf("while_loop===%d\n",while_loop);			
			printf("WORK_DIR==%s\n",work_dir);
			printf("DIR-NAME==%s\n",entry->d_name);
#endif
			
			//�ж��Ƿ�Ϊ"."    �� ".."���ǵĻ��ͽ�����ǰѭ��
			//Ȼ������жϴ�Ŀ¼�µ���һ��Ŀ¼�����
			if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
			{
#ifdef DEG_FTW_NO_CHDIR
				printf("DIR=.=..==\n");
#endif
				//������ǰѭ��,�����ж����Ŀ¼�µ���һ��Ŀ¼���ļ�
				continue;
			}

			//ִ�е�����˵���Ѿ�����"."    �� ".."  ����һ��Ŀ¼
			//Ȼ�������Ŀ¼��Ϊ�����ݹ����
#ifdef DEG_FTW_NO_CHDIR
			printf("before memset=work_dir==%s\n",work_dir);
			//printf("work==%d===temp==%x===\n",work_dir,temp_work_dir);
#endif
			memset(temp_work_dir,0,sizeof(temp_work_dir));			
#ifdef DEG_FTW_NO_CHDIR
			printf("while(1)==work_dir==%s\n",work_dir);
#endif
			//�޸�Ŀ¼��ʹ��ǰĿ¼ָ������һ����Ŀ¼�����������Ŀ¼
			//���ֵ���ǰĿ¼�ĺ���
			strcat(temp_work_dir,work_dir);			
			strcat(temp_work_dir,"/");
			strcat(temp_work_dir,entry->d_name);
			//ʹ��strdup��Ϊ�˷�ֹ������temp_work_dir���ڴ��ֱַ�Ӵ���ָ��work_dir��
			//�����Ļ���ǰ���temp_work_dir����ʱ�Ͱ�ָ��Ҳ��������
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
#ifdef DEG_FTW_NO_CHDIR			
			printf("here?\n");
			printf("before check_dir==work_dir==%s\n",work_dir);
#endif
			//�ݹ���ã������Ǹղ��ҵ��ĳ�"."  ��".. "���Ŀ¼���ļ�������
			//lint -e534
			check_dir_val = check_dir_user(	work_dir,
							fn,
							//dir_depth,
							user_data_tmp,
							sort_mode,
							rm_empty_dir,
							err_fn,
							dir_info
							);
#if 0
			//�Է���ֵ�����жϣ��Ƿ�ֱ�ӷ���
			if(check_dir_val == FN_RETURN_CONTINUE )
			{
				//dir_info->retu_val = check_dir_val;
				return (0);
			}
#endif


			
			if((check_dir_val != FN_RETURN_CONTINUE) && (check_dir_val != 0))
				{	
					/*	
						���ط�FN_RTURN_CONTINUE��رյ�ǰĿ¼���������Ŀ¼
						�µ����Ŀ¼��û�������ر��ˣ����ᵼ����
						ls /proc/pid/fd -l ʱ�������Ŀ¼���ڴ��ţ��Ͳ����ˣ�
						�ͻ��д���.2007-05-21
					*/
					closedir(dp);					
					dir_info->retu_flag = 1;
					dir_info->retu_val = check_dir_val;
					return (dir_info->retu_val);
				}


			//lint +e534			
			if(dir_info->rm_empty_flag == 1)
			{
				dir_no --;

				//��Ŀ¼ɾ����־���³�ʼ��Ϊ0
				dir_info->rm_empty_flag = 0;
			}

			//����������ǵ�����ǰĿ¼·����Ҳ���Ƿ��ص��ڵ���check_dirǰ��Ŀ¼
			//����һ��Ŀ¼
			//lint -e1055
			work_dir_length = strlen(work_dir);
			//lint +e1055
#ifdef DEG_FTW_NO_CHDIR
			printf("strrchr(%s)==%s\n",work_dir,strrchr(work_dir,'/'));
#endif
			//lint -e668
			strr_length = strlen(strrchr(work_dir,'/'));
			memset(temp_work_dir,0,sizeof(temp_work_dir));
			strncpy(temp_work_dir,work_dir,work_dir_length-strr_length);
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
			
#ifdef DEG_FTW_NO_CHDIR				
			printf("after check_dir===work_dir==%s\n",work_dir);
#endif
		
		}

		/*
			����������������ͷ���scandir���õĲ���namelist��
			ռ�õ��ڴ�ռ䣬������ִ��ftw_sort��ʱ��ռ��
			ϵͳ��Դ���ڴ��ڲ�ͣ��������������������
			2007-06-20
		*/	
#if 1
		while(dir_total_bak--)
		{
			free(namelist[dir_total_bak]);
		}
		free(namelist);
#endif
 
		
#if 1		
		if((closedir(dp))== ERROR_VAL)
		{
			printf("closedir error\n");
			//dir_info->retu_val = -1;
			//return (dir_info->retu_val);
		}
#endif		
			dir_info->ftw_dir_no--;

	}
	else
	{
#ifdef DEG_FTW_DIS_DIR
		//ִ�е�����˵������Ŀ¼�����ļ�
		printf("document:\t%s\n",dir);
#endif
		
		//����Ŀ¼��һ���ļ�
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_F,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				dir_info->retu_val = fn_ret_val;
				return (dir_info->retu_val);
			}
		}
		//zw-mod-20071207<----
		
	}
	
#ifdef DEG_FTW_NO_CHDIR
	//printf("=exit==(%d)=done.\n",check_loop);
	printf("mem_dir==%s\n",mem_dir);
#endif
	if(strcmp(dir_info->rm_dir,mem_dir)!=0 )
	{				
		//remove_res = remove(mem_dir);
	}
	else
	{
#ifdef DEG_FTW_NO_CHDIR	
		printf("return the ==%s==\n",mem_dir);
#endif
	}

#ifdef DEG_FTW_NO_CHDIR
	printf("leaving<----dir_info->ftw_dir_no=[%ld]\n",dir_info->ftw_dir_no);
#endif

	if((dir_no == 2) && (rm_empty_dir == 1))
	{	
		dir_info->rm_empty_flag = 1;
		
		//������Ŀ¼zw-mod-20071207---->
		if(dir_info->ftw_dir_no!=0)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("leaving[%ld],,,remove=%s\n",dir_info->ftw_dir_no,dir);
#endif
			remove(dir);
		}
		else
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("i am 0\n");
#endif
		}
		
		//zw-mod-20071207<----
	}	

	//�ͷ��ڴ�
	//free(mem_dir);
	//printf("dir=%s\n",dir);
	return dir_info->retu_val;
	 
}





/**
	@brief	�ӿں������ڲ�����check_dir()	
	@param  *dir    Ҫ������Ŀ¼��
	@param  (*fn)      �����ļ���Ŀ¼ʱ���õĺ���,fn���ط�0ֵʱ�˳�����,  ftw_sort��
					��fn  ���ص�ֵ
	@param  dir_depth ������Ŀ¼��ȣ��������������ftw_sort����������һ��
			����
	@param  sort_mode    ����ʱ������ģʽ,ȡֵΪFTW_SORT_ALPHA...
	@param  rm_empty_dir  1:������ֿ�Ŀ¼��ɾ��0:��ɾ����Ŀ¼
	@param  (*err_fn)  ��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
			�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
			�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
	@return	�����ж��򷵻�fn()�����ķ���ֵ,ȫ���������򷵻� 0.����
			�������򷵻�-1(��scandir����),�������errno

*/
#if 0
int ftw_sort_user(	const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag),
					int dir_depth,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum)
			)
#endif

int ftw_sort_user(      const char *dir,
                                        int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data),
                                        void *user_data,
                                        int sort_mode,
                                        int rm_empty_dir,
                                        int (*err_fn)(const char *dir_file,     int errnum)
                        )

{
	struct DIR_INFO *dir_info;				//����ָ��ṹ��ָ��
	struct DIR_INFO ftw_dir_info;			//����һ�� Ŀ¼��Ϣ�Ľṹ
	
	int ftw_no_chdir_val = 0;					//����ֵ��ʼ��

	//�������������
	//dir_depth = dir_depth;				
	sort_mode = sort_mode;
	rm_empty_dir = rm_empty_dir;	
/*
	//��ʼ���ṹָ��
	//lint -e10
	if((dir_info = (struct DIR_RM_INFO *)malloc(sizeof(struct DIR_RM_INFO)))==NULL)
	{
		ftw_no_chdir_val = -1;
		return ftw_no_chdir_val;
	}
	//lint +e10
*/

	int while_cnt=0;

	//ָ���ʼ��
	dir_info = &ftw_dir_info;	

	//��ʼ���ṹ��Ա
	dir_info->rm_dir = dir;					//����Ŀ¼��ʼ��	 
	dir_info->retu_val = 0;					//����ֵ��ʼ��	
	dir_info->ftw_dir_no = 0;				//��¼������Ŀ¼���
	dir_info->retu_flag = 0;					//��ʼ�����ر�־
	dir_info->rm_empty_flag = 0;			//��ʼ��ɾ����Ŀ¼��־	
	dir_info->fn_continue = 0;				//��ʼ��fn������һ���ڵ�ı�־
	dir_info->pre_dp=NULL;
	//dir_info->check_loop = 0;				//����fn����ֵʱʹ��
	
	if(err_fn==NULL)
	{
		return -1;
	}
	

	printf("test ftw_sort_user\n");


	//���ñ����������б���
	ftw_no_chdir_val = check_dir_user(	dir,
							fn,
							//dir_depth,
							user_data,
							sort_mode,
							rm_empty_dir,
							err_fn,
							dir_info
							);

	printf("finish the dir=%s------%d\n",dir,while_cnt);

	
	return ftw_no_chdir_val;
}

#if 0
/*wsyadd, ���ڴ������ftw_sort����ʱ����Ҫ������̵����*/
int  fix_disk_ftw(const char *dirname, int err)
{
	if(dirname==NULL)
		return -EINVAL;
	
	gtlogerr("ftw ����, %s, %s\n",dirname, strerror(err));
	fix_disk(dirname,err);
	return FN_DISK_ERR;
}
#endif
