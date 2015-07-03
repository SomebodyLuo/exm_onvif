#include<stdio.h>
#include<stdlib.h>
#include<devinfo.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<gtlog.h>
#include<diskinfo.h>
#include<guid.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include<commonlib.h>
#include<errno.h>
#include <sys/ioctl.h>


#define DISK_FILE		("/log/diskstate.txt")
#define DISK_NAME		("/dev/sda")

//#define USE_SD

#define BUF_LEN  100
#define PACKAGE    			("init_all_ide_drvs")

#define VERSION 				("1.18")
//ver:1.18	���Ӷ�ȡtf/sd���ķ������֣��ж�Ҫ��Ҫ��ʽ�����ſ�
//ver:1.17	����������tf������sd�������Զ����ز�ͬ����������
//ver:1.16	��sd�Ľڵ�����Ϊ/dev/hda����ΪӲ�̴�����ֻ��һ����
//		����汾��Ҫ���sd��������(sd���Ľڵ���Ϊ/dev/hda���ǰ�),initdisk_ver:1.17
//ver:1.15	��sd�����ص�/hqdata/hda1��
//ver:1.14	diskinfo��֧��sd��
//ver:1.13	�޸�init_ide_drvs�ķ���ֵ����
//ver:1.12	֧�ָ�SD����������ʽ�������ص�/hqdata/hda1��
//ver:1.11	�޸���/dev�½�����sd���ڵ�Ϊsda������ԭ����cpesda
//ver:1.10	�޸�init_ide_drvs�е�gtlogerr()�еĴ������ʽΪ[%d],����[0x%d]�ĸ�ʽ
//ver:1.09	�ڼ��Ӳ��֮ǰ������������:
//			/lib/modules/ide-core.ko
//			/lib/modules/ide-generic.ko
//			/lib/modules/ide-disk.ko
//ver:1.08	�޸�ver:1.07�д�����������/dev/hda1�ķ�ʽΪ/dev/cpesda1
//ver:1.07	�޸�ver:1.06�汾Ϊֻ�������sd������������������
//ver:1.06	֧�ּ��SD����������ʽ��������
//ver:1.05	����ǰ�������ʧ�ܸ�ʽ��Ӳ��
//ver:1.04
//���tune2fs /dev/hda1 -m 0
//�޸�LOCK_FILE·��Ϊ/tmp/init_all_ide_drvs

//#define LOCK_FILE 	("/lock/vserver/init_all_ide_drvs")
#define LOCK_FILE	("/lock/ipserver/init_all_ide_drvs")
#define REC_FILE		("/log/init_all_ide_drvs_state.txt")

#if 0
#define DDISK0		("/dev/hda")
#define DDISK1		("/dev/hdb")
#define DDISK2		("/dev/hdc")
#define DDISK3		("/dev/hdd")
#endif

//mount ʱ�õ���
#define MDISKA		("/hqdata/sda")
#define MDISKB		("/hqdata/sdb")
#define MDISKC		("/hqdata/sdc")
#define MDISKD		("/hqdata/sdd")

#define MDPATH			("/hqdata/")

/**********************************************************************************************
* ������   :get_fac_flag()
* ����  :       ��ȡ������־
* ����  :      			
* ���  :       void        
* ����ֵ:   1������,0����ǰ
**********************************************************************************************/
int get_fac_flag(void)
{
	int ret;
	
	ret=get_lfc_flag();

	return ret;

}

