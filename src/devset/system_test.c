#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <file_def.h>
#include <gtthread.h>
#include <pthread.h>

#include <string.h>
#include <stdarg.h>
//#include <stdafx.h>

#include <iniparser.h>
#include <update.h>
#include <gt_dev_api.h>
#include <devinfo.h>
#include <diskinfo.h>

#include "communication.h"
#include "cmd_process.h"
#include "system_test.h"
#include "system_para_set.h"
#include "sys_error.h"
#include <guid.h>

//#include <gt_errlist.h>

test_info test_st[TEST_STAGE];		//ÿһ�����Թ����״̬����
/*
*****************************************************
*��������: set_test_state_record
*��������: ���ò���״̬��¼��Ϣ
*���룺int index 	//���Թ��������
	    unsigned char* name	//���Թ��������
	    int value  	//���Թ����״̬����
*�������
����ֵ��0��ʾ�ɹ�����ֵ��ʾʧ�ܡ�
*�޸���־��
*****************************************************
*/ 
int set_test_state_record(unsigned char* name, int value, int index)
{
	int ret;
	dictionary	*	ini =NULL;
	FILE *s=NULL;
	char buf[BUF_LEN*2];

	ini = iniparser_load(TEST_STATE_FILE);
	if (ini==NULL) 
	{
		fprintf(stderr, "cannot parse file [%s]", TEST_STATE_FILE);
		return -1 ;
	}		
	else
	{
		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%s:%s%d", M_TEST_STATE,TEST_STATE_NAME,index+1);
		ret = iniparser_setstr(ini, buf, name);
//		printf("%s\n",buf);
		if(ret<0)
		{
			fprintf(stderr, "cannot parse file [%s]", TEST_STATE_FILE);
		}
		memset(test_st[index].name, 0 , sizeof(test_st[index].name));
		sprintf(test_st[index].name, "%s", name);

		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%s:%s%d", M_TEST_STATE,TEST_STATE_VALUE,index+1);
//		printf("%s\n",buf);
		ret = iniparser_setint(ini, buf, value);
		if(ret<0)
		{
			fprintf(stderr, "cannot parse file [%s]", TEST_STATE_FILE);
		}
		test_st[index].value = value;
	}
	s=fopen(TEST_STATE_FILE, "w+");
	if(s!=NULL)
	{
		iniparser_dump_ini(ini, s);
		iniparser_freedict(ini);
		fclose(s);
	}
	else
	{
		iniparser_freedict(ini);
		return -1;
	}
	return 0;
}
/*
*****************************************************
*��������: get_test_state_record
*��������: ���ز���״̬��¼��Ϣ
*���룺int index ���Թ��������
*�������
����ֵ������״̬���ݽṹָ��
*�޸���־��
*****************************************************
*/ 
test_info* get_test_state_record( int index)
{
	return &test_st[index];
}
/*
*****************************************************
*��������: read_test_state_record
*��������: ���ļ��ж�ȡ����״̬��¼��Ϣ
*���룺int index //���Թ��������
*�������
����ֵ������״̬���ݽṹָ��
*�޸���־��
*****************************************************
*/ 
test_info* read_test_state_record( int index)
{
	dictionary	*	ini =NULL;
	char *s=NULL;
	char buf[BUF_LEN*2];

	if(index>=TEST_STAGE)
	{
		index = TEST_STAGE-1;
	}
	if(index<0)
	{
		index = 0;
	}
	
	ini = iniparser_load(TEST_STATE_FILE);
	if (ini==NULL) 
	{
		fprintf(stderr, "cannot parse file [%s]", TEST_STATE_FILE);
		return NULL;
	}		
	else
	{
		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%s:%s%d", M_TEST_STATE,TEST_STATE_NAME,index+1);
		s = iniparser_getstr(ini, buf);
		if(s!=NULL)
		{
			memset(test_st[index].name, 0 , sizeof(test_st[index].name));
			sprintf(test_st[index].name, "%s", s);
		}
		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%s:%s%d", M_TEST_STATE,TEST_STATE_VALUE,index+1);
		test_st[index].value = iniparser_getint(ini, buf, 0);
	}
	iniparser_freedict(ini);
	return &test_st[index];
}
/*
*****************************************************
*��������: clear_test_state_record
*��������: ������Լ�¼��Ϣ
*���룺��
*�������
����ֵ��0��ʾ�ɹ�����ֵ��ʾʧ�ܡ�
*�޸���־��
*****************************************************
*/ 
int clear_test_state_record(void)
{
	int i;
	for(i=0;i<TEST_STAGE;i++)
		set_test_state_record("no_name", 0, i);
	return 0;
}
/*
*****************************************************
*��������: open_test_log_file
*��������: ��ȡ������־�ļ����
*���룺��
*�������
*����ֵ��������־�ļ�ָ��
*�޸���־��
*****************************************************
*/ 
FILE* open_test_log_file(unsigned char* path)
{
	FILE* fp=NULL;
	unsigned char*cp;
	char sbuf[120];
	unsigned char buf[120];
//	create_and_lockfile(path);
	fp= fopen(path, "a+");
	if(fp==NULL)
	{
		memset(buf , 0, sizeof(buf));
		memset(sbuf , 0, sizeof(sbuf));

		strncpy(buf,path,strlen(path));
		cp=strrchr(buf,'/');

		if(cp!=NULL)
		{
			*cp='\0';
			sprintf(sbuf,"mkdir %s -p\n", buf);
			system(sbuf);
		}
		fp= fopen(path, "a+");
	}
	return fp;	
}
/*
*****************************************************
*��������: test_log_info
*��������: д������־��Ϣ
*���룺
		unsigned char *format     ��Ϣ��ʽ
		...	��ʾ��Ϣ����
*�������
����ֵ��0��ʾ�ɹ�����ֵ��ʾʧ�ܡ�
*�޸���־��
*****************************************************
*/ 
//int test_log_info(unsigned char *format, unsigned char* info)
int test_log_info(char *format, ...)
{
	FILE* fp=NULL;
	time_t time_cur;
	struct tm *tp;
	va_list paramList;
	char  data[1024];
//	memset(paramList, 0 , sizeof(paramList));
	fp = open_test_log_file(TEST_LOG_FILE);//�򿪲�����־�ļ�
	if(fp==NULL)
	{
		return -1;
	}
	else
	{
		time_cur = time(NULL);
//		time(&time_cur);
//		tp = gmtime(&time_cur);
		tp = localtime(&time_cur);
		fprintf(fp, "<%04d-%02d-%02d ", (1900+tp->tm_year),(1+tp->tm_mon), tp->tm_mday);
		fprintf(fp, "%02d:%02d:%02d> ", tp->tm_hour, tp->tm_min, tp->tm_sec);

		va_start(paramList, format);
		memset(data, 0 ,sizeof(data));
		vsprintf(data, format, paramList);
		va_end(paramList);
//		fprintf(fp, "devset[version%s]:", VERSION);
//		printf(format , data);
		fprintf(fp, data);
		fprintf(fp,"\n");
		fflush(fp);
	}
	fclose(fp);
	return 0;
}

