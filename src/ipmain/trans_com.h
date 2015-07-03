//͸������
#ifndef TRANS_COM_H
#define TRANS_COM_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "mod_socket.h"

struct trans_com_struct{		//����͸�����ڵĽṹ
	pthread_mutex_t mutex;		//���ʽṹҪ�õ�����
	pthread_t thread_id;
	int	ch;						//����ͨ��
	int flag;						//���ӱ�־,����Ѿ���Զ����������1,����Ϊ0
	int listen_fd;					//�������ӵ�tcp������
	int net_fd;					//ӳ�䵽��Ӧ��tcp������
	int tcp_port; 					//tcp�˿�
	struct sockaddr_in allow_addr;//�������͸�����ڵ�Զ�̼������ַ
	int local_fd;					//�����ļ�������
	char path[20];				//�����豸·��
	DWORD	baud;				//������
	BYTE	databit;				//����λ��һ��Ϊ8
	BYTE	parity;				//�Ƿ�Ҫ��żУ��λ,һ��Ϊ����Ҫ'N'	
	BYTE	stopbit;				//ֹͣλ��һ��Ϊ1
	BYTE	flow;				//�������ƣ�һ�㲻��Ҫ
	DWORD	net_rec_buf[256];	//�����������ݵĻ�����
	DWORD	com_rec_buf[256];	//���մ������ݵĻ�����
};


typedef struct{
	BYTE start;		//��λ 0xfe
	BYTE crc;		//У���ֽ�
	BYTE cmd;		//��������
	BYTE ichannel;	//ͨ����
	BYTE bitstr;	//�ӵ�����ʹ��
	BYTE heart;    //����
	BYTE reserved[2]; //���� 0
}GT_SUB_CMD_STRUCT;


/**********************************************************************************************
 * ������	:init_trans_com_var()
 * ����	:��ʼ��͸�����ڷ����õ��ı���
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void init_trans_com_var(void);

/**********************************************************************************************
 * ������	:get_trans_com_info()
 * ����	:��ȡָ��ͨ����͸�����ڵ������ṹָ��
 * ����	:ch:����ͨ����
 * ����ֵ	:����ָ��ͨ���ŵ�͸�����ڵĽṹָ��
 *			 NULL��ʾ����
 **********************************************************************************************/
struct trans_com_struct *get_trans_com_info(int ch);


/**********************************************************************************************
 * ������	:creat_trans_com_thread()
 * ����	:����͸�����ڷ����߳�
 * ����	:attr:�߳�����
 *			 arg:Ҫ������͸�����ڷ���������ṹָ��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 **********************************************************************************************/
int creat_trans_com_thread(pthread_attr_t *attr,void *arg);



/*****************************************************************************************
 *������:keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart)
 *����  :ͨ�����Դ�����Ƭ�������ַ�����������ʾ���豸û������
 *����  :alarmchannel �˾���ͨ���ţ������ȼ���audiochannel ������Ƶͨ���ţ�heart��ʾ�Ƿ�������
 *���  :��
 *����  :��ȷ����0�����󷵻ظ�ֵ
 * ***************************************************************************************/                                                 
int keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart); 

/****************************************************************************************                                                        
 *������:keepalive_open_com_internal()
 *����  :��com_internal����ȡ����fd������������������,�˴���Ϊ�ڲ������ô���                                                                
 *����  :��                                                                                                                                      
 *���  :��
 *����  :��������0�����󷵻ظ�ֵ                                                                                                                 
 * **************************************************************************************/                                                       
int keepalive_open_com_internal(void);

/****************************************************************************************                                                        
 *������:keepalive_set_com_mode(int enable,int interval)
 *����  :���ô��ڹ���ģʽ                                                               
 *����  :enable �Ƿ�ʹ������  interval ���������(����Ϊ��λ)                                                                                                                                      
 *���  :��
 *����  :��������0�����󷵻ظ�ֵ                                                                                                                 
 * **************************************************************************************/
int keepalive_set_com_mode(int enable,int interval);


int update_set_com_mode(int enable,int interval);

#endif