/**********************************************************************************************
* ������   :print_help()
* ����  :       ������Ϣ
* ����  :      			
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
void print_help(int exval)
{
	int i;
	unsigned char *tmp[]={"\n����:\n",
						"(1) ��δ��ʽ�����Ӳ�̽������¸�ʽ��;\n",
						"(2) ��δ����״̬�£�����δ��ʽ����Ӳ���Զ���ʽ��,������ֻ��¼��־;\n",
						"(3) ʹ��e2fsck���ÿ�������ڵ�;\n",
						"(4) �������ڵ�mount����ӦĿ¼;\n\n",
						NULL};

	
	for(i=0;tmp[i]!=NULL;i++)
	{
		printf("%s",tmp[i]);
	}


         printf("����: %s, �汾: %s \n", PACKAGE, VERSION);
         printf("%s [-h] [-V] \n\n", PACKAGE);
         printf("  -h              ��ӡ������Ϣ���˳�\n");
         printf("  -V              ��ӡ�汾��Ϣ���˳�\n\n");
	 printf("  -B   <�������> 0  Ĭ��Ϊ��ʽ��������  1 ����  \n");

         exit(exval);
}

/**********************************************************************************************
* ������   :env_parser()
* ����  :       ��������
* ����  :      
* ���  :       void        
* ����ֵ:   boot_flag	������־��1������0������,Ĭ�ϲ�����
**********************************************************************************************/
//int env_parser(multicast_sock* net_st,unsigned char*result, int argc, char * argv[])
int env_parser(int argc,char *argv[])

{
	int opt;
	int boot_flag=0;

	while((opt = getopt(argc, argv, "hVB:")) != -1) 
	{
		switch(opt)
		{
			case 'h':
				print_help(0);
				break;

			case 'V':
				printf("\n%s %s\n\n", PACKAGE, VERSION); 
				exit(0);
				break;

                        case 'B':
                                boot_flag = atoi(optarg);
                                if((boot_flag!=0)&&(boot_flag!=1))
                                {
                                        printf("����������� -B (0,1)%d\n", boot_flag);
                                        boot_flag = 0;
                                }
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
				break;
		}
	}
	return boot_flag;
}

/****************************************************************************************
 *��������: init_sd_drv()
 *��    ��: ����sd��������������������
 *��    ��: ide_flag Ϊ2��ʾsd��Ϊ3��ʾtf��������ֵ�����˳�
 *��    ��: ��
 *��    ��: ��
 * **************************************************************************************/
void init_sd_drv(int ide_flag)
{
        char diskcmd[100];
        int ret=0;
	int fd;
	int fst;

        //���ڵ��Ƿ���� 

        //����SD��������/lib/modules/ftsdc010.ko
	switch(ide_flag)
	{
		case 2:
			sprintf(diskcmd,"/sbin/insmod %s","/lib/modules/ftsdc010_sd.ko");
			break;
		case 3:
			sprintf(diskcmd,"/sbin/insmod %s","/lib/modules/ftsdc010_tf.ko");
			break;
		default:
			printf("*******[ERR]*******���ҵĲ���Ӳ�̣�����SD����Ҳ����TF��\n");
			return;
	}

        printf("��������[%s]\n",diskcmd);
        ret=system(diskcmd);

	printf("waitting...\n");	
	sleep(2);
	//mdev -s
        memset(diskcmd,0,sizeof(diskcmd));
        sprintf(diskcmd,"mdev -s");
        printf("ִ��[%s]\n",diskcmd);
        ret=system(diskcmd);

#if  USE_SD //20090811�ڵ�����Ӳ�̵�һ�������÷���������
        //������������ 
        memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","ln -s /dev/sda /dev/hda");
        printf("������������[%s]\n",diskcmd);
        ret=system(diskcmd);
#endif

	memset(diskcmd,0,sizeof(diskcmd));
#if USE_SD
	sprintf(diskcmd,"mknod /dev/sda1 b 254 1");
#else
	sprintf(diskcmd,"mknod /dev/sda1 b 254 1");
#endif
	printf("�����ڵ�[%s]\n",diskcmd);
	ret=system(diskcmd);

	
#if USE_SD
	//������������
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","ln -s /dev/sda1 /dev/hda1");
        printf("������������[%s]\n",diskcmd);
        ret=system(diskcmd);
#endif
	//zw-add 2010-07-26-------->
	//�����ⲿ�ֵ���ftsdc010.ko��ȡSD/TF���ķ����������ж������
	//��ɶ�����ļ�ϵͳ���ͣ��������linux��ʽ(0x83)�ľ͸������ˡ���
	//FAT32/16�����ľ͸�����
	fd=open("/dev/sda",O_RDONLY);
	if(fd<0)
	{
		printf("open /dev/sda error\n");
		return ;
	}
	fst=0;
	ret=ioctl(fd,0x4513,&fst);
	if(ret<0)
	{
		printf("��ȡ�������ʹ���\n");
	}
	printf("������̷�������Ϊ[0x%02x] :",(int)fst);
	close(fd);

	if(fst!=0x83)
	{
		printf("����linux�ĸ�ʽ,Ҫ��ʽ��һ��\n");
		system("/ip1004/initdisk -B 1");
	}
	else
	{
		printf("�����ʽ����������\n");
	}
	//<-----zw-add 2010-07-26

	#if 1
	//ͨ��sd���Ķ���������/proc/partitions��sd��������Ϣ
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","fdisk -l /dev/sda");
	printf("ʹ������[%s]�鿴SD����.\n",diskcmd);
	ret=system(diskcmd);
	#endif


}			  