/*
*****************************************************
*��������: create_test_state_file
*��������: ��������״̬��¼�ļ�
*���룺��
*		
*�������
����ֵ��0��ʾ�ɹ�����ֵ��ʾʧ�ܡ�
*�޸���־��
*****************************************************
*/ 
int create_test_state_file(void)
{
	int i;
	FILE* fp=NULL;
	char buf[BUF_LEN*2];
	memset(&test_st, 0 ,sizeof(&test_st));
	fp= fopen(TEST_STATE_FILE, "r");
	if(fp==NULL)	//û���ļ�������һ��
	{
		memset(buf , 0, sizeof(buf));
		sprintf(buf, "%s%s","mkdir ", TEST_LOG_PATH);
		system(buf);
		memset(buf , 0, sizeof(buf));
		fp= fopen(TEST_STATE_FILE, "w+");
		if(fp!=NULL)	// ���ļ��� д��һЩĬ�ϵ���Ϣ
		{
			memset(buf , 0, sizeof(buf));
			sprintf(buf, "\n[%s]\n", M_TEST_STATE);	// д��ini�ļ��ڵ�����
			fprintf(fp,"%s",buf);
			for(i=0;i<TEST_STAGE;i++)		//5�����Բ���
			{
				memset(buf , 0, sizeof(buf));
				memset(test_st[i].name, 0 , sizeof(test_st[i].name));
				sprintf(test_st[i].name, "%s", "no_name");
				sprintf(buf, "\n%s%d = %s\n",TEST_STATE_NAME,i+1,"no_name"); //���Բ����״̬����
				fprintf(fp,"%s",buf);
				
				memset(buf , 0, sizeof(buf));
				test_st[i].value = 0;
				sprintf(buf, "\n%s%d = 0\n",TEST_STATE_VALUE,i+1); //���Բ����״̬����
				fprintf(fp,"%s",buf);
			}
			fflush(fp);
			fclose(fp);
		}
		else
		{
			return -1;
		}
	}
	else		//�ļ��Ѿ����ڣ�����ļ� ��ȡ����
	{
		for(i=0;i<TEST_STAGE;i++)
		read_test_state_record(i);	//��������ȡ���ŵ� test_st��
	}
	return 0;	
}

