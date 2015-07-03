/** 
	@file		rm_file_select.c
	@brief	ɾ��ָ��Ŀ¼�µ���Ŀ¼
*/
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include"filelib.h"

///rm_select_file.c�еĺ궨��,��ʾ������Ϣ�Ŀ���
//#define DEG_RM_NO_CHDIR					(0)///<��ʾrm_file_select���Կ���
//#define DEG_RM_DIS_DIR					(0)///<��ʾrm_file_selectĿ¼���ļ����� 


/**
	@brief  rm_file_select.c�м�¼Ŀ¼����Ϣ
*/
struct DIR_RM_INFO
{
//int while_loop;					//whileִ�еļ���
//int check_loop;					//checkִ�еļ���
const char *rm_dir;				///<��Ҫ������Ŀ¼·��
const char *rm_file_path;			///<��Ҫ�������ļ�·��
int retu_val;						///<����ֵ����
};


/**
	@fn  	int ftw_no_chdir(const char *dir,struct DIR_RM_INFO *dir_rm_info)  
	@brief     �˺����ݹ����ָ��Ŀ¼ 
	@param  dirָ��Ҫ������Ŀ¼
	@param  *dir_rm_infoĿ¼�ṹ��Ϣ
	@return   ftw_no_chdir�ɹ�ִ�з���0�����򷵻�-1

	�ɺ���rm_file_select()���ã���Ҫ��ɱ���ָ��Ŀ¼����
  	���ݱ���·��������ֵ������Ҫ������Ŀ¼�����
  	����rm_file_select()��file_path = NULL    ��ɾ��ָ��Ŀ¼������
  	��Ŀ¼���ļ���������ָ��Ŀ¼�µ���Ŀ¼����
  	��ɾ��������Ŀ¼���ļ� ��
*/
int ftw_no_chdir(const char *dir,struct DIR_RM_INFO *dir_rm_info)
{
	DIR *dp;						//����һ����Ŀ¼��
	struct stat statbuf;			//�ļ���Ϣ
	struct dirent *entry;			//ָ��Ŀ¼�������ָ��
	int strr_length = 0;			//����strrchr�󷵻ص��ַ�����
	int work_dir_length = 0;		//work_dirĿ¼�ĳ���
	int remove_res = 0;
	//const char *work_dir;			//ָ��ǰĿ¼��ָ��
	char work_dir[1024];
	char temp_work_dir[1024];	//�ļ�������������1024���ַ��ڣ��ɾ������
	//char *mem_dir;				//ÿ�εݹ�����ڵ�Ŀ¼��¼
	char mem_dir[1024];			//ÿ�εݹ�����ڵ�Ŀ¼��¼

	remove_res = remove_res;
	//����malloc�ᵼ�²�������
	//mem_dir =(char *) malloc(strlen(dir));
	//mem_dir = strdup(dir);

	strcpy(mem_dir,dir);
	
#ifdef DEG_RM_NO_CHDIR
	printf("==entry-ftw_no_chdir==\n");
	printf("mem_dir===%s\n",mem_dir);
#endif	
	strcpy(work_dir,dir);
	//check_loop ++;
#ifdef DEG_RM_NO_CHDIR
	//printf("entry----check_loop==%d\n",check_loop);
	printf("entry---dir==%s\n",dir);
#endif

	if(dir_rm_info->rm_file_path != NULL)
	{
		if(strcmp(dir_rm_info->rm_file_path,mem_dir) == 0)
		{
#ifdef DEG_RM_NO_CHDIR
			printf("i can not delet the important dir\n");
#endif
			return 0;
		}
	}

	//ȡ��Ŀ¼��Ϣ
	if ((lstat(dir, &statbuf)) != 0) 
	{
#ifdef DEG_RM_NO_CHDIR
		printf("%s==error\n", dir);		
#endif
		dir_rm_info->retu_val = -1;
		return (dir_rm_info->retu_val);
	}
	
	//���ж��Ƿ�ΪĿ¼����Ŀ¼�Ļ����п���
	//�ݹ���ú���check_dir()
	if (S_ISDIR(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)) 
	{
#ifdef DEG_RM_DIS_DIR
		printf("DIR:\t\t%s\n",dir);
#endif
		//��Ŀ¼
		if(!(dp = opendir(dir))) 
		{
			printf("opendir error\n");
			dir_rm_info->retu_val = -1;
			return (dir_rm_info->retu_val);
		}

		//��ȡĿ¼��������һ����ȡ��Ŀ¼�����
		while ((entry = readdir(dp))!=NULL) 
		{
			//while_loop++;
#ifdef DEG_RM_NO_CHDIR
			//printf("while_loop===%d\n",while_loop);			
			printf("WORK_DIR==%s\n",work_dir);
			printf("DIR-NAME==%s\n",entry->d_name);
#endif
			
			//�ж��Ƿ�Ϊ"."    �� ".."���ǵĻ��ͽ�����ǰѭ��
			//Ȼ������жϴ�Ŀ¼�µ���һ��Ŀ¼�����
			if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
			{
#ifdef DEG_RM_NO_CHDIR
				printf("DIR=.=..==\n");
#endif
				//������ǰѭ��,�����ж����Ŀ¼�µ���һ��Ŀ¼���ļ�
				continue;
			}

			//ִ�е�����˵���Ѿ�����"."    �� ".."  ����һ��Ŀ¼
			//Ȼ�������Ŀ¼��Ϊ�����ݹ����
#ifdef DEG_RM_NO_CHDIR
			printf("before memset=work_dir==%s\n",work_dir);
			//printf("work==%d===temp==%x===\n",work_dir,temp_work_dir);
#endif
			memset(temp_work_dir,0,sizeof(temp_work_dir));			
#ifdef DEG_RM_NO_CHDIR
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
#ifdef DEG_RM_NO_CHDIR			
			printf("here?\n");
			printf("before check_dir==work_dir==%s\n",work_dir);
#endif
			//�ݹ���ã������Ǹղ��ҵ��ĳ�"."  ��".. "���Ŀ¼���ļ�������
			//lint -e534
			ftw_no_chdir(work_dir,dir_rm_info);
			//lint +e534
			//����������ǵ�����ǰĿ¼·����Ҳ���Ƿ��ص��ڵ���check_dirǰ��Ŀ¼
			//����һ��Ŀ¼
			//lint -e1055
			work_dir_length = strlen(work_dir);
			//lint +e1055
#ifdef DEG_RM_NO_CHDIR
			printf("strrchr(%s)==%s\n",work_dir,strrchr(work_dir,'/'));
#endif
			//lint -e668
			strr_length = strlen(strrchr(work_dir,'/'));
			memset(temp_work_dir,0,sizeof(temp_work_dir));
			strncpy(temp_work_dir,work_dir,work_dir_length-strr_length);
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
			
#ifdef DEG_RM_NO_CHDIR				
			printf("after check_dir===work_dir==%s\n",work_dir);
#endif
		}
//		closedir(dp);
		
		if((closedir(dp))== ERROR_VAL)
		{
			printf("closedir error\n");
			//dir_rm_info->retu_val = -1;
			//return (dir_rm_info->retu_val);
		}

	}
	else
	{
#ifdef DEG_RM_DIS_DIR
		//ִ�е�����˵������Ŀ¼�����ļ�
		printf("document:\t%s\n",dir);
#endif
	}
#ifdef DEG_RM_NO_CHDIR
	//printf("=exit==(%d)=done.\n",check_loop);
	printf("mem_dir==%s\n",mem_dir);
#endif
	if(strcmp(dir_rm_info->rm_dir,mem_dir)!=0 )
	{				
		remove_res = remove(mem_dir);
	}
	else
	{
#ifdef DEG_RM_NO_CHDIR	
		printf("return the ==%s==\n",mem_dir);
#endif
	}

	//�ͷ��ڴ�
	//free(mem_dir);
	return dir_rm_info->retu_val;
	
}





