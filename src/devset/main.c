
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <file_def.h>
#include <commonlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "system_para_set.h"
#include "system_test.h"
#include "gt_dev_api.h"
#include "devinfo.h"
#include "nv_pair.h"
#include "communication.h"
#include "cmd_process.h"

#include <gtlog.h>
#include <gtthread.h>
#include <signal.h>

#define DEVSET_LOCK_FILE "/lock/ipserver/devset"
// ����ͨѶ�Ĳ������ݽṹ
multicast_sock net_port0; 
multicast_sock net_port1;
static pthread_mutex_t	cmd_process_mutex=PTHREAD_MUTEX_INITIALIZER;		//�����ڽ��մ������ݵĻ�����

void printbuffer(char *buf, int len)
{
	int i=0;
	int j=0;

	for(i=0;i<len;i++)
	{
		printf("%02x",buf[i]&0xff);
		j++;
		if(j>=16)
		{
			j=0;
			printf("\n");
		}
	}
	printf("\n");
}
/*
*************************************************************************
*������	:netport_thread
*����	:�������ݽ������߳�
 *����	:multicast_sock* nt ����ӿڽ��շ�����������Ĳ���
*���	:��
*�޸���־:
*************************************************************************
*/
void netport_thread(multicast_sock* nt)
{
	int ret;
	NVP_TP *dist = NULL;
	char rcv_buf[CMD_BUF_LEN];
	
	dist = nvp_create();					//����һ�������ֵ�ԵĻ�����
	nvp_set_equal_mark(dist, eq_mark);	//������ֵ�Եĵ��ڷ���
	nvp_set_seperator(dist, sep_mark);	//������ֵ�Եķָ�����

	if(dist==NULL)
	{
		printf("can not use nv_pair lib\n");
		gtlogerr("devset ģ��:nv_pair lib�޷�ʹ���˳�\n");		
		pthread_exit(NULL);
	}
	ret = init_net_port(nt->hostname, nt->multi_port, nt);
	while(ret)
	{
		sleep(5);
		
		reset_network(ETH0);// lsk 2007 -6-13 
		if(get_eth_num()==2)// lsk 2007 -6-13 
		reset_network(ETH1);// lsk 2007 -6-13 
		perror("init net port:");
		ret = init_net_port(nt->hostname, nt->multi_port, nt);
	}
	printf("init net port ok\n");
	gtloginfo("start server thread for %s -> %d\n", nt->hostname, nt->multi_port);

	while(1)
	{
      	//��ȡ����
      		//// lsk 2009 -2-10 for gtvs3022
		  if(virdev_get_virdev_number()==2)
			ret =dual_id_recv_dev_pkt(nt->fd, &nt->recv_addr, nt->self_id, nt->self_id1,
							nt->target_id, nt->recv_id, rcv_buf, sizeof(rcv_buf)-1,
							&nt->enc_type, nt->flag);			
		  else
			ret = recv_dev_pkt(nt->fd, &nt->recv_addr, nt->self_id, nt->target_id,
							rcv_buf, sizeof(rcv_buf)-1, &nt->enc_type, nt->flag);
		if(ret <0)
		{
			perror("recvfrom");
			gtlogerr("recv_dev_pkt error ret=%d\n",ret);
		}
		else	
		{
		//�������յ����������ݰ�
			rcv_buf[ret]='\0';
			if(nvp_parse_string(dist, rcv_buf)<0)
			{
 				printf(" can not parse packet \n");
				gtlogerr("devset ģ��:�յ��޷������������\n");		
			}
			else
			{
				//������������
				pthread_mutex_lock(&cmd_process_mutex);
//				printf("net port get data !!!!!!!!!!!");
				cmd_handdle(dist , nt);// �����
				pthread_mutex_unlock(&cmd_process_mutex);
			}
	//		printbuffer(buf, ret);
	//	��ս��ն���
			memset(rcv_buf , 0 ,sizeof(rcv_buf));
		}	
	}
	nvp_destroy(dist);	//// lsk 2009-5-14 
}

