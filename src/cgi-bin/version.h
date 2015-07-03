#ifndef __VERSION_H
#define	__VERSION_H
	/*
		0.24 ����no.8~no.11 ���ĸ��������
	    0.23 fixbug ALARM_CONF ����multichannleһ��
	    0.22 fixbug MAX_MOD_SOCKET_CMD_LEN
		0.21 ֧�ֶԽ��豸AAC�汾
		0.19-20 buzalarm videox:inst_place
		0.18 �жϿ������ö�ͨ��ѡ�ѡ�������4ͨ���ط�ѡ��
		0.17 ���Ӹ�ʽ��������Ϣ
		0.16 �޸Ĳ��ܸ�ʽ������������
		0.15 �޸�webҳ��hd1��ʾ������������
		0.14 ���Ӱ�װ��ַѡ�����osd��׼��ֵ�ַ���
		0.13 www �����ط�û�ж�·��ѡ��
		0.12 ����multichannelѡ��
		0.11 www�޸�var status֧��ie	
		0.10 �޸�����bug ��make_json_str��ip1004.ini��û��ֵ����0������null
		0.09 ����a_channel�ֶ�֧��¼�����Ƶ
		0.08 ����ҳ�����ӹ��ܣ�Ӳ�̵���������ʽ����Ӳ�����·�������ʱ�����ʾ��־
 		0.07 �޸�����������rebooΪhwrbt
		0.06 ���Ӷ�cookies���ж�
		0.05 ���Ӷ�¼������������
		0.04 �����˲鿴��־���ܣ��޸���Ƶ�����з�Χ[0,15]
		0.03 ������֧����ҳ������Ƶ�����й���
			������֧���޸����ڵ����ƶ���������ȵ�����
		0.02 ֧�����µ��ֶ�bitratecon;netencoder:maxbitrate
			�޸����ϸ��汾֡�ʣ��ֱ��ʲ����޸ĵ�����
		0.01 �׷�
	*/
	const char VERSION[]="0.24";

	#define VER_FILE_LEN	50
typedef struct ERR_PARA
{
	char 	ipaddr;
	char	net_mask;
	char	default_gate;
	char	dns_svr;
	char	rmt_gate1;
	char	rmt_gate2;
	char	rmt_gate3;
	char	rmt_gate4;
	char	alarm_server;
	char	cmd_port;
	char	video_port;
	char	audio_port;
	char	com0_port;
	char	com1_port;
	char	file_port;
	char	pasv_port;
	char	telnet_port;
	
}err_para;
	
typedef struct PROG_VERSION
{
	char ipmain[VER_FILE_LEN];
	char rtimage[VER_FILE_LEN];
	char cgi[VER_FILE_LEN];
	char encbox[VER_FILE_LEN];
}prog_version;
#endif

