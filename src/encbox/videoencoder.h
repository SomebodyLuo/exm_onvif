#ifndef VIDEO_ENCODER
#define VIDEO_ENCODER
#include <typedefine.h>
#include <fenc_api.h>
#include <mshmpool.h>
#include <media_api.h>
typedef struct{
//��Ƶ�������ṹ
	int						venc_cnt;		//���ڼ��ӱ������ļ�����
	int						venc_err_cnt;	//��Ƶ����������Ĵ���
	int						venc_first_start;	//�����������־
	media_source_t			media;			//ý��Դ�ṹ
}video_enc_t;

typedef struct{
//��Ƶ�������ṹ
	int						enc_first_start;	//�����������־
	media_source_t			media;			//ý��Դ�ṹ
}audio_enc_t;

int get_onvif_device_num(void);

DWORD get_videoencstatint(void);





#endif