int  update_system_nfs(int fd , char* path)
{
	return 0;
}
/*
********************************************
�������ƣ�update_system_ftp
���ܣ�����FTP Э��Ͷ˿ڶ��豸�����������
���룺
Username:	���ڵ�¼Զ�̷��������û���
Pswd:		����
Ftpip:		Զ�̷�����ip��ַ
Port:		Զ�̷�����ftp�˿�
Path:		��ftp�ϵ�������·��(��"/"��ͷ)
size: 		�����ļ��Ĵ�С
��� :
����ֵ��0��ʾ�ɹ�����ֵ��ʾʧ�ܡ�
******************************************************
*/
int update_system_ftp(char * username, char * pswd, char *ftpip, int port, int size, char *path)
{
	char up_buf[500];
	int ret= -1;
	memset (up_buf , 0 , sizeof(up_buf));
	//�������������ַ���
	generate_updatemsg(username,pswd, ftpip, port, path , up_buf);
	//��ʼ����
	printf("%s\n", up_buf);
	ret = update_software(size , up_buf, 300/*5����*/);
	return ret;
}
/*
*****************************************************
*��������: test_bd
*��������: �����豸�忨�߳�
*���룺 multicast_sock *ns ����������ݽṹ
*		
*�����
*�޸���־��
*****************************************************
*/ 
void test_bd(multicast_sock *ns)
{
	int flag=0;
	multicast_sock net_st;
	struct GT_GUID targ_ID;
	unsigned char targ_hex_ID[20];
	unsigned char buf[400];
	
	memset(buf, 0, sizeof (buf));
	memcpy(&net_st,ns,sizeof(net_st));
	memcpy((unsigned char*)&targ_ID,net_st.target_id,sizeof( targ_ID));
	guid2hex(targ_ID, targ_hex_ID);
//	printf("%02x%02x%02x%02x, %s\n", net_st.target_id[0],net_st.target_id[1],net_st.target_id[2],net_st.target_id[3],targ_hex_ID);
//	printf("%02x%02x%02x%02x, %s\n", net_st.target_id[4],net_st.target_id[5],net_st.target_id[6],net_st.target_id[7],targ_hex_ID);
	gtloginfo("start board test thread\n");
	test_log_info("start board test thread\n");
	printf("start board test thread\n");
	
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	set_sys_flag(TEST_BD_FLAG, 1);	//��־��λ
//	sprintf(buf, "%s %s \n", PROG_TEST_BD , TEST_BD_RESULT);
	printf("get_dev_family = %d \n", get_dev_family());
	if(get_dev_family()==GTDEV_FAMILY_GTVM)
	{
		sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
		PROG_TEST_GTVM, net_st.hostname, net_st.multi_port, net_st.flag, 
		net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_BD_RESULT);
	}
	else
	{
#if 0			//// lsk 2009-2-18 changed by lsk 
		if(virdev_get_virdev_number()==2)
		{
			if(net_st.recv_id[GUID_LEN-1]==net_st.self_id1[GUID_LEN-1])
			{
				send_test_report(&net_st, "�忨���Խ���", 100);
				result_rpt(0, "�忨���Գɹ�",&net_st);
				set_sys_flag(TEST_BD_FLAG, 0);	//�����־
				pthread_exit(NULL);
			}
			sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
			PROG_TEST_BD, net_st.hostname, net_st.multi_port, net_st.flag, 
			net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_BD_RESULT);
#if 0
			channel=1;
			sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -C %d -r %s\n",
			PROG_TEST_BD, net_st.hostname, net_st.multi_port, net_st.flag, 
			net_st.enc_type, net_st.seq_num, targ_hex_ID, channel, TEST_BD_RESULT);
