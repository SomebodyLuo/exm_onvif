#include <file_def.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <gtthread.h>
#include <commonlib.h>
#include <gt_dev_api.h>
#include <devinfo.h>
#include <nv_pair.h>
#include "diskinfo.h"

/*
*****************************************************
*��������: format_disk
*��������: ��ʽ������ ���մ��̵�����ѡ��ͬ�ĸ�ʽ������
*���룺char* disk_name ��������
*�����
����ֵ:0 �ɹ���1��ʾû�д��̣�2��ʾ��ʽ��ʧ��
*�޸���־��
*****************************************************
*/ 
int format_disk(char* disk_name)
{
	int ret;
	long int cap;
	cap = get_sys_disk_capacity(disk_name);
	if (cap<=0)
	{
		printf("�Ҳ���%s IDE �豸\n", disk_name);
		return 1;
	}
	printf("�ҵ�  %s IDE�豸\n", disk_name);
		
	if(cap<200)
	{
		printf("%s capacity = %ld less than 200M \n", disk_name, cap);
		printf("û�м�⵽���õ�CF��!!\n");
		return 1;
	}
	else if((cap>=200)&&(cap<=2000))
	{
		printf("��⵽���õ�CF��\n");
		printf("%s disk capacity = %ldM\n", disk_name, cap);
		ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 65536\n");	//�ٸ�ʽ��
		printf("ret = %d \n",ret);
		if(ret!=0)
		{
			return 2;
		}
	}
	else		// if(cap[i]>2000)
	{
		printf("��⵽���õ�Ӳ��\n");
		printf("%s disk capacity = %ldG\n", disk_name, cap/1000);
		ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 524288\n");	//�ٸ�ʽ��
		if(ret!=0)
		{
			return 2;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret=0;
	char disk_name[50];
	if(argc<2)
	{
		printf("lack parameter\n");
		printf("format <disk name>\n");
		exit(1);
	}
#if 0
	system("killall -9 watch_proc \n");
	system("killall -9 hdmodule\n");
	system("killall -9 diskman\n");
	system("killall -9 vsftpd\n");
	system("umount /hqdata\n");
#endif
	memset(disk_name,0,sizeof(disk_name));
	memcpy(disk_name, argv[1], strlen(argv[1]));
	ret = format_disk(disk_name);
	exit(ret);
}

