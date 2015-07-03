#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <file_def.h>
#include <sys/vfs.h>
#include "testmod.h"
#include "pub_err.h"
#include "testIDE.h"
#include <signal.h>
#include <diskinfo.h>// added by lsk 2006 -12-14
#include <commonlib.h>
//#include "multicast_ctl.h"

typedef struct 
{
	int CF_flag;
	int Disk_flag;
	int Disk_partition;
}Disk_struct;

//static int IDE_fd=-1;
Disk_struct disk[4];
#if 0
/*
 * �����Խ�����ݽṹ���������ָ�����ļ�
 */

/*
 * ģ���������
 */
//��ȡ����������kΪ��λ
long get_disk_total(char *mountpath)
{
		 struct statfs	buf;
		 if(mountpath==NULL)
			return -1;
		 if(statfs(mountpath,&buf)<0)
		 {
			printf("error at check_disk\n");
			printf("��ȡ�����ܴ�Сʱʧ�ܣ�·��%s\n",mountpath);
			return -1;
		 }
		return buf.f_blocks*(buf.f_bsize>>10);
}
#endif
//���������������64K ����ΪCF������
//������ΪCF��������
#if 0
int get_cf_avail(void)
{
	
	long disktotal;
	disktotal=get_disk_total(HDMOUNT_PATH);
	printf(" ��������= %ld M\n", disktotal);
//	if (disktotal<1024*64)
	if (disktotal<200)
	{
		if(CF_flag==1)
		{
			printf("����С��200M  !!\n");
			printf("û�м�⵽���õ�CF��!!\n");
		 	CF_flag=0;
		} 
		return(-1);
	}
	else if(disktotal>20000)
	{
		if(CF_flag==0) 
		{	
			CF_flag=1;
			printf("��⵽���õ�Ӳ��\n");
		}
		return(0);
	}
	else 
	{
		if(CF_flag==0) 
		{	
			CF_flag=1;
			printf("��⵽���õ�CF��\n");
		}
		return(0);
	}
}
#endif
/*
*****************************************************
*��������: IDE_read_write_test
*��������: ���Դ����ļ���д
*����	: ��
*����ֵ   : 0 �ɹ� -1 ʧ��
*lsk 2007-11-8
*****************************************************
*/
#define IDE_TEST_FILE_SIZE	1024*10
#define IDE_TEST_BUF_LEN	256
int IDE_read_write_test(void)
{
	FILE* fp=NULL;
	int i,j,ret;
	BYTE temp[IDE_TEST_BUF_LEN];
	BYTE check[IDE_TEST_BUF_LEN];
	char * testfile="/hqdata/sda1/ide_test.txt"; 	// ������/hqdata·���´����ļ�
	BYTE cmd_buf[BUF_LEN];
	memset(cmd_buf,0,sizeof(cmd_buf));
	sprintf(cmd_buf,"rm -rf %s\n",testfile);
	fp = fopen(testfile, "wb+");
	if(fp==NULL)
	{
		printf("error open %s\n", testfile);
		gtlogerr("error open %s\n", testfile);
		return 3;
	}
	for(i=0;i<sizeof(temp);i++)
	{
		temp[i] = i;
	}
////���ļ���д���������
	for(j=0;j<(IDE_TEST_FILE_SIZE/IDE_TEST_BUF_LEN);j++)
	{
		for(i=0;i<sizeof(temp);i++)
		{
			ret = fwrite(&temp[i], 1,1,fp);
			if(ret!=1)
			{
				fclose(fp);
				system(cmd_buf);
				printf("write file %s error \n", testfile);
				return 4;
			}
		}
	}
	fclose(fp);
	fp = NULL;
	sleep(1);
	fp = fopen(testfile, "rb");
	if(fp==NULL)
	{
		fclose(fp);
		system(cmd_buf);
		printf("error open %s\n", testfile);
		gtlogerr("error open %s\n", testfile);
		return 3;
	}
////��ȡ���ݱȽ�
	for(j=0;j<(IDE_TEST_FILE_SIZE/IDE_TEST_BUF_LEN);j++)
	{
		memset(check, 0, sizeof(check));
		for(i=0;i<sizeof(check);i++)
		{
			ret = fread(&check[i], 1,1,fp);
			if(ret!=1)
			{
				fclose(fp);
				system(cmd_buf);
				printf("read file %s error \n", testfile);
				gtlogerr("read file %s error \n", testfile);
				return 5;
			}
			if(check[i]!=temp[i])
			{
				fclose(fp);
				system(cmd_buf);
				printf("read data from file %s error \n", testfile);
				gtlogerr("read data from file %s error \n", testfile);
				return 5;
			}
		}
	}
	fclose(fp);
	system(cmd_buf);
	printf("IDE file test ok\n");
	gtloginfo("IDE file test ok\n");
	return 0;
}

