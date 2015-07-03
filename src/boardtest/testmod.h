#ifndef TESTMOD_H
#define TESTMOD_H
#include "testIDE.h" 
/*
#include "testdsp.h"
#include "test6410.h"
#include "test2834.h"
*/
#include "multicast_ctl.h"

#define NETENC 1
#define HQENC   0
#define IDE_DEV 		"/dev/hda"

struct dev_test_struct
{
        int ide_stat;		//Ӳ�̻�cf�����Խ��
        int netenc_stat;	//������Ƶ���������Խ��
        int hqenc0_stat;	//������¼��0·оƬ���Խ��
        int audio_stat;		//����оƬ���Խ��
        int quad_stat;		//����ָ�оƬ���Խ��
        int tw9903_stat;
        int rtc_stat;
        int usb_stat;
};
void print_stat(int err, int stat, FILE * fp);
void print_code(int err, int stat, FILE * fp);
int save_result_to_file(char *filename,struct dev_test_struct *devstat,multicast_sock* ns);
int testsim(struct dev_test_struct *stat, multicast_sock* net_st, int prog);
/*
int test_IDE(void);
int test_Quad(void);
int test_6410(int channal);
int test_dsp(void);
*/
#endif
