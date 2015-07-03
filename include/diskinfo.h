#ifndef DISK_INFO_H
#define DISK_INFO_H

#define MASTER_DISK	"sda"//yk change dha ->sda
#define SLAVE1_DISK	"sdb"
#define SLAVE2_DISK	"sdc"
#define SLAVE3_DISK	"sdd"

//addby wsy
#define DEVDISK0	"/dev/sda"
#define DEVDISK1	"/dev/sdb"
#define DEVDISK2	"/dev/sdc"
#define DEVDISK3	"/dev/sdd"

//add by zw
#define SD_MASTER	("sda")
#define SD_SLAVE1	("sdb")

//add by lsk
#define SD_DISK0	("/dev/sda")
#define SD_DISK1	("/dev/sdb")

#ifndef IN
#define IN
#define IO
#define OUT
#endif

/*
*****************************************************
*��������: get_sys_disk_num
*��������: ��ȡ��������
*���룺
*�����
����ֵ: �ɹ����ش��̸���
*�޸���־��
*****************************************************
*/ 
int get_sys_disk_num(void);
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
long int get_sys_disk_capacity(const char* disk_name);
/*
*****************************************************
*��������: get_sys_partition_num
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������
*�����
����ֵ: �ɹ����ش���������0��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
int get_sys_partition_num(const char*disk_name);




/*****************************************************
*��������: get_sys_disk_name
*��������: ��ȡ��������
*���룺diskno:���̱��(��0��ʼ)
*�����
����ֵ: �ɹ����ش��������ַ���,����"hda"��ʧ�ܷ���null
*****************************************************
*/ 
char *get_sys_disk_name(int diskno);


/*
*****************************************************
*��������: get_sys_partition_capacity
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������
*	    int partition_index ���̷���������
*�����
*����ֵ: �ɹ����ش�������(��λM)��0��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************
*/ 
long int get_sys_partition_capacity(const char*disk_name, int partition_index);




/*add-by-wsy 2007-11-20
*****************************************************
*��������: get_sys_disk_devname
*��������: ��ȡ���̽ڵ�����
*���룺diskno:���̱��(��0��ʼ)
*�����
����ֵ: �ɹ����ش��̽ڵ������ַ���������"/dev/hda",ʧ�ܷ���null
*****************************************************
*/ 
char *get_sys_disk_devname(int diskno);


/*****************************************************
*��������: get_sys_disk_partition_num
*��������: ��ȡ���̷�������
*���룺const char* disk_name ��������,����"/dev/hda"
*�����
����ֵ: �ɹ����ش��̷���������0 ��ʾû�з�������ֵ��ʾʧ��
*�޸���־��
*****************************************************/
int get_sys_disk_partition_num(char *disk_devname);


/*************************************************************************
 * 	������:	get_sys_disk_partition_name()
 *	����:	��ȡָ����ŵķ����ڵ���
 *	����:	diskno,������ţ���0-3
 *			part_no,������ţ���1��ʼ
 *	���:	partitionname, ���÷����ڵ����Ƶ��ַ���ָ��
 * 	����ֵ:	���� "/dev/hda1"�ķ����ڵ����� ,���󷵻�NULL
 *************************************************************************/
char * get_sys_disk_partition_name(IN int diskno, IN int part_no,OUT char * partitionname);

#endif