#endif
		}
#else
		/////// lsk 2009-5-22 ���ݲ���Ӳ�̵ı�־����������
		flag = get_sys_flag(TEST_IDE_FLAG);
		sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
		PROG_TEST_BD, net_st.hostname, net_st.multi_port, net_st.flag, 
		net_st.enc_type,  net_st.seq_num, targ_hex_ID,TEST_BD_RESULT);
		//////////// end of change 

		
//		sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
//		PROG_TEST_BD, net_st.hostname, net_st.multi_port, net_st.flag, 
//		net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_BD_RESULT);
		
#endif
	}
//	printf("%s",buf);
	system(buf);
	set_sys_flag(TEST_BD_FLAG, 0);	//�����־
//	result_report(TEST_BD_FLAG,ns);	//���Ͳ��Խ��
	gtloginfo("board test thread finished\n");
	test_log_info("board test thread finished\n");
	pthread_exit(NULL);
}
/*
*****************************************************
*��������: test_trig
*��������: �����豸�����߳�
*���룺 multicast_sock *ns ����������ݽṹ
*		
*�����
*�޸���־��
*****************************************************
*/ 
void test_trig(multicast_sock *ns)
{
	multicast_sock net_st;
	struct GT_GUID targ_ID;
	unsigned char targ_hex_ID[20];
	unsigned char buf[400];
	
	memset(buf, 0, sizeof (buf));
	memcpy(&net_st,ns,sizeof(net_st));
	memcpy((unsigned char*)&targ_ID,net_st.target_id,sizeof( targ_ID));
	guid2hex(targ_ID, targ_hex_ID);
	gtloginfo("start triger test thread\n");
	test_log_info("start triger test thread\n");
	
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	set_sys_flag(TEST_TG_FLAG, 1);	//��־��λ
	if(get_dev_family()==GTDEV_FAMILY_GTVM)
	{
		sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
		PROG_TEST_GTVM, net_st.hostname, net_st.multi_port, net_st.flag, 
		net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_TG_RESULT);
	}
	else
	{
#if 0
		if(virdev_get_virdev_number()==2)
		{
			if(net_st.recv_id[GUID_LEN-1]==net_st.self_id1[GUID_LEN-1])
			{
				send_test_report(&net_st,"���Ӳ��Խ���" ,100);
				result_rpt(0, "���Ӻʹ��ڲ��Գɹ�", &net_st);
				set_sys_flag(TEST_TG_FLAG, 0);	//�����־
				pthread_exit(NULL);
			}
			sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s  -r %s\n",
			PROG_TEST_TG, net_st.hostname, net_st.multi_port, net_st.flag, 
			net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_TG_RESULT);
		}
#else
		sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
		PROG_TEST_TG, net_st.hostname, net_st.multi_port, net_st.flag, 
		net_st.enc_type, net_st.seq_num, targ_hex_ID, TEST_TG_RESULT);
#endif
	}
//	printf("%s",buf);
//	sprintf(buf, "%s %s \n", PROG_TEST_TG , TEST_TG_RESULT);
	//zw-add
	system("/ip1004/inittrigin 0");	//��������ӳ�ʼ��Ϊ����zw-add-20100416
	system(buf);
	//lc change for ip1004xm,always 0
	system("/ip1004/inittrigin 0");	//��������ӳ�ʼ��Ϊ����zw-add-20100416
	
	set_sys_flag(TEST_TG_FLAG, 0);	//�����־
