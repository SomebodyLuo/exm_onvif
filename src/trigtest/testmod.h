#ifndef __TESTMOD_H
#define __TESTMOD_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>




#define BUF_LEN  100


#define TESTMOD_LOCK_FILE 		("/lock/ipserver/testmod")
#define PACKAGE    				("Trigtest")
#define VERSION					("0.01")

//v0.01 ��vs3024����ֲ����Ϊ��һ���汾




#define RESULT_FILE_NAME		("/tmp/testtrig.txt")



#if 0
//// ����ͨѶ��ص�����
typedef struct
{
	int loop;							//��·�շ�������
	int fd;							//socket������
	int enc_type;						//��������
	int flag;							// ���ͱ�־
	int multi_port;					//�鲥�˿�
	struct sockaddr_in recv_addr;		//���յ�ַ
	struct sockaddr_in send_addr;		//���͵�ַ
	struct ip_mreq mreq;				//�鲥��
	unsigned char self_id[8];				// �豸�����GUID
	unsigned char target_id[8];			// Ŀ���豸��GUID
	unsigned char ip_addr[BUF_LEN];		//�豸�����IP��ַ
	unsigned char hostname[50];			//����IP��ַ
	unsigned char seq_num[50];
}multicast_sock;
#endif


/**********************************************************************************************
* ������   :s_test_rp()
* ����  :       ���Ͳ��Ա���
* ����  :      buf			���͵��ַ���
*				num		���̺�
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
int s_test_rp(unsigned char *buf,int num);


#endif
