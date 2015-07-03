#ifndef _SYS_ERROR_H_
#define _SYS_ERROR_H_
// �������
#if 0
#define FMAT_OK		300
#define NO_DISK		301
#define NO_HDA1		302
#define MOUNT_ERR	303
#define FDISK_ERR	304
#define FMAT_ERR	305
#endif
#define CLR_OK		400
#define NO_LOG		401
#define NO_HQDATA	402
#define RM_HQD_ERR	403
#define RM_LOG_ERR	404
#if 0
#define NO_SERVER_IP 	501
#define SERVER_IP_ERR 	502
#define NO_PATH		 	503
#define NO_PASS		 	504
#define NO_USER		 	505
#define NO_SIZE		 	506
#define NO_PORT		 	507
// ��ʽ������ʧ�ܴ�����Ϣ
static const char* format_OK	=	"���̸�ʽ���ɹ�";
static const char* format_err	=	"��ʽ������ʧ��";
static const char* no_disk		=	"û��Ӳ�̻�cf��";
static const char* no_hda1		=	"�Ҳ���/dev/hda1�ڵ�";
static const char* mount_err	=	"���ش���ʧ��";
static const char* fdisk_err	=	"���̷���ʧ��";
//#define FDISK_ERROR		"��ʽ������ʧ��"
#endif

static const char* clear_OK	=	"������Լ�¼�ɹ�";

#if 0
// ������Լ�¼ʧ�ܴ�����Ϣ
static const char* clear_OK	=	"������Լ�¼�ɹ�";
static const char*	no_log 		=	"û��/logĿ¼";
static const char* no_hqdata	=	"û��/hqdataĿ¼";
static const char* rm_hqd_err 	=	"ɾ��/hqdata���ļ�ʧ��";
static const char* rm_log_err	=	"ɾ��/log���ļ�ʧ��";
#endif
#endif
