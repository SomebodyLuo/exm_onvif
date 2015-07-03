///��������Ƿ�����
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <file_def.h>
#include <netinfo.h>
#include <confparser.h>
#include <commonlib.h>
#include <errno.h>
#include <wait.h>
#include <devinfo.h>
#include <sys/time.h>
#include <unistd.h>

#define  VERSION	"0.53"
/*
 * 0.53		�޸ĵ����¼���������ʽ��120s�������������5�������¼�����������������5��ʹ��ifconfig xxxup/down����
 * 0.52		���߻ָ�ʱ����rtcʱ��ͬ����ϵͳʱ��
 * 0.51		�����gtvs3000���¼������������Ĺ���(��Щ�豸ʹ�ù����������Ʋ�����������Ҳ������)
 * 0.50		�����arp�󶨵�֧��
 */

char *net_dev="eth0";	///Ҫ�������� eth0,eth1
int  internet_mode=0;	///internet����ģʽ
int  dhcp_mode=0;
char default_gw[100];	///���ص�ַ
char eth0_ip[100];
char eth0_mask[100];
char eth0_mac[100];

int  route_mac_bind=0;
char route_mac[100];	
///��������״̬�ı�
///stat 0:��ǰΪ����״̬�쳣
///	1:��ǰΪ����״̬����
void insert_mac(void)
{
        int tmp_errno;
        int ret;

        while(1)
        {
                errno=0;
                ret=system("lsmod | grep \"ftmac100\"");
                tmp_errno=errno;
                //printf("�������������Ľ��[%d]\n",ret);
                if(tmp_errno==0)
                {
                        if(ret==0)
                        {
                                //printf("���������������\n");
                                break;
                        }
                        else
                        {
                                //û���ҵ�������������������ȥ����   
                                errno=0;
                                //printf("û����������׼����׼��������������...\n");
                                system("/sbin/insmod /lib/modules/ftmac100.ko use_fiq=0");
                        }
                }
                else
                {
                        printf("ִ��system����errno=[%d]:[%s]\n",tmp_errno,strerror(tmp_errno));
                        return;
                }
		sleep(1);
        }
}



static int process_ifstat_change(char *dev,int stat,int flag)
{
	int st;
	pid_t pid;
	char *typestr=NULL;
	if(dev==NULL)
		return -EINVAL;
	if((stat!=0)&&(stat!=1))
		return -EINVAL;
	pid=fork();
	if(pid>0)
	{
		pid=wait(&st);
		return 0;
	}
	else if((int)pid<0)
		return errno;
	int ret;
	char pbuf[256];
	close_all_res();//�ر������Ѿ��򿪵���Դ
	typestr=get_devtype_str();
	if(stat==1)
	{
		if(strncmp(dev,"eth0",5)==0)
		{
			//if(0)//���������⣬����ʱ�᲻���� strncasecmp(typestr,"GTVS3",5)==0)
			if(strncasecmp(typestr,"GTVS3",5)==0&&(flag==1))
			{

				ret=system("ifconfig eth0 down");
				gtloginfo("���¼�����������!\n");
				ret=system("rmmod ftmac100");
				sleep(1);
				//ret=system("/sbin/insmod /lib/modules/ftmac100.ko use_fiq=0");
				insert_mac();
				sprintf(pbuf,"ifconfig eth0 hw ether %s",eth0_mac);
				ret=system(pbuf);
				sprintf(pbuf,"ifconfig eth0 %s netmask %s",eth0_ip,eth0_mask);
				ret=system(pbuf);
				ret=system("ifconfig eth0 mtu 1454");
					
			}
		
			printf("�����������...\n");
			ret=system("ifconfig eth0 up");
			ret=system("route add -net 224.0.0.0 netmask 255.0.0.0 dev eth0");
			ret=system("route add -net 225.0.0.0 netmask 255.0.0.0 dev eth0");
			ret=system("route add -net 239.0.0.0 netmask 255.0.0.0 dev eth0");
			if(internet_mode!=0)
			{
				sprintf(pbuf,"route add default gw %s eth0",default_gw);
				ret=system(pbuf);
				if(route_mac_bind==1)
				{
					sprintf(pbuf,"arp -s %s %s",default_gw,route_mac);
					ret=system(pbuf);
				}
			}			
			ret=system("killall -9 tcpdump");
			ret=system("/usr/sbin/tcpdump -i eth0 -U root udp port 3260 > /dev/null &");			
			if(strncasecmp(typestr,"GTVM",4)==0)
				ret=system("/gt1000/xvsnatset");	///����xvs�Ҳ����ļ������
			if((internet_mode==1)&&(dhcp_mode==1))
			{
				ret=system("killall -9 dhcpcd");
				ret=system("/gt1000/dhcpcd eth0 &");
				
			}
			ret=system("/sbin/hwclock -s ");
		}
		else if(strncmp(dev,"eth1",5)==0)
		{
			ret=system("ifconfig eth1 up");
			ret=system("route add -net 226.0.0.0 netmask 255.0.0.0 dev eth1");
			ret=system("killall -9 tcpdump1");               
                        ret=system("/usr/sbin/tcpdump1 -i eth1 -U root udp port 3260 > /dev/null &");       
			ret=system("/gt1000/xvsnatset");	///����xvs�Ҳ����ļ������
		}
	}
	else 
	{
		printf("�ر��������...\n");
	        if(strncmp(dev,"eth0",5)==0)
                {
               		ret=system("ifconfig eth0 down"); 
                }
                else if(strncmp(dev,"eth1",5)==0)
                {
			ret=system("ifconfig eth1 down");
                }	
	}
	exit(0);	
}

