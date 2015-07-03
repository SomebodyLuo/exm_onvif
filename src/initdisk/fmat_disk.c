#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <file_def.h>
#include <string.h>
#include "errno.h"
#include <sys/ioctl.h>

#include <diskinfo.h>
#include <devinfo.h>
#include "fmat_disk.h"

#define FMAT_OK		300
#define NO_DISK		301
#define NO_HDA1		302
#define MOUNT_ERR	303
#define FDISK_ERR	304
#define FMAT_ERR	305

// ��ʽ������ʧ�ܴ�����Ϣ

static const char* format_OK	=	"���̸�ʽ���ɹ�";
static const char* format_err	=	"��ʽ������ʧ��";
static const char* no_disk		=	"û��Ӳ�̻�cf��";
static const char* no_hda1		=	"�Ҳ���/dev/hda1�ڵ�";
static const char* mount_err	=	"���ش���ʧ��";
static const char* fdisk_err	=	"���̷���ʧ��";

/*���õ�ǰ��¼�����*/
void set_record_partition(char *partition_name)
{

    dictionary    *ini=NULL;
    FILE            *fp=NULL;

    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("set record partition,cannot parse ini file [%s]\n", DISK_INI_FILE);
          gtlogerr("init_devinfo() cannot parse ini file [%s]", DISK_INI_FILE);
          return;
    }

    gtloginfo("��ʽ����¼�������Ϊ%s\n",partition_name);
    //��ǰ¼�������Ϊsda1
    iniparser_setstr(ini, "diskinfo:record_disk", partition_name);
    save_inidict_file(DISK_INI_FILE,ini,&fp);
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }

    iniparser_freedict(ini);

}
/*
*****************************************************
*��������: disk_format
*��������: ��ʽ������ ���մ��̵�����ѡ��ͬ�ĸ�ʽ������
*���룺char* disk_name ��������
*�����
����ֵ:0 �ɹ���1��ʾû�д��̣�2��ʾ��ʽ��ʧ��
*�޸���־��
*****************************************************
*/ 
int disk_format(char* disk_name)
{
	int ret;
	long int cap;
	char cmd[200];
	int i;
	
	cap = get_sys_disk_capacity(disk_name);
	if (cap<=0)
	{
		printf("�Ҳ���%s IDE �豸\n", disk_name);
		return 1;
	}
	printf("�ҵ�  %s IDE�豸\n", disk_name);
		
	if(cap<200)
	{
		printf("%s capacity = %ld less than 200M \n", disk_name, cap);
		printf("û�м�⵽���õĴ洢�豸!!\n");
		return 1;
	}
	else 
	{
		printf("��⵽���õ�Ӳ��%s disk capacity = %ldG\n", disk_name, cap/1000);
		for(i=1;i<= get_sys_partition_num(disk_name);i++)
		{
			sprintf(cmd, "/sbin/mke2fs -T ext3 /dev/%s%d -b 4096 -j -L hqdata%d -m 1 -i 1048576 ",disk_name,i,i);/*yk change ext3*/
			ret = system(cmd);       //wsy, 1M /node
			if(ret!=0)
			{
				return 2;
			}
			//wsyadd tune2fs
			sprintf(cmd,"/sbin/tune2fs -i 0 -c 0 /dev/%s%d\n",disk_name,i);
			ret = system(cmd);     
			if(ret!=0)
			{
				return 2;
			}
			sprintf(cmd,"/sbin/tune2fs -m 1 /dev/%s%d\n",disk_name,i);
			ret = system(cmd);       
			if(ret!=0)
			{
				return 2;
			}
			
		}
	}
	return 0;
}

