#ifndef UPNP_SEND_STATE_H
#define UPNP_SEND_STATE_H

#include "upnpd.h"

#define	MOD_COM_CHANNEL		0
#define	MOD_SOCKET_CHANNEL	1


/*
 *��ʼ����������ͨѶ������ͨ��
 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int init_com_channel(void);
/*
 * ����״̬�������� 
 * ����:channel: ��ΪMOD_SOCKET_CHANNEL��MOD_COM_CHANNEL

 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int send_upnpd_stat2main(int channel);


//��������vmmain�Ĳ�ѯ���������״̬
void *recv_modsocket_thread (void *data);

//��������vsmain�Ĳ�ѯ���������״̬
void *recv_modcom_thread(void *data);


/*����ǰ״̬���͸�modcom��modsocket����ͨ��*/
void send_state();

int create_recv_modsocket_thread(void);
#endif

