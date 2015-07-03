/*
********************************************
�ļ����ƣ�system_para_set.c
��д�ߣ� lsk
��д���ڣ�2006-9-12
��Ҫ������������ö�ȡϵͳ�����Ͳ�Ʒ��Ϣ�Ĺ���
�޸ļ�¼��
�޸���־��
*******************************************
*/

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <file_def.h>
#include <gtthread.h>
#include <commonlib.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>

#include "nv_pair.h"
#include "gt_dev_api.h"
#include "confparser.h"
#include "devinfo.h"
#include "iniparser.h"
#include "crc16.h"

#include "communication.h"
#include "system_para_set.h"
#include "system_test.h"

#define  IFCONFIG "ifconfig"
#define  ROUTE	  "route"

#define 	DEV_INFO_PATH 		"/conf/devinfo.ini"	// test should be /conf/devinfo.ini
#define 	DEV_CONF_PATH	 	"/conf/config"		//test  should be /conf/config
#define	IP1004_INI_PATH	"/conf/ip1004.ini"

////// paras in config file 
#define 	ETH0_IPADDR			"ETH0_IPADDR"
#define 	ETH0_NETMASK			"ETH0_NETMASK"
#define 	ETH0_MAC_ADDRESS		"MAC_ADDRESS"
#define 	ETH1_IPADDR			"ETH1_IPADDR"
#define 	ETH1_NETMASK			"ETH1_NETMASK"
#define 	ETH1_MAC_ADDRESS		"MAC1_ADDRESS"
#define	ROUTE_DEFAULT 			"ROUTE_DEFAULT"


/*
*********************************************
/////////paras in gt1000.ini file
#define INST_NAME	 	"inst_name"
#define FIRMWARE	 		"firmware"
#define CMD_PORT 		"cmd_port"
#define IMAGE_PORT	 	"image_port"
#define AUDIO_PORT	 	"audio_port"
#define FTP_PORT	 		"ftp_port"
#define TELNET_PORT	 	"telnet_port"
#define WEB_PORT	 		"web_port"
#define COM0_PORT	 	"com0_port"
#define COM1_PORT	 	"com1_port"
*********************************************
*/
#define PORT_INI				"port"
#define INSTALL_INI			"install"
#define INSTALL_PL			"inst_place"
#define DEV2_PORT_INI		"port_dev2"			//// lsk 2009 -2-10 for gtvs3022
#define DEV2_INSTALL_INI	"install_dev2"

// �������ļ��ĵ�ַ�����Դ��еõ������firmware��Ϣ
// �汾��Ϣ�ĸ�ʽΪfirmwareĿǰΪ��ʽΪvsmain-tcprtimg-hdmodule
#define	VSMAIN_LOCK_FILE	"/lock/ipserver/ipmain"
#define	TCPRT_LOCK_FILE	"/lock/ipserver/rtimage"
#define	HD_LOCK_FILE		"/lock/ipserver/hdmodule"
//static const char *eq_mark="==";	// equal mark in name value pair
//static const char *sep_mark="&&";	// seperator in name value pair


//sys_info �����洢���е�ϵͳ��������Ϣ
dev_info sys_info;
// config�ļ��Ĳ�������
confdict *conf_st=NULL;
FILE* conf_fp=NULL;
/*
*************************************************************************
*������	:open_config_file
*����	:��config�ļ� 
*����	:  ��
*���	: �������0 ��ȷ -1 ����
*�޸���־:
*************************************************************************
*/
int open_config_file(void)
{
	conf_st = confparser_load_lockfile(DEV_CONF_PATH, 1 , &conf_fp);
//	printf("%ld %ld !!!!!!!!\n",(long int )conf_st, (long int)(conf_fp));
	if (conf_fp == NULL)
	return -1;	
//	printf("config doc has %d lines \n", conf_st->lines);
	
	return 0;
}
/*
*************************************************************************
*������	:close_config_file
*����	:�ر�config�ļ� 
*����	:  ��
*���	: �������0 ��ȷ -1 ����
*�޸���־:
*************************************************************************
*/
int close_config_file(void)
{
//	printf(" !!!!!!!!!!%ld %ld \n" , (long int )conf_st,(long int)(conf_fp));
	confparser_dump_conf(DEV_CONF_PATH, conf_st, conf_fp);
//	printf(" !!!!!!!!!!%ld %ld \n" , (long int )conf_st,(long int)(conf_fp));
	if(conf_fp!=NULL)
	{
	    unlock_file(fileno(conf_fp));
            fsync(fileno(conf_fp));
            fclose(conf_fp);
      	}
//	fclose(conf_fp);
	confparser_freedict(conf_st);
	return 0;
}
/*
*************************************************************************
*������	:prinf_sys_info
*����	:��ӡ�豸��Ϣ 
*����	:  ��
*���	: �������0 ��ȷ -1 ����
*�޸���־:
*************************************************************************
*/
void prinf_sys_info(void)
{
#if 0
	printf("%s = %s \n", ETH0_IPADDR, sys_info.para.eth0_ip);
	printf("%s = %s \n", ETH0_NETMASK, sys_info.para.eth0_mask);
	printf("%s = %s \n", ETH0_MAC_ADDRESS, sys_info.para.eth0_mac);
	printf("%s = %s \n", ETH1_IPADDR, sys_info.para.eth1_ip);
	printf("%s = %s \n", ETH1_NETMASK, sys_info.para.eth1_mask);
	printf("%s = %s \n", ETH1_MAC_ADDRESS, sys_info.para.eth1_mac);
	printf("%s = %s \n", ROUTE_DEFAULT, sys_info.para.def_route);
	
	printf("%s = %d \n", ETH_NUM, sys_info.eth_num);
	printf("%s = %s \n", FIRMWARE, sys_info.firmware);
	printf("%s = %s \n", DEV_TYPE, sys_info.fac.dev_type);
	printf("%s = %s \n", LEAVE_FAC, sys_info.fac.leave_fac);
	printf("%s = %s \n", GUID, sys_info.fac.guid);
	printf("%s = %s \n", BOARD_SEQ, sys_info.fac.board_seq);
	printf("%s = %s \n", BATCH_SEQ, sys_info.fac.batch_seq);

	printf("%s = %d \n", CMD_PORT, sys_info.cmd_port);
	printf("%s = %d \n", AUDIO_PORT, sys_info.audio_port);
	printf("%s = %d \n", IMAGE_PORT, sys_info.image_port);
	printf("%s = %d \n", FTP_PORT, sys_info.ftp_port);
	printf("%s = %d \n", TELNET_PORT, sys_info.telnet_port);
	printf("%s = %d \n", COM0_PORT, sys_info.com0_port);
	printf("%s = %d \n", COM1_PORT, sys_info.com1_port);
	printf("%s = %d \n", WEB_PORT, sys_info.web_port);
#endif	
}

