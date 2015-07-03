/*
 * ʹ�����ļ������µĽ�
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <iniparser.h>
#include <file_def.h>
#include <commonlib.h>
#include <errno.h>
#include "converts.h"
#include <devinfo.h>
#include <confparser.h>
#define PARA_BAK_FILE		"/conf/ip1004_bak.ini"
#define	DEVINFO_PARA_FILE_BAK	"/conf/devinfo_bak.ini"
#define CONFIG_FILE		"/conf/config"


//��vm_tools��Makefile�ж��� #define USE_VM			//wsy,ʹ��gtvmʱ����

#define VERSION "0.62"
// 0.62 �����max_wan_usr Ĭ������
// 0.61 �����/conf/devinfo�ļ���ʧ��Ļָ�
// 0.60 �����/conf/config�ļ���ʧ��Ļָ�
// 0.59 �����upnp��dhcp����������
// 0.58 �����GTVM��֧��
// 0.57 ����fix eth1 mac��ַ�Ĺ���
// 0.56 ���뱨����ʾ���ı���actiondefine:40
// 0.55 ���뽫leave_fac�з�ɢ��ʱ��ת��Ϊһ���ַ���
// 0.54 �����/conf/config�ļ��е�eth_port������֧��,����������ļ��ͱ����ļ��Ƚ�ʱ��Ϣ��¼��׼ȷ������
// 0.53 ����ӱ����ļ��лָ����ƻ��������ļ��Ĺ���
// 0.52 �����setalarmĬ��ֵ���豸����Ĭ��ֵ��ƥ��������֧��
// 0.51 ����Ա���ץͼ�����ļ���֧��
// 0.5 �����˶������ļ��б�������ʹ�ܵĻָ�
// 0.4 �޸���mac_conv��ʵ�֣�ʹ��֧�ֵ����޸�mac��ַ
// 0.3 ���쿴�����ļ��ڵĹ��ܣ����Ӷ�devinfo.ini�ļ��ڵ��ж�
// 0.2 ������������ļ���û�б������Ա��������alarm:trig_inl�����ɱ�������
typedef struct{
	//����Ҫ��飬���û������Ҫ���ӵı����ṹ
	char *key;	//������
	char *defval;	//����ֵ���ַ�����ʽ
}KeyStruct;
static KeyStruct IP1004NeedKeys[]=
{//ip1004.ini 
	{"actiondefine:40",		"������ʾ��"		},
	{"netencoder:max_wan_usr",      "8"}
};

static  char *IP1004NeedSection[]={
//��Щ��û��������ӽ�
	"alarmversion",
	"actiondefine",
	"alarm0",
	"alarm1",
	"alarm2",
	"alarm3",
	"alarm4",
	"alarm5",
	"motion0",
	"motion1",
	"motion2",
	"motion3",
	"a_user_def"
};
static char *DevinfoNeedSection[]={
//��Щ��û�����������Ӧ�Ľ�
	"devinfo",	
	"resource",
	"leave_fac"
};

static char *restore_section[]={
//��Щ��û�����ӱ����ļ��лָ�
	"install",
	"port",
	"remote_gate",
	"netencoder",
	"video0"	
};

///devinfo.ini�����û����Щ�������ӱ����ļ��лָ�
static char *restore_devinfo_section[]={
	"devinfo",
	"leave_fac",
	"resource",
};
static char *restore_conf_var[]={
///conf/config �ļ����û����Щ����������para_conv -s�ָ�
	"MAC_ADDRESS",
	"ETH_PORT",
	"ETH0_IPADDR",
	"ETH0_NETMASK",
	"LOGIN_PORT"
};
static int NeedSectionFlag[100];//����Ƿ���Ҫ���ӽڵı�־
int ProcessAlarmSec(dictionary      *ini)
{
	int i;
	//int Ret;
	int Cnt=0;
	int Val;
	unsigned long temp;
	char *pstr=NULL;
	char Section[256];
	int AlarmAttrib[8];//������������
	int AlarmEnable[8];//����ʹ������
	int BadFlag=0;
	pstr=iniparser_getstring(ini,"alarm:trig_in",NULL);
	if(pstr==NULL)
	{
		temp=0;
	}
	else
		temp=atohex(pstr);	
	for(i=0;i<8;i++)
	{
		if(temp&(1<<i))
			AlarmAttrib[i]=1;
		else
			AlarmAttrib[i]=0;
	}
	for(i=0;i<6;i++)
	{//alarm0~alarm5
		sprintf(Section,"alarm%d:attrib",i);
		Val=iniparser_getint(ini,Section,-100);
		if((Val!=0)&&(Val!=1))
		{
			//�����ļ���ԭ��û�иñ���
			iniparser_setint(ini,Section,AlarmAttrib[i]);
			gtloginfo("restore %s %d->%d\n",Section,Val,AlarmAttrib[i]);
			printf("restore %s %d->%d\n",Section,Val,AlarmAttrib[i]);
			Cnt++;
			BadFlag=1;	//����setalarm���������Ĭ��ֵ(ʹ�ܱ�־��Ĭ��ֵΪ0)������ͨ��������ж������ļ���
					//ʹ��λ�Ƿ���Ҫ�޸�
		}
		
	}

	for(i=0;i<8;i++)
		AlarmEnable[i]=1;	//��������Ҫ������,�����������Ĭ��ֵΪ������
        for(i=0;i<6;i++)
        {//alarm0~alarm5
                sprintf(Section,"alarm%d:enable",i);
                Val=iniparser_getint(ini,Section,-100);
        	if(((Val!=0)&&(Val!=1))||(BadFlag))
                {
                        //�����ļ���ԭ��û�иñ���
                        iniparser_setint(ini,Section,AlarmEnable[i]);
			gtloginfo("restore %s %d->%d\n",Section,Val,AlarmEnable[i]);
			printf("restore %s %d->%d\n",Section,Val,AlarmEnable[i]);
                        Cnt++;
                }

        }	
       for(i=0;i<6;i++)
        {//alarm0~alarm5
	//��������Ҫ������,Ĭ��Ϊ����
                sprintf(Section,"alarm%d:setalarm",i);
                Val=iniparser_getint(ini,Section,-100);
                if(((Val!=0)&&(Val!=1))||(BadFlag))
                {
                        //�����ļ���ԭ��û�иñ���
                        iniparser_setint(ini,Section,AlarmEnable[i]);
                        gtloginfo("restore %s %d->%d\n",Section,Val,1);
                        printf("restore %s %d->%d\n",Section,Val,1);
                        Cnt++;
                }

        }


	
	return Cnt;
}
int FixVideoEncPara(dictionary *ini)
{///����Ƶ�����������еı�����ȥ��
	int total;
	int i;
	int change_cnt=0;
	char key[200];
	int num;
	total=get_videoenc_num();
	for(i=0;i<total;i++)
	{	
		sprintf(key,"%s:bitratecon",get_video_enc_ini_sec(i));
		num=iniparser_getint(ini,key,-1);
		if(num==0)
		{//������
			printf("fix enc%d bitratecon vbt->hbr\n",i);
			gtloginfo("fix enc%d bitratecon vbr->hbr\n",i);
			printf("set enc%d min=%d max=%d\n",i,256,2000);
			gtloginfo("set enc%d min=%d max=%d\n",i,256,2000);
			iniparser_setint(ini,key,2);	///���óɻ������
			sprintf(key,"%s:minbitrate",get_video_enc_ini_sec(i));
			iniparser_setint(ini,key,256);
			sprintf(key,"%s:maxbitrate",get_video_enc_ini_sec(i));
			iniparser_setint(ini,key,2000);
			change_cnt++;
		}
	}
	return change_cnt;
}
int CheckAndAddKeys(char *FileName,KeyStruct *Keys,int Total)
{
	int SaveFlag=0;
	int i,ret;
	dictionary *ini=NULL;
	FILE	*fp=NULL;
	KeyStruct *k=NULL;
	printf("%s need %d keys\n",FileName,Total);
	ini=iniparser_load_lockfile(FileName,0,&fp);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",FileName);
                return -errno;
        }
	for(i=0;i<Total;i++)
	{
		k=&Keys[i];
		ret=iniparser_find_entry(ini,k->key);
                if(ret!=1)
                {
                        printf("can't find key:%s\n",k->key);
			iniparser_setstr(ini,k->key,k->defval);
			printf("add key [%s]=%s to %s\n",k->key,k->defval,FileName);
        		gtloginfo("add key [%s]=%s to %s\n",k->key,k->defval,FileName);
	                SaveFlag++;
                }
	}
	if(SaveFlag>0)
	{
		save_inidict_file(FileName,ini,&fp);
		fp=NULL;
	}
	if(fp!=NULL)
		fclose(fp);
	iniparser_freedict(ini);	
	return SaveFlag;
}
int CheckAndAddSection(char *FileName,char *Sections[],int Total)
{
	int		ret;
	int 		i;
	int		LostSecCnt=0;
	char 		*Sec;
	dictionary      *ini;
	FILE 		*fp=NULL;
	int         lock;
	
	printf("%s need %d sections\n",FileName,Total);
	ini=iniparser_load(FileName);
	if(ini==NULL)
	{
		printf("can't load file %s!!\n",FileName);
		return -errno;
	}
	for(i=0;i<Total;i++)
	{
		Sec=Sections[i];
		ret=iniparser_find_entry(ini,Sec);
		if(ret!=1)
		{
			printf("can't find entry:%s\n",Sec);
			LostSecCnt++;
			NeedSectionFlag[i]=1;
		}
		else
		{
			NeedSectionFlag[i]=0;
		}
	}
	iniparser_freedict(ini);
	
	if(LostSecCnt!=0)
	{	
		fp=fopen(FileName,"a");
		if(fp==NULL)
		{
			printf("can't create %s!!\n",FileName);
			return -errno;
		}
		lock=lock_file(fileno(fp),0);
		if(lock<0)
		{
			gtloginfo("iniparser_load_lockfile lock=%d(%s)!!!!\n",lock,strerror(errno));
			fclose(fp);
			return -1;
		}
		else
		{		
			fprintf(fp,"\n");
			for(i=0;i<Total;i++)
			{
				Sec=Sections[i];
				if(NeedSectionFlag[i])
				{
					fprintf(fp,"[%s]\n",Sec);
					gtloginfo("add section[%s] to %s\n",Sec,FileName);
				}
			}
		}
		unlock_file(fileno(fp));
		fclose(fp);
	}		
	return 0;
	
}
//�ж�ip1004.ini�Ƿ��ƻ�,����ƻ�����лָ�
static int restore_conf_file(void)
{       
        confdict 	*conf=NULL;
        int total=sizeof(restore_conf_var)/sizeof(char*);
        int i,ret,lost_cnt=0;
	char *vstr=NULL;
        char buf[100];
        conf=confparser_load("/conf/config");
        if(conf==NULL)
        {
                printf("can't load file %s!!\n","/conf/config");
                gtlogerr("can't load file %s!!\n","/conf/config");
        	lost_cnt=100;       
        }
	else
	{
        
        	for(i=0;i<total;i++)
        	{
                	vstr=confparser_getstring(conf,restore_conf_var[i],NULL);
                	if(vstr==NULL)
                	{
                        	printf("can't find entry:%s\n",restore_conf_var[i]);
                        	gtlogerr("can't find entry:%s\n",restore_conf_var[i]);
                        	lost_cnt++;
                	}
        	}       
        	confparser_freedict(conf);
	}
        if(lost_cnt!=0)
        {
                sprintf(buf,"/ip1004/para_conv -s\n");
                ret=system(buf);
                printf("/conf/config���ƻ�,��/conf/ip1004.ini�лָ�,ret=%d!\n",ret);
                gtlogwarn("/conf/config���ƻ�,��/conf/ip1004.ini�лָ�,ret=%d!\n",ret);
        }
        return lost_cnt;
}

//�ж�gt1000.ini�Ƿ��ƻ�,����ƻ�����лָ�
static int restore_file(void)
{
	dictionary      *ini=NULL;
	int total=sizeof(restore_section)/sizeof(char*);
    	int i,ret,lost_cnt=0;
	char buf[100];
    	ini=iniparser_load(IPMAIN_PARA_FILE);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",IPMAIN_PARA_FILE);
                gtlogerr("can't load file %s!!\n",IPMAIN_PARA_FILE);
		lost_cnt=100;
        }
	else
	{
	    for(i=0;i<total;i++)
	    {
                ret=iniparser_find_entry(ini,restore_section[i]);
                if(ret!=1)
                {
                        printf("can't find entry:%s\n",restore_section[i]);
			gtlogerr("can't find entry:%s\n",restore_section[i]);
                        lost_cnt++;
                }
	    }
		iniparser_freedict(ini);
	}
	if(lost_cnt!=0)
	{
		ret=ini_diff(PARA_BAK_FILE,IPMAIN_PARA_FILE);
		sprintf(buf,"cp -f %s %s\n",PARA_BAK_FILE,IPMAIN_PARA_FILE);
		ret=system(buf);
		printf("%s���ƻ�,��%s�лָ�,ret=%d!\n",IPMAIN_PARA_FILE,PARA_BAK_FILE,ret);
		gtlogerr("%s���ƻ�,��%s�лָ�,ret=%d!\n",IPMAIN_PARA_FILE,PARA_BAK_FILE,ret);
	}
	return lost_cnt;
}

//�ж�devinfo.ini�Ƿ��ƻ�
static int restore_devinfo_file(void)
{
        dictionary      *ini=NULL;
        int total=sizeof(restore_devinfo_section)/sizeof(char*);
        int i,ret,lost_cnt=0;
        char buf[100];
        ini=iniparser_load(DEVINFO_PARA_FILE);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",DEVINFO_PARA_FILE);
                gtlogerr("can't load file %s!!\n",DEVINFO_PARA_FILE);
                lost_cnt=100;
        }
        else
        {
            for(i=0;i<total;i++)
            {
                ret=iniparser_find_entry(ini,restore_devinfo_section[i]);
                if(ret!=1)
                {
                        printf("can't find entry:%s\n",restore_devinfo_section[i]);
                        gtlogerr("can't find entry:%s\n",restore_devinfo_section[i]);
                        lost_cnt++;
                }
            }
                iniparser_freedict(ini);
        }
        if(lost_cnt!=0)
        {
                ret=ini_diff(DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
                sprintf(buf,"cp -f %s %s\n",DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
                ret=system(buf);
                printf("%s���ƻ�,��%s�лָ�,ret=%d!\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK,ret);
                gtlogerr("%s���ƻ�,��%s�лָ�,ret=%d!\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK,ret);
        }
        return lost_cnt;
}
int fix_config_file(void)
{//�鿴config�ļ��е�����,��������Ӧ��ֵ
	confdict *conf=NULL;
	FILE *fp=NULL;
	int	num;
	char	buf[20];
	int	needwrite=0;
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if(conf==NULL)
	{
		printf("can't parse %s!!\n",CONFIG_FILE);
		gtlogerr("can't parse %s!!\n",CONFIG_FILE);
		return -1;
	}
	
	//������
	num=confparser_getint(conf,"ETH_PORT",-1);
	if(num!=get_eth_num())
	{
		sprintf(buf,"%d",get_eth_num());
		confparser_setstr(conf,"ETH_PORT",buf);
		printf("%s:%s %d->%d\n",CONFIG_FILE,"ETH_PORT",num,get_eth_num());
		gtloginfo("%s:%s %d->%d\n",CONFIG_FILE,"ETH_PORT",num,get_eth_num());
		needwrite++;		
	}
	num=confparser_getint(conf,"USE_DHCP",-1);
	if(num<0)
	{
		confparser_setstr(conf,"USE_DHCP","0");
		printf("%s:%s NONE->0\n",CONFIG_FILE,"USE_DHCP");
		gtloginfo("%s:%s NONE->0\n",CONFIG_FILE,"USE_DHCP");
		needwrite++;
	}
        num=confparser_getint(conf,"USE_UPNP",-1);
        if(num<0)
        {
                confparser_setstr(conf,"USE_UPNP","0");
                printf("%s:%s NONE->0\n",CONFIG_FILE,"USE_UPNP");
                gtloginfo("%s:%s NONE->0\n",CONFIG_FILE,"USE_UPNP");
                needwrite++;
        }


	if(needwrite!=0)
	{
		confparser_dump_conf(CONFIG_FILE,conf,fp);
	}
	confparser_freedict(conf);
	if(fp!=NULL)
		fclose(fp);
	return needwrite;
	
}
int fix_devinfo_file(void)
{
	dictionary      *ini=NULL;
	char		*date=NULL;
	char		date_buf[100];
	int 		year,mon,day,hour,min,sec;
	int		change_flag=0;
	ini=iniparser_load(DEVINFO_PARA_FILE);
	if(ini==NULL)
		return -ENOENT;
	//[leave_fac] 
	//����ɢ��ʱ��ṹת��ΪYYYY-MM-DD HH:MM:SS���ַ����ŵ�date������
	year=iniparser_getint(ini,"leave_fac:year",-1);
	mon=iniparser_getint(ini,"leave_fac:mon",-1);
	day=iniparser_getint(ini,"leave_fac:day",-1);
	hour=iniparser_getint(ini,"leave_fac:hour",0);
	min=iniparser_getint(ini,"leave_fac:min",0);
	sec=iniparser_getint(ini,"leave_fac:sec",0);
	if((year<0)||(mon<0)||(day<0))//�����ļ���û�г���������Ϣ
		return 0;
	sprintf(date_buf,"%04d-%02d-%02d %02d:%02d:%02d",year,mon,day,hour,min,sec);
	date=iniparser_getstring(ini,"leave_fac:date",NULL);
	if(date==NULL)
	{
		iniparser_setstr(ini,"leave_fac:date",date_buf);
		change_flag++;
	}
	else
	{
		if(strcmp(date_buf,date)!=0)
		{
			iniparser_setstr(ini,"leave_fac:date",date_buf);
                	change_flag++;
		}
	}
	/////

	if(change_flag>0)
		save_inidict_file(DEVINFO_PARA_FILE,ini,NULL);
	iniparser_freedict(ini);
	return 0;
}
int main(void)
{
	dictionary      *ini;
	int ret;

	int change_flag=0;

	char buf[100];
	gtopenlog("patchconf");
	printf("run patchconf version:%s\n",VERSION);
	gtloginfo("run patchconf version:%s\n",VERSION);
	restore_devinfo_file();
	fix_devinfo_file();
	CheckAndAddSection(DEVINFO_PARA_FILE,DevinfoNeedSection,sizeof(DevinfoNeedSection)/sizeof(char*));

	init_devinfo();
	ret=mac_check_conv(0);//���mac��ַ�Ƿ�Ϸ�������Ƿ����дΪguidת��������
	if(get_eth_num()>1)
		ret=mac_check_conv(1);
	restore_file();//�ж��Ƿ���Ҫ�ָ�IPMAIN_PARA_FILE
	CheckAndAddSection(IPMAIN_PARA_FILE,IP1004NeedSection,sizeof(IP1004NeedSection)/sizeof(char*));
	CheckAndAddKeys(IPMAIN_PARA_FILE,IP1004NeedKeys,sizeof(IP1004NeedKeys)/sizeof(KeyStruct));	
	ini=iniparser_load(IPMAIN_PARA_FILE);
	if(ini==NULL)
	{
		printf("can't load file %s!!\n",IPMAIN_PARA_FILE);
		exit(1);
	}
	if(ProcessAlarmSec(ini)>0)
	{//����������
		change_flag=1;
	}
	if(FixVideoEncPara(ini)>0)
	{//������Ƶ����������
		change_flag=1;
	}	
	if(change_flag)
	{
		save_inidict_file(IPMAIN_PARA_FILE,ini,NULL);
	}	
	iniparser_freedict(ini);

//�Ƚ�devinfo.ini�ļ��ͱ����Ƿ���ͬ
        ret=ini_diff(DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
        if(ret!=0)

        {       //���±����ļ�

                sprintf(buf,"cp -f %s %s\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK);

                ret=system(buf);

                printf("���±��������ļ�%s,ret=%d!\n",DEVINFO_PARA_FILE_BAK,ret);

                gtloginfo("���±��������ļ�%s,ret=%d!\n",DEVINFO_PARA_FILE_BAK,ret);

        }



	//�Ƚ�gt1000.ini�ļ��ͱ����Ƿ���ͬ
	//sprintf(buf,"cmp %s %s\n",IPMAIN_PARA_FILE,PARA_BAK_FILE);
	//ret=system(buf);
	ret=ini_diff(PARA_BAK_FILE,IPMAIN_PARA_FILE);
	if(ret!=0)
	{	//���±����ļ�
		sprintf(buf,"cp -f %s %s\n",IPMAIN_PARA_FILE,PARA_BAK_FILE);
                ret=system(buf);
                printf("���±��������ļ�%s,ret=%d!\n",PARA_BAK_FILE,ret);
                gtloginfo("���±��������ļ�%s,ret=%d!\n",PARA_BAK_FILE,ret);
	}
	restore_conf_file();
	fix_config_file();
	exit(0);
}

