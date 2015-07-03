#ifndef AENC_READ_H
#define AENC_READ_H
#include <media_api.h>


#ifndef IN
	#define IN
	#define OUT
	#define IO
#endif
/**********************************************************************************************
 * ������	:init_audio_enc()
 * ����	:��ʼ����Ƶ����������ؽṹ
 * ����	:��
 * ���	:�ڿ����Ѿ�������һ����̬��������Ƶ�������ṹ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:�ڳ�������ʱ����һ��
 **********************************************************************************************/
int init_audio_enc(void);

/**********************************************************************************************
 * ������	:get_audio_enc()
 * ����	:��ȡ��Ƶ������������Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,��������NULL
 **********************************************************************************************/
media_source_t *get_audio_enc(IN int no);

/**********************************************************************************************
 * ������	:get_audio_enc_playback()
 * ����	:��ȡ��Ƶ������������Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,��������NULL
 **********************************************************************************************/
media_source_t *get_audio_enc_playback(IN int no);

/**********************************************************************************************
 * ������	:get_audio_read_buf()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ������ָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,��������NULL
 **********************************************************************************************/
void *get_audio_read_buf(IN int no);

/**********************************************************************************************
 * ������	:get_audio_read_buf_len()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ����������
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:��ʱ�������ĳ���
 **********************************************************************************************/
int get_audio_read_buf_len(IN int no);

/**********************************************************************************************
 * ������	:connect_audio_enc()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_enc(IN int no,IN int type,IN char *name,IN int pre_sec);

/**********************************************************************************************
 * ������	:connect_audio_enc()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_enc_playback(IN int no,IN int type,IN char *name,IN int pre_sec);


/**********************************************************************************************
 * ������	:disconnect_audio_enc()
 * ����	:�Ͽ���ָ��������������
 * ����	:no:��Ҫ�Ͽ�����Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int disconnect_audio_enc(IN int no);

/**********************************************************************************************
 * ������	:reactive_audio_enc()
 * ����	:���¼����Ƶ������������
 * ����	:no:��Ҫ���¼������Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:Ӧ�ó���Ӧ���ڵ���,��ֹ�����Լ�һ��ʱ��û����Ӧ����
 *			 audioenc����Ͽ�
 **********************************************************************************************/
int reactive_audio_enc(IN int no);

/**********************************************************************************************
 * ������	:read_audio_frame()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag);

/**********************************************************************************************
 * ������	:read_audio_playback()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_playback(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag);

/**********************************************************************************************
 * ������	:get_aenc_stat()
 * ����	:��ȡָ����ŵ���Ƶ������״̬
 * ����	:no:��Ҫ��ȡ״̬����Ƶ���������
 * ����ֵ	:��ֵ��ʾ����-EINVAL:�������� -ENOENT:�豸��û������
 *					ENC_NO_INIT:δ��ʼ��
 *					ENC_STAT_OK:״̬����
 *					ENC_STAT_ERR:����������
 **********************************************************************************************/
int get_aenc_stat(IN int no);

/**********************************************************************************************
 * ������	:get_aenc_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_aenc_attrib(int no);

/**********************************************************************************************
 * ������	:get_adec_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_adec_attrib(int no);
int recover_playback_aframe2normal(IN int no,IN int count);



/**********************************************************************************************
 * ������	:get_audio_rec_read_buf_len()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ����������
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:��ʱ�������ĳ���
 **********************************************************************************************/
int get_audio_rec_read_buf_len(IN int no);



/**********************************************************************************************
 * ������	:get_audio_rec_read_buf()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ������ָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,��������NULL
 **********************************************************************************************/
void *get_audio_rec_read_buf(IN int no);
/**********************************************************************************************
 * ������	:reactive_audio_rec_enc()
 * ����	:���¼����Ƶ������������
 * ����	:no:��Ҫ���¼������Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:Ӧ�ó���Ӧ���ڵ���,��ֹ�����Լ�һ��ʱ��û����Ӧ����
 *			 audioenc����Ͽ�
 **********************************************************************************************/
int reactive_audio_rec_enc(IN int no);

/**********************************************************************************************
 * ������	:disconnect_audio_enc()
 * ����	:�Ͽ���ָ��������������
 * ����	:no:��Ҫ�Ͽ�����Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int disconnect_audio_rec_enc(IN int no);

/**********************************************************************************************
 * ������	:read_audio_rec_frame()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_rec_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag);
/**********************************************************************************************
 * ������	:get_aenc_rec_stat()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
int get_aenc_rec_stat(IN int no);

/**********************************************************************************************
 * ������	:get_audio_enc_rec_remain()
 * ����	:��ȡָ����Ƶ���������ڴ�صĸ���
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
int get_audio_enc_rec_remain(IN int no);
/**********************************************************************************************
 * ������	:get_aenc_rec_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_aenc_rec_attrib(int no);

/**********************************************************************************************
 * ������	:connect_audio_rec_enc()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_rec_enc(IN int no,IN char *name,IN int pre_sec);
/**********************************************************************************************
 * ������	:init_audio_rec_enc()
 * ����	:��ʼ����Ƶ����������ؽṹ
 * ����	:�ڼ�·��Ƶ
 * ���	:�ڿ����Ѿ�������һ����̬��������Ƶ�������ṹ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:�ڳ�������ʱ����һ��
 **********************************************************************************************/
int init_audio_rec_enc(int no);
int init_audio_rec_enc_all();


#endif