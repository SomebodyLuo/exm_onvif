/** @file	       maincmdproc.c
 *   @brief 	���ղ�����������̷���������ĺ�������
 *   @date 	2007.03
 */
#ifndef MAIN_CMD_PROC_H_20070305
#define MAIN_CMD_PROC_H_20070305



/** 
 *   @brief     ��������ʼ����������ͨѶ������ͨ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int init_com_channel(void);

/** 
 *   @brief     �������ղ����������̷�����������߳�
 *   @param  ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int creat_mod_cmdproc_thread(void);

/** 
 *   @brief     ������������Ƶ����æ״̬
 *   @param  busy 0��ʾû���û����� 1��ʾ������һ���û���������Ƶ
 */ 
void set_net_enc_busy(int busy);


/**
*	@brief 	����������緢�͵����ݰ�����
*	@param	��
*	@return	0
*/
int creat_snd_pkts_thread(void);


int send_rtimg_stop_playback(void);

#endif

