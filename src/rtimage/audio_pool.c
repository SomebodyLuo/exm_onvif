/** @file	audio_pool.c
 *   @brief 	������Ƶ����صĺ�������
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include "debug.h"
static  media_source_t  audio_enc[MAX_AUDIO_ENCODER];       ///<��Ƶ�������ṹ


/** 
 *   @brief     ������Ƶ����������
 *   @param  afmt     ׼�������Ƶ���������Ե�ָ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 *   
 */ 
static int set_audio_enc_attrib(OUT audio_format_t *afmt)
{

	if(afmt==NULL)
		return -EINVAL;
	afmt->a_wformat=7;		//WFORMAT_ULAW			0x0007//������ʽ
	afmt->a_sampling=8000;	//����������
	afmt->a_channel=1;		//����ͨ��
	afmt->a_nr_frame=1;		//һ�����������м�������
	afmt->a_bitrate=8000;		//��������
	afmt->a_bits=8;			//��Ƶ����λ��
	afmt->a_frate=8;			//(8000/1024)��Ƶ֡�� FIXME
	return sizeof(audio_format_t);
}

/** 
 *   @brief     ��ʼ����Ƶ���뻺�����ر���
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int init_audio_enc_pool(void)
{
    int i;
    int ret;
    media_source_t *aenc=&audio_enc[0];
    init_media_rw(aenc,MEDIA_TYPE_AUDIO,i,32*1024);
    ret=create_media_write(aenc,get_audio_enc_key(0),"tcprtimg",128*1024);
    if(ret>=0)
    {
        set_audio_enc_attrib(&aenc->attrib->fmt.a_fmt);
    }
    
    return ret;

}


/** 
 *   @brief     ��ȡ��Ƶ���뻺��ؽṹָ��
 *   @param  no ��Ƶ���������
 *   @return   ��Ƶ���뻺��ؽṹָ��
 */ 
media_source_t *get_audio_enc(IN int no)
{
    return &audio_enc[no];
}