/**
	@fn 		int rm_file_select(const char *dir,const char *file_path)
	@brief   	rm_file_select�ӿں���  
	@param  *dir������ Ŀ¼
	@param  *file_path ��Ҫ������Ŀ¼�����file_path = NULL ��ɾ��
			ָ��Ŀ¼�µ�����Ŀ¼,   ����file_pathָ����Ŀ¼��ɾ������
 			��Ŀ¼���ļ���
 	@return	ftw_no_chdir�ķ���ֵ

 	�����ڲ��ٴε���ftw_no_chdir
 */

int rm_file_select(const char *dir,const char *file_path)
{
	struct DIR_RM_INFO *dir_rm_info;
	struct DIR_RM_INFO dir_info;
	
	int ftw_no_chdir_val = 0;
/*
	//��ʼ���ṹָ�룬ʹ����malloc
	//lint -e10
	if((dir_rm_info = (struct DIR_RM_INFO *)malloc(sizeof(struct DIR_RM_INFO)))==NULL)
	{
		ftw_no_chdir_val = -1;
		return ftw_no_chdir_val;
	}
	//lint +e10
*/

	//ָ���ʼ��
	dir_rm_info = &dir_info;
	
	//��ʼ���ṹ��Ա
	dir_rm_info->rm_dir = dir;					//����Ŀ¼��ʼ��
	dir_rm_info->rm_file_path = file_path;		//����Ŀ¼��ʼ��	 
	dir_rm_info->retu_val = 0;					//����ֵ��ʼ��
	
	//���ñ����������б���
	ftw_no_chdir_val = ftw_no_chdir(dir,dir_rm_info);

	return ftw_no_chdir_val;
}