int main(int argc,char *argv[])
{
	confdict	*conf=NULL;
	char		*pstr=NULL;
	int 		ret,old_stat=-1;
	char 		logname[40];
	int		change_cnt=0;
	int 		reload=0;
	int		clock_flag=0;
	int 		err_cnt=0;
	struct timeval tv;
	struct timezone tz;
	unsigned long clock_tm=0;
	unsigned long tmp_cnt=0;



	if(argc>1)
		net_dev=argv[1];
	sprintf(logname,"ifcheck(%s)",net_dev);
	gtopenlog(logname);
	init_devinfo();
	conf=confparser_load("/conf/config");
	if(conf!=NULL)
	{

		internet_mode=confparser_getint(conf,"INTERNET_MODE",0);
		route_mac_bind=confparser_getint(conf,"ROUTE_MAC_BIND",0);
		pstr=confparser_getstring(conf,"ROUTE_MAC","NO");
		sprintf(route_mac,"%s",pstr);

		pstr=confparser_getstring(conf,"ETH0_IPADDR","10.0.1.0");
		sprintf(eth0_ip,"%s",pstr);
		pstr=confparser_getstring(conf,"ETH0_NETMASK","255.0.0.0");
		sprintf(eth0_mask,"%s",pstr);
		pstr=confparser_getstring(conf,"MAC_ADDRESS","00:aa:bb:cc:dd:ee");
		sprintf(eth0_mac,"%s",pstr);
		



		dhcp_mode=confparser_getint(conf,"USE_DHCP",0);

		pstr=confparser_getstring(conf,"ROUTE_DEFAULT","");
		sprintf(default_gw,"%s",pstr);
		confparser_freedict(conf);
	}
	else
	{
		gtlogerr("can't parse /conf/config!!\n");
	}
	while(1)
	{
		sleep(1);
		ret=get_link_stat(net_dev);
		if(ret!=old_stat)
		{
			if(++change_cnt<2)
				continue;
			if(ret==0)
			{//���������쳣
				printf("%s:���������쳣!!\n",net_dev);
				gtloginfo("%s:���������쳣!!\n",net_dev);
				
				gettimeofday(&tv,&tz);
				tmp_cnt=tv.tv_sec;
				if(clock_flag==0)
				{
					clock_tm=tmp_cnt+120; //���úö�ʱ��
					//printf("���ӵĿ�ʼʱ��:[%ld]\n",clock_tm);
					clock_flag=1;
				}
			
				//printf("��ǰʱ��[%ld]\n",tmp_cnt);	
				if(tmp_cnt<clock_tm)
				{
					//���������
					if(err_cnt<5)
					{
						printf("ʹ����ͣ����취\n");
						reload=0;	//ʹ����ͣ����İ취	
					}
					else
					{
						printf("ʹ�ü��������취\n");
						reload=1;	//���¼�������
						err_cnt=0;	//��λ���������
					}
				}
				else
				{
					//printf("��ʱ����120s,�����������0��������\n");
					err_cnt=0;
					clock_flag=0;	
				}
				//printf("��[%d]�ζ���\n",err_cnt);
				err_cnt++;
			}
			else if(ret==1)
			{//������������
				printf("%s:������������\n",net_dev);
				gtloginfo("%s:������������\n",net_dev);
			}
			process_ifstat_change(net_dev,ret,reload);
			old_stat=ret;
			change_cnt=0;
		}
		else
			change_cnt=0;
		
	}
	exit(0);
}