/*
*************************************************************************
*������	:get_config_info
*����	: 	��config�ļ��л�ȡ�ַ���
*����	:  confdict *dist ��config�ļ���õ�ָ��
			BYTE* info  ��Ϣ�ؼ���
			BYTE* ret  ���ض�������Ϣ�ַ���
*���	: ��
*�޸���־:
*************************************************************************
*/
void get_config_info(confdict *dist, BYTE* info , BYTE* ret)
{
	BYTE *buf;
	buf= NULL;
	buf = confparser_getstring(dist, info, NULL);
	if(buf==NULL)
		return;
	//// lsk 2009-1-14 !!!!!!
	if(strlen(buf)<BUF_LEN)		
	memcpy(ret, buf, strlen(buf));
	else
	{
		gtlogerr("error data in %s",info);
		test_log_info("error data in %s",info);
	}
	return;
}
/*
*************************************************************************
*������	:default_value_set
*����	:set default value to string
*����	:  unsigned char *info 
*���	: no
*�޸���־:
*************************************************************************
*/
void default_value_set(unsigned char *info)
{
	if(info== NULL)
		return;
	if(info[0]!='\0')
	{
//		printf("%s\n",info);
		return;
	}
	info[0]='0';
//	printf("%s\n",info);
}
/*
*************************************************************************
*������	:get_sys_config
*����	:��config �ļ��Ľ��������л�ȡ��Ϣ 
*����	:  ��
*���	: 0 ��ȷ  �����˳�
*�޸���־:
*************************************************************************
*/
int get_sys_config(void)
{
//	confdict *conf_st= NULL;
//	FILE* conf_fp=NULL;
#if 0
	conf_st = confparser_load_lockfile(DEV_CONF_PATH, 1 , &conf_fp);
//	printf("%ld %ld !!!!!!!!\n",(long int )conf_st, (long int)(conf_fp));
	if (conf_fp == NULL)
	return -1;	
	printf("config doc has %d lines \n", conf_st->lines);
#endif
	memset(&sys_info, 0 , sizeof(dev_info));
	if(open_config_file()<0)  // ��config �ļ������ļ�����
	{
		gtlogerr("devset ģ��:open config fail\n");		
		return -1;		
//		exit(1);
	}
	// clear struct to get new paras from documents
	//get system information from config file
	get_config_info(conf_st, ROUTE_DEFAULT, sys_info.para.def_route);		
	default_value_set(sys_info.para.def_route);	//lsk 2007-6-6
	get_config_info(conf_st, ETH0_IPADDR, sys_info.para.eth0_ip);
	default_value_set(sys_info.para.eth0_ip);	//lsk 2007-6-6
	get_config_info(conf_st, ETH0_NETMASK, sys_info.para.eth0_mask);
	default_value_set(sys_info.para.eth0_mask);	//lsk 2007-6-6
	get_config_info(conf_st, ETH0_MAC_ADDRESS, sys_info.para.eth0_mac);
	default_value_set(sys_info.para.eth0_mac);	//lsk 2007-6-6
	get_config_info(conf_st, ETH1_IPADDR, sys_info.para.eth1_ip);
	default_value_set(sys_info.para.eth1_ip);	//lsk 2007-6-6
	get_config_info(conf_st, ETH1_NETMASK, sys_info.para.eth1_mask);
	default_value_set(sys_info.para.eth1_mask);	//lsk 2007-6-6
	get_config_info(conf_st, ETH1_MAC_ADDRESS, sys_info.para.eth1_mac);
	default_value_set(sys_info.para.eth1_mac);	//lsk 2007-6-6

#if 0
	printf(" !!!!!!!!!!%ld %ld \n" , (long int )conf_st,(long int)(conf_fp));
	confparser_dump_conf(DEV_CONF_PATH, conf_st, conf_fp);
	printf(" !!!!!!!!!!%ld %ld \n" , (long int )conf_st,(long int)(conf_fp));
	if(conf_fp!=NULL)
	{
	    unlock_file(fileno(conf_fp));
            fsync(fileno(conf_fp));
            fclose(conf_fp);
      }

//	fclose(conf_fp);
	confparser_freedict(conf_st);
#endif
	close_config_file();	//�ͷ��ļ�������
	return 0;
}
/*
*************************************************************************
*������	:parse_ini_File
*����	:gt1000.ini �ļ��Ľ������� 
*����	:   ini �ļ���
*			channel �豸��	
*���	: �������0 ��ȷ -1 ����
*�޸���־:	lsk Feb 2009 ���Ӷ������豸��֧��
*************************************************************************
*/
int parse_ini_File(char * ini_name, int channel)
{
	dictionary	*	ini = NULL;
	char inibuf[BUF_LEN*2];
	char str[20];
	char *s=NULL;
	DEV_DIFF_PARA *dev=NULL;

	ini = iniparser_load(ini_name);
	if (ini==NULL) {
//		fprintf(stderr, "cannot parse file [%s]", ini_name);
		gtlogerr("devset ģ��:cannot parse file [%s]\n", ini_name);		
		return -1;
	}
	memset(str,0,sizeof(str));
	memset(inibuf, 0 , sizeof(inibuf));


	if(channel==0)	//// lsk 2009-2-10 for gtvs3022
	{
		memcpy(str,PORT_INI,strlen(PORT_INI));
		dev=&sys_info.dev;
		sprintf(inibuf, "%s:%s", INSTALL_INI, INSTALL_PL);
	}
	else
	{
		memcpy(str,DEV2_PORT_INI,strlen(DEV2_PORT_INI));
		dev=&sys_info.dev1;
		sprintf(inibuf, "%s:%s", DEV2_INSTALL_INI, INSTALL_PL);
	}
	s = iniparser_getstr(ini, inibuf);
	if(s!=NULL)
	{	////////lsk 2009 -1-14   ������50���ֽ���!!!!!!!! 
		if(strlen(s)<50)
		memcpy(dev->inst_name, s , strlen(s));
		else
		memcpy(dev->inst_name,s,50);
	}

	memset(inibuf, 0 , sizeof(inibuf));
	sprintf(inibuf, "%s:%s", str, WEB_PORT);
	dev->web_port= iniparser_getint(ini, inibuf, 8094); 
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, CMD_PORT);
	dev->cmd_port = iniparser_getint(ini, inibuf, 8095); 
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, IMAGE_PORT);
	dev->image_port= iniparser_getint(ini, inibuf, 8096);
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, AUDIO_PORT);
	dev->audio_port= iniparser_getint(ini, inibuf, 8097); 
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, COM0_PORT);
	dev->com0_port= iniparser_getint(ini, inibuf, 8098);
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, COM1_PORT);
	dev->com1_port= iniparser_getint(ini, inibuf, 8099); 
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, FTP_PORT);
	dev->ftp_port= iniparser_getint(ini, inibuf, 21); 
	memset(inibuf, 0 , sizeof(inibuf));

	sprintf(inibuf, "%s:%s", str, TELNET_PORT);
	dev->telnet_port= iniparser_getint(ini, inibuf, 23);
	memset(inibuf, 0 , sizeof(inibuf));
	
	iniparser_freedict(ini);
	return 0 ;
}
/*
*************************************************************************
*������	:get_sys_firmware
*����	:gt1000.ini �ļ��Ľ������� 
*����	:  char* firmware �洢firmware�Ļ�����ָ��
*���	: �������0 ��ȷ -1 ����
*�޸���־:
*************************************************************************
*/
int get_sys_firmware(char * firmware)
{
	BYTE*  ver=NULL;
	ver = get_prog_ver();
//	printf("%s\n",ver);
	memcpy(firmware,ver, strlen(ver));
#if 0
	BYTE  buf[256];
	int len;
	len = 0;
	memset(buf, 0 ,256);
/// get firmware
	if(0>get_prog_version(&buf[len], VSMAIN_LOCK_FILE))
	{
		printf("error get version %s\n", VSMAIN_LOCK_FILE);
//remed info by shixin
		//gtlogerr("devset ģ��: error get version %s\n", VSMAIN_LOCK_FILE);		
		strncpy(&buf[len+1], "0.00", 4);
//		return -1;
	}
//	printf("%s\n",buf);
	len =strlen(buf);
	buf[len]='-';
	if(0>get_prog_version(&buf[len+1], TCPRT_LOCK_FILE))
	{
		printf("error get version %s\n", TCPRT_LOCK_FILE);
//		gtlogerr("devset ģ��: error get version %s\n", TCPRT_LOCK_FILE);
		strncpy(&buf[len+1], "0.00", 4);
//		return -1;
	}
//	printf("%s\n",buf);
	len =strlen(buf);
	buf[len]='-';
	if(0>get_prog_version(&buf[len+1], HD_LOCK_FILE))
	{
		printf("error get version %s\n", HD_LOCK_FILE);
//		gtlogerr("devset ģ��: error get version %s\n", HD_LOCK_FILE);		
		strncpy(&buf[len+1], "0.00", 4);
//		return -1;
	}
//	printf("%s\n",buf);
	len =strlen(buf);
//	printf("%s  len = %d\n", buf, len);
	memcpy(firmware, buf, len);
#endif
	return 0;
}
/*
*************************************************************************
*������	:get_dev_info
*����	:devinfo.ini �ļ��Ľ������� 
*����	:  ��
*���	: �������0 ��ȷ -1 ����
*�޸���־: lsk 2009-2-10 for gtvs3022
*************************************************************************
*/
int get_dev_info(void)
{
	BYTE*s;
	struct tm *lvfac_time;

	if(init_devinfo()<0)
	{
		gtlogerr("devset ģ��: init devinfo error\n");		
		return -1;
	}

	lvfac_time = NULL;
	s=NULL;
///////lsk 2009 -2-10
	if(virdev_get_virdev_number()==2)
	{
		s= virdev_get_devid_str(0);
		if(s)
		memcpy(sys_info.fac.guid , s , strlen(s));
		else
		gtlogerr("get guid error");
		s= virdev_get_devid_str(1);
		if(s)
		memcpy(sys_info.fac.guid1 , s , strlen(s));  
		else
		gtlogerr("get guid error");
	}
	else
	{
		s=get_devid_str();
		memcpy(sys_info.fac.guid , s , strlen(s));  
	}
	////////////////////////////  lsk 2009-7-8  �ֽ�������14λ
	lvfac_time =get_lvfac_time();
	sprintf(sys_info.fac.leave_fac,"%04d%02d%02d%02d%02d%02d", 
		(1900+lvfac_time->tm_year), (lvfac_time->tm_mon+1),
		lvfac_time->tm_mday, lvfac_time->tm_hour,
		lvfac_time->tm_min, lvfac_time->tm_sec);
#if 0
	printf("%d %d %d %d %d %d\n", (1900+lvfac_time->tm_year),
		(lvfac_time->tm_mon+1), lvfac_time->tm_mday, lvfac_time->tm_hour,
		lvfac_time->tm_min, lvfac_time->tm_sec);
#endif
	s=get_devtype_str();
	memcpy(sys_info.fac.dev_type , s , strlen(s));

	sys_info.eth_num = get_eth_num();		// get number of net port

	s=get_board_seq();					
	memcpy(sys_info.fac.board_seq , s , strlen(s));  
	
	s = get_batch_seq();
	memcpy(sys_info.fac.batch_seq , s , strlen(s));  
		
	return 0;
}
/*
*************************************************************************
*������	:init_sys_paras
*����	:��ʼ��ϵͳ���� 
 *����:  ��
*���	: �������: 0 ��ȷ,  -1 ����
*�޸���־:
*************************************************************************
*/
int init_sys_paras(void)
{
	int ret=0;
	if(get_sys_config()<0)
		ret = -1;
	if(get_dev_info())
		ret = -1;
	if(get_sys_firmware(sys_info.firmware))
		ret = -1;
	if(parse_ini_File(IP1004_INI_PATH,0))
	ret = -1;
	if(virdev_get_virdev_number()==2)
	{
		if(parse_ini_File(IP1004_INI_PATH,1))
		ret = -1;
	}

//	printf("%s %s\n",sys_info.fac.guid, sys_info.fac.guid1);
	return ret;
}
/*
*****************************************************
*��������: reset_network
*��������: ���������豸�������鲥��ַ
*���룺 
*			
*����� 
*�޸���־��
*****************************************************
*/ 
void reset_network(int index)
{
	unsigned char buf[200];

	memset(buf, 0 , sizeof(buf));
	if(index==ETH0)
	{
		
//		sprintf(buf , "route add -net %s netmask %s dev eth0 \n",DEF_MULT_ADDR0,DEF_MULT_ADDR0);
//		sprintf(buf , "/sbin/route add -net %s netmask %s dev eth0 \n",DEF_MULT_ADDR,DEF_MULT_NETMASK);
//		system(buf);	//lsk 2007 -6 -13
//		memset(buf, 0 , sizeof(buf));
		sprintf(buf , "/sbin/route add -net %s netmask %s dev eth0 \n",DEF_MULT_ADDR0,DEF_MULT_NETMASK);	//lsk 2007-2-2
		printf(buf);	//lsk 2009-1-13 test
		system(buf);	//lsk 2007 -6 -13
	}
	if(index==ETH1)
	{
//		sprintf(buf , "route add -net %s netmask %s dev eth1 \n", DEF_MULT_ADDR1, DEF_MULT_ADDR1);
//		sprintf(buf , "/sbin/route add -net %s netmask %s dev eth1 \n",DEF_MULT_ADDR,DEF_MULT_NETMASK);
//		system(buf);	//lsk 2007 -6 -13
//		memset(buf, 0 , sizeof(buf));
		sprintf(buf , "/sbin/route add -net %s netmask %s dev eth1 \n", DEF_MULT_ADDR1, DEF_MULT_NETMASK);
		printf(buf);	//// lsk 2009 -1-13 test 
		system(buf);	//lsk 2007 -6 -13
	}
//	sleep(1);
//	system(buf);
}
/*
*****************************************************
*��������: init_net_port
*��������: ��ʼ����������
*���룺 host_name host_name1 �����鲥��ַ
*			port �鲥�˿ں�
*			net_st ���������������ݽṹ
*����� 0 ��ȷ  �����˳�����
*�޸���־��
*****************************************************
*/ 
 int init_net_port(char* host_name, int port, multicast_sock* net_st)
{
	int ret = -1;
	if(net_st==NULL)
	{
		return -1;
	}

	//construct socket
	net_st->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(net_st->fd == -1)
	{
		perror("socket");
		return -1;
	}
//		 printf("socket fd = %d\n ", net_port0.fd);

#if 1
	net_st->enc_type = 0 ;
	memset(net_st->self_id, 0 , sizeof(net_st->self_id));
	memset(net_st->target_id, 0 , sizeof(net_st->target_id));
	memset(net_st->seq_num, 0 , sizeof(net_st->seq_num));
	//
	bzero(&net_st->send_addr,sizeof(net_st->send_addr));
	bzero(&net_st->recv_addr,sizeof(net_st->recv_addr));
#endif
	if(host_name!=NULL)
	{
		net_st->recv_addr.sin_family = AF_INET;
		net_st->recv_addr.sin_addr.s_addr = inet_addr(host_name); //htonl(INADDR_ANY);
		net_st->recv_addr.sin_port =htons(port);
	}
#if 1
	//bind socket 
	if(bind(net_st->fd, (struct sockaddr *)&net_st->recv_addr, 
		sizeof(net_st->recv_addr)) == -1)
	{
		perror("bind error");
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
#endif
	//setsocketopt
	net_st->loop = 1;
	if(setsockopt(net_st->fd,SOL_SOCKET,SO_REUSEADDR,	
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:SO_REUSEADDR");
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
	// do not receive packet sent by myself
	net_st->loop = 0;	// test loop =0 no cycle , loop =1 cycle enable  
	if(setsockopt(net_st->fd,IPPROTO_IP,IP_MULTICAST_LOOP, 
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:IP_MULTICAST_LOOP");
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}

//join one multicast group
	if(host_name!=NULL)
	{
//		reset_network(ETH1);
//		reset_network(ETH0);
		net_st->mreq.imr_multiaddr.s_addr = inet_addr(host_name);
		net_st->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
//		net_st->mreq.imr_interface.s_addr = htonl(port);
		if(net_st->mreq.imr_multiaddr.s_addr == -1)
		{
			perror("not a legal multicast address!");
			goto error_handdle;	// lsk 2006 -12 -28
//			return -1;
		}
// lsk 2006 -12 -28
#if 1
		if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
			&net_st->mreq , sizeof(net_st->mreq))<0)
		{
			reset_network(ETH1);
			reset_network(ETH0);
			if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
				&net_st->mreq , sizeof(net_st->mreq))<0)
			{
				perror("setsockopt:IP_ADD_MEMBERSHIP");
				goto error_handdle;	// lsk 2006 -12 -28
//				return -1;
			}
		}
#endif
	}
	// set GUID as self ID
	if(virdev_get_virdev_number()==2)
	{
		ret =virdev_get_devid(0,net_st->self_id);
		if(ret!=DEV_GUID_BYTE)
		{
			printf("can not get GUID ret = %d \n", ret);
			goto error_handdle;	// lsk 2006 -12 -28
		}
		ret =virdev_get_devid(1,net_st->self_id1);
		if(ret!=DEV_GUID_BYTE)
		{
			printf("can not get GUID ret = %d \n", ret);
			goto error_handdle;	// lsk 2006 -12 -28
		}
	}
	else
	{
		ret = pass_dev_GUID(net_st->self_id);	//test
		if(ret!=DEV_GUID_BYTE)
		{
			printf("can not get GUID ret = %d \n", ret);
			goto error_handdle;	// lsk 2006 -12 -28
		}
	}
//	memcpy(net_st->hostname,host_name,strlen(host_name));
//	net_st->multi_port=port;
	memcpy((void*)&net_st->send_addr, (void*)&net_st->recv_addr,sizeof(net_st->recv_addr));
	 return 0;
error_handdle: 
	close(net_st->fd);
	net_st->fd = -1;
	return -1;
}
/*
*****************************************************
*��������: set_sys_IP
*��������: �����豸������IP
*���룺 int net_adp_index  ������
*			unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ -1 ����
*�޸���־��
*****************************************************
*/ 
int set_sys_IP(int net_adp_index, unsigned char *cmd )
{
	BYTE buf[100];
	int ret =0;
	open_config_file();
	memset(buf, 0 ,100);
	printf("%s\n",cmd);
 ///// set  IP
 	if(net_adp_index==0)
 	{
 		sprintf(buf, "%s %s \n", "/sbin/ifconfig eth0  " , cmd);
		printf("%s",buf);	//test
		ret = system(buf); // ifconfig eth0  [IP]
		if(ret)
			ret=-1;
		else
		{
		 	memset(sys_info.para.eth0_ip, 0, sizeof(sys_info.para.eth0_ip));
			memcpy(sys_info.para.eth0_ip, cmd, strlen(cmd)); 	
			confparser_setstr(conf_st, ETH0_IPADDR, cmd);
		}
	}
 	if(net_adp_index==1)
	{
		sprintf(buf, "%s %s \n", "/sbin/ifconfig eth1  " , cmd);
		printf("%s",buf);	//test
 		ret = system(buf); // ifconfig eth1  [IP]
		if(ret)
			ret=-1;
		else
		{
		 	memset(sys_info.para.eth1_ip, 0, sizeof(sys_info.para.eth1_ip));
			memcpy(sys_info.para.eth1_ip, cmd, strlen(cmd)); 	
			confparser_setstr(conf_st, ETH1_IPADDR, cmd);
		}
	}

	close_config_file();
	reset_network(net_adp_index);
	return ret;
} 

