/** @file	audio_pool.c
 *   @brief 	������Ƶ����صĺ�������
 *   @date 	2007.03
 */
#ifndef AUDIO_POOL_20070305
#define AUDIO_POOL_20070305
#include <media_api.h>

/** 
 *   @brief     ��ʼ����Ƶ���뻺�����ر���
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int init_audio_enc_pool(void);

/** 
 *   @brief     ��ȡ��Ƶ���뻺��ؽṹָ��
 *   @param  no ��Ƶ���������
 *   @return   ��Ƶ���뻺��ؽṹָ��
 */ 
media_source_t *get_audio_enc(IN int no);
int get_audio_enc_remain(int no);

#endif

