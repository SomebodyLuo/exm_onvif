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
#include <nv_pair.h>
#include<gt_dev_api.h>
#include<devinfo.h>

#include"testmod.h"
#include"net_mod.h"

static const char *eq_mark="==";	// equal mark in name value pair
static const char *sep_mark="&&";	// seperator in name value pair



/******************************************************
*��������: send_test_cmd
*��������: �����鲨����
*���룺 host_name host_name1 �����鲥��ַ
*			port �鲥�˿ں�
*			net_st ���������������ݽṹ
*����� 0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
int send_multicast_pkt(multicast_sock *net_st,char *buf)
{
	int ret = -1;
	int len = 0 ;
	
	len = strlen(buf);

	printf("send to %s\n", inet_ntoa(net_st->send_addr.sin_addr) );
	ret =send_dev_pkt(net_st->fd, &net_st->send_addr, net_st->target_id,
	net_st->self_id, buf, len , net_st->enc_type, net_st->flag);
	if(ret<0)
	{
		printf("ret =%d %s\n", ret, strerror(errno));
	}
	return ret;
}



/******************************************************
*��������: send_test_report
*��������: ���Ͳ��Խ��̲�����ʵʱ��Ϣ
*���룺 	
*		net_st ���������������ݽṹ
*		unsigned char* info	���Ե�ʵʱ��Ϣ
*		int prog ����0-100������
*����� 	0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
 int send_test_report(multicast_sock* ns, unsigned char* info,int prog)
{
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	int ret = 0;
	if(ns->fd<=0)
	return 0;

	dist = nvp_create();					//����һ�������ֵ�ԵĻ�����
	if(dist==NULL)
	{
		printf("can not use nv_pair lib \n");
		exit(1);
	}
	nvp_set_equal_mark(dist, eq_mark);	//������ֵ�Եĵ��ڷ���
	nvp_set_seperator(dist, sep_mark);		//������ֵ�Եķָ�����

//// lsk 2007 -6-1
#if 0
	nvp_set_pair_str(dist, CMD_STR, M_HD_PROG_RETURN);		// cmd == M_DEV_INFO
	nvp_set_pair_str(dist, SEQ_STR, ns->seq_num);	//lsk 2007 -2-14
	nvp_set_pair_int(dist, BDTEST_PROG, prog);
	nvp_set_pair_str(dist, BDTEST_INFO, info);
#endif
	nvp_set_pair_str(dist, CMD_STR, RPT_PKT);		// cmd == process_report
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	
	nvp_set_pair_int(dist, PROGRESS, prog);
	nvp_set_pair_str(dist, DETAIL_STR, info);

////end of change 
	buf = nvp_get_string(dist);
	ret =send_multicast_pkt(ns, (char*)buf);
#if 0
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
#endif
	nvp_destroy(dist);
//	printf("ret = %d errno = %d %s\n", ret, errno, strerror(errno));

	return ret;

	//return 0;
}



/******************************************************
*��������: init_dev_net_port
*��������: ��ʼ����������
*���룺 	
*		net_st ���������������ݽṹ
*����� 	0 ��ȷ  �����˳�����
*�޸���־��
******************************************************/ 
 int init_dev_net_port(multicast_sock* net_st)
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
//	memset(&net_st, 0 ,sizeof(&net_st));
#if 1
	net_st->enc_type = 0 ;
	memset(net_st->self_id, 0 , sizeof(net_st->self_id));
//	memset(net_st->target_id, 0 , sizeof(net_st->target_id));
	//
	bzero(&net_st->send_addr,sizeof(net_st->send_addr));
	bzero(&net_st->recv_addr,sizeof(net_st->recv_addr));
#endif
	if((strlen(net_st->hostname))!=0)
	{
		net_st->recv_addr.sin_family = AF_INET;
		net_st->recv_addr.sin_addr.s_addr = inet_addr(net_st->hostname); //htonl(INADDR_ANY);
		net_st->recv_addr.sin_port =htons(net_st->multi_port);
	}

#if 0
	//bind socket 
	if(bind(net_st->fd, (struct sockaddr *)&net_st->recv_addr, 
		sizeof(net_st->recv_addr)) == -1)
	{
		perror("bind error");
		goto error_handdle;	// lsk 2006 -12 -28
	}
#endif
	//setsocketopt
	net_st->loop = 1;
	if(setsockopt(net_st->fd,SOL_SOCKET,SO_REUSEADDR,	
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:SO_REUSEADDR");
		goto error_handdle;	// lsk 2006 -12 -28
	}
	// do not receive packet sent by myself
	net_st->loop = 0;	// test loop =0 no cycle , loop =1 cycle enable  
	if(setsockopt(net_st->fd,IPPROTO_IP,IP_MULTICAST_LOOP, 
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:IP_MULTICAST_LOOP");
		goto error_handdle;	// lsk 2006 -12 -28
	}
//join one multicast group
	if((strlen(net_st->hostname))!=0)
	{
		net_st->mreq.imr_multiaddr.s_addr = inet_addr(net_st->hostname);
		net_st->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if(net_st->mreq.imr_multiaddr.s_addr == -1)
		{
			perror("not a legal multicast address!");
			goto error_handdle;	// lsk 2006 -12 -28
		}
// lsk 2006 -12 -28
#if 1
		if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
			&net_st->mreq , sizeof(net_st->mreq))<0)
		{
			perror("setsockopt:IP_ADD_MEMBERSHIP");
			goto error_handdle;	// lsk 2006 -12 -28
		}
#endif
	}

	// set GUID as self ID
	ret = pass_dev_GUID(net_st->self_id);	//test
	if(ret!=DEV_GUID_BYTE)
	{
		printf("can not get GUID ret = %d \n", ret);
//		printbuffer(net_st->self_id , DEV_GUID_BYTE);
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
	//// test server guid
//	memcpy(net_st->target_id, server_GUID, sizeof(server_GUID));
	memcpy((void*)&net_st->send_addr, (void*)&net_st->recv_addr,
		sizeof(net_st->recv_addr));
//	memcpy(net_st->hostname, net_st->host_name,strlen(net_st->host_name));
//	sprintf(net_st->seq_num, "%s", "0");
	
	 return 0;
error_handdle: 
	close(net_st->fd);
	net_st->fd = -1;
	return -1;
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