/**********************************************************************************************
* ������   :init_ide_drvs()
* ����  :       ��ʼ��Ӳ��
* ����  :      			
* ���  :       void        
* ����ֵ:   void
**********************************************************************************************/
int init_ide_drvs(void)
{
	int fd;
	int ret;
//	unsigned int u_ret;
	int ret_n;
//	unsigned int m_ret;
	unsigned char cmd_buf[64];
	int part_num=0;
	int disk_num=0;
	int j;
	int i;
	int ide_flag=0;
//	unsigned char *disks[]={DDISK0,DDISK1,DDISK2,DDISK3,NULL};
//	unsigned char *mdisks[]={MDISKA,MDISKB,MDISKC,MDISKD,NULL};

	unsigned char disks[32];
	unsigned char mdisks[32];
	unsigned char *tmp=NULL;

	ide_flag=get_ide_flag();
	printf("ide_flag:%d\n",ide_flag);
	if(ide_flag!=1) 
	{
		
		//��ʱ�˿̣�ide����sd����cf-20100709
		init_sd_drv(ide_flag);
		//zw-20090811 if(access("/dev/sda",F_OK)==0)  //��sd�Ľڵ��Ϊ��Ӳ�̵�һ����/dev/hda
		if(access("/dev/sda",F_OK)==0)
		{
			printf("��SD��\n");
		} 
	}
	else
	{
		//printf("��%d��Ӳ��\n",disk_num);
		//yk del 20130704
		//system("/sbin/insmod  /lib/modules/ide-core.ko");
		//system("/sbin/insmod  /lib/modules/ide-generic.ko");
		//system("/sbin/insmod  /lib/modules/ide-disk.ko");
	}

	//���Ӳ���ϴθ�ʽ��ʱ���ж�û
	printf("��ʼ���Ӳ���ϴθ�ʽ��ʱ�Ƿ��ж�-->");
	fd=access(DISK_FILE,F_OK);
	if(fd==0)
	{
		gtloginfo("��Ȼ���ж���\n");
		printf("��Ȼ���ж���\n");
		system("/ip1004/initdisk -B 1");
	}
	else
	{
		gtloginfo("û�б��ж���\n");
		printf("û�б��ж�\n");
	}

	//����м�������
	disk_num= get_sys_disk_num();
	if(ide_flag==2)
	{
		//SD���� ,09-06-12 ���������Ҫ�ֶ��޸�Ϊ1��sd��
		/////disk_num=1;
		printf("ʹ�õ���sd���Լ��жϵ�ֵ��sd������disk_num=[%d]\n",disk_num);
	}

	if(disk_num<=0)
	{
		gtlogerr("û�з��ִ���.\n");
		return -1;
		//exit(1); //Ϊɶ��ʱ����Ҫ��exit
	}
	printf("��%d��Ӳ��\n",disk_num);

	printf("��ʼ���Ӳ�̽ڵ�....\n");
	//���ÿ��Ӳ�̽ڵ��ǲ��Ƕ�����
	for(i=0;i<disk_num;i++)
	{
		memset(disks,0,sizeof(disks));	
		tmp=get_sys_disk_devname(i);
		if(tmp==NULL)
		{
			printf("����Ӳ�̽ڵ�������\n");
			gtlogerr("����Ӳ�̽ڵ�������\n");
			continue;
		}
		else
		{
			//SD
			#if USE_SD //��ʱ���������չ�sd��������Ӳ�̴��� 20090811-zw
			if(ide_flag==2)
			{
				//09-06-12 Ŀǰֻ֧��һ��SD����д��Ϊ/dev/sda,��/dev/sdb,/dev/sdc��Щ�������
				sprintf(disks,"%s","/dev/sda");	
			}
			else
			#endif
				sprintf(disks,"%s",tmp);
				//yk add 20130704
				printf("disks =%s\n",disks);
		}

		ret=access(disks,F_OK);
		if(ret<0)
		{
#if 0
			if(get_lfc_flag()==0)
			{	
				//δ��������initdisk����
				printf("%s���ڣ�δ����,����initdisk\n",disks[i]);
				system("/gt1000/initdisk -B 1");
			}
			else
			{
				gtlogerr("û��Ӳ�̽ڵ�%s\n",disks[i]);
				printf("û��Ӳ�̽ڵ�%s\n",disks[i]);			
			}
#endif
			gtlogerr("Ӳ�̽ڵ�%s���ʴ���\n",disks);
			continue;
		}

		//��ȡ����Ӳ�̵ķ����ڵ����
		//��ʱ��֪Ӳ���Ƿ��Ѿ��������Ȼ�ȡ���̷�������,Ȼ����ݸ����ж��Ƿ���� 
		ret= get_sys_disk_partition_num(disks);		//   /dev/hdx
		printf("[%s]����Ϊ[%ld]MB\n",disks,get_sys_partition_capacity("sda",1));//yk 20130704 change hda sda
////
		#if USE_SD //��ʱ���������չ�sd��������Ӳ�̴���
		if(ide_flag==2)
		{
			 ////ret=1;	//sd��ֻ��һ������������Ŀǰ��/proc/partitions����û��sd����������Ϣ����ʱ������
			 printf("������sd��,��������Ϊ[%s],���̷�������Ϊ[%d]\n",disks,ret);
		}
		#endif
////
		printf("%s��%d������\n",disks,ret);
		gtloginfo("%s��%d������\n",disks,ret);
		if(ret>=0)
		{
			//��û�з���
			if(ret==0)
			{
				printf("��û�з���\n");
				if(get_lfc_flag()==1)
				{	
					//�������ֻ�ܼ���־
					gtlogerr("�ѳ���,�����̻�û�з���\n");
					printf("�������û�з���\n");
					continue;
				}
				else
				{
					//����ǰ�͵���initdisk				
					printf("����ǰû�з���,����initdisk����.\n");
					gtloginfo("����ǰû�з���,����initdisk����.\n");
					system("/ip1004/initdisk -B 1");
					
					//����initdisk��������ȡ���̷�������
					ret_n=0;
					ret_n=get_sys_disk_partition_num(disks);
					printf("����ǰ��û�з���������initdisk���������¼�����,ʹ��[%s]����,����Ϊ[%d]\n",disks,ret_n);
////
					#if USE_SD
					if(ide_flag==2)
					{
						//ret=1; //sd��ͬ��
						ret=ret_n;
						printf("[%s]����Ϊ[%ld]\n",disks,get_sys_partition_capacity("sda",1));
					}
					#endif
////
					if(ret_n>0)
					{
						part_num=ret_n;
					}
					else
					{
						gtlogerr("����ǰ���̻�û�з���\n");
						printf("����ǰû�з���\n");
						continue;
					}
				}		
			}
			else
			{
				printf("�Ѿ�������\n");
				part_num=ret;
			}
		}
		else if(ret<0)
			{
				printf("��ȡ���̷����ڵ��������\n");
				gtlogerr("��ȡ���̷����ڵ��������\n");
				return -1;
			}


		

		for(j=1;j<part_num+1;j++)
		{
			memset(mdisks,0,sizeof(mdisks));
			tmp=get_sys_disk_name(i);
			if(tmp==NULL)
			{
				printf("��ȡ%s��[%d]�������ڵ����\n",disks,j);
				gtlogerr("��ȡ%s��[%d]�������ڵ����\n",disks,j);
			}
			else
			{
				sprintf(mdisks,"%s",tmp);
				printf("---mdisks=[%s]\n",mdisks);
			}
		
			printf("\n\n��ʼ����%d�������ڵ�------>\n",j);
			//����x�����̵ĵ�n�������Ƿ����
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%d",disks,j);		//   /dev/hdij
			ret=access(cmd_buf,F_OK);
			printf("��ʼ���[%s]�Ƿ����...\n",cmd_buf);
			if(ret<0)
			{
				if(get_lfc_flag()==1)
				{
					//�ѳ���,����־
					gtlogerr("���̹���[%d]�������ڵ�,��[%d]���ڵ��޷�����\n",part_num,j);
					printf("���̹���[%d]�������ڵ�,��[%d]���ڵ��޷�����\n",part_num,j);
				}
				else
				{
					//��û����,�ط���
					printf("�豸��û���������·����͸�ʽ��\n");
					gtloginfo("�豸��û���������·����͸�ʽ��\n");
					system("/ip1004/initdisk -B 1");
				}
			}
			else////
			{
				printf("[%s]����\n",cmd_buf);
			}
#if 1
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"umount %s%s%d",MDPATH,mdisks,j);
			printf("ʹ��%s\n",cmd_buf);
			system(cmd_buf);
#endif
			//ʹ��e2fsck���------e2fsck -y /dev/hda1
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%s%d","e2fsck -y ",disks,j);
			printf("ʹ������[%s]���\n",cmd_buf);
			system(cmd_buf);


			//ʹ��tune2fs�����ļ�ϵͳ-----tune2fs /dev/hda1 -m 0
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s %s%d %s","tune2fs",disks,j,"-m 0");
			printf("ʹ������[%s]�����ļ�ϵͳ\n",cmd_buf);
			system(cmd_buf);

			
			//׼��mount,���Ŀ��ڵ���û
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%d",mdisks,j);		// /hqdata/hdij
			printf("׼��mountʹ������[access %s]���\n",cmd_buf);
			ret=access(cmd_buf,F_OK);
			if(ret<0)
			{
				//     /hqdata����û�нڵ�,�´���һ��
				memset(cmd_buf,0,sizeof(cmd_buf));
				sprintf(cmd_buf,"mkdir %s%s%d",MDPATH,get_sys_disk_name(i),j);
				printf("û��,�����ڵ�,%s\n",cmd_buf);
				system(cmd_buf);
			}
			else
				printf("%s%d����\n",mdisks,j);

			//��ʼmount
			#if 0
			//zw-del 2009-09-20
			memset(cmd_buf,0,sizeof(cmd_buf));
			if(get_ide_flag()==2)
			{	//wsyadd
				sprintf(cmd_buf,"mkdir /hqdata/hda1");
				system(cmd_buf);
				sprintf(cmd_buf,"mount /dev/sda1 /hqdata/hda1");
			}
			else
			#endif
				sprintf(cmd_buf,"mount %s%d %s%s%d",disks,j,MDPATH,get_sys_disk_name(i),j);	
			printf("��ʼmount,����:%s\n",cmd_buf);
			errno=0;
			ret=system(cmd_buf);
			if(errno==0)			
			{
				//errrno=0-->system�ɹ�ִ���꣬retΪsystemִ��cmd_buf��cmd_buf���ص�ֵ
				if((ret!=0))
				{	
					if(get_lfc_flag()==0)
					{
						//����ʧ�ܣ������ڳ���ǰ����ʽ��
						gtlogerr("ʧ��:[״̬:����ǰ]��%s%d�������ص�%s%s%dʧ�ܣ�������:[%d],����ԭ��:%s,ִ�����¸�ʽ��Ӳ��\n",disks,j,MDPATH,get_sys_disk_name(i),j,ret,strerror(ret));
						system("/ip1004/initdisk -B 1");
					}
					else
					{
						gtlogerr("ʧ��:[״̬:������]��%s%d�������ص�%s%s%dʧ�ܣ�������:[%d],����ԭ��:%s\n",disks,j,MDPATH,get_sys_disk_name(i),j,ret,strerror(ret));
					}
				}
				else
				{
					printf("mount�ɹ�\n");
				}
			}
		}
		
	}

	return 0;
}

 
/**********************************************************************************************
 * ������   :check_db_file()
 * ���� : 	������ݿ�������Ƿ�ϵ磬ͨ���������ļ��Ƿ���ڵķ������еĻ�ɾ��   
 * ����  :                       
 * ���  :       void        
 * ����ֵ:   void
 **********************************************************************************************/
