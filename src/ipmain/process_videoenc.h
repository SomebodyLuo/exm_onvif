#ifndef PROCESS_VIDEOENC_H
#define PROCESS_VIDEOENC_H

#include "mod_socket.h"


/**********************************************************************************************
 * ������	:process_videoenc_cmd()
 * ����	:����videoencģ�鷢��������
 * ����	:cmd:���յ���videoencģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_videoenc_cmd(mod_socket_cmd_type *cmd);
int process_videoenc_vda_state(mod_socket_cmd_type *cmd);
int process_videoenc_err(mod_socket_cmd_type *cmd);

#endif
