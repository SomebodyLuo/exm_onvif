#ifndef PLAY_BACK_H
#define PLAY_BACK_H

#include "rtimage2.h"

typedef struct{

	pthread_mutex_t 	mutex;
	int pb_first_fg[MAX_VIDEO_ENCODER]; //��ʼ���ӻط�ָ��ı�־
	int pb_audio_flag[MAX_AUDIO_ENCODER];
	int playback_flag[MAX_VIDEO_ENCODER];//���ڻطŵ�ͨ��
	int pb_ct[MAX_VIDEO_ENCODER];
	int pb_venc[MAX_VIDEO_ENCODER];//�ط�ָ�����ӻ���ص�״̬-1:������0:δ����
	int pb_aenc[MAX_AUDIO_ENCODER];
	int pb_vct[MAX_VIDEO_ENCODER];
	int pb_act[MAX_AUDIO_ENCODER];
	int current_net_ch[TCPRTIMG_MAX_AVUSR_NO+1];
	int frame_adjust[MAX_VIDEO_ENCODER];
	int aframe_adjust[MAX_AUDIO_ENCODER];
	int default_screen;



}playback_t;
playback_t * get_playback_parm(void);

void init_playback_parm(void);


int set_net_scr_ch(int usr_no,int ch);

int get_net_scr_ch(int usr_no);

int get_playback_stat(int enc_no);

void set_playback_en(int no);
void mutichannel_set_playback_en(int no);


/*

	�����ط�״̬
*/

void set_playback_cancel(int no);
/*

	ֻ���ط�״̬�л���ʵʱ����״̬
*/
void set_playback_to_live(void);
void mutichannel_set_playback_to_live(int enc_no);
int get_playback_frame_adjust(int no);

void set_playback_frame_adjust(int no,int frame_ct);


#endif
