#ifndef PROCESS_GATECMD_H
#define PROCESS_GATECMD_H
#include <gate_cmd.h>
#include "mod_socket.h"
#include "mod_cmd.h"
#include "process_modcmd.h"




int process_net_index(char *index,struct query_index_struct *query);
//��ʼ����������ͨѶ������ͨ��
int init_com_channel(void);

int	creat_modcmdproc_thread(void);


int send_state2main(void);

//���õ�encno���������Ĵ���״̬�����иı����͸�������
int set_enc_error(int encno,int flag);

//���õ�chͨ������Ƶ��ʧ�����иı����͸�������
int set_video_loss(int ch,int flag);

//���õ�chͨ�����ƶ�������Ƶ�ڵ�,�����͸�������
int set_video_motion_blind(int ch,int motion, int blind);

int require_up_audio(int no);
#endif
