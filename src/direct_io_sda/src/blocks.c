/****************************************************************************
 * ���ܣ�year_blocks.c date_blocks.c����صĹ��ܺ��������������ʱ���
 *
 ****************************************************************************/

#include "blocks.h"
/*ȫ�ֱ���*/
media_source_t media;


static long long max_blocks=MAXBLOCKS;
/*Ӳ������־λ��Ӳ�����󣬾�Ҫѭ��д�룻�����Ƿ�������ʽ��ͬ*/
BOOL hd_full_mark = FALSE;

inline long long get_hd_max_blocks()
{
	return max_blocks;
}

/*����Ӳ������־λΪ��*/
inline void hd_full_mark_set(BOOL mark)
{
	hd_full_mark = mark;
}
/*��ȡӲ������־λ*/
inline BOOL hd_full_mark_get()
{
	return hd_full_mark;
}

/*******************************ʱ�亯��**********************/
/*��ȡʱ�䣬ʱ����Ǵ�1970�굽���ڵ�����*/
int get_my_time()
{
	return (int) time(NULL);
}
#if 0
struct tm *gmtime(const time_t *calptr)
struct tm
{
	int tm_sec; /* Seconds.	[0-60] (1 leap second) */
	int tm_min; /* Minutes.	[0-59] */
	int tm_hour; /* Hours.	[0-23] */
	int tm_mday; /* Day.		[1-31] */
	int tm_mon; /* Month.	[0-11] */
	int tm_year; /* Year	- 1900.  */
	int tm_wday; /* Day of week.	[0-6] */
	int tm_yday; /* Days in year.[0-365]	*/
	int tm_isdst; /* DST.		[-1/0/1]*/
}

time_t mktime (struct tm *__tp)
#endif

/*******************************ʱ�亯��**********************/

/***************debug***************************************************/
/************************************************************
 * ��ʽ���������������
 ************************************************************/
int myprint(unsigned char *p, long size)
{
	unsigned int i;
	for (i = 0; i < size; i++)
	{
		if (i % 10 == 0)
			printf("\n%-8d", i);
		printf("    %-2X", *(p + i));
	}
	printf("\n");
	return 0;
}

/********fifo***************************************************************/
static int fd;
/*��ʼ���ܵ�*/
int fifo_init()
{
	mkfifo("test.264", 0777);
	int ret;
	fd = open("test.264", O_WRONLY);
	if (fd < 0)
	{
		perror("openfile error");
		return -1;
	}
	return 0;
}
/*д�ܵ�*/
int fifo_write(char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		//perror("write error\n");
		return -1;
	}
	return ret;
}
/*�ͷŹܵ�*/
void fifo_free()
{
	close(fd);
}
/********fifo***************************************************************/
