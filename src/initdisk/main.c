#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <file_def.h>
#include <commonlib.h>
#include <nv_pair.h>
#include <devinfo.h>
#include <guid.h>

#include "multicast_ctl.h"
#include "fmat_disk.h"

//#define FOR_3022		/// lsk 2009 -2-11	for 3022
#ifdef  FOR_3022
#include <devinfo_virdev.h>
#endif

//����sd��Ӳ������ʱʹ������ĺ�
//#define USE_SD

//#define RESULT_FILE_NAME	"/tmp/fmat_info.txt"
#define PACKAGE    			"initdisk"
#define VERSION 				" 1.21"
/*
v1.21   ����ֻ���ip1004��һ���ͺŵ�bug
V1.20	killall ��ӵȴ�
V1.19	ZW-2011-07-18 �޸��ڸ�ʽ��Ӳ��ǰ�ȹر�DMA�������п��������ʽ��������ϵͳ��������ʹ��Ŀǰ�ⷿ�������Ӳ�̾��д����󣬾���ԭ��û������
V1.18	��Ӷ�TF����֧��
V1.17	����汾���sd��������ver:0.02ʹ��,initallidedrvs_ver:1.16,sd���Ľڵ�Ϊ/dev/hda
V1.16	diskinfo��֧��sd��
V1.15	�޸�sd���ڵ�/dev/cpesda��Ϊ/dev/sda
V1.14	ͬʱ֧��GTVS1k��3kϵ��
V1.13	ȥ������FOR_3022 
V1.12	���ӷ�����ļ��
V1.11	�޸�V1.10�汾�У�������SD�����ʹ������������Ƶ�init_all_ide_drv��
V1.10	��Ӽ���SD����
V1.09	֧��SD����������ʽ��
V1.08	֧��gtvs3022
V1.07	��ʽ����������Ӳ������������watch_proc��vsftpd
V1.06 
      ��ʽ��ʱ���ٶ�����������Ҳ���cf��+Ӳ��
      ��ʽ��ʧ�ܺ󲻶�����������ͳɹ���Ϣ��
      ��������ΪӲ������
      �����˶�Ӳ�̶������ʽ����֧��
V1.05 �ı���ͨѶЭ�� 
V1.04 �����˸�ʽ��������������� 
V1.03 ����ʽ������ͷ��������Ϊǰ̨����
V1.02 ����ʽ������ͷ��������Ϊ��̨����
V1.01 2007 -4-25�����˸�ʽ�����̵ı�־����
*/

#define LOCK_FILE 	"/lock/ipserver/initdisk"
#define REC_FILE		"/log/diskstate.txt"
//#define FMAT_INFO 	"/tmp/fmat_info.txt"	

multicast_sock net_port;
int channel=0;
unsigned int boot_flag;
static int init_all_disks_flag=1;//Ϊ1��ʾ��ʼ�����ҵ�������Ӳ�̣�Ϊ0��ʾָ��Ӳ��
static char diskname[100]; //ָ����Ҫ��ʼ����Ӳ��
void print_help(int exval) 
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
	 printf("  -B 	<�������> 0  Ĭ��Ϊ��ʽ��������  1 ������  \n");//lsk 2007-4-26
	 

	 exit(exval);
}
int env_parser(multicast_sock* net_st,unsigned char*result, int argc, char * argv[])
{
	int opt;
	int stat=0;
	char* p_str=NULL;
	struct GT_GUID guid_st;
	while((opt = getopt(argc, argv, "hVI:d:P:F:E:C:D:S:r:B:")) != -1) 
	{
		  switch(opt)
		  {
			   case 'h':
				print_help(0);
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


				//wsyadd
			   case 'd'://ָ����Ҫ��ʼ����Ӳ��
			   p_str= optarg;
				printf("׼����ʽ��:%s\n",p_str);
				init_all_disks_flag = 0;
				memcpy(diskname, p_str,strlen(p_str));
				diskname[strlen(p_str)]='\0';
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
#if 0
			   case 'C':
				channel= atoi(optarg);
				if(channel)
					channel=1;
				stat++;
			    break;
#endif			    
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
//				printf("%s\n",p_str);
				memcpy(result, p_str,strlen(p_str));
				result[strlen(p_str)]='\0';
			   break;

			   case 'B':
				boot_flag = atoi(optarg);
				printf("boot_flag = %d\n",boot_flag);
				if((boot_flag!=0)&&(boot_flag!=1))
				{
					printf("error input -B (0,1)%d\n", boot_flag);
					boot_flag = 1;
				}
				printf("boot_flag = %d\n",boot_flag);
				stat++;
			    break;
			   
			   case ':':
				fprintf(stderr, "%s: Error - Option `%c' needs a value\n\n", PACKAGE, optopt);
				print_help(1);
			    break;

			   case '?':
				fprintf(stderr, "%s: Error - No such option: `%c'\n\n", PACKAGE, optopt);
				print_help(1);
			    break;
			    default:
				stat = 0;
			    break;
		   }
	}
	return stat;
}