//��ʽ��Ӳ��Ϊ������,��gt1kѡ��
int init_ide_drv_single_partition(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	
    int ret = -1;
    long int cap=0;
    unsigned char buf[200];
    
	// format disk
    memset(buf,0,sizeof(buf));
    ret = system("fdisk /dev/hda -f -a \n");        //�ȷ���
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",FDISK_ERR, fdisk_err);
            send_test_report(ns, "���̷���ʧ��", 80);// lsk 2007 -6-1
            result_report(FDISK_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    send_test_report(ns, "���̷����ɹ�", 30);
    // ���մ��̵Ĵ�С���и�ʽ��
 	memset(buf,0,sizeof(buf));
    send_test_report(ns, "��ʽ��Ӳ�̿�ʼ...", 40);
    ret = disk_format("hda");       //��ʽ������
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",FMAT_ERR, format_err);
            sprintf(buf, "%s",format_err);
            send_test_report(ns, buf, 90);
            result_report(FMAT_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    memset(buf,0,sizeof(buf));
    send_test_report(ns, "���̸�ʽ���ɹ�", 80);
//�ж�����hda1�ڵ�
    memset(buf,0,sizeof(buf));
    if(access("/dev/hda1",F_OK)!=0)
    {
            fprintf(fp, "%d:%s,",NO_HDA1, no_hda1);
            sprintf(buf, "%s",no_hda1);
            send_test_report(ns, buf, 90);
            result_report(NO_HDA1, ns);// lsk 2007 -6-1
            goto endpoint;
    }

// mount disk
    memset(buf,0,sizeof(buf));
    ret = system("mount /dev/hda1 /hqdata\n");
    printf("mount /dev/hda1 /hqdata ret =%d \n", ret);
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",MOUNT_ERR, mount_err);
            sprintf(buf, "%s",mount_err);
            send_test_report(ns, buf, 90);
            result_report(MOUNT_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    fprintf(fp, "%d:%s,",FMAT_OK, format_OK);
    memset(buf,0,sizeof(buf));
    sprintf(buf, "%s",format_OK);
    send_test_report(ns, buf, 80);


    system("rm -rf /hqdata/update");
    system("mkdir /hqdata/update");
    system("mount -t tmpfs none /hqdata/update");
// lsk 2006 -12-15
//      cap = get_hd_capacity();
    memset(buf,0,sizeof(buf));
    cap = get_sys_partition_capacity("hda", 1);
 if((cap<0)||(cap<200))
    {
            fprintf(fp, "%d:%s,",NO_DISK, no_disk);
//              sprintf(buf, "%s",NO_DISK);
            result_report(NO_DISK, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    if(cap>2000)
    {
            fprintf(fp, "��������:%ldGB,", cap/1000);
            sprintf(buf, "��������:%ldGB", cap/1000);
    }
    else
    {
            fprintf(fp, "��������:%ldMB,",cap);
            sprintf(buf, "��������:%ldMB", cap);
    }
    send_test_report(ns, buf, 90);
    ret=0;
    gtloginfo("fmatdisk succeed\n");
endpoint:
    if(ret)
    {
    	gtlogerr("fmatdisk failed\n");
		return ret;
    }	
    send_test_report(ns, "format disk finished", 100);
    result_report(0, ns);
	ret = 0;
//lsk 2007 -6-1
//      result_report(FORMAT_FLAG , ns);        //���ͽ��
	return ret;
}

int init_ide_drv_multi_partition(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	unsigned char buf[200];
	char cmd[200];
	char partition2_name[100];
	char disk_node[100];
	int ret;
	char devname[20];
	long cap;
	
	if((fp==NULL)||(ns==NULL)||(diskname==NULL))
		return -EINVAL;
	
	memset(buf,0,sizeof(buf));
/*yk del 20130705 fdisk can't use this way*/
#if 0
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//�ȷ���
	sprintf(cmd,"/sbin/fdisk %s -f ",diskname);
	ret = system(cmd);	//�ȷ���
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//�ȷ���
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//�ȷ���
#endif
	long long  tmp_cap;//Ӳ�̴�СG
	tmp_cap=get_disk_capacity("sda");
	printf("the cap is:%lld G\n",tmp_cap);
	/*ɾ�����з���*/
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n1\nw\n","sda");
	ret = system(cmd);	//�ȷ���
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n2\nw\n","sda");
	ret = system(cmd);	//�ȷ���
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n3\nw\n","sda");
	ret = system(cmd);	//�ȷ���
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n4\nw\n","sda");
	ret = system(cmd);	//�ȷ���
	


	printf("\n\n\n\n\n");
	sleep(1);
	//exit(1);
	/*����������Ӳ�̷ֳ��ĸ���*/
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n1\n1\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n2\n\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n3\n\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n4\n\n\n\nw\n",\ 
					"sda");
	ret = system(cmd);



	//wsy,�����Ƕ���fdisk��workaround�� to be fixed
	
	sprintf(partition2_name,"%s2",diskname);
	printf("partition2_name:%s\n",diskname);
	if((ret!=0)||(access(partition2_name,F_OK)!=0))
	{	
	    sprintf(buf,"%d:����%s����ʧ��\n",FDISK_ERR,diskname);
		fprintf(fp, buf);
		printf("%s",buf);
		
		send_test_report(ns,buf, 90);// lsk 2007 -6-1
		result_report(FDISK_ERR, ns);// lsk 2007 -6-1
		gtlogerr("%s",buf);
		return -1;
	}
	else
	{
		sprintf(buf,"����%s�����ɹ�\n",diskname);
		send_test_report(ns, buf,15+20*(diskno+1)/disknum);
		printf("%s",buf);
		gtloginfo("%s",buf);
	}
// ���մ��̵Ĵ�С���и�ʽ��
	memset(buf,0,sizeof(buf));
	sprintf(buf,"����%s��ʽ����ʼ...\n",diskname);
	send_test_report(ns, buf, 15+40*(diskno+1)/disknum);
	gtloginfo("%s",buf);
	printf("debug:%s\n",buf);
	strncpy(disk_node,diskname+5,4);
	ret = disk_format(disk_node);	//��ʽ������
	if(ret!=0)
	{
		gtloginfo("disk_format!!!\n");
		fprintf(fp, "%s-%d:%s,",diskname,FMAT_ERR, format_err);
		sprintf(buf, "%s-%s",diskname,format_err);
		send_test_report(ns, buf, 90);
		result_report(FMAT_ERR, ns);// lsk 2007 -6-1
		return -1;
	}
	memset(buf,0,sizeof(buf));
	sprintf(buf,"����%s��ʽ���ɹ�\n",diskname);
	send_test_report(ns,buf,15+60*(diskno+1)/disknum);
	fprintf(fp,buf);
	gtloginfo("%s",buf);
	printf("debug:%s",buf);
#if 0
//�ж�����hda1�ڵ�
	memset(buf,0,sizeof(buf));
	if(access("/dev/hda1",F_OK)!=0)
 	{
		fprintf(fp, "%d:%s,",NO_HDA1, no_hda1);
		sprintf(buf, "%s",no_hda1);
		send_test_report(ns, buf, 90);
		result_report(NO_HDA1, ns);// lsk 2007 -6-1
		goto endpoint;
 	}
#endif
// lsk 2006 -12-15
//	cap = get_hd_capacity();
	memset(buf,0,sizeof(buf));
	strcpy(devname,strstr(diskname,"sd"));/*yk change hd->sd 20130708*/
	gtloginfo("devname is %s, %s",devname,diskname);
	cap = get_sys_disk_capacity(devname);//get_sys_partition_capacity(get_sys_disk_name(i), 1);
	if(cap<200)                
	{
		sprintf(buf, "%d:����%s�޷���������\n", NO_DISK,diskname);	
		fprintf(fp,buf);
		gtlogerr("%s",buf);
		result_report(NO_DISK, ns);// lsk 2007 -6-1
		return -1;	
	}
	else
	{
		sprintf(buf, "����%s ����:%ldGB\n", diskname,cap/1000);	
		gtloginfo("%s",buf);
	}
	fprintf(fp,buf);
	send_test_report(ns, buf, 15+80*(diskno+1)/disknum);
	ret=0;
	gtloginfo("fmatdisk %s succeed\n",diskname);
	
	return 0;


}

/*****************************************************
*��������: init_ide_drv
*��������: ��ʽ������Ӳ��
*����:	multicast_sock *ns ����������ݽṹ
*		fp: �洢��ʽ��������ļ���ָ��
*		diskname: ��Ҫ��ʽ���Ĵ�����������/dev/hda
*		diskno:�ô��̵ı�ţ�0��disknum-1
*		disknum:ϵͳһ���еĴ��̸���
*�����
����ֵ:0 �ɹ���������ʾʧ��
*�޸���־��
*****************************************************/


int init_ide_drv(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	if((fp==NULL)||(ns==NULL)||(diskname==NULL))
		return -EINVAL;
	
	init_devinfo();
	
	//if(strstr(get_devtype_str(),"IP1004")!=NULL) //IP1004ϵ�� YK CHANGE 20130708
	if(1)
	{
		return init_ide_drv_multi_partition(ns,fp,diskname,diskno,disknum);
	}	
	else
	{
		return init_ide_drv_single_partition(ns,fp,diskname,diskno,disknum);
	}

}

/****************************************************
 *��������: init_sd_drv
 *��������: SD������
 *��    ��: multicast_sock *ns ����������ݽṹ
 *��    ��:
 *��    ��: ��
 ****************************************************/
void init_sd_drv(multicast_sock *ns, char *diskname)
{
	char diskcmd[100];
	int ret=0;
	
	//���ڵ��Ƿ����
	//��ʼ��SD���������ͷ�һ����
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"fdisk %s -f a",diskname);
	printf("��sd��������������[%s]\n",diskcmd);
	ret=system(diskcmd);

	#if USE_SD
	//������汾�У�sd�������ｫsd�Ľڵ�����Ϊ/dev/hda��
	//���Բ����ٴ�������������
	if(access("/dev/sda1",F_OK)==0)
	{
		memset(diskcmd,0,sizeof(diskcmd));
		////sprintf(diskcmd,"ln -s /dev/cpesda1 /dev/hda1");
		sprintf(diskcmd,"ln -s /dev/sda1 /dev/hda1");
		printf("������ִ��:%s\n",diskcmd);
		ret=system(diskcmd);
	}
	else
	{
		printf("sd����������û��/dev/sda1\n");
		gtlogerr("sd����������û��/dev/sda1");
		return ;
	}
	#endif

	//��ʼ��ʽ��	
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"mke2fs -j %s1",diskname);	
	printf("��sd����ʽ����������[%s]\n",diskcmd);
	ret=system(diskcmd);
	
}

/*
 * �����Խ�����ݽṹ���������ָ�����ļ�
 */
/*
*****************************************************
*��������: format_dev_disk
*��������: ��ʽ�������߳�
*���룺 multicast_sock *ns ����������ݽṹ
*		
*�����
*�޸���־��
*****************************************************
*/ 
void format_dev_disk(multicast_sock *ns , unsigned char*file_name, int init_all_flag, char *diskname)
{
	int i;
	int ret = -1;
//	long int cap=0;
	int disknum = 0; 
	FILE *fp = NULL;
	unsigned char buf[200];
//	char cmd[200];
	memset(buf,0,sizeof(buf));
	gtopenlog("initdisk");
// ����ʱ��¼�ļ�
	fp = fopen(file_name,"w");
	if(fp==NULL)
	{
		printf("can not open %s\n", file_name);
		sprintf(buf, "can not open %s",file_name);
		send_test_report(ns, buf, 80);
		return ;
	}
	send_test_report(ns, "��ʼinitdisk��ʽ��Ӳ��", 10);
	gtloginfo("start format disk"); // lsk 2007 -4 -27
//���ļ��м�¼��ʽ�����̵���Ϣ
	fprintf(fp,"[%s]\n",FMAT_NODE);
	fprintf(fp,"%s=", REPORT);
	fflush(fp);
	
//ɱ����ؽ���

	system("killall -9 watch_proc \n");
	system("killall -15 hdmodule\n");
	system("killall -9 diskman\n");
	system("killall -9 tcpsvd\n");
	system("killall -9 e2fsck\n");
	system("killall -9 playback\n");
	sleep(2);

//ж�ش���
	//if(strstr(get_devtype_str(),"GTIP1004")!=NULL||strstr(get_devtype_str(),"GTIP2004")!=NULL)//gtvs3kϵ��  //yk change gtvs3k->GTIP1004
		system("umount /hqdata/sd*");
/*
	else
	{
		system("umount /hqdata/update");
		system("umount /hqdata");
	}
*/

//�ж�����Ӳ��
	memset(buf,0,sizeof(buf));
	
	ret=get_ide_flag();

	if(ret==1)	//װ����Ӳ��
	{
		printf("׼����ȡ���̸���...\n");
		disknum = get_sys_disk_num();
		if(disknum==0)
		{
			gtlogerr("no disk\n");
			fprintf(fp, "%d:%s,",NO_DISK, no_disk);
			result_report(NO_DISK, ns);// lsk 2007 -6-1
			goto endpoint;
		}
		sprintf(buf,"find %d disk(s)\n",disknum);
		gtloginfo("%s",buf);
		send_test_report(ns, buf, 15);//20);
		fprintf(fp,buf);

		printf("ϵͳ���̸���Ϊ[%d]\n",disknum);
	
		if(init_all_flag == 1)//��ʽ�����д��ڵ�Ӳ��
		{
			gtloginfo("debug:init_all_flag=1\n");
			for(i=0;i<disknum;i++)
			{
				// ����
				ret=init_ide_drv(ns,fp,get_sys_disk_devname(i),i,disknum);
			}
		}
		else//��ʽ��ָ����Ӳ��
		{
			gtloginfo("debug:init_all_flag!=1    sdfa\n");
			ret = init_ide_drv(ns,fp,diskname,0,1);
		}
endpoint:
		if(ret)
		{	
		gtlogerr("fmatdisk failed\n");
		}
		else
		{	
			send_test_report(ns, "��ʽ��Ӳ�̳ɹ�����", 100);
			result_report(0, ns);
			gtloginfo("fmatdisk succeed\n");
			set_record_partition("sda1");/*����sda1Ϊ��ǰ��¼�����*/
		}
	}
	else if((ret==2)||(ret==3))
		{
			//װ����SD��
			//ret=3ʱװ����TF�� zw-add 2010-07-12
			init_sd_drv(ns,get_sys_disk_devname(0));
			
		}
	else if(ret==0)
		{
			printf("��ȡ�豸���̱�־����,ret=[%d]\n",ret);
		}
	
	fclose(fp);
}