//����IDE�豸
/*
*****************************************************
*��������: test_IDE
*��������: ����IDE����ģ�麯��
*����	: 
*		multicast_sock*ns ���������������ݽṹ
*		*int prog ����	
*����ֵ   : �������
*lsk 2006-12-14 
*****************************************************
*/
int test_IDE(multicast_sock* ns, int* prog)
{
	int i;
	int part=0;
	int part_fg=0;
	int disk_fg=0;
	long int cap[4];
	char disk_name[50];
	unsigned char buf[200];
	i= get_sys_disk_num();
	if (i<=0)
	{
		printf("�Ҳ��� IDE �豸\n");
		*prog+=20;
		send_test_report(ns, "�Ҳ���IDE�豸", *prog);
		return 1;
	}
	printf("�ҵ�  %d ��IDE�豸\n", 1);
	for(i=0;i<1;i++)
	{
		disk[i].CF_flag = 0;
		disk[i].Disk_flag = 0;
		disk[i].Disk_partition = 0;
		part = 0;
		switch(i)
		{
			case 0:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, MASTER_DISK, strlen(MASTER_DISK));
			break;
			case 1:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE1_DISK, strlen(SLAVE1_DISK));
			break;
			case 2:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE2_DISK, strlen(SLAVE2_DISK));
			break;
			case 3:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE3_DISK, strlen(SLAVE3_DISK));
			break;
		}
		
		cap[i] = get_sys_disk_capacity(disk_name);
		
		if(cap[i]>0)
		{
			disk_fg = 1;
			memset(buf, 0, sizeof(buf));
			if(cap[i]<200)
			{
				sprintf(buf, "%s %s disk capacity = %ldM", "û�м�⵽���õ�CF��!", 
						disk_name, cap[i]);
			}
			else if((cap[i]>=200)&&(cap[i]<=2000))
			{
				disk[i].CF_flag = 1;
				sprintf(buf, "%s %s disk capacity = %ldM", "��⵽���õ�CF��!", 
						disk_name, cap[i]);
			}
			else		// if(cap[i]>2000)
			{
				disk[i].Disk_flag = 1;
				sprintf(buf, "%s %s disk capacity = %ldG", "��⵽���õ�Ӳ��!", 
						disk_name, cap[i]/1000);
			}
			printf("%s\n", buf);
			*prog=20;
			send_test_report(ns, buf, *prog);

			if(disk[i].Disk_flag||disk[i].CF_flag)
			{
				memset(buf, 0, sizeof(buf));
				part = get_sys_partition_num(disk_name);
				if(part==0)
				{
					sprintf(buf, "%s","����û�и�ʽ������");
					disk[i].Disk_partition=0;
					part_fg=1;	//not formated disk
				}
				else
				{
					sprintf(buf, "%s ��%d������", disk_name, part);
					disk[i].Disk_partition=part;
				}
				printf("%s\n", buf);
				*prog=25;
				send_test_report(ns, buf, *prog);
			}
		}
	}

	if(disk_fg==1)
	{
		if(part_fg==1)
			return 2;
		else
			return IDE_read_write_test();	/// lsk 2007 -11-8 �����ļ���д����
	}
	return 1;
}

#if 0
int test_IDE(void)
{
	if (access(IDE_DEV,F_OK)==0)
	{
		printf("�ҵ� IDE �豸\n");
		if(IDE_fd<0)
		{
			IDE_fd = open(IDE_DEV, O_RDWR);
			printf("open IDE...... \n");
		}
		if(IDE_fd<0)
		{
			printf("�򲻿�IDE �豸!\n");
			return 2;
		}
		if (get_cf_avail())
			return 3;
	}
	else
	{
		printf("�Ҳ��� IDE �豸\n");
		return 1;
	}
	return 0; 
}
#endif

