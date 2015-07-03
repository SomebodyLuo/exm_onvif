#ifndef VENCAPI_H
#define VENCAPI_H

#include <mshmpool.h>
#include <AVIEncoder.h>

#ifndef FRAMETYPE_I
#define FRAMETYPE_I		0x0		// IME6410 Header - I Frame
#define FRAMETYPE_P		0x1		// IME6410 Header - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// IME6410 Header - Audio Frame
#define FRAMETYPE_ADPCM	0x6		// IME6410 Header - Audio Frame
#define FRAMETYPE_MD	0x8
#define VSIG_I			0xb0010000
#define VSIG_P			0xb6010000
#endif

#define 	ENC_NO_INIT		0		//������û�г�ʼ��
#define	ENC_STAT_OK		1		//��������������
#define	ENC_STAT_ERR		2		//����������	
typedef struct{//���������Խṹ
	int				EncType;	//����������
	int				State;		//������״̬  0:��ʾδ��ʼ�� 1��ʾ���� 2��ʾ����
	struct defAVIVal 	EncVal;		//��Ƶ��ʽ(avi�ļ����õ�)
}ENC_ATTRIB;


/**************************************************************************
  *������	:InitVEncVar
  *����	:����ָ��ͨ���ų�ʼ����Ƶ�������õ�enc�ṹ�е��ֶ�
  *����	: encno:��������� 0,1,2...
  			  Enc:ϣ����ʼ���������ݽṹ  		
  *����ֵ	:����0��ʾ�ɹ� ��ֵ��ʾ����
  *ע:��������ֱ�����豸�򽻵��ĳ����в���Ҫʹ��(����
  *CreateVEncDevice�����ĳ���)��������(����AttachVEncDevice�ĳ���)
  *����Ҫ
  *************************************************************************/
int InitVEncVar(int encno,struct compress_struct *Enc);
/**************************************************************************
  *������	:GetVEncIniSec
  *����	:��ȡָ����Ƶ��������ص������ļ��Ľ���
  *����	: enc:�Ѿ���ʼ���õ�������������Ϣ�Ľṹ
  *����ֵ	:�����ļ�(ini)�е����ý���
  *ע:��������ֱ�����豸�򽻵��ĳ����в���Ҫʹ��(����
  *CreateVEncDevice�����ĳ���)��������(����AttachVEncDevice�ĳ���)
  *����Ҫ
  *************************************************************************/
//char *GetVEncIniSec(struct compress_struct *enc);

/**************************************************************************
  *������	:RefreshVencDevicePara
  *����	:�����豸�����������ڴ�
  *����	:
  *			 Enc:�Ѿ���ʼ���õ�������������Ϣ�Ľṹ
  *			 Pool:һ���Ѿ���ʼ���������������Ϣ�Ľṹ
   *����ֵ	:0��ʾ�������豸������
  *			 -EINVAL:��������
  *			
  *			-
  *ע:������Ӧ����ֱ�����豸�򽻵��ĳ����е���
  *************************************************************************/
int RefreshVencDevicePara(struct compress_struct   *Enc,MSHM_POOL *Pool);

/**************************************************************************
  *������	:CreateVEncDevice
  *����	:����һ����Ƶ�������豸�Ĺ�������,���򿪱�����
  *����	:name: �����豸�ĳ�����
  *			 Enc:�Ѿ���ʼ���õ�������������Ϣ�Ľṹ
  *			 Pool:һ��û�г�ʼ���������������Ϣ�Ľṹ
  *			 bytes:ϣ������Ĺ���������С 0��ʾ�Զ�ѡ���С
  *����ֵ	:0��ʾ�������豸��������
  *			 -EINVAL:��������
  *			 -ENODEV:�򲻿��豸
  *			-
  *ע:������Ӧ����ֱ�����豸�򽻵��ĳ����е���
  *************************************************************************/
int CreateVEncDevice(char *name,struct compress_struct   *Enc,MSHM_POOL *Pool,int bytes);
/**************************************************************************
  *������	:OpenVEncDevice
  *����	:��һ����Ƶ�������豸
  *����	:
  *			 Enc:�Ѿ���ʼ���õ�������������Ϣ�Ľṹ
  *			 Pool:һ���Ѿ���ʼ���������������Ϣ�Ľṹ
  *			
  *����ֵ	:>=0��ʾ�������豸
  *			 -EINVAL:��������
  *			 -ENODEV:�򲻿��豸
  *			-
  *ע:������Ӧ����ֱ�����豸�򽻵��ĳ����е���
  *************************************************************************/
  int OpenVEncDevice(struct compress_struct   *Enc,MSHM_POOL *Pool);

/**************************************************************************
  *������	:AttachVEncDevice
  *����	:���ӵ�һ���Ѿ��򿪵Ļ����
  *����	:name: �����豸�ĳ�����
  *			 Enc:�Ѿ���ʼ���õ�������������Ϣ�Ľṹ
  *			 Pool:һ��û�г�ʼ���������������Ϣ�Ľṹ
  *			 bytes:ϣ������Ĺ���������С 0��ʾ�Զ�ѡ���С
  *����ֵ	:0��ʾ���������豸������
  *			 -EINVAL:��������
  *			 -EAGAIN:�豸���ڳ�ʼ������
  *			 --ENODEV:�豸����
  *			-
  *ע:������Ӧ���ɴӻ������ж�ȡ���ݵĳ������
  *************************************************************************/
int AttachVEncDevice(int EncNo,MSHM_POOL *Pool,char *UsrName,int type);
/**************************************************************************
  *������	:FreeVEncDevice
  *����	:�ͷ��Ѿ��򿪵Ĺ������
  *����	: pool:����ؽṹָ��
  *����ֵ	:����0��ʾ�ɹ� ��ֵ��ʾʧ�ܴ����Ǹ���errno
  *************************************************************************/
int FreeVEncDevice(MSHM_POOL *Pool);
/**************************************************************************
  *������	:PutVFrame2Pool
  *����	:��һ�����ݷ��뻺���
  *����	: pool:����ؽṹָ��,Frame���õ�ý��ṹ����
  *����ֵ	:����0��ʾ�ɹ� ��ֵ��ʾʧ�ܴ����Ǹ���errno
  *************************************************************************/
int PutVFrame2Pool(MSHM_POOL *Pool,struct stream_fmt_struct *Frame);

//int MShmPoolGetResource(MSHM_POOL *pool,void *buf,int buflen,int *elelen,int *flag);
/**************************************************************************
  *������	:GetVFrameFromPool
  *����	:�ӻ�����л�ȡһ������
  *����	: pool:����ؽṹָ��,
  *			 Frame��Ҫ���Ļ�����
  *			 fblen����仺�����ĳ���
  *			 eleseq:��ȡ��Ԫ�����(���)
  *			 flag:��ȡ��Ԫ�صı�־(���)
  *����ֵ	:������ֵ��ʾ��ȡ��Ԫ�ش�С��ʾ�ɹ� ��ֵ��ʾʧ�ܴ����Ǹ���errno
  *************************************************************************/
static __inline__ int GetVFrameFromPool(MSHM_POOL *pool,void *frame,int fblen,int *eleseq,int *flag)
{
	return MShmPoolGetResource(pool,frame,fblen,eleseq,flag);
}

#define	GetVEncUsrValid	GetUsrValid
#endif



