#ifndef MP_HDMODULE_H
#define MP_HDMODULE_H

#include "gtlog.h"

#define  VERSION		"0.09"
/*
0.09   1. ֱ��ʹ��Ӳ���ϵ����ݿ⣬ֻ�ڽ�������ʱ�������ݿ�
         2. ������Ƶ
0.08 ��Ӳ�����ڴ�����ݿ��ļ�������������ڷ����ĸ��̵Ŀռ䶼����ʱֹͣ¼���߳�
0.07 ¼���ļ�����ʹ��/conf/diskinfo.ini
0.06 ������¼���̼���־
0.05 �ڿռ�С��250Mʱ����
0.04 �����˶�д���ݻ������Ľӿڣ����ڱ���¼����Щ�ӿ�ʹ����ǰ�Ļ�ȡ����Ƶ���ݵķ�����
0.03 ������������,���óɴ���¼�������¼���̣߳�3024��Ҳ�������⣬�޸ģ�
        �����˱����ļ������ļ������Ĺ��ܡ�
0.02 �����ڴ�
0.01 2013-07-08 ����

*/

//���Զ���

#define SHOW_WORK_INFO

#if EMBEDED==0
	#define FOR_PC_MUTI_TEST		//֧����ͬһ̨pc���������������
#endif

//#define     USE_FFMPEG_LIB                  ///ʹ��ffmpeg������Ƶ����FIXME:��ʱ��ʾ�� ,���Ҫ��ʽʹ�ã��ⲿ��������Ҫ�������





//#define	D1_MAX_FRAME_SIZE	0x20000 //I���������size
#ifndef FOR_PC_MUTI_TEST
	#define  HDENC0_KEY_NUMBER 	123 //���ڴ�����Ϣ���е�KEYֵ
#else
	extern int HDENC0_KEY_NUMBER; 	 //���ڴ�����Ϣ���е�KEYֵ
#endif
#define HQMODULE_USE
#if EMBEDED
#define PATH_TYPE 0 //���أ�Ϊ1���ڲ�ѯ�����.txt�ļ�����ʾ/hqdata/....     
#else
#define PATH_TYPE 1 //���أ�Ϊ1���ڲ�ѯ�����.txt�ļ�����ʾ/hqdata/....     
#endif

//#define RECORD_PS_FILE	//���أ����ѡ�У���ͬʱ¼ps����mpg�ļ�

#define GET_PIC_SPACE     5 //ץͼ������С���̿ռ䣬��MΪ��λ


#ifndef FOR_PC_MUTI_TEST
#define POOLSIZE		  15   //ץͼ����ش�Сchanged by shixin from 60
#else
#define POOLSIZE		  5
#endif
//zw-del 2011-06-15 #define TAKE_PIC_MAX      20    //һ���������ץͼ������

#define TAKE_PIC_MAX        (70)

#define MOTION_PRE_REC	10	//�ƶ��ʹ���¼��Ԥ¼����
#define MOTION_DLY_REC	10	//�ƶ��ʹ���¼���ӳ�����


#include <file_def.h>
#include <mod_com.h>

int get_gtthread_attr(pthread_attr_t *attr);
#endif