int check_db_file(void)
{
        //int ret;
        int i;
        char tmp_file[64];
        char rm_cmd[128];

        for(i=0;i<4;i++)
        {
                memset(tmp_file,0,sizeof(tmp_file));
                memset(rm_cmd,0,sizeof(rm_cmd));
                sprintf(tmp_file,"%s%d%s","/hqdata/sda",i+1,"/creating_db");
                printf("����ļ�%s�Ƿ����?---",tmp_file);
                if(access(tmp_file,F_OK)==0)    //�ļ����ڣ�ɾ
                {
                        sprintf(rm_cmd,"%s%s","rm -f ",tmp_file);
                        printf("�ļ�:%s���ڣ�ִ��%s\n",tmp_file,rm_cmd);
                        system(rm_cmd);
                        memset(tmp_file,0,sizeof(tmp_file));
                        sprintf(tmp_file,"%s%d%s","/hqdata/sda",i+1,"/index.db");
                        if(access(tmp_file,F_OK)==0)
                        {
                                memset(rm_cmd,0,sizeof(rm_cmd));
                                sprintf(rm_cmd,"%s%s","rm -f ",tmp_file);
                                printf("�ļ�:%s���ڣ�ִ��%s\n",tmp_file,rm_cmd);
                                system(rm_cmd);
                        }
                }
                else    
                {
                        printf("������\n");
                }
        }

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
	int lock_file=-1;
	int boot_flag;
	int ret;
	char pbuf[100];
	
	gtopenlog("init_all_ide_drvs");
	boot_flag=0;

	init_devinfo();
	
	boot_flag=env_parser(argc,argv);

	memset(pbuf,0,sizeof(pbuf));
	lock_file=create_and_lockfile(LOCK_FILE);
	if(lock_file<=0)
	{
		printf("initidealldrv are running!!\n");
		gtlogerr("initidealldrv ģ�������У���������Ч�˳�\n");		
		exit(0);
	}
        sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
        write(lock_file,pbuf,strlen(pbuf)+1);

	printf("%s",pbuf);

	system("killall -9 watch_proc");
	system("killall -15 hdmodule");
	system("killall -9 diskman");//yk add 20130731

	ret=init_ide_drvs();
	if(ret<0)
	{
		close(lock_file);
		printf("initidealldrv ��ʼ��ide�豸ʧ��,�˳�\n");
		gtlogerr("initidealldrv ��ʼ��ide�豸ʧ�ܣ��˳�\n");
		return -1;	
	}
	else
	{
		printf("initlostfd\n");
		gtloginfo("initlostfd\n");
		check_db_file();
		system("/ip1004/initlostfd");
		close(lock_file);
	}

        if(boot_flag ==1)
        {
                printf("swrbt\n");
                gtloginfo("swrbt\n");
		system("/ip1004/swrbt");                                                      //ϵͳ��λ
        }

	return 0;
}
