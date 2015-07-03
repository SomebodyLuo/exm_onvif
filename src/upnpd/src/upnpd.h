 #ifndef UPNPD_H
#define UPNPD_H
#include "upnptools.h"

//upnpd��״̬
#define	UPNPD_STATE_INIT		2
#define UPNPD_STATE_GETROUTER	3
#define	UPNPD_STATE_OK			UPNPD_SUCCESS
#define	UPNPD_STATE_FAILED		UPNPD_FAILURE


#define VERSION		"0.07"
//0.07  lc      2012-12                 ��ֲ��ip1004�豸��
//0.06	wsy	2008-03-27		�������ڴ�й¶������
//0.05  wsy 2008-03-19	�����dhcp,��һֱ�ȵ�dhcp�ҵ�����·�������½���
//0.04  wsy 2008-01-10	��·��������ʱҲ���Զ����ֲ�����ӳ��
//						�˿�ӳ�������ȥ��ǰ���"port:"	
//0.03	wsy	2007-10-25	֧���Զ�Ѱ�ҵ�ǰ·�ɵĹ���
//0.02  wsy 2007-10-24	��д��ȫ�ֱ�������,������ģ��ͨѶ��״̬����߳�
//0.01	wsy	2007-10-19	��ʼ��

#define UPNPD_LOCKFILE			"/lock/ipserver/upnpd"

#define	PORT_AVAILABLE		0	//�˿ڿ���δ��ӳ��
#define PORT_OCCUPIED		1	//�˿ڱ���������ռ��
#define PORTMAP_FAILED		2	//�˿�ӳ��ʧ��
#define	PORTMAP_SUCCESSFUL	3	//�˿��ѱ�������ӳ��ɹ�


#define IP_SERVICE_TYPE 		"urn:schemas-upnp-org:service:WANIPConnection:1"//upnp�˿�ӳ��Ĺؼ���
#define UPNPD_PARA_FILE			"/conf/ip1004.ini" //upnpd �����ļ�
#define UPNPD_SAVE_FILE			"/conf/ip1004-upnp.ini" //upnpd������ļ������ں��������Ƚ�

#define SEARCH_ROUTER_INTERVAL	15	//��λΪs,��ΧΪ2-80��������Ѱ����·��������ļ��ʱ��
#define KEEPALIVE_INTERVAL		10 //��λΪs, ��ô�ò�ѯһ�ζ˿�ӳ���Ƿ���,��ʧ�ܾ��ж���Ҫ���¿�ʼ��Ѱ���õ�·����
#define SENDSTATE_INTERVAL		180 //��λΪs,��ô��û�п���·�����ͱ��������̣��Ӷ���������

typedef struct 
{
	char	*port_dscrp;	//�˿�����,ͬ�����ļ��б���һ�£���"port:cmd_port"
}
port_t; //�����˿ڵĽṹ


/********************************************************************************
 * ������:get_current_state()
 * ����: ���ص�ǰ״̬
 ********************************************************************************/
int get_current_state(void);

#endif
