#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<stdio.h>
#include<ftw.h>
#include"../filelib.h"
//#include"memwatch.h"




/*****************************************
err_fn ��Ҫ�Լ����壬�����Ϊ��ʾ
*****************************************/
int err_fn_user(const char *dir_file,int errnum)
{
	printf("in the err_fn\n");
//	printf("#######################===%s=====%d\n",dir_file,errnum);
//	printf("stop\n");
	return (-1);
}


/******************************************
fn������Ҫ�Լ����壬�����Ϊ��ʾ
******************************************/
int fn_user(const char *file,const struct stat *sb,int flag,void *user_data)
{
	char *dirtest = NULL;

	printf("user_data=[%s]\n",(char *)user_data);	
	printf("in the fn===file==\t%s\n",file);
	
	//printf("\t\t\tflag = %d\n",flag);
		sb = sb;
		flag = flag;
		/*��Ŀ¼���к���"2d-2d"ʱ����1ֵ,��������:

		a)   ������ֵΪFN_RETURN_CONTINUEʱ�������Ե�ǰ�ڵ�
		���ӽڵ�ı����Ͳ��������ж��Ƿ���Ҫɾ����
		Ŀ¼�Ȳ���������ת����ǰ�ڵ����һ��ƽ�н�
		��ȥ������

		*/
		dirtest = strstr(file,"1e-13e");
		if(dirtest != 0 )
		{	
			printf("found 1e-1e \n");
			return FN_RETURN_CONTINUE;
		}

		/*��Ŀ¼���к���"4b-1b"ʱ�����ز���FN_RETURN_CONTINUE������
		����ֵ����������:

		b)  ������ֵΪ��������ֵʱ������������
		     ����ֵ���ظ��û���
		*/
		dirtest = strstr(file,"13-1e4");
		//ret = strcmp(file,"/hqdata/2007/06/14/05/HQ_C00_D20070614050744_L10_T00.AVI");
		if(dirtest != 0 )
		{	
			printf("found 4b-1b\n");
			return 4;
		}
		
		return 0;
	
}
	
/**************************************
������
***************************************/
int main(int argc,char *argv[])
{
	
	int ftw_re_val = 0;
	char *dir;	
	char *str_test="user_data_test";
	

	if(argc == 1)
	{
		dir ="dirtest";
	}
	if(argc == 2)
	{
		dir = argv[1];
	}
	
	//char * dir ="/hqdata";
	printf("================================\n");
	printf("========DIR_DEPTH=================\n");
		
	printf("the target dir= %s\n",dir);
	printf("++++++++++++++++++before entry the check_dir()\n");

	printf("==main()=done......... to=the==%s=\n",get_current_dir_name());

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	int while_cnt = 0;

//	while(1)
	{
			ftw_re_val = ftw_sort_user(dir,fn_user,(void *)str_test,1,0,err_fn_user);
			
			printf("==ftw_sort_return=====%d\n",ftw_re_val);
		
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@===%d\n",while_cnt);
		while_cnt ++;		
	//getchar();
		sleep(1);
	}
	return (0);
}