/*
*****************************************************
*��������: set_sys_netmask
*��������: �����豸����������
*���룺 int net_adp_index  ������
*			unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ -1 ����
*�޸���־��
*****************************************************
*/
int set_sys_netmask(int net_adp_index,  unsigned char *cmd)
{
	BYTE buf[100];
	int ret=0;
	open_config_file();
	memset(buf, 0 ,100);
	printf("%s\n",cmd);
 ///// set netmask IP
 	if(net_adp_index==0)
 	{
		sprintf(buf, "%s %s \n", "/sbin/ifconfig eth0 netmask " , cmd);
 		printf("%s",buf);	//test
		ret = system(buf); // ifconfig eth0 netmask [IP]
		if(ret)
			ret=-1;
		else
		{
		 	memset(sys_info.para.eth0_mask, 0, sizeof(sys_info.para.eth0_mask));
			memcpy(sys_info.para.eth0_mask, cmd, strlen(cmd)); 	
			confparser_setstr(conf_st, ETH0_NETMASK, cmd);
		}

 	}
 	if(net_adp_index==1)
	{
 		sprintf(buf, "%s %s \n", "/sbin/ifconfig eth1 netmask " , cmd);
 		printf("%s",buf);	//test
		ret = system(buf); // ifconfig eth1 netmask [IP]
		if(ret)
			ret=-1;
		else
		{
		 	memset(sys_info.para.eth1_mask, 0, sizeof(sys_info.para.eth1_mask));
			memcpy(sys_info.para.eth1_mask, cmd, strlen(cmd)); 	
			confparser_setstr(conf_st, ETH1_NETMASK, cmd);
		}
 	}
 //    error process
	close_config_file(); 	
//	reset_network(net_adp_index);
	return ret;
} 
/*
*****************************************************
*��������: set_sys_mac_addr
*��������: �����豸������MAC��ַ
*���룺 int net_adp_index  ������
*			unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ -1 ����
*�޸���־��2007 -11-1����ʱ����mac��ַ
*****************************************************
*/
int set_sys_mac_addr(int net_adp_index,  unsigned char *cmd)
{
	BYTE buf[200];
	open_config_file();			
	memset(buf, 0 ,sizeof(buf));
 	printf("%s\n",cmd);
///// set MAC address	mod lsk 2007 -11-1����ʱ����mac��ַ
 	if(net_adp_index==ETH0)
 	{
#if 0
		system("/sbin/ifconfig eth0 down\n");	//close net port
 		sprintf(buf, "%s %s\n", "/sbin/ifconfig eth0 hw ether" , cmd);
 		printf("%s", buf);
 		system(buf); // ifconfig eth0 hw [IP]
 		system("/sbin/ifconfig eth0 up\n");	//open net port
 #endif		
	 	memset(sys_info.para.eth0_mac, 0, sizeof(sys_info.para.eth0_mac));
		memcpy(sys_info.para.eth0_mac, cmd, strlen(cmd));	// record new mask address 	
		confparser_setstr(conf_st, ETH0_MAC_ADDRESS, cmd);	// write down to config file
 	}
 	if(net_adp_index==ETH1)
	{
#if 0
 		system("ifconfig down eth1 \n");
		sprintf(buf, "%s %s \n", "ifconfig eth1 hw ether" , cmd);
 		system(buf); // ifconfig eth1 hw [IP]
 		system("ifconfig up eth1 \n");
#endif
	 	memset(sys_info.para.eth1_mac, 0, sizeof(sys_info.para.eth1_mac));
		memcpy(sys_info.para.eth1_mac, cmd, strlen(cmd)); 	
		confparser_setstr(conf_st, ETH1_MAC_ADDRESS, cmd);
 	}
	close_config_file();
//	reset_network(ETH0);
//	reset_network(ETH1);
	return 0;
} 
/*
*****************************************************
*��������: set_sys_gateway
*��������: �����豸����������
*���룺unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ -1 ����
*�޸���־��
*****************************************************
*/
int set_sys_gateway( unsigned char *cmd)
{
//	int ret;
	BYTE buf[100];

	open_config_file();

	memset(buf, 0, 100);
	printf("%s\n",cmd);
// remove old gateway address 
	if(strlen(sys_info.para.def_route))
	{
		sprintf(buf , "%s %s eth0\n", "/sbin/route del default gw " , sys_info.para.def_route);
		system(buf);
		memset(buf, 0, 100);
	}
//��¼�µ�����
 	memset(sys_info.para.def_route, 0, sizeof(sys_info.para.def_route));
	memcpy(sys_info.para.def_route, cmd, strlen(cmd));
 ///// set gateway address
 	sprintf(buf, "%s %s eth0\n" , "/sbin/route add default gw", cmd);
 	system(buf); //  route add gw [IP] 
	confparser_setstr(conf_st, ROUTE_DEFAULT, cmd);
	close_config_file();
	return 0;	
}

