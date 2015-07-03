/*
 * ��ȡָ����Ŀ¼����Ŀ¼�еĳ���汾��Ϣ��ʾ����Ļ 
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <commonlib.h>
#define LOCK_FILE_DIR	"/lock/vserver/"


int main(void)
{
	int ret;
	struct dirent **namelist;
        struct dirent *name;
	char filename[256];
	char version[256];
	char prog_name[100];
	int total,i;
	total=scandir(LOCK_FILE_DIR,&namelist,0,alphasort);
	if(total<=2)
	{
		printf("û���ҵ����е�gt1000Ӧ�ó���!!\n");
		exit(1);
	}
	for(i=0;i<total;i++)
	{
		name=namelist[i];
		if(name->d_type==DT_REG)
		{
			memset(prog_name,' ',sizeof(prog_name));
			sprintf(filename,"%s%s",LOCK_FILE_DIR,name->d_name);
			sprintf(prog_name,"%s",name->d_name);
			prog_name[strlen(name->d_name)]=(char)' ';
			prog_name[20]='\0';
			ret=get_prog_version(version,filename);
			if(ret==0)
			{
				printf("%s:%s\n",prog_name,version);
			}
			else
				printf("%s:unknow\n",prog_name);
		}
	}
	while(total--) 
		free(namelist[total]);
	free(namelist);
	
	
	return 0;
}