//	result_report(TEST_TG_FLAG, ns);	//���Ͳ��Խ��
	gtloginfo("triger test thread finished\n");
	test_log_info("triger test thread finished\n");
	pthread_exit(NULL);
}

/*
*****************************************************
*��������: format_dev_disk
*��������: ��ʽ�������߳�
*���룺 multicast_sock *ns ����������ݽṹ
*		
*�����
*�޸���־��
*****************************************************
*/
void format_dev_disk(multicast_sock *ns )
{
	multicast_sock net_st;
	struct GT_GUID targ_ID;
	unsigned char targ_hex_ID[20];
	unsigned char buf[400];
	
	memset(buf, 0, sizeof (buf));
	memcpy(&net_st,ns,sizeof(net_st));
	memcpy((unsigned char*)&targ_ID,net_st.target_id,sizeof( targ_ID));
	guid2hex(targ_ID, targ_hex_ID);

	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	set_sys_flag(FORMAT_FLAG, 1);//��־��λ

	sprintf(buf, "%s  -I %s -P %d -F %d -E %d -S %s -D %s -r %s\n",
			PROG_FMAT, net_st.hostname,	net_st.multi_port, net_st.flag, 
			net_st.enc_type, net_st.seq_num, targ_hex_ID, FMAT_INFO);
	
//	printf("%s",buf);
	system(buf);
	set_sys_flag(FORMAT_FLAG, 0);	//�����־
	pthread_exit(NULL);
}

/*
*****************************************************
*��������: result_report
*��������: ���Խ���㱨
*���룺 multicast_sock *ns ����������ݽṹ
*			int index	�㱨����Ϣ����
*�����
*����ֵ: 0��ȷ��ֵ��ʾ����
*�޸���־��
*****************************************************
*/ 
int result_report(int index, multicast_sock *ns)
{
	int len = 0;
	int ret = -1;
	dictionary	*	ini = NULL;
	char inibuf[100];
	char *s=NULL;
	unsigned char buf[1024];

	memset(buf, 0 , sizeof(buf));
	memset(inibuf, 0 , sizeof(inibuf));

	if(ns==NULL)
	{
		return -1;
	}
	
	switch(index)
	{			
		case TEST_TG_FLAG:
		ini = iniparser_load(TEST_TG_RESULT);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", TEST_TG_RESULT);
			return -1;
		}
		sprintf(inibuf, "%s:%s", TRIG_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s", sep_mark, CMD_STR, eq_mark, M_TEST_DEV_RETURN,
			sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", TRIG_NODE, REPORT,TEST_TG_RESULT);
			iniparser_freedict(ini);
			return -1;
		}
			break;
			
		case TEST_BD_FLAG:
		ini = iniparser_load(TEST_BD_RESULT);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", TEST_BD_RESULT);
			return -1;
		}
		sprintf(inibuf, "%s:%s", BOARD_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s", sep_mark, CMD_STR, eq_mark, M_TEST_DEV_RETURN,
			sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", BOARD_NODE, REPORT,TEST_BD_RESULT);
			iniparser_freedict(ini);
			return -1;
		}
			break;
			
		case FORMAT_FLAG:
		ini = iniparser_load(FMAT_INFO);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", FMAT_INFO);
			return -1;
		}
		sprintf(inibuf, "%s:%s", FMAT_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s", sep_mark, CMD_STR, eq_mark, M_TEST_DEV_RETURN,
			sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", FMAT_NODE, REPORT,FMAT_INFO);
			iniparser_freedict(ini);
			return -1;
		}

			break;
		case CLEAN_FLAG:
			sprintf(buf, "%s%s%s%s%s%s%s%d:%s", sep_mark, CMD_STR, eq_mark, M_CLEAR_DEV_RETURN,
			sep_mark, RESULT_STR ,eq_mark, CLR_OK, clear_OK);
			break;
			
		default:
			printf("can not report result : get error index \n");
			return -1;
			break;
	}

	len = strlen(buf);
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	return ret;
}

#if 1
//soft restart system
void restart_soft(void)
{
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	system("/ip1004/swrbt\n");
}
//hard restart system
void restart_hard(void)
{
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	system("/ip1004/hwrbt\n");
}
#endif