/*
*****************************************************
*��������: pass_sys_gateway
*��������: ��ȡ�豸����������
*���룺unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ -1 ����
*�޸���־��
*****************************************************
*/
int pass_sys_gateway( unsigned char *cmd)
{
 	memcpy(cmd, sys_info.para.def_route, strlen(sys_info.para.def_route));
	return 0;
}

/////////////////////ifconfig eth0 hw ether MAC��ַ

/*
*****************************************************
*��������: atoi_time
*��������: ���ַ�����ʱ��[yyyymmddhhmmss]
				ת��������ʱ�䷵��struct tm
*���룺 BYTE* time_byte ʱ���ַ���
				struct tm *time_int ת��������ݽṹָ��
*����� ��
*�޸���־��
*****************************************************
*/
void atoi_time(BYTE* time_byte, struct tm *time_int)
{
	int i;
	sys_time_set time_ascii;

	memset ((BYTE*)&time_ascii, 0 ,sizeof(time_ascii));
#if 1
///// lsk 2009 -1-13
	if(strlen(time_byte)<14)
	{
		for(i=strlen(time_byte);i<14;i++)
		{
			time_byte[i]='0';
		}
		time_byte[i]=0;
	}
////����14λ���Զ�����'0'	
#endif
	i=0;
	memcpy(time_ascii.year, &time_byte[i], 4);
	i+=4;
	memcpy(time_ascii.mon, &time_byte[i], 2);
	i+=2;
	memcpy(time_ascii.day, &time_byte[i], 2);
	i+=2;
	memcpy(time_ascii.hour, &time_byte[i], 2);
	i+=2;
	memcpy(time_ascii.min, &time_byte[i], 2);
	i+=2;
	memcpy(time_ascii.sec, &time_byte[i], 2);
	time_int->tm_year 	= atoi(time_ascii.year)-1900;
	time_int->tm_mon 	= atoi(time_ascii.mon)-1;
	time_int->tm_mday 	= atoi(time_ascii.day);
	time_int->tm_hour 	= atoi(time_ascii.hour);
	time_int->tm_min  	= atoi(time_ascii.min);
	time_int->tm_sec  	= atoi(time_ascii.sec);
}
/*
*****************************************************
*��������: set_produce_date
*��������: �����豸��������
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_produce_date(unsigned char *cmd)
{
	int ret;
	struct tm time_prd;
	memset(sys_info.fac.leave_fac, 0, sizeof(sys_info.fac.leave_fac));
	memcpy(sys_info.fac.leave_fac, cmd, strlen(cmd));
	atoi_time(cmd, &time_prd);
#if 1
	printf("%d %d %d %d %d %d\n", (1900+time_prd.tm_year),
		(time_prd.tm_mon+1), time_prd.tm_mday, time_prd.tm_hour,
		time_prd.tm_min, time_prd.tm_sec);
#endif
	ret = set_lvfac_time(&time_prd);
	return ret;
}
/*
*****************************************************
*��������: pass_produce_date
*��������: ��ȡ�豸��������
*���룺 struct tm *ptime  ʱ�����ݻ�����
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_produce_date(struct tm *ptime)
{
	atoi_time(sys_info.fac.leave_fac, ptime);
	return 0;
}

/*
*****************************************************
*��������: set_sys_date
*��������: �����豸ϵͳʱ��
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_sys_date(unsigned char *cmd)
{
	int ret;
	struct tm ntime;
	memset(sys_info.para.dev_time, 0, sizeof(sys_info.para.dev_time));
	memcpy(sys_info.para.dev_time, cmd, strlen(cmd));
	printf("%s\n",cmd);
	atoi_time(cmd, &ntime);
#if 1
	printf("%d %d %d %d %d %d\n", (1900+ntime.tm_year),
		(ntime.tm_mon+1), ntime.tm_mday, ntime.tm_hour,
		ntime.tm_min, ntime.tm_sec);
#endif
	
 	ret = set_dev_time(&ntime);
	return ret;
}
/*
*****************************************************
*��������: record_hd_serial_NO
*��������: �����豸ϵͳʱ��
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
void record_hd_serial_NO(unsigned char *cmd)
{
	memset(sys_info.HD_serial_NO, 0, sizeof(sys_info.HD_serial_NO));
	memcpy(sys_info.HD_serial_NO, cmd, strlen(cmd));
}

/*
*****************************************************
*��������: pass_sys_date
*��������: ��ȡ�豸ϵͳʱ��
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_sys_date(unsigned char *cmd)
{
	memcpy(cmd, sys_info.para.dev_time, strlen(sys_info.para.dev_time));
	return 0;
}

/*
*****************************************************
*��������: set_dev_type
*��������: �����豸ID
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_dev_type(unsigned char *cmd)
{
	int ret;
	memset(sys_info.fac.dev_type, 0, sizeof(sys_info.fac.dev_type));
	memcpy(sys_info.fac.dev_type, cmd , strlen(cmd));
//	printf("%s\n", sys_info.fac.dev_type);
	ret = set_devtype_str(cmd);
	return ret;
}
/*
*****************************************************
*��������: set_dev_GUID
*��������: �����豸GUID
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_dev_GUID(unsigned char *cmd)
{
	int ret;
	BYTE *s=NULL;
	ret = set_devid_str(cmd);
	if(ret<0)
		return ret;
	memset(sys_info.fac.guid, 0, sizeof(sys_info.fac.guid));
	memcpy(sys_info.fac.guid, cmd , strlen(cmd));
	if(virdev_get_virdev_number()==2)
	{
		s= virdev_get_devid_str(1);
		if(s)
		{
			memset(sys_info.fac.guid1, 0, sizeof(sys_info.fac.guid1));
			memcpy(sys_info.fac.guid1 , s , strlen(s));  
		}
		else
		gtlogerr("get guid err");
	}

//	printf("set guid ret = %d  guid len �� %d\n" , ret, strlen(cmd));
	return ret;
}
/*
*****************************************************
*��������: pass_dev_GUID
*��������: ����GUID
*���룺 
*����� unsigned char *cmd  ���guid �Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_GUID(unsigned char *cmd)
{
	return (get_devid(cmd));
}
/*
*****************************************************
*��������: pass_dev_type
*��������: ����dev_type
*���룺 
*����� unsigned char *cmd  ���dev_type �Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_type(unsigned char *cmd)
{
	memcpy(cmd,sys_info.fac.dev_type,strlen(sys_info.fac.dev_type));
	return 0;
}

/*
*****************************************************
*��������: pass_dev_IP
*��������: �����豸IP
*���룺 
*����� unsigned char *cmd  ���IP �Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_IP(unsigned char *cmd, int index)
{
	if(index==ETH0)
	memcpy(cmd,sys_info.para.eth0_ip,strlen(sys_info.para.eth0_ip));
	if(index==ETH1)
	memcpy(cmd,sys_info.para.eth1_ip,strlen(sys_info.para.eth1_ip));
	return 0;
}
/*
*****************************************************
*��������: pass_dev_netmask
*��������: �����豸��������
*���룺 
*����� unsigned char *cmd  �����������Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_netmask(unsigned char *cmd,  int index)
{
	if(index==ETH0)
	memcpy(cmd,sys_info.para.eth0_mask,strlen(sys_info.para.eth0_mask));
	if(index==ETH1)
	memcpy(cmd,sys_info.para.eth1_mask,strlen(sys_info.para.eth1_mask));
	return 0;
}
/*
*****************************************************
*��������: pass_dev_mac
*��������: ����mac
*���룺 
*����� unsigned char *cmd  ���mac��ַ �Ļ�����
			�������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_mac(unsigned char *cmd, int index)
{
	if(index==ETH0)
	memcpy(cmd,sys_info.para.eth0_mac, strlen(sys_info.para.eth0_mac));
	if(index==ETH1)
	memcpy(cmd,sys_info.para.eth1_mac, strlen(sys_info.para.eth1_mac));
	return 0;
}

static void dump_identity (const struct hd_driveid *id)
{
        printf("\n Model=%.40s, FwRev=%.8s, SerialNo=%.20s\n",
                id->model, id->fw_rev, id->serial_no);
	gtloginfo("Ӳ����Ϣ: Model=%.40s, FwRev=%.8s, SerialNo=%.20s\n",
	        id->model, id->fw_rev, id->serial_no);
	test_log_info("Ӳ����Ϣ: Model=%.40s, FwRev=%.8s, SerialNo=%.20s\n",
	        id->model, id->fw_rev, id->serial_no);
}

// ��ȡӲ�����кţ��ɹ�����0�� ʧ�ܷ���-1
// szDevNameΪӲ�̱�ʶ��
// szSNΪȡ�õ����кţ��ռ���������Ԥ�ȷ���ã�
//�����ռ䳤����nLimit�������ݸ�get_dev_hd_sn()����
int get_dev_hd_sn(const char* szDevName, char* szSN, size_t nLimit)
{
	struct hd_driveid id;
	int  nRtn = -1;                                                                                                              
	int  fd = open(szDevName, O_RDONLY|O_NONBLOCK);  

	if (fd < 0)
	{
		perror(szDevName);
		close(fd);
		return nRtn;
	}
//	printf("open %s ok\n",szDevName);
	if(!ioctl(fd, HDIO_GET_IDENTITY, &id))
	{
		strncpy(szSN, id.serial_no, nLimit);
		dump_identity(&id);
		record_hd_serial_NO(szSN);
		nRtn = 0;
	}
	else
	{
		printf("fail : ioctl(fd, HDIO_GET_IDENTITY, &id) \n");
		gtlogerr("can not get %s serial number\n", szDevName);
	}
	close(fd);
	return nRtn;
}

#if 0
/*
*****************************************************
*��������: pass_equal_mark
*��������: ���صȺ�
*���룺 
*����� unsigned char *  �Ⱥŵ�ָ��
*�޸���־��
*****************************************************
*/
const char * pass_equal_mark(void)
{
	return eq_mark;
}
/*
*****************************************************
*��������: pass_seperator
*��������: �����豸IP
*���룺 
*����� unsigned char *  �ָ��ŵ�ָ��
*�޸���־��
*****************************************************
*/
const char * pass_seperator(void)
{
	return sep_mark;
}
#endif
/*
*****************************************************
*��������: set_dev_batch_seq
*��������: �����豸�������α��
*���룺 
*����� 
*�޸���־��
*****************************************************
*/
int set_dev_batch_seq( unsigned char *cmd)
{
	int ret;
	memset(sys_info.fac.batch_seq, 0 , sizeof(sys_info.fac.batch_seq));
	memcpy(sys_info.fac.batch_seq, cmd , strlen(cmd));
	ret = set_batch_seq(cmd);
	return ret;
}
/*
*****************************************************
*��������: pass_dev_batch_seq
*��������: ��ȡ�����豸�������α��
*���룺 
*����� 
*�޸���־��
*****************************************************
*/
int pass_dev_batch_seq( unsigned char *cmd)
{
	memcpy(cmd , sys_info.fac.batch_seq, strlen(sys_info.fac.batch_seq));
	return 0;
}

