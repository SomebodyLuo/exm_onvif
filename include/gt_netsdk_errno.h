/**  @file	gt_netsdk_errno.h
 *   @brief gt_netsdk�еĴ����붨��
 *   @date 	2007.06
 */
#ifndef GT_NETSDK_ERRNO_H
#define	GT_NETSDK_ERRNO_H

//��Ƕ��ʽ�豸�����Ĵ�����
#define RESULT_SUCCESS			0	//�ɹ�
#define ERR_DVC_INTERNAL		0x1001	//�豸�ڲ���
#define ERR_DVC_INVALID_REQ		0x1002	//�ͻ��������ݸ�ʽ��
#define ERR_DVC_BUSY			0X1003	//�豸æ
#define ERR_DVC_FAILURE			0x1004  //�豸����
#define ERR_EVC_CRC_ERR			0x1005	//�豸�յ�һ��crc��������ݰ�
#define ERR_EVC_NOT_SUPPORT		0x1006  //�豸�յ�һ����֧�ֵ�����
#define ERR_ENC_NOT_ALLOW		0x1007  //�豸�յ�һ�������������
#define ERR_DVC_NO_RECORD_INDEX	0x1008	//�豸û�в�ѯ������


//������صĴ����� 
#define ERR_DVC_INVALID_NAME	0x1010  //�����ļ����ָ�ʽ����
#define ERR_DVC_LOGIN_FTP    	0x1011  //�޷���¼ftp������
#define ERR_DVC_NO_FILE      	0x1012  //ftp����������ָ�����ļ����û������޶�Ȩ��
#define ERR_DVC_UNTAR        	0x1013  //��ѹ�ļ�ʧ��
#define ERR_NO_SPACE         	0x1014  //�豸�洢�ռ䲻�����޷�����
#define ERR_DVC_PKT_NO_MATCH	0x1015	//���������豸�ͺŲ�ƥ��
#define ERR_DVC_UPDATE_FILE		0x1016	//�����豸�ļ�(֤�飬�����ļ�)����
#define ERR_DVC_WRONG_SIZE		0x1017	//�ļ���С����




//�����豸�����Ĵ����������
#define	ERR_SDK_ERRNO_BASE		0x2000	///���������
#define	ERR_SDK_EPERM			(ERR_SDK_ERRNO_BASE + 1)	///<Operation not permitted(������Ĳ���)
#define	ERR_SDK_ENOENT			(ERR_SDK_ERRNO_BASE + 2)	///<No such file or directory(û��ָ�����ļ���Ŀ¼)
#define ERR_SDK_EBUSY			(ERR_SDK_ERRNO_BASE + 3)	///<Device or resource busy(�豸����Դæ)
#define ERR_SDK_EINVAL			(ERR_SDK_ERRNO_BASE + 4)	///<Invalid argument(��������)
#define ERR_SDK_EMFILE			(ERR_SDK_ERRNO_BASE + 5)	///<Too many open files(�򿪵��ļ�����)
#define ERR_SDK_EAGAIN			(ERR_SDK_ERRNO_BASE + 6)	///<Try again(�Ժ�����)
#define ERR_SDK_ENOMEM			(ERR_SDK_ERRNO_BASE + 7)	///<Out of memory(�ڴ治��)
#define ERR_SDK_EFBIG           (ERR_SDK_ERRNO_BASE + 8)    ///<File too large (�ļ�����)
#define ERR_SDK_UNKNOW			(ERR_SDK_ERRNO_BASE + 9)	///<unknow error(δ֪����)
#define ERR_SDK_ECFILE			(ERR_SDK_ERRNO_BASE + 10)	//�����ļ�ʧ��
#define ERR_SDK_UNKNOW_CERT		(ERR_SDK_ERRNO_BASE	+ 11)	//��֧�ֵ�֤���ʽ
#define ERR_SDK_OP_TIMEOUT		(ERR_SDK_ERRNO_BASE	+ 12)	//������ʱ
#define ERR_SDK_NOT_SUPPORT		(ERR_SDK_ERRNO_BASE	+ 13)	//SDK��֧�ֵĲ���
///������ش���
#define ERR_SDK_ENETDOWN        	(ERR_SDK_ERRNO_BASE + 100)        ///<Network is down  
#define ERR_SDK_ENETUNREACH     	(ERR_SDK_ERRNO_BASE + 101)        ///<Network is unreachable ���粻�ɴ�
#define ERR_SDK_ENETRESET       	(ERR_SDK_ERRNO_BASE + 102)       ///<Network dropped connection because of reset	���ӶϿ�
#define ERR_SDK_ECONNABORTED    	(ERR_SDK_ERRNO_BASE + 103)       ///<Software caused connection abort	��������
#define ERR_SDK_ECONNRESET      	(ERR_SDK_ERRNO_BASE + 104)       ///<Connection reset by peer ���Ӹ�λ
#define ERR_SDK_ENOBUFS         	(ERR_SDK_ERRNO_BASE + 105)       ///<No buffer space available	����������
#define ERR_SDK_EISCONN         	(ERR_SDK_ERRNO_BASE + 106)       ///<Transport endpoint is already connected �����ѽ���
#define ERR_SDK_ENOTCONN        	(ERR_SDK_ERRNO_BASE + 107)       ///<Transport endpoint is not connected ����δ����
#define ERR_SDK_ESHUTDOWN       	(ERR_SDK_ERRNO_BASE + 108)       ///<Cannot send after transport endpoint shutdown �Է����ӶϿ�,���ܷ���
#define ERR_SDK_ETOOMANYREFS    	(ERR_SDK_ERRNO_BASE + 109)       ///<Too many references: cannot splice 
#define ERR_SDK_ETIMEDOUT       	(ERR_SDK_ERRNO_BASE + 110)       ///<Connection timed out	���ӳ�ʱ
#define ERR_SDK_ECONNREFUSED    	(ERR_SDK_ERRNO_BASE + 111)       ///<Connection refused		���ӱ��ܾ�

///USBKEY���
#define ERR_SDK_NO_KEYDRIVER		(ERR_SDK_ERRNO_BASE + 200)		///<û�а�װUSBKEY������
#define ERR_SDK_INIT_KEY			(ERR_SDK_ERRNO_BASE + 201)		///<��ʼ��KEYʧ��
#define ERR_SDK_NO_KEY				(ERR_SDK_ERRNO_BASE + 202)		///<û�в���key
#define ERR_SDK_NO_VALID_CERT		(ERR_SDK_ERRNO_BASE + 203)		///<û�п��õ�֤��
#define ERR_SDK_CERT_VALIDITY		(ERR_SDK_ERRNO_BASE + 204)		///<֤�����

//��ת�������������Ĵ�����
#define	ERR_BE_TRA_INTERNAL				0x7000		//�ڲ���
#define ERR_BE_TRA_INVALID_REQ			0X7001		//�ͻ��������ݸ�ʽ����ȷ
#define ERR_BE_TRA_UNREACHABLE_DEVICE_D	0X7002		//ǰ���豸��ϵ����(ֱ������)
#define ERR_BE_TRA_INVALID_TOKEN		0X7003		//�û�Ȩ����֤ʧ��
#define ERR_BE_TRA_PROTOCAL				0X7004		//��֧�ֵĴ���Э��
#define ERR_BE_TRA_INVALID_REQ_TYPE		0x7005		///��֧�ֵ�������������
#define ERR_BE_TRA_USER_EXISTS			0x7006		//ͬ���û��Ѵ���
#define ERR_BE_TRA_USER_NOT_EXISTS		0x7007		//û�и��û������Ӵ���
#define ERR_BE_TRA_DEVICE_TEMP_INVALID	0x7008		//�豸��ʱ������
#define ERR_BE_TRA_DEVICE_GUID_INVALID	0x7009		//GUID��ʽ����
#define ERR_BE_TRA_UNREACHABLE_DEVICE_N	0X700a		//ǰ���豸��ϵ����(ͨ������֪ͨ)



#endif