/** 
 *   @brief     ����־�ϼ�¼�˳�����
 *   @param  signo �ź������
 */
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:       ///<���ѹرյ�socket��д�����ź�
			printf("process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:      ///<��ͨ��kill�ź�
		case SIGKILL:       ///<kill -9�ź�
			gtloginfo("devset ��kill,�����˳�!!\n");
			exit(0);
		break;
		case SIGINT:         ///<ctrl-c�ź�
			gtloginfo("devset ���û���ֹ(ctrl-c)\n");
			exit(0);
		break;
		case SIGSEGV:       ///<�δ����ź�
			gtlogerr("devset �����δ���\n");
			printf("devset segmentation fault\n");
			exit(0);
		break;
	}
	return;
}

int main(void)
{
	int lock_file=-1;
	char pbuf[100];
	pthread_t  eth1_thread, eth0_thread; // eth0_thread;
	int eth1_thread_node, eth0_thread_node; //, eth0_thread_node;


/**********�жϱ������Ƿ��Ѿ�����*************************/
	gtopenlog("devset");		///added by shixin ����־��¼����
	lock_file=create_and_lockfile(DEVSET_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("devset are running!!\n");
		gtlogerr("devset ģ�������У���������Ч�˳�\n");		
		exit(1);
	}
	memset(pbuf, 0 , sizeof(pbuf));
	sprintf(pbuf,"%d\n version:%s\n",getpid(),VERSION);//��¼����汾��Ϣ
	write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���

	printf("���� devset(ver:%s)\n", VERSION);
	gtloginfo("���� devset(ver:%s)\n", VERSION);

////ע���źŴ�����
#ifndef _WIN32
	///ע���źŴ�����
	signal(SIGKILL,exit_log);		 ///<kill -9�ź�
	signal(SIGTERM,exit_log);		 ///<��ͨ��kill�ź�
	signal(SIGINT,exit_log);		        ///<ctrl-c�ź�
	signal(SIGSEGV,exit_log);		 ///<�δ����ź�
	signal(SIGPIPE,exit_log);		 ///<���ѹرյ�socket��д�����ź�
#endif	
	
	init_sys_paras();	//��ʼ��ϵͳ����
	create_test_state_file();// ���ɲ���״̬��¼�ļ� lsk 2007-2-8
//	printf("ok \n");
	if(get_eth_num()==2)	//����豸����������
	{

////��ʼ������1 ���������߳�
		net_port1.multi_port = MULTI_CAST_PORT;
		memset(net_port1.hostname,0,sizeof(net_port1.hostname));
		memcpy(net_port1.hostname, HOSTNAME_ETH1,strlen(HOSTNAME_ETH1));
		eth1_thread_node = GT_CreateThread(&eth1_thread, (void*) &netport_thread, &net_port1);					 
		if(eth1_thread_node==0)
		{
			printf("open  net port 2 thread ok !\n");
		}
		else
		{
			gtlogerr("devset ģ��:fail to open net port 2 thread !\n");		
		 	printf("fail to open net port 2 thread !\n");
//			exit(1);
		}
	}


////��ʼ������0 ���������߳�
	net_port0.multi_port = MULTI_CAST_PORT;
	memset(net_port0.hostname,0,sizeof(net_port0.hostname));
	memcpy(net_port0.hostname, HOSTNAME_ETH0,strlen(HOSTNAME_ETH0));
	eth0_thread_node = GT_CreateThread(&eth0_thread, (void*) &netport_thread, &net_port0);					 
	if(eth0_thread_node==0)
	{
		printf("open  net port 1 thread ok !\n");
	}
	else
	{
		gtlogerr("devset ģ��:fail to open net port 1 thread !\n");		
	 	printf("fail to open net port 1 thread !\n");
//	 	exit(1);
	}

	sleep(30);
#if 0 //for ip1004,û��Ӳ��
	memset(pbuf, 0 , sizeof(pbuf));
	get_dev_hd_sn("/dev/hda",pbuf,sizeof(pbuf));
#endif	
	while(1)	// do nothing here 
	{
		sleep(50);
	}
	close(lock_file);
	exit(0);
}



