#ifndef ADECAPI_H
#define ADECAPI_H
#include <mshmpool.h>
#include <soundapi.h>

#define 	ADEC_NO_INIT		0		//������û�г�ʼ��
#define		ADEC_STAT_OK		1		//��������������
#define		ADEC_STAT_ERR		2		//����������	
//���������Խṹ
typedef struct{
	int				EncType;	//����������
	int				State;		//������״̬  0:��ʾδ��ʼ�� 1��ʾ���� 2��ʾ����
}ADEC_ATTRIB;


int AttachADecDevice(int DecNo,MSHM_POOL *Pool,char *UsrName,int type);
int GetADecState (MSHM_POOL *Pool);
int PutAData2DecPool(MSHM_POOL *Pool,void *data,int datalen);

static __inline__ int GetADataFromDecPool(MSHM_POOL *pool,void *data,int fblen,int *eleseq,int *flag)
{
	return MShmPoolGetResource(pool,data,fblen,eleseq,flag);
}
#endif



