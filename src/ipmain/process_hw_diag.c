#include "ipmain.h"
#include "maincmdproc.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "devinfo.h"




int process_hw_diag_state(mod_socket_cmd_type *cmd)
{
	//static DWORD old_state=0;
	int i;
	DWORD *state=(DWORD*)&cmd->para[sizeof(pid_t)];
       hw_diag_state_t *hw_state=(hw_diag_state_t*)&cmd->para[sizeof(pid_t)];
	struct ip1004_state_struct * gtstate=NULL;
	//struct videoenc_state_struct *newstate,*change_state;
	gtloginfo("recv a hw_diag HW_DIAG_STATE_RETURN cmd state=0x%08x\n",(int )*state);
	for(i=0;i<virdev_get_virdev_number();i++)
	{
        if(hw_state->ide_err)
        {///ֻ����hw_diagģ��Ĺ��ϳ���,��������ϻָ�
              gtstate=get_ip1004_state(i);
		pthread_mutex_lock(&gtstate->mutex);
              gtstate->reg_dev_state.cf_err=hw_state->ide_err;
              pthread_mutex_unlock(&gtstate->mutex);
        }
		send_dev_state(-1,1,0,0,0,i);
	}

	return 0;
}

/**********************************************************************************************
 * ������	:process_hw_diag_cmd()
 * ����	:����hw_diagģ�鷢��������
 * ����	:cmd:���յ���hw_diagģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_hw_diag_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{
		case HW_DIAG_STATE_RETURN:
			process_hw_diag_state(cmd);
		break;
		
		default:
			printf("recv a unknow hw_diag cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow hw_diag cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}


