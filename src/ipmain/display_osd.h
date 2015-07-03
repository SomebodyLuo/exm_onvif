#ifndef DISPLAY_OSD_H
#define DISPLAY_OSD_H

#include "pthread.h"
#include "mod_socket.h"

#define OSD_NET_CH		0	//����ͨ��
#define OSD_LOCAL_CH 	1	//���ظ������洢




#define	MAX_DISPLAY_CHAR_D1		18	//D1ʱ�������ʾ���ַ���Ŀ
#define MAX_DISPLAY_CHAR_CIF	80	//cifʱ�������ʾ���ַ���Ŀ

#define OSD_POSX_MAX	29  //to be fixed~
#define OSD_POSY_MAX	17  //to be fixed

#define	INSTPLACE_DEF_POSX	1		//osd��ʾ��װ�ص�ʱ��ȱʡ���λ��,��ͬ
#define INSTPLACE_DEF_POSY	0		//to be fixed

#define FULL_CAMERANAME_POSX 58
#define FULL_CAMERANAME_POSY 17	
#define QUAD0_CAMERANAME_POSX 29
#define QUAD0_CAMERANAME_POSY 8 

#define NOQUAD_CAMERANAME_POSX	31
#define NOQUAD_CAMERANAME_POSY	17

struct osd_init_struct
{	
	int inst_place_display;		//�Ƿ���ʾ��װ�ص�osd,1Ϊ��ʾ������Ϊ����ʾ
	int inst_place_max_len;		//��װ�ص����������ַ���,���ַ�Ϊ��λ
	int inst_place_posx;		//��װ�ص�osd�����������,�����Ͻ�Ϊԭ�㣬ȡֵ0-44,ȱʡ1
	int inst_place_posy;		//��װ�ص�osd�����������,�����Ͻ�Ϊԭ�㣬ȡֵ0-17,ȱʡ1
};



void osd_second_proc(void);

//����Ӧͨ��(net/local)дosd��Ϣ,�����ж��Ƿ���Ҫд���Լ����ݺ�����
int write_osd_info(int osd_ch, int vidpicsize);

//��install:osd_max_len�������õ�ip1004.ini�ļ�
int osd_set_maxlen_to_file(void);


int creat_osd_proc_thread(pthread_attr_t *attr,void *arg);


#endif
