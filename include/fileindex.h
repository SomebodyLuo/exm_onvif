/*
		����¼���ļ���������غ���  --wsy Dec 2007
*/

#ifndef FILEINDEX_H
#define FILEINDEX_H


#include "sys/stat.h"
#include "unistd.h"
#include <sys/types.h>
#include <fcntl.h>
#include "stdio.h"
#ifndef IN
#define IN
#define	OUT
#define IO
#endif

#define 	FILEINDEX_VERSION		("0.01")
//ver:0.01 2010-06-17	zw		�޸�open_dbʱ��ʹ��cpǰ���ж�Ŀ���ļ�


struct query_index_process_struct{
FILE 	*index_fp;
time_t	start;
time_t	stop;
int 	ch;
int 	trig_flag;

};




int fileindex_init_filelock();


//��ʼ��������������ֵ������
int fileindex_init_db(IN char * partition);


/*************************************************************************
 * 	������:	fileindex_add_to_partition()
 *	����:	��һ���ļ������������ڵķ����������ļ���ĩβ
 *	����:	mountpath,���ڷ���������/hqdata/hda1
 *			filename,¼���ļ���
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_add_to_partition(IN char* mountpath, IN char *filename);

/*************************************************************************
 * 	������:	fileindex_add_to_partition_upidx()
 *	����:	��һ���ļ������������ڵķ����������ļ���ĩβ
 *	����:	mountpath,���ڷ���������/hqdata/hda1
 *			filename,¼���ļ���
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_add_to_partition_upidx(IN char* mountpath, IN char *filename);



/*************************************************************************
 * 	������:	fileindex_del_oldest()
 *	����:	�ӷ���������ɾȥ���ϵ�δ������¼��ɾ����Ӧ�ļ�
 *	����:	mountpath,�������ƣ�����/hqdata/hda1
 *			no,��Ҫɾ�����ļ���Ŀ
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_del_oldest(IN char* mountpath, int no);


/*************************************************************************
 * 	������:	fileindex_rename_in_partition()
 *	����:	�����������ļ���ָ���ļ����ĳɸ������ļ���
 *	����:	mountpath,��������
 *			oldname,������
 			newname,������
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_rename_in_partition(IN char*mountpath, IN char *oldname, IN char* newname);

int fileindex_lock_by_time(IN char* mountpath, IN int flag,IN int  starttime, IN int stoptime, IN int  trig, IN int ch);



/*************************************************************************
 * 	������:	fileindex_create_index()
 *	����:	Ϊ���������µ�¼���ļ���������
 *	����:	path��������������,��"/hqdata/hda2"
 *			forced: 0��ʾ��ǰû������ʱ�Ŵ���,1��ʾ������ζ����´�����1
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_create_index(IN  char *path, IN int forced);




/*************************************************************************
 * 	������:	fileindex_get_oldest_file_time()
 *	����:	��ȡ���������ϵĿ�ɾ���ļ��Ĵ���ʱ��
 *	����:	mountpath��������������,��"/hqdata/hda2"
 *	���:	
 * 	����ֵ:	�ɹ����ش���ʱ��,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_get_oldest_file_time(char *mountpath);




int fileindex_query_index(char *mountpath, struct query_index_process_struct *qindex);


int fileindex_convert_ing(char *mountpath);

int InitAllDabase();
int CloseAllDabase();
#endif
