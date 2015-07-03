#include <stdio.h>
#include <devinfo.h>
#include<gtlog.h>
#include <gtthread.h>
#include <guid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <commonlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include"testmod.h"
#include "io_test.h"
#include"com_mod.h"
#include"put_errr.h"
#include"multicast_ctl.h"
//#include"../../vs3_drivers/vs3_iodrv/api/gtvs_io_api.h"


#define ZW_TEST


multicast_sock net_port;


/**********************************************************************************************
* ������   :recv_dev_pkt()
* ����  :       ��ӡ�����˵�
* ����  :       void
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
void print_help(void) 
{
	 printf("Program %s, Version %s \n", PACKAGE, VERSION); 
	 printf("%s [-h] [-V] [-n time ][-s time ] [-o FILE]\n\n", PACKAGE);

	 printf("  -h              print this help and exit\n");
	 printf("  -V              print version and exit\n\n");
//	 printf("  -u   second           ���������������� \n");
   	 printf("  -I	<multicast IP address> 	 \n");
	 printf("  -P  	<multicast port number>            \n");
	 printf("  -F  	<flag>  ���ͱ�־          \n");
	 printf("  -E  	<enc_type> ��������           \n");
	 printf("  -D 	<server_ID>\n");
	 printf("  -S 	<packet sequent number>     \n");
	 printf("  -r 	<result file path>     \n"); 

}




/**********************************************************************************************
* ������   :env_parser()
* ����  :       �����������
* ����  :      net_st		�鲥���ݽṹָ��
*			 result		���ؽ��
*			argc			��������
*			argv			����ָ��
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
int env_parser(multicast_sock* net_st,unsigned char*result, int argc, char * argv[])
{	
	int stat=0;
	int opt;
	char *p_str=NULL;
	struct GT_GUID guid_st;

	while((opt = getopt(argc, argv, "hVI:P:F:E:D:S:r:")) != -1) 
	{
		 switch(opt)
		{
			case 'h':
				print_help();
				break;

			case 'V':
				printf("%s %s\n\n", PACKAGE, VERSION); 
				exit(0);
			    break;
				
			case 'I':
				p_str = optarg;
				memcpy(net_st->hostname, p_str, strlen(p_str));
//				printf("%s\n",p_str);
				stat++;
				break;

			case 'P':
				net_st->multi_port = atoi(optarg);
//				printf("%d\n", net_st->multi_port); 
				stat++;
				break;

			case 'F':
				net_st->flag= atoi(optarg);
//				printf("%d\n", net_st->flag);	
				stat++;
				break;
								
			case 'E':
				net_st->enc_type= atoi(optarg);
//				printf("%d\n", net_st->enc_type);	
				stat++;
				break;
								
			case 'D':
				p_str = optarg;
//				printf("%s\n",p_str);
				guid_st = hex2guid(p_str);
				memcpy(net_st->target_id, &guid_st, sizeof(guid_st));
				stat++;
				break;

			case 'S':
				p_str = optarg;
//				printf("%s\n",p_str);
				memcpy(net_st->seq_num, p_str, strlen(p_str));
				stat++;
				break;
				
			case 'r':
				p_str= optarg;
				printf("%s\n",p_str);
				memcpy(result, p_str,strlen(p_str));
				result[strlen(p_str)]='\0';
				break;
							   
			case ':':
				fprintf(stderr, "%s: Error - Option `%c' needs a value\n\n", PACKAGE, optopt);
				print_help();
				break;

			case '?':
				fprintf(stderr, "%s: Error - No such option: `%c'\n\n", PACKAGE, optopt);
				print_help();
				break;

			default:
				stat = 0;
				break;

		}
	}

	return stat;

}

/**********************************************************************************************
* ������   :s_test_rp()
* ����  :       ���Ͳ��Ա���
* ����  :      buf			���͵��ַ���
*				num		���̺�
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
int s_test_rp(unsigned char *buf,int num)
{
#ifndef  ZW_TEST
	return send_test_report(&net_port,buf, num);
#endif
	return 0;
}


/**********************************************************************************************
* ������   :main()
* ����  :       main
* ����  :      			
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
int main(int argc,char *argv[])
{
	int stat;
	int trig_ret;
	int com_ret;
	char result_file[120];
	unsigned char tmp[32];
	int lock_file=-1;
	char pbuf[40];

	 //TODO��ʼ����·
	if(init_devinfo()<0)
	{
		printf("trigtest ģ��: init devinfo error\n"); 	 
		exit(1);
	}
	
	memset(result_file,0,sizeof(result_file));
	memset(tmp,0,sizeof(tmp));
	memcpy(result_file, RESULT_FILE_NAME,strlen(RESULT_FILE_NAME));
	memset(&net_port, 0 ,sizeof(net_port));
	stat=env_parser(&net_port,result_file,argc,argv);
	if(stat>=4)
	{
		if(init_dev_net_port(&net_port)<0)
		{
			printf("can not open net port%s : %d\n", net_port.hostname, net_port.multi_port);
			exit(1);
		}
	}

	
	lock_file=open(TESTMOD_LOCK_FILE,O_RDWR|O_CREAT,0640);//�����ļ�
	if(lock_file<0)
	{
		mkdir("/lock",0770);
		mkdir("/lock/ipserver",0770);
		lock_file=open(TESTMOD_LOCK_FILE,O_RDWR|O_CREAT,0640);
		if(lock_file<0)
		{
	#ifdef MYPRINT
			printf("testmod create lock file error!\n");
	#endif
			exit(1);
		}
	}
	if(flock(lock_file,LOCK_EX|LOCK_NB)!=0)//�����̱�־�ļ��������Է�ֹ�������ͬһ����Ķ������
	{
		printf("testmod module are running!!\n");
		s_test_rp("trigtest are running", 0);
		exit(0);
	}
	//��������ж�ģ���Ƿ��Ѿ�ִ�еĹ���
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���
	//////////kill all programs//////////////////
	system("killall -9 watch_proc\n");
	system("killall -15 rtimage \n");
	system("killall -15 hdmodule \n");
	system("killall -9 ipmain \n");
	system("killall -9 diskman \n"); 					
	system("killall -15 encbox \n");					
    
	system("clear\n");
	sleep(1);

	trig_ret=0;
	com_ret=0;
	sprintf(tmp,"���԰汾:%s",VERSION);
	printf("%s\n",tmp);
	send_test_report(&net_port, tmp, 8);

	
	trig_ret=test_io_mod();
	if(trig_ret<0)
	{
		trig_ret=1;
	}

	printf("���Ӳ��Խ��Ϊ:[%d]\n",trig_ret);
	send_test_report(&net_port,"��ʼ���Դ���[RS232/RS485]",40);



	com_ret=init_com();
	if(com_ret<0)
	{
		com_ret=1;
	}

	//send_test_report(&net_port,"���Ӳ��Խ���",100);

	if((trig_ret==0)&&(com_ret==0))
	{
		result_report(ERR_NO,&net_port);
		printf("������ȷ\n");
	}
	else
	{
		result_report(ERR_TEST,&net_port);
		printf("���Դ���\n");
	}

	system("/ip1004/swrbt &");

	close(lock_file);
	
	return 0;
}
