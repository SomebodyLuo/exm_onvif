#include "ipmain.h"
#include "mod_cmd.h"
#include "leds_api.h"
#include "mod_com.h"
#include "stdio.h"
#include "ipmain_para.h"
#include "process_pppoe_watch_cmd.h"

/**********************************************************************************************
 * ������	:process_pppoe_watch_cmd()
 * ����	:����pppoe_watchģ�鷢��������
 * ����	:cmd:���յ���pppoe_watchģ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int process_pppoe_watch_cmd(mod_socket_cmd_type *cmd)
{
	DWORD *state;
	in_addr_t addr;
	printf("ipmain recv a pppoe_watch cmd\n");
	gtloginfo("ipmain�յ�һ��pppoeģ������0x%04x\n",cmd->cmd);
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	switch(*state)
	{
		case PPPOE_SUCCESS:	//adsl��������
			gtloginfo("pppoe���������֪adsl����������\n");

			refresh_netinfo();			
			if(get_current_netled()< NET_ADSL_OK)
			{
				//gtloginfo("pppoe,֮ǰnetledΪ%d,����Ϊadslok\n",get_current_netled());
				//set_net_led_state(NET_ADSL_OK);
				#ifdef USE_LED
					set_net_led_state(0);
				#endif
			}
			break;
		case PPPOE_NO_MODEM:		//�Ҳ���adsl modem
			gtloginfo("pppoe���������֪�Ҳ���adsl modem\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
					#ifdef USE_LED
						set_net_led_state(0);
					#endif
					//set_net_led_state(NET_NO_MODEM);
				}
			}
			break;
		case PPPOE_PASSWD_ERR:		//adsl�ʺ��������
			gtloginfo("pppoe���������֪adsl�˺������д�\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_INVAL_PASSWD);
				}
			}
			break;
		case PPPOE_USR_TWICE:		//�ʺ��ظ�����
			gtloginfo("pppoe���������֪adsl�˺��ظ�����\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_LOGINED);
				}
			}
			break;
		case PPPOE_USR_INVALID:		//�ʺ���Ч
			gtloginfo("pppoe���������֪	adsl�˺���Ч\n");	
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
				//	set_net_led_state(NET_INVAL_USR);	
				}
			}
			break;
		case PPPOE_PAP_FAILED:		//�û���������֤ʧ��
			gtloginfo("pppoe���������֪	adsl�û���������֤ʧ��\n");	
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
									#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_PAP_FAILED);	
				}
			}
			break;
		default:
			printf("ipmain recv a unknow state:%04x from pppoe\n",(int)*state);
		break;
	}
	return 0;


}
