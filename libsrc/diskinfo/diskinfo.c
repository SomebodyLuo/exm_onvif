#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <file_def.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
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
#define VERSION 		"0.03"
//ver:0.03    �޸���get_sys_disk_devname ��get_sys_disk_name
//ver:0.02	��Ӷ�sd����֧��
//

#define FILE_PATH	 "/proc/partitions"

//#define NO_DISK				3003
#define NO_DISK				0
#define DISK_NAME_ERROR 	3001
#define PART_INDEX_ERROR	3002
#define NO_PARTITIONS		3004
#define ERROR_OPEN_FILE		3005


#define TEMP_SD         ///��ʱΪsd���ӵĶ�������ʽ����Ҫ����sd������������/proc/partitions��������ʾ����
#ifdef TEMP_SD
#include <devinfo.h>
#endif



/*
*****************************************************
*��������: check_disk_name
*��������: ����������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ�����0����ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
static  int check_disk_name(const char *disk_name)
{
	char buf[200];
	
	if(disk_name==NULL)
	{
		return -1;
	}
	memset(buf, 0 , sizeof(buf));
	sprintf(buf,"%s",disk_name);
	if(strncmp(buf, MASTER_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE1_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE2_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE3_DISK, strlen(buf))==0)
		return 0;
	
	//��Ӽ��sd���ֵĲ���add by zw
	if(strncmp(buf, SD_MASTER, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SD_SLAVE1, strlen(buf))==0)
		return 0;

	return -1;
}
/*
*****************************************************
*��������: open_partition_file
*��������: ��/proc/partitions �ļ�
*���룺
*�����
����ֵ: �ɹ������ļ�ָ�룬NULL��ʾʧ��
*�޸���־��
*****************************************************
*/ 
static  FILE* open_partition_file(void)
{
	FILE* fp=NULL;
	fp=fopen(FILE_PATH, "r")	;
	if(fp==NULL)
	{
		return NULL;
	}
	return fp;
}
/*
*****************************************************
*��������: search_disk
*��������: ��ȡ����+����������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش��̷�����������ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
static  int search_disk(const char*disk_name)
{
	FILE* fp=NULL;
	char* cp=NULL;
	int num=0;
	char buf[200];
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd��
    ////        return 1;
    ////#endif
	fp = open_partition_file();
	if(fp==NULL)
	{
		return -ERROR_OPEN_FILE;
	}
	if(disk_name==NULL)
	{
		fclose(fp);
		return -NO_DISK;
	}
		
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		cp = strstr(buf, disk_name);
		if(cp!=NULL)
		num++;
		memset(buf, 0, sizeof(buf));
		cp = NULL;
	}
	fclose(fp);
	if(num>0)
		return num;
	return -NO_DISK;
}
/*
*****************************************************
*��������: find_disk_capacity
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش��̷���������(��λK)��0 û�д��̣���ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
static  long int find_disk_capacity(const char *disk_name)
{
	FILE* fp=NULL;
	char name[20];
	char buf[2][50];
	long int cap;
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd��
    ////        return 2*1000*1000; //2G��sd��
    ////#endif    
	fp = open_partition_file();
	if(fp==NULL)
	{
		return -ERROR_OPEN_FILE;
	}
	if(disk_name==NULL)
	{
		fclose(fp);
		return -NO_DISK;
	}
		
	memset(name, 0 ,sizeof(name));
	fgets(buf[0], sizeof(buf[0]), fp);//discard first line
	memset(buf,0,sizeof(buf));
	memcpy(name,disk_name, strlen(disk_name));
	while(feof(fp)==0)
	{
		fscanf(fp, "%s", buf[0]);
		if(strncmp(buf[0], name, strlen(name))==0)
		{
			cap = atol(buf[1]);
			fclose(fp);
			return (cap/1000)*1024;
		}
		memset(buf[1],0,sizeof(buf[1]));
		memcpy(buf[1], buf[0], strlen(buf[0]));
		memset(buf[0],0,sizeof(buf[0]));
	}
	fclose(fp);
	return -NO_DISK;
}
/*
*****************************************************
*��������: get_sys_disk_num
*��������: ��ȡ��������
*���룺
*�����
����ֵ: �ɹ����ش���������
*�޸���־��
*****************************************************
*/ 
int get_sys_disk_num(void)
{
	int num=0;
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd��
    ////       return 1;
    ////#endif    
    #if 0
	init_devinfo();
	if(get_ide_flag()==2)
	{
		//����SD�������	
		if(search_disk(SD_MASTER)>0)
			num++;
		if(search_disk(SD_SLAVE1)>0)
			num++;
	}    
	else
	{
	#endif
		//����Ӳ�̵����
		if(search_disk(MASTER_DISK)>0)
			num++;
		if(search_disk(SLAVE1_DISK)>0)
			num++;
		if(search_disk(SLAVE2_DISK)>0)
			num++;
		if(search_disk(SLAVE3_DISK)>0)
			num++;
	//}

//	if(num>0)
	return num;
//	return -NO_DISK;
}



/*
*****************************************************
*��������: get_sys_disk_capacity
*��������: ��ȡ��������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش�������(��λM)��0��ʾû�д��̣���ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
long int get_sys_disk_capacity(const char* disk_name)
{
	int ret=0;
	if(check_disk_name(disk_name))
	return -NO_DISK;
	ret = find_disk_capacity(disk_name);
	if(ret<0)
	return ret;
	return ret/1000;
}
/*
*****************************************************
*��������: get_sys_partition_num
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش��̷���������0 ��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
int get_sys_partition_num(const char*disk_name)
{
	int num=0;

    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd��
    ////        return 1;
    ////#endif    
   
	if(check_disk_name(disk_name))
		return -NO_DISK;  
	num = search_disk(disk_name);

	if(num<0)
	return num;

//	if(num ==1)
//	return -NO_PARTITIONS;

//	if(num==0)
//	return -NO_DISK;

	return num-1;
}
/*
*****************************************************
*��������: get_sys_partition_capacity
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������
*	    int partition_index ���̷���������
*�����
����ֵ: �ɹ����ش�������(��λM)��0��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
long int get_sys_partition_capacity(const char*disk_name, int partition_index)
{
	int ret=0;
	char buf[100];

	if(check_disk_name(disk_name))
		return -NO_DISK;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s%d",disk_name,partition_index);
	ret = find_disk_capacity(buf);
	if(ret<0)
	return ret;
	return ret/1000;
}



/*add-by-wsy 2007-11-21
*****************************************************
*��������: get_sys_disk_devname
*��������: ��ȡ���̽ڵ�����
*���룺diskno:���̱��(��0��ʼ)
*�����
����ֵ: �ɹ����ش��̽ڵ������ַ���,����"/dev/hda"��ʧ�ܷ���null
*****************************************************
*/ 
char *get_sys_disk_devname(int diskno)
{
#if 0
//// lsk 2009-8-9  sd check
	int type;
	init_devinfo();
	type=get_ide_flag();
	if(type==2)
	{
		switch(diskno)
		{
			case(0): return SD_DISK0;
			case(1): return SD_DISK1;
			default: return NULL;
		}
	}
#endif
	switch(diskno)
	{
		case(0):	return DEVDISK0;
		case(1):	return DEVDISK1;
		case(2):	return DEVDISK2;
		case(3):	return DEVDISK3;
		default:	return NULL;
	}	
}

/*****************************************************
*��������: get_sys_disk_name
*��������: ��ȡ��������
*���룺diskno:���̱��(��0��ʼ)
*�����
����ֵ: �ɹ����ش��������ַ���,����"hda"��ʧ�ܷ���null
*****************************************************
*/ 
char *get_sys_disk_name(int diskno)
{
//// lsk 2009-8-9  sd check
#if 0
	int type;

	init_devinfo();
	type=get_ide_flag();
	if(type==2)
	{
		switch(diskno)
		{
			case(0): return SD_MASTER;
			case(1): return SD_SLAVE1;
			default: return NULL;
		}
	}
#endif	
	switch(diskno)
	{
		case(0):	return MASTER_DISK;
		case(1):	return SLAVE1_DISK;
		case(2):	return SLAVE2_DISK;
		case(3):	return SLAVE3_DISK;
		default:	return NULL;
	}	
}


/*****************************************************
*��������: get_sys_disk_partition_num
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������,����"/dev/hda"
*�����
����ֵ: �ɹ����ش��̷���������0 ��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************/
int get_sys_disk_partition_num(char *disk_devname)
{
	int num=0;
	char disk_name[20];//����"hda",�Ա����get_sys_partition_num
	
	if(disk_devname == NULL)
		return -EINVAL;
	
	strncpy(disk_name,disk_devname+5,19);
	
	return get_sys_partition_num(disk_name);
}

/*************************************************************************
 * 	������:	get_sys_disk_partition_name()
 *	����:	��ȡָ����ŵķ����ڵ���
 *	����:	diskno,������ţ���0-3
 *			part_no,������ţ���1��ʼ
 *	���:	partitionname, ���÷����ڵ����Ƶ��ַ���ָ��
 * 	����ֵ:	���� "/dev/hda1"�ķ����ڵ����� ,���󷵻�NULL
 *************************************************************************/
char * get_sys_disk_partition_name(IN int diskno, IN int part_no,OUT char * partitionname)
{
	FILE* fp=NULL;
	char* cp=NULL;
	int num=0;
	int result = -NO_PARTITIONS;
	char buf[200];
	
	if(partitionname == NULL)
		return NULL;
#if 0
    #ifdef TEMP_SD
    	init_devinfo();
        if(get_ide_flag()==2)   //sd��
        {
            sprintf(partitionname,"/dev/sda1");
            return partitionname;
        };
    #endif    
#endif
	fp = open_partition_file();
	if(fp==NULL)
	{
		return NULL;
	}
	
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		cp = strstr(buf, get_sys_disk_name(diskno));
		if(cp!=NULL)
		{
			if(++num == part_no+1)
			{
				sprintf(partitionname,"/dev/%s",cp);
				cp = index(partitionname,'\n');
				if(cp!= NULL)
					*cp = '\0';
				fclose(fp);
				return partitionname;
			}
		}	
	}
	fclose(fp);
	return NULL;
	
}


/*
*****************************************************
*��������: get_disk_capacity
*��������: ֱ�Ӵ�/sys/block/sda/size�»�ȡ���̷�������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش��̷���������(��λG)��0 û�д��̣���ֵ��ʾʧ��
*�޸���־��yk add20130705
*****************************************************
*/ 
long long  get_disk_capacity(const char *disk_name)
{
	int fd;
	char tmp[200]={0};
	char buf[20]={0};
	unsigned long long int cap;
	sprintf(tmp,"/sys/block/%s/size",disk_name);
	fd=open(tmp, O_RDONLY);
	read(fd,buf,20);
	printf("read buf string:%s\n",buf);
	sscanf(buf, "%lld", &cap);
	printf("the cap is %d \n",cap);
	return cap*512/1000000000;	
}
