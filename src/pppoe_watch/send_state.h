#ifndef SEND_STATE_H
#define SEND_STATE_H






/*
 *��ʼ����������ͨѶ������ͨ��
 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int init_com_channel(void);
/*
 * ����״̬�������� 
 * ����:stat:Ҫ���͵�״̬
 * ����ֵ:0 ��ʾ���ͳɹ�
 * ��ֵ:    ��ʾ����
*/
int send_pppoe_stat2main(int stat);

//��������vm/smain�Ĳ�ѯ���������״̬

void *recv_modsocket_thread (void *data);
void *recv_modcom_thread (void *data);

int creat_recv_modsocket_thread(void);
#endif