/*
*****************************************************
*��������: set_dev_board_seq
*��������: �����豸�忨�������α��
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_dev_board_seq( unsigned char *cmd)
{
	int ret;
	memset(sys_info.fac.board_seq, 0 , sizeof(sys_info.fac.board_seq));
	memcpy(sys_info.fac.board_seq, cmd , strlen(cmd));
	ret = set_board_seq(cmd);
	return ret;
}
/*
*****************************************************
*��������: pass_dev_board_seq
*��������: ��ȡ�豸�忨�������α��
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int pass_dev_board_seq( unsigned char *cmd)
{
	memcpy(cmd , sys_info.fac.board_seq, strlen(sys_info.fac.board_seq));
	return 0;
}

/*
*****************************************************
*��������: handdle_search_dev
*��������: ���豸����������͸�������
*���룺 multicast_sock* ns  ������������Ĳ���
*			NVP_TP *nv ��ֵ�Դ�ŵĻ�����ָ��
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
//int handdle_search_dev(NVP_TP *dist, multicast_sock * ns)
int handdle_search_dev(multicast_sock * ns, int channel)	//// lsk 2009-2-4 testing
{
	const char* buf = NULL;
	int len = 0;
	int ret = 0;
	NVP_TP *dist = NULL;
	init_sys_paras();	// lsk 2006 -12- 28  

	//// create nvp struct to store name&value pairs
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"
	
////lsk 2007 -7 -5 
	if(strncasecmp(ns->seq_num, "0" , 1)==0)
	{////  old protocal
////lsk 2007 -5 -31 
		nvp_set_pair_str(dist, CMD_STR, M_DEV_INFO);		// cmd == M_DEV_INFO
	}
	else
	{////new protocal
		nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	// lsk 2007 -5 -31 cmd == ack
	//	nvp_set_pair_str(dist, SEQ_STR, ns->seq_num);	//lsk 2007 -2-14
		nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	//lsk 2007 -5-31
		nvp_set_pair_str(dist, ERR_STR, "no error");	//lsk 2007 -5-31
		nvp_set_pair_str(dist, RET_VAL, "0");	//lsk 2007 -5-31
	}

////end of change
	nvp_set_pair_str(dist, DEV_TYPE, 		sys_info.fac.dev_type);
	nvp_set_pair_str(dist, BATCH_SEQ, 	sys_info.fac.batch_seq);
	nvp_set_pair_str(dist, BOARD_SEQ, 	sys_info.fac.board_seq);
	nvp_set_pair_str(dist, LEAVE_FAC, 		sys_info.fac.leave_fac);
	
	nvp_set_pair_str(dist, ETH0_IP, 		sys_info.para.eth0_ip);
	nvp_set_pair_str(dist, ETH0_MASK, 		sys_info.para.eth0_mask);
	nvp_set_pair_str(dist, ETH0_MAC, 		sys_info.para.eth0_mac);
	if(sys_info.eth_num==2)
	{
		nvp_set_pair_str(dist, ETH1_IP, 		sys_info.para.eth1_ip);
		nvp_set_pair_str(dist, ETH1_MASK, 		sys_info.para.eth1_mask);
		nvp_set_pair_str(dist, ETH1_MAC, 		sys_info.para.eth1_mac);
	}
	nvp_set_pair_str(dist, DEF_ROUTE, 			sys_info.para.def_route);
	nvp_set_pair_str(dist, FIRMWARE, 			sys_info.firmware);
	nvp_set_pair_int(dist, ETH_NUM, 			sys_info.eth_num);
	
	nvp_set_pair_int(dist, UPDATING, 			sys_info.updating);
	nvp_set_pair_int(dist, FORMATTING, 		sys_info.formatting);
	nvp_set_pair_int(dist, BOARD_TESTING, 	 sys_info.board_testing);
	nvp_set_pair_int(dist, TRIG_TESTING, 		sys_info.trig_testing);

//////////  for gtvs 3022 channel 
	if(channel==0)  
	{
		nvp_set_pair_str(dist, GUID, 			sys_info.fac.guid);
		nvp_set_pair_int(dist, CMD_PORT, 		sys_info.dev.cmd_port);
		nvp_set_pair_int(dist, IMAGE_PORT, 	sys_info.dev.image_port);
		nvp_set_pair_int(dist, AUDIO_PORT, 	sys_info.dev.audio_port);
		nvp_set_pair_int(dist, FTP_PORT, 		sys_info.dev.ftp_port);
		nvp_set_pair_int(dist, TELNET_PORT, 	sys_info.dev.telnet_port);
		nvp_set_pair_int(dist, WEB_PORT, 		sys_info.dev.web_port);
		nvp_set_pair_int(dist, COM0_PORT, 	sys_info.dev.com0_port);
		nvp_set_pair_int(dist, COM1_PORT, 	sys_info.dev.com1_port);
		nvp_set_pair_str(dist, INST_NAME, 		sys_info.dev.inst_name);
	}
/////// change guid to channel 1
	else
	{
		nvp_set_pair_str(dist, GUID, 			sys_info.fac.guid1);
		nvp_set_pair_int(dist, CMD_PORT, 		sys_info.dev1.cmd_port);
		nvp_set_pair_int(dist, IMAGE_PORT, 	sys_info.dev1.image_port);
		nvp_set_pair_int(dist, AUDIO_PORT, 	sys_info.dev1.audio_port);
		nvp_set_pair_int(dist, FTP_PORT, 		sys_info.dev1.ftp_port);
		nvp_set_pair_int(dist, TELNET_PORT, 	sys_info.dev1.telnet_port);
		nvp_set_pair_int(dist, WEB_PORT, 		sys_info.dev1.web_port);
		nvp_set_pair_int(dist, COM0_PORT, 	sys_info.dev1.com0_port);
		nvp_set_pair_int(dist, COM1_PORT, 	sys_info.dev1.com1_port);
		nvp_set_pair_str(dist, INST_NAME, 		sys_info.dev1.inst_name);
	}
	
	buf = nvp_get_string(dist);
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	if(channel==1)
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id1, 
						(void*)buf , len, ns->enc_type, ns->flag);
	else
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	nvp_destroy(dist);
//	printf("ret = %d errno = %d %s\n", ret, errno, strerror(errno));

	return ret;
}
/*
*****************************************************
*��������: handdle_search_dev
*��������: ���豸����������͸�������
*���룺 multicast_sock* ns  ������������Ĳ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
void set_sys_flag(int index , int flag)
{
	switch(index)
	{
		case UPDATA_FLAG:
			sys_info.updating = flag;
			break;
			
		case TEST_TG_FLAG:
			sys_info.trig_testing = flag;
			break;
			
		case TEST_BD_FLAG:
			sys_info.board_testing = flag;
			break;
			
		case FORMAT_FLAG:
			sys_info.formatting = flag;
			break;
			
		case CLEAN_FLAG:
			sys_info.clearing = flag;
			break;
		case TEST_IDE_FLAG:
			sys_info.test_ide= flag;
			break;
			
		default:
			break;
	}
}
/*
*****************************************************
*��������: get_sys_flag
*��������: ��������������ѯ����״̬
*���룺 multicast_sock* ns  ������������Ĳ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int get_sys_flag(int index)
{
	switch(index)
	{
		case UPDATA_FLAG:
			return (sys_info.updating);
			break;
			
		case TEST_TG_FLAG:
			return (sys_info.trig_testing);
			break;
			
		case TEST_BD_FLAG:
			return (sys_info.board_testing);
			break;
			
		case FORMAT_FLAG:
			return (sys_info.formatting);
			break;

		case CLEAN_FLAG:
			return (sys_info.clearing);
			break;
		case TEST_IDE_FLAG:
			return(sys_info.test_ide);
			break;

		default:
			return 0;
			break;
	}
}
/*
*****************************************************
*��������: is_bussy
*��������: ��������״̬��ѯ
*���룺 
*�����
����ֵ:	0 Ŀǰû��������ִ��
			1 ��������ִ��
*�޸���־��
*****************************************************
*/
int is_bussy(void)
{
	return (sys_info.updating|sys_info.trig_testing|sys_info.formatting
			|sys_info.board_testing|sys_info.clearing);
}
#if 0
/*
*****************************************************
*��������: set_config_info
*��������: �����豸GUID
*���룺 unsigned char *cmd  ���õ��ַ���
*����� �������0 ��ȷ ��ֵ ����
*�޸���־��
*****************************************************
*/
int set_config_info(BYTE* info , BYTE* val)
{
	return 0;
}
#endif

