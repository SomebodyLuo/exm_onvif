#ifndef IME6410API_H
#define IME6410API_H

#ifndef _WIN32

#include <sys/ipc.h>
#include <ime6410.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif //_WIN32

#include <pthread.h>

#define MEDIA_VIDEO		0x01		//��Ƶ����
#define MEDIA_AUDIO		0x02		//��Ƶ����

#define IDX1_VID  		0x63643030	//AVI����Ƶ�����
#define IDX1_AID  		0x62773130	//AVI����Ƶ���ı��


#ifndef struct_HDR

#define struct_HDR
struct NCHUNK_HDR {	//avi��ʽ�����ݿ�ͷ��־�ṹ
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
#endif



struct compress_struct
{

	int		fd;					//ѹ���豸���豸������
#ifndef _WIN32
	int		init_tw9903flag;		//�Ѿ���ʼ����tw9903��־
	int		NTSC_PAL;			//pal�������־
	int		frame_rate;			//֡��
	int		AudioFrameType;	//��Ƶ���� 0��ʾ����Ҫ��Ƶ ����ֵ��ʾ��Ƶ���� �����ime6410.h
	pthread_mutex_t mutex;		//��Ҫ��ռ����ʱ�õ��Ļ�����
	int		full_ch;				//9903ʹ�õ���Ƶ����ͨ����
	int     	saturation; 			//9903 ���Ͷ�,128Ϊȱʡ,��ΧΪ0-255����ͬ
	int     	hue;        			//9903 ɫ��
	int     	contrast;   			//9903 �Աȶ�
	int     	brightness; 			//9903 ����
	int		sharp1;
	int		sharp2;
	int 		md_sense;		    // �ƶ�����������0-5  0��ʾ��ֹ�˶����,5,4,3,2,1��ʾ�����ȸ�,�ϸ�,��,�ϵ�,��   lsk 2007-1-5
	int 		md_enable;		    //�ƶ����ʹ�� 0 : ��ֹ  1 : ʹ��   lsk 2007-1-5
  
