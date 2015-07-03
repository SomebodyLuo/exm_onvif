#ifndef FIX_DISK_H
#define FIX_DISK_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>

#define FIXDISK_INTERVAL   4*60*60  //��λΪ��

#ifndef IN
#define IN
#define OUT
#define IO
#endif

/*
 * ������	:fix_disk()
 * ����	:���errnoӦ��������̣�������ط����������¼�������ѡ���������
 * ����	:path:����"/hqdata/hda2/xxx"���ַ�����ֻҪǰ׺��"/hqdata/hda2"�Ϳ���
 *	 	 errno:�����룬��ֵ
 * ����ֵ:��
*/
void fix_disk( char *path,int diskerrno);

/*
 * ������	:is_disk_error()
 * ����	:�жϴ�������errno�Ƿ���Ӧ��������̵�����
 * ����	:error:������
 * ����ֵ	:1��ʾӦ������̣�0��ʾ�����������
*/
int is_disk_error(int error);

/*wsyadd, ���ڴ������ftw_sort����ʱ����Ҫ������̵����*/
int  fix_disk_ftw(const char *dirname, int err);

#endif
