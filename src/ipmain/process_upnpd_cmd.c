#include "ipmain.h"
#include "mod_cmd.h"
#include "leds_api.h"
#include "mod_com.h"
#include "stdio.h"
#include "ipmain_para.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "process_upnpd_cmd.h"
#include "devinfo.h"

/**********************************************************************************************
 * ������	:process_upnpd_cmd()
 * ����	:����upnpdģ�鷢��������
 * ����	:cmd:���յ���upnpdģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_upnpd_cmd(mod_socket_cmd_type *cmd)
{
	DWORD *state;
	struct ip1004_state_struct *gtstate;
	printf("vsmain recv a upnpd cmd\n");
	gtloginfo("vsmain�յ�һ��upnpdģ������0x%04x\n",cmd->cmd);
	state=(DWORD*)&cmd->para[sizeof(pid_t)];

	int i;
	
	switch(*state)
	{
		case UPNPD_SUCCESS:	//upnpd�˿�ӳ��ɹ�
			
			gtloginfo("upnpd���������֪�˿�����ӳ��\n");
			for(i=0;i<virdev_get_virdev_number();i++)
			{
				gtstate = get_ip1004_state(i);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_per_state.upnp_err = 0;
				pthread_mutex_unlock(&gtstate->mutex);
				send_dev_state(-1,1,0,0,0,i);
			}
		break;
		case UPNPD_FAILURE:		//upnpd�˿�ӳ��ʧ��
			gtloginfo("upnpd���������֪�˿�ӳ��ʧ��\n");
			for(i=0;i<virdev_get_virdev_number();i++)
			{
				gtstate = get_ip1004_state(i);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_per_state.upnp_err = 1;
				pthread_mutex_unlock(&gtstate->mutex);
				send_dev_state(-1,1,0,0,0,i);
			}
		break;
		default:
			printf("vsmain recv a unknow state:%04x from upnpd\n",(int)*state);
		break;
	}
			
	return 0;


}