	int 		md_var[4];			//�ƶ����仯��		lsk 2007-1-5
	struct I64Reg i64reg;			//�������Ĳ����ṹ
	key_t EncKey;				//��Ƶ�������õ��Ļ����key
#endif
	char ini_sec[60];				//�����ļ��еĽ���
	char dev_node[60];			//�豸�ڵ���
	char dev_name[60];			//�豸��
	char driver_name[60]; 		//����������	

};

struct stream_fmt_struct
{							//����֡�Ľṹ
	struct timeval tv;			//���ݲ���ʱ��ʱ���
	unsigned long	channel;	//ѹ��ͨ���������6410�����һ��ͨ����6400��4��
	unsigned short media;		//media type ��Ƶ����Ƶ
	unsigned short  type;		//frame type	I/P/����...
	 long len;				//frame len �������Ƶ����Ƶ�����ݵĳ���
	struct NCHUNK_HDR chunk;//���ݿ�ͷ��־��Ŀǰʹ��avi��ʽ
	char data[4];				//frame data changed by shixin 060327
};


/**********************************************************************************************
 * ������	:read_enc_para_file()
 * ����	:��ָ�������ļ����ָ�����ж�ȡ�������
 * ����	:filename:�����ļ���
 *			 section:�����ļ��еĽ�,��"netencoder","hqenc0"...�������ܳ���20byte
 * ���	 enc:ָ��Ҫ��Ų�����ָ��	,��������ʱ�ᱻ����
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 **********************************************************************************************/
int read_enc_para_file(char *filename,char *section, struct compress_struct   *enc);
#define ReadEncParaFile read_enc_para_file


/**********************************************************************************************
 * ������	:init_enc_default_val()
 * ����	:����������������ΪĬ��ֵ,Ӧ���ڶ�ȡ�����ļ�֮ǰ����һ��
 * ����	:��
 * ���:encoder ָ����Ҫ�����ı����������ṹ��ָ��
 *					��������ʱ������ 
 * ����ֵ	:��
 **********************************************************************************************/
void init_enc_default_val(struct compress_struct *encoder);

/**********************************************************************************************
 * ������	:open_ime6410()
 * ����	:��6410��
 * ����	:devname:�豸�ڵ���
 *			 encoder:����������������ݵĽṹָ��
 * ����ֵ	:0 �����򿪣��ļ���������䵽encoder->fd
 *			  ��ֵ��ʾ ����
 *	����豸�Ѿ�����ֱ�ӷ���0
 **********************************************************************************************/
int open_ime6410(char *devname,struct compress_struct *encoder);

#if 1 //wsy sep 2006
//�ٷֱȵ�ֵ
int set_imevideo_brightness(struct compress_struct *encoder,int val);
int set_imevideo_saturation(struct compress_struct *encoder,int val);
int set_imevideo_contrast(struct compress_struct *encoder,int val);
int set_imevideo_hue(struct compress_struct *encoder,int val);
//��ime6410ǰ�˵�9903�л���ָ��ͨ��

/*
	��ʼ��6410
	encoder:�Ѿ����õĲ���
	����ֵ 0:�ɹ�
			 <0:����
*/
int switch_imevideo_channel(struct compress_struct *encoder,int ch);
#endif

/**********************************************************************************************
 * ������	:init_ime6410()
 * ����	:��������Ĳ����ṹ��ʼ��ime6410Ӳ��
 * ����	:encoder:�Ѿ����ò��������豸�Ѿ��򿪵Ľṹ��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 **********************************************************************************************/
int init_ime6410(struct compress_struct *encoder);
#define InitVEncoder init_ime6410

/**********************************************************************************************
 * ������	:is_keyframe()
 * ����	:�ж�һ֡�����Ƿ��ǹؼ�֡,���������ļ��и�ʱ���õ�
 * ����	:frame ��Ҫ���жϵ�����֡
 * ����ֵ	:0 ��ʾ�ǹؼ���
 *			 1��ʾ�ǹؼ���
 **********************************************************************************************/
int is_keyframe(struct stream_fmt_struct *frame);

/**********************************************************************************************
 * ������	:i64_read_frame()
 * ����	:��6410�ж�ȡ1֡��Ƶ����1֡��Ƶ����
 * ����	:encoder:�Ѿ����ò��������豸�Ѿ��򿪵Ľṹ��
 *			 buf_len:����������Ĵ�С�����һ�����ݴ������ֵ��ض�	
 *			 key_f:��ȡ�ؼ����־�����Ϊ1���������죬ֻ��ȡiframe
 *				�����ļ��ĵ�һ������ʱ��Ҫ���ô˲���������ʱ��Ϊ0
 * ���	 buffer:Ŀ�껺����,���������Ѷ�ȡ�������ݷ���˻�����
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 * 			-6 ��ʾencoder�е��豸fdΪ��ֵ
 * 			-5 ��ʾ����Ĳ���encoderΪ��
 * 			-4 ��ʾ�����bufferΪ��
 * 			-3��ʾ��������
 * 			-2 ��ʾ�ļ�����
 * 			-1��ʾ��������
 **********************************************************************************************/
int i64_read_frame(struct compress_struct *encoder,struct stream_fmt_struct *buffer,int buf_len,int req_keyf);
#define ReadEncFrame i64_read_frame

/**********************************************************************************************
 * ������	:close_ime6410()
 * ����	:�ر��Ѿ��򿪵�6410
 * ����	:encoder���������������ݽṹ
 * ����ֵ	:0 ��ʾ�ɹ� ����ֵ��ʾʧ��
 **********************************************************************************************/
int close_ime6410(struct compress_struct *encoder);
#define CloseVEnc	close_ime6410


/**********************************************************************************************
 * ������	:I64_Start()
 * ����	:����6410����ʼѹ������
 * ����	:enc���������������ݽṹ
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int I64_Start(struct compress_struct *enc);
#define StartVEnc I64_Start


/**********************************************************************************************
 * ������	:I64_CheckRegister()
 * ����	:��ӡ6410�ļĴ������նˣ�һ���ò���
 * ����	:encoder���������������ݽṹ
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int I64_CheckRegister(struct compress_struct *encoder);

/**********************************************************************************************
 * ������	:reinstall_encdrv()
 * ����	:���°�װ6410��������
 * ����	:enc���������������ݽṹ
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int reinstall_encdrv(struct compress_struct *enc);
#define ReinstallEncDrv reinstall_encdrv


/**********************************************************************************************
 * ������	:OpenEnc()
 * ����	:�򿪱����� ��ͬ��open_ime6410��ֻ�ǽڵ����Ѿ�����ڽṹ����
 * ����	:encoder���������������ݽṹ
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int OpenEnc(struct compress_struct *encoder);


/**********************************************************************************************
 * ������	:fill_driver_name()
 * ����	:�����������������ṹ�е��ֶ�
 * ����	:name:����������(�ڵ���)����Ϊ RT_6410_DEV��HQDEV0
 * ���	:enc:���������������Ľṹָ�룬
 *			 ����ʱ������ڵ�����Ӧ�Ľڵ���������·�����������ƽ������
 * ����ֵ	:��
 **********************************************************************************************/
 #include <file_def.h>

static __inline__ int fill_driver_name(struct compress_struct   *enc,char *name)
{
	if(enc==NULL)
		return -1;
/*
	��ʼ���ڵ㼰��������
*/
	sprintf(enc->dev_node,"%s",RT_6410_DEV);
	sprintf(enc->dev_name,"%s",NET6410_NAME);
	sprintf(enc->driver_name,"%s",NET6410_DRV);
	if(name==NULL)
		return 0;
	if(memcmp(name,HQDEV0,strlen(HQDEV0))==0)
	{
		sprintf(enc->dev_node,"%s",HQDEV0);
		sprintf(enc->dev_name,"%s",LOCAL6410_NAME);
		sprintf(enc->driver_name,"%s",LOCAL6410_DRV);			
	}
	return 0;
}
int get_pic_width(struct compress_struct *enc);
int get_pic_height(struct compress_struct *enc);

//����9903����Ƶ��ʧ
int read_9903_video_loss(struct compress_struct *encoder);

int get_frame_rate(int ispal,int rate);
int get_picture_size(int ispal,int size);
//int SetVEncBitRate(struct compress_struct *Enc,int bitrate);
//int RestartVCompress(struct compress_struct *encoder);
#endif