int main(int argc, char * argv[])
{
	int rc;
	int stat=0;
	int lock_file=-1;
	FILE* statfile=NULL;
	char pbuf[100];
	char result_filename[120];

	boot_flag = 0;
	memset(result_filename,0,sizeof(result_filename));
	memcpy(result_filename, FMAT_INFO,strlen(FMAT_INFO));
//	memcpy(result_filename, RESULT_FILE_NAME,strlen(RESULT_FILE_NAME));
	memset(&net_port, 0 ,sizeof(net_port));
	stat = env_parser(&net_port,result_filename, argc, argv);
	if(init_devinfo()<0)
	{
		gtlogerr("initdisk ģ��: init devinfo error\n");		
		exit(1);
	}
	if(stat>=4)
	{
		if(init_dev_net_port(&net_port)<0)
		{
			printf("can not open net port%s : %d\n", net_port.hostname, net_port.multi_port);
			exit(1);
		}
#if 0
		virdev_get_devid(channel, net_port.self_id);
#endif
	}
	
	lock_file=create_and_lockfile(LOCK_FILE);
	if(lock_file<=0)
	{
		printf("initdisk are running!!\n");
		gtlogerr("initdisk ģ�������У���������Ч�˳�\n");
		send_test_report(&net_port, "initdisk are running", 0);
		exit(0);
	}
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���
////create an record file 
	statfile = fopen(REC_FILE, "w+");
	if(statfile==NULL)
	{
		printf("error create disk state file\n");
		gtloginfo("error create disk state file"); // lsk 2007 -4 -27
	}
	else
	{
		fprintf(statfile, "%s", "start to init disk\n");
		fflush(statfile);
		fclose(statfile);
	}
//// start format disk
	printf("Version is %s\n", VERSION);
	sleep(1);
	printf("turn OFF the DMA...\n");		//zw-add 2011-07-18
	system("/ip1004/hdparm -d 0 /dev/sda");		//zw-add 2011-07-18
	format_dev_disk(&net_port, result_filename, init_all_disks_flag, diskname);	

////remove record file
	memset(pbuf,0,sizeof(pbuf));
	sprintf(pbuf, "rm %s\n", REC_FILE);
	system(pbuf);
	
	close(lock_file);
// �����豸 2007-4-26 lsk
	if(boot_flag ==0)
	{
		printf("now hardware restart system !!!!!!!!!");
		system("/ip1004/hwrbt\n");							//ϵͳ��λ
	}
	else
	{
#if 0	
		printf("now soft restart system !!!!!!!!!");
		system("/ip1004/watch_proc > /dev/null &");
		system("tcpsvd -vE 0.0.0.0 21 ftpd /hqdata/ &");
#endif        
	}
	return rc; //wsy exit(rc);											//���ز��Խ������
}



