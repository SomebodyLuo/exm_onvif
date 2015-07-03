/*	GT1000ϵ����Ƶ�������鲥����(UDP)Э��ײ�⺯��
 *		 2006.9 shixin
 *    ���������ṩ��ʵ��Ƕ��ʽϵͳ��Զ�̼����ʹ���鲥��ʽͨѶ�ĵײ�ӿ�
 *    �ϲ��Ӧ�ýӿڼ����ݽṹ�����ڱ����ʵ�ַ�Χ
 *    ���������ṩ�Ĺ�����
 *    1.���������ݱ�����GTϵ����Ƶ���������鲥ͨѶЭ���ʽ���д�������ܣ�crc32У�飬֮���͸�ָ���ĵ�ַ
 *    2.��ָ����socket�н������ݰ�������crcУ�飬�����ı任�����ķ��ظ����ý��̣��������ݰ��ķ�������Ϣ���ص�����
 *    
 */
#ifndef GTDEV_API_H
#define	GTDEV_API_H

//#pragma once  //lsk 2007-4-10  win32�±���ʱ�õ��Ķ������ ֻ�������һ��

#ifndef IN
	#define	IN
	#define	OUT
	#define	IO
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
	//windows ʹ��
#pragma once  //lsk 2007-4-10
	#define EXPORT_DLL __declspec(dllexport)
#else
	//linux ʹ��
	#define EXPORT_DLL
#endif

//���ܷ�ʽ����
#define GTDEV_ENC_NONE		0		//����Ҫ����

#define GUID_LEN                8  //GUIDռ�õ��ֽ���,�ӿ����õ���id���ȶ��Ǹ�ֵ
/**********************************************************************************************
* ������	:send_dev_pkt()
* ����	:��һ�����İ���ָ���ļ��ܷ�ʽ�γ��������ݰ�,���͵�ָ����socket��ַ
*
 ����	:	fd:Ŀ��socket�ļ�������
* 		sin:����Ŀ���ַ��Ϣ�Ľṹָ��
* 		target_id:Ŀ��id��,����ΪGUID_LEN
* 		source_id:������id��,����ΪGUID_LEN
* 		buf:Ҫ���ͳ�ȥ�����ݻ�����
* 		buflen:�������е���Ч�ֽ���
* 		enc_type:Ҫʹ�õļ��ܷ�ʽ()
* 		flag:����ѡ���������ʱд0
* ����ֵ:	
* 		��ֵ��ʾ���ͳ�ȥ���ֽ�������ֵ��ʾ����
**********************************************************************************************/
EXPORT_DLL int send_dev_pkt(IN int fd,IN struct sockaddr_in *sin,IN unsigned char *target_id,IN unsigned char *source_id,IN void *buf,int buflen,IN int enc_type,IN int flag);

/**********************************************************************************************
* ������	:recv_dev_pkt()
* ����	:	��ָ���ӿڽ���һ�����ݣ����û������������
* ����	:	fd:Ҫ�����ļ���socket�ļ�������
*		selfid:�Լ���id�ţ�����Ϊ GUID_LEN
*		buflen:������Ϣ�Ļ���������
*		flag:���ձ�־����������ʱд0
* ���	:	
* 		sin:���������ߵ�ַ��Ϣ�Ľṹָ��
* 		source_id:������id�� ����ΪGUID_LEN
* 		msgbuf:���յ�����Ϣ
* 		enc_type:���յ������ݰ��ļ�������
* ����ֵ:	
* 		��ֵ��ʾ���յ�msgbuf�е���Ч�ֽ�������ֵ��ʾ����
*		-ENOMEM ��ʾ����������
**********************************************************************************************/
EXPORT_DLL int recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid,OUT unsigned char *sourceid,OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag);

/**********************************************************************************************
 * * ������        :dual_id_recv_dev_pkt()
 * * ����  :       ��ָ���ӿڽ���һ�����ݣ����û������������
 * * ����  :       fd:Ҫ�����ļ���socket�ļ�������
 * *               selfid:�Լ���id�ţ�����Ϊ GUID_LEN
 * *		   selfid1:����һ��id�� ����ΪGUID_LEN
 * *               buflen:������Ϣ�Ļ���������
 * *               flag:���ձ�־����������ʱд0
 * * ���  :       
 * *               sin:���������ߵ�ַ��Ϣ�Ľṹָ��
 * *               source_id:������id�� ����ΪGUID_LEN
 * *               msgbuf:���յ�����Ϣ
 * *               enc_type:���յ������ݰ��ļ�������
 * *	           recv_id:���յ���id�� 
 * * ����ֵ:       
 * *               ��ֵ��ʾ���յ�msgbuf�е���Ч�ֽ�������ֵ��ʾ����
 * *               -ENOMEM ��ʾ����������
 * *	lsk 2009-2-9 ������һ�������id������id�κ�һ����ȷ���ܹ��������ݰ�
 * **********************************************************************************************/
EXPORT_DLL int dual_id_recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid, IN unsigned char *selfid1,OUT unsigned char *sourceid,OUT unsigned char *recv_id, OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag);

#ifdef __cplusplus
}
#endif

#endif

