#ifndef __NET_MOD_H
#define __NET_MOD_H

#include"testmod.h"


#define ACK_PKT	"ack"
#define SEQ_ACK	"seq_ack"
#define RET_VAL	"ret"
#define ERR_STR	"err"

#define RPT_PKT		"process_report"
#define PROGRESS		"progress"
#define DETAIL_STR	"detail"

#ifdef  TEST224
#define HOSTNAME_ETH0 		"224.0.0.1"
#define DEF_MULT_ADDR0		"224.0.0.0"
#else
#define DEF_MULT_ADDR		"224.0.0.0"
#define HOSTNAME_ETH0 		"225.0.0.1"
#define DEF_MULT_ADDR0		"225.0.0.0"
#endif
#define HOSTNAME_ETH1 		"226.0.0.1"
#define DEF_MULT_ADDR1		"226.0.0.0"
#define DEF_MULT_NETMASK	"255.0.0.0"

#define MULTI_CAST_PORT 3310

#define CMD_STR 	"cmd"
#define SEQ_STR 	"seq"

#define	DEV_GUID_BYTE		8	//GUIDռ�õ��ֽ���





/******************************************************
*��������: init_dev_net_port
*��������: ��ʼ����������
*���룺 	
*		net_st ���������������ݽṹ
*����� 	0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
 int init_dev_net_port(multicast_sock* net_st);

/******************************************************
*��������: send_test_report
*��������: ���Ͳ��Խ��̲�����ʵʱ��Ϣ
*���룺 	
*		net_st ���������������ݽṹ
*		unsigned char* info	���Ե�ʵʱ��Ϣ
*		int prog ����0-100������
*����� 	0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
 int send_test_report(multicast_sock* ns, unsigned char* info,int prog);

/******************************************************
*��������: send_test_cmd
*��������: �����鲨����
*���룺 host_name host_name1 �����鲥��ַ
*			port �鲥�˿ں�
*			net_st ���������������ݽṹ
*����� 0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
int send_multicast_pkt(multicast_sock *net_st,  char *buf);


/*
*****************************************************
*��������: pass_dev_GUID
*��������: ����GUID
*���룺 
*����� unsigned char *cmd  ���guid �Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_GUID(unsigned char *cmd);








#endif
