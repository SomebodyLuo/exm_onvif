#ifndef MEDIA_SVR_H
#define MEDIA_SVR_H
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mshmpool.h>
#include <VEncApi.h>
//#include <AEncApi.h>

#define	MAX_VENC_NUM				2			//֧�ֵ������Ƶ��������Ŀ
#define 	MAX_AV_USER_NUM			8			//ͬʱ��������Ƶ����û���
#define	MAX_WAN_USER_NUM		1			//Ĭ�ϵĹ��������û���
#define	MAX_LAN_USER_NUM			2			//MAX_AV_USER_NUM-MAX_WAN_USER_NUM	//Ĭ�ϵľ������û�������
#define	MAX_BUF_FRAMES			300		//���Ļ�������(���ڷ��仺����)
#define	DEFAULT_DROP_P			30			//Ĭ�ϵĶ���P�����ֵ
#define	DEFAULT_DROP_A			20			//Ĭ�ϵĶ�����Ƶ����ֵ

typedef struct{
//��������Ƶ���ݵĽṹ
	int				Flag;		//��ǣ���Ƶ������Ƶ
	int				Size;			//���ݵĳ���(����ͷβ���)
}BUF_FRAME_T;
typedef struct{
//���緢�ͻ�������Ϣ
	int					VFrames;				//�������е���Ƶ����
	int					AFrames;				//�������е���Ƶ����
	int					Head;				//ͷλ��(���ϵ�����)
	int					Tail;					//βλ��(�����µ����ݵ�λ��)
	//head==tail��ʾû������
	BUF_FRAME_T	Bufs[MAX_BUF_FRAMES];	//��Ż�������Ϣ
}SEND_BUF_T;
typedef struct{
	pthread_mutex_t		Mutex;			//�����û������õ��Ļ�����
	pthread_t				ThreadId;		//�����û�������̺߳�
	int					No;				//�û����
	int					Enable;			//�û���Ч��־
	int					SockOLen;		//socket���ͻ������Ĵ�С
	int					NetFd;			// ��������������,���NetFd<0��ʾ���û���Ч
//	int					NonBlock;		//ʹ�÷�������ʽ��socket
	int					ThTimeOut;		//�ж���ʱ����ֵ
	int					TimeOut;		//�������ݵĳ�ʱ������
	struct sockaddr_in		Addr;			//Զ�����ӵĵ�ַ		
	char 				UsrName[32];	//�û���
	struct timeval 		ConnectStart;	//��ʼ�������ӵ�ʱ��
	struct timeval 		CmdStart;		//�յ����ķ������ʼʱ��
	struct timeval		LastATime;		//��󷢳���������ʱ��
	struct timeval 		LastVTime;		//��󷢳�����Ƶ��ʱ��
	int					VEncNo;			//���û���Ҫ����Ƶ������ͨ��
	int					FirstFlag;		//��һ�ν���ͨѶ��־
	int					AudioFlag;		//�Ƿ���Ҫ���� 0��ʾ����Ҫ 1��ʾ��Ҫ
	int					ThDropP;			//������Ƶ�����ֵ
	int					ThDropA;			//������Ƶ���ݵ���ֵ
	int					DropFlag;		//�������ݱ�־ 0��ʾ����Ҫ����
	int					DropAFlag;		//������Ƶ��־
	int					DropFrames;	//��������Ƶ����
	int					LastVSeq;		//��һ����Ƶ���ݵ����
	int					LastASeq;		//��һ����Ƶ���ݵ����

	unsigned long			BufBytes;		//���뻺�������ֽ���
	unsigned long			SendBytes;		//�ѷ��͵��ֽ���
	unsigned long			SendBufBytes;	//���ͻ������ڵ�����	
	SEND_BUF_T			SendBufInfo;		//���ͻ���������Ϣ
}AVUSR_TYPE;

typedef struct{//���������Խṹ
	int				EncType;	//����������
	int				State;		//������״̬  0:��ʾδ��ʼ�� 1��ʾ���� 2��ʾ����
}AENC_ATTRIB;

typedef union{
	ENC_ATTRIB		VEncAttrib;		//��Ƶ������������
	AENC_ATTRIB		AEncAttrib;		//��Ƶ������������
}ATTRIB_TYPE;
//ý��Դ�ṹ
#define		MEDIA_TYPE_VIDEO		0
#define		MEDIA_TYPE_AUDIO		1
typedef struct{
	pthread_mutex_t	Mutex;
	int				MediaType;				//ý������
	int				No;						//��Դ���
	int				DevState;				//-1��ʾ��û�����ӵ�����ı����豸����� 
	pthread_t			ThreadId;				//�߳�id
	int				MaxDataLen;			//���豸�����ݿ���󳤶�(��̬ˢ��)
	MSHM_POOL		MPool;					//ý���õ�����Դ�������
	void *			*ReadBuf;				//��ȡ�����õĻ�����(��DWORD����)
	int				BufLen;					//����������,��ֵ��ʾ�ڴ����ʧ��
	ATTRIB_TYPE		*Attrib;					//�豸����		
}MEDIA_SOURCE;
typedef struct{
	pthread_mutex_t	Mutex;						//�������ݽṹ�õ��Ļ�����
	int				SvrPort;						//����˿ں�
	int				MaxUsr;						//֧�ֵ�����û���
	int				MaxWanUsr;					//���������û���
	int				MaxLanUsr;					//���������û���
	int				UsrNum;					//�û���
	int				WanUsrNum;					//�������û�����
	int				LanUsrNum;					//�������û�����
	MEDIA_SOURCE	VEnc[MAX_VENC_NUM];			//��Ƶ������
	MEDIA_SOURCE	AEnc;						
	AVUSR_TYPE		Users[MAX_AV_USER_NUM+1];	//��һ�����ڴ���æ״̬
}AVSERVER_TYPE;

int posix_memalign(void **memptr, size_t alignment, size_t size);
void MediaSvrSecondProc(void);
int CreateUsrThreads(int svr_port);
int SendAckPkt(int Fd,WORD Cmd,WORD Result,char *buf,int datalen);
#endif

