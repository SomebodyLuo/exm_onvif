#include "ipmain.h"
#include "hdmodapi.h"
#include "maincmdproc.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "mod_socket.h"
#include "devinfo.h"

int process_hdplayback_state(mod_socket_cmd_type *cmd)
{
	int i;

	static DWORD old_state=0;
	DWORD *state,change;
	struct ip1004_state_struct * gtstate;
	struct hdplayback_state_struct *newstate,*change_state;
	if(cmd==NULL)
		return -1;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	gtloginfo("recv a hdplayback HDPLAYBACK_STATE_RETURN cmd state=0x%08x\n",(int)*state);

	return 0;
}


/**********************************************************************************************
 * ������	:process_hdplayback_cmd()
 * ����	:����hdplaybackģ�鷢��������
 * ����	:cmd:���յ���hdplaybackģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_hdplayback_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{
		case MOD_BYPASSTO_GATE_CMD:
			printf("recv hdplayback MOD_BYPASSTO_GATE_CMD\n");
			gtloginfo("recv hdplayback MOD_BYPASSTO_GATE_CMD\n");
			bypass2gate(cmd);
		break;
		case HDMOD_STATE_RETURN:
			process_hdplayback_state(cmd);
		break;
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv hdplayback MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv hdplayback MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
		break;
		default:
			printf("recv a unknow hdplayback cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow hdplayback cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}
