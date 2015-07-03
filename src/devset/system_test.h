/*
**************************************************************
�ļ����ƣ�system_para_set.h
��д�ߣ� lsk
��д���ڣ�2006-9-12
��Ҫ�����������ļ����ƣ�system_para_set.c��ϵͳ�������õĺ���
�޸ļ�¼��
�޸���־��
***************************************************************
*/
#ifndef _SYSTEM_TEST_H_
#define _SYSTEM_TEST_H_

#include "communication.h"
typedef struct 
{
	int value;			// ���Թ�������
	unsigned char name[100];// ���Թ��������
}test_info;
#define VERSION	"0.02"

/*
  v0.01.������һ���汾
*/

#define TEST_STAGE 	5		//Ĭ��5�����Թ���
//// lsk add 2008-1-17 
#define TEST_GTVM_PR	"testgtvm"
#define TEST_BD_PR		"testbd"
#define TEST_TG_PR		"trigtest"


#define PROG_TEST_GTVM	"/ip1004/testgtvm"
#define PROG_TEST_BD	"/ip1004/testbd"
#define PROG_TEST_TG	"/ip1004/trigtest"
#define TEST_BD_RESULT	"/tmp/testbd.txt"
#define TEST_TG_RESULT	"/tmp/testtrig.txt"

#define TRIG_NODE 		"trigtest"
#define BOARD_NODE 		"boardtest"
#define FMAT_NODE		"format"
#define CLR_NODE		"clear"
#define REPORT			"report"
//path of temp file that record infotmation about clearing and formating 
#define PROG_FMAT	"/ip1004/initdisk"
#define FMAT_INFO 	"/tmp/init_info.txt"	
#define CLR_INFO 	"/tmp/clr_info.txt"	
//// path of test infomation record files 
#define TEST_LOG_PATH		"/log/testinfo"
#define TEST_LOG_FILE		"/log/testinfo/testinfo.txt"
#define TEST_STATE_FILE		"/log/testinfo/teststate.txt"

int update_system_nfs(int fd , char* path);
int update_system_ftp(char * username, char * pswd, char *ftpip, int port, int size, char *path);
void test_bd(multicast_sock *ns);
void test_trig(multicast_sock *ns);
void format_dev_disk(multicast_sock *ns );
int result_report(int index, multicast_sock *ns);
// lsk 2007 -2-7
int set_test_state_record(unsigned char* name, int value, int index);
test_info* get_test_state_record( int index);
int clear_test_state_record(void);
//int test_log_info(unsigned char *format, unsigned char* info);
int test_log_info(char *format, ...);

int create_test_state_file(void);
void restart_soft(void);
void restart_hard(void);

#endif
