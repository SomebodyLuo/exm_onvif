/*���ڽ����������ļ����ж�תһ��һת��Ĺ��ߣ�wsy august 2006*/
#include <stdio.h>
#include <stdlib.h>
#include "file_def.h"
#include "iniparser.h"
#include "confparser.h"
#include "commonlib.h"
#include <devinfo.h>

#define DHCP_FILE	"/etc/sysconfig/network-scripts/ifcfg-eth0"

#define CHAP_FILE 	"/etc/ppp/chap-secrets"	
#define PAP_FILE  	"/etc/ppp/pap-secrets"
#define PPP0_FILE 	"/etc/sysconfig/network-scripts/ifcfg-ppp0"
#define VSFTPD_FILE "/etc/vsftpd.conf"
#define BOA_FILE 	"/etc/boa/boa.conf"

#define CANT_FIND_KEY "no_find_key" //getstring�Ҳ�����Ӧ�ؼ���ʱ��default����

#define VERSION "2.1"           //�汾��
/*
//2.1 ����˹���Ϊ��ip1004�¿�ʹ�ã���Ӧ����Ҫ�ı�
//2.0 ����ת��devinfo.ini��resource:disk_capacity
//1.9 ����gt1000.ini��net_config:route_mac_bind��route_mac��config��ӳ��
//1.8 ����devinfo:devtypestring->config:HOST_NAME
//1.7 ִ��-sʱ��gt1000.ini��devinfo:devtypestring���õ�config�ļ���HOST_NAME	
//1.6 ����boa����ÿ����һ�ζ�һ�п���
//1.5 ��PORT��port:telnet_port��Ӧ������������netconfig:telnet_port
//1.42 �����ini_to_conf()������conf�ļ���û���ƶ�keyʱ����δ��������,�޸���һЩ��ʾ�ַ���,�޸���ini_to_conf�������޸ı�����ʵ��

//1.41 ������Ϣ��Ϊ�����Ľ�[eth0][eth1]...
//1.4 ������get_boa_str�����ַ������⣬���䵼�µ�boa��ز���������������
//1.3 ��������־��һת��д��"��תһ"������
//1.2 ���������¸Ķ�
		1.��־��¼������࣬�磺
			û�б����������ɾ������"NULL"��ʾ,�仯����"->"��ʾ
		�� devinfo:devtype NULL->1
		2.��Ϊ���б������ݶ�������gt1000.ini�У�������־��¼ֻҪ��¼gt1000.ini�ı仯�Ϳ�����;
			���磺
				ִ��para_conv -m ��ʱ��ֻҪ��¼leave_fac:mon 08->09
				ִ��para_conv -s ��ʱ��ֻҪ��¼net_config:eth0_ipaddr 192.168.3.1->192.168.3.29
		3.����-m��-s��������ʱ����¼������Ϣ(�汾�ţ�����)����־  DONE
			��:run para_conv -s ,version:1.01 
		4.���ĵ�����Ĭ��ֵ��û��Ĭ��ֵ�Ĳ������򲻴���
		5.devinfo��ÿ��������ԭ���ŵ�gt1000.ini,ʵ��dev_to_gt1000����
		6.������****
		7.��־�����
		8.��־���Ƿ��޸ģ�����grep
		
//1.1 ȥ����flags��ȷ����ȷ�Ժͳ���ļ�
//1.0 initial
*/
static int change_flag=0;

//���������Ϣ���˳�
int print_helpmsg(void)
{
	printf("\n��ӭʹ��para_conv�����ļ�ת������!(version %s)\n\n",VERSION);
	printf("ʹ��˵��: para_conv [OPTION]\n\n");
	printf("\t-h\t���������Ϣ\n");
	printf("\t-v\t����汾��Ϣ\n");
	printf("\t-m\t��תһ���� ��������ļ� -> gt1000.ini\n");
	printf("\t-s\tһת�࣬��   gt1000.ini -> ������������ļ�\n\n");
	return(0);
}

//����汾��Ϣ���˳�
int print_version(void)
{
	printf("\nThis is para_conv version %s\n\n",VERSION);
	return (0);
}


//��fpָ���chap�����ļ��ж���usr,pswd,�ɹ�����0�����򷵻ظ�ֵ
int get_chap_user_pswd(FILE *fp, char *usr, char *pswd)
{
	char buf[500];
	char tmp[100];
	char *lp,*lk,*lm,*ln;
	
	if((fp==NULL)||(usr==NULL)||(pswd==NULL))
	{	
		gtloginfo("get_chap_user_pswd��������Ϊ��,����\n");
		printf("get_chap_user_pswd��������Ϊ��,����\n");
		return -1;
	}
		
	while(1)
	{
		if(fgets(buf,500,fp)==NULL)//should never get here
			{
				return -1;
			}
		if(buf[0]=='#')//ע���в�����
			{
				continue;
			}
		lp=index(buf,'\"');//Ѱ�ҵ�һ��˫����
		if(lp!=NULL)
			{	
				lp++;
				lk=index(lp,'\"');//Ѱ�ҵڶ���˫����
				if(lk!=NULL)
					{
						strncpy(tmp,lk,100);//�ѵڶ�������֮���cpy��tmp
						*lk='\0';	
						strncpy(usr,lp,100);//��user����ȡ����cpy��usr
						lm=index(tmp,'*');//Ѱ��*��
						if(lm!=NULL)
							{
								lm=index(lm,'\"');//Ѱ�ҵ���������
								if(lm!=NULL)
								{
									lm++;
									ln=index(lm,'\"');//Ѱ�����һ������
									if(ln!=NULL)
									{
										*ln='\0';
										strncpy(pswd,lm,100);//��pswd����ȡ����cpy��pswd
										return 0;
									}
								}
							}
					}			
			}

	}
	return -1; //��ʲô����û�����������
}

//��fpָ���chap�����ļ���д��usr��pswd
int set_chap_user_pswd(FILE *fp, char *usr, char *pswd)
{
	char buf[5500];
	char line[100];
	char *lp;

	if((fp==NULL)||(usr==NULL)||(pswd==NULL))
	{	
		gtloginfo("set_chap_user_pswd��������Ϊ��,����\n");
		printf("set_chap_user_pswd��������Ϊ��,����\n");
		return -1;
	}
	sprintf(line,"\n\"%s\" * \"%s\"",usr,pswd);

	rewind(fp);
	fread(buf,5500,1,fp);
	lp=index(buf,'"');//�ҵ���һ������
	if(lp!=NULL)
		{
			*lp='\0';
		}
	strcat(buf,line);
	rewind(fp);
	fwrite(buf,strlen(buf),1,fp);
	
	return 0;
}
//��boa�ļ���ȡ��key��Ӧstr,�ɹ�����0�����򷵻ظ�ֵ
int get_boa_str(FILE *fp, char *key, char *str)
{
	char buf[500];
	char *lp,*lk,*lm;
	char tmp;
	if((fp==NULL)||(key==NULL))
	{	
		gtloginfo("get_boa_str��������Ϊ��,����\n");
		printf("get_boa_str��������Ϊ��,����\n");
		return -1;
	}
	
	while(1)
	{
		if(fgets(buf,500,fp)==NULL)
			{
				return -1;
			}
		if(buf[0]=='#')
			continue;
		lp=strstr(buf,key);
		if(lp!=NULL)
			{
				lp++;
				lk=index(lp,' ');
				if(lk!=NULL)
					{
						lk++;
						while(1)
							{
								if(*lk==' ')
									lk++;
								else
									break;
							}
						
						lm=index(lk,' ');
					
						if(lm==NULL)
							{
							
							lm=index(lk,'\t');
							}
						
						if(lm==NULL)
							{
							lm=index(lk,'\n');
							}
						if(lm==NULL)
							{
							lm=index(lk,'\0');
							}
						if(lm==NULL)
							{
							return -1;
							}
						tmp=*lm;
						*lm='\0';
						break;
					}			
			}
	}
	//printf("lk is %s\n",lk);
	sprintf(str,lk,strlen(lk));
	return 0;
}

//��fpָ���boa�ļ��е�key��д��value��ֵ�����ɹ�����-1���ɹ�����0
int set_boa_str(FILE *fp,char *key,char *value)
{
	char keyline[100];
	char tmp[15000];
	char line[200];
	char  buf[15000];
	char *lp,*lk;

	if((fp==NULL)||(key==NULL)||(value==NULL))
	{	
		gtloginfo("set_boa_str��������Ϊ��,����\n");
		printf("set_boa_str��������Ϊ��,����\n");
		return -1;
	}
	sprintf(keyline,"\n%s ",key);//"Port "
	sprintf(line,"%s %s\n",key,value);
	
	rewind(fp);
	fread(tmp,15000,1,fp);
	
	lp=strstr(tmp,keyline);
	if(lp==NULL)//û�иýڣ�����
		{
			strncat(tmp,line,15000);
			//printf("strlen tmp is %d\n",strlen(tmp));
			rewind(fp);
			fwrite(tmp,strlen(tmp),1,fp);
		}
	else //�иýڣ���д
		{
			
			lk=index(lp+1,'\n');
			if(lk==NULL)
				{
					sprintf(lp,"\n%s",line);
					
				}
			else
			{
				lk++;
				strncpy(buf,lk,strlen(lk));
				sprintf(lp,"\n%s%s",line,buf);
			}
			rewind(fp);
			fwrite(tmp,strlen(tmp),1,fp);
		}
	
	return 0;
}

/*��ini�ļ���inisec�ڶ�������conf�ļ���confsec�ڶ������ݱȽϣ�����ͬ���дini
  ���ĸ��������򴴽��ĸ�����ȱʡֵ*/
int conf_to_ini(char *confname, dictionary *ini, confdict *conf, char * inisec, char *confsec, char * def)
{
	char *inistr;
	char *confstr;
	
	if((confname==NULL)||(ini==NULL)||(conf==NULL)||(inisec==NULL)||(confsec==NULL))
	{	
		gtloginfo("conf_to_ini��������Ϊ��,����\n");
		printf("conf_to_ini��������Ϊ��,����\n");
		return -1;
	}
	
	confstr=confparser_getstring(conf,confsec,NULL);
	if(confstr==NULL)//�Ҳ�����Ӧ����
	{
		if(def==NULL)//��ȱʡֵ
			return -1;
		else
		{
			printf("%sû��%s\t,����ȱʡΪ%s\n",confname,confsec,def);
			confparser_setstr(conf,confsec,def);
			change_flag=1;
			confstr=def;
		}
	}
	
	inistr=iniparser_getstring(ini, inisec, CANT_FIND_KEY);
	#if 0 //shixin del these 2006.11.16
	if(strcmp(inistr,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����Ϊ%s\n",inisec,confstr);
			gtloginfo("%-30sNULL->%s\n",inisec,confstr);
			iniparser_setstr(ini,inisec,confstr);
			change_flag=1;
			inistr=confstr;
		}
	#endif
	if(strcmp(inistr,confstr)!=0)
	{
			printf("%-30s%s->%s\n",inisec,inistr,confstr);
			gtloginfo("%-30s%s->%s\n",inisec,inistr,confstr);
			change_flag=1;
			iniparser_setstr(ini,inisec,confstr);
	}
	return 0;
}

/*��ini�ļ���inisec�ڶ�������conf�ļ���confsec�ڶ������ݱȽϣ�����ͬ���дconf
  ���ĸ��������򴴽��ĸ�����ȱʡֵ*/
int ini_to_conf(char *confname,dictionary *ini, confdict *conf, char * inisec, char *confsec, char * def)
{
	char *inistr;
	char *confstr;
	
	if((confname==NULL)||(ini==NULL)||(conf==NULL)||(inisec==NULL)||(confsec==NULL))
	{	
		gtloginfo("conf_to_ini��������Ϊ��,����\n");
		printf("conf_to_ini��������Ϊ��,����\n");
		return -1;
	}
	inistr=iniparser_getstring(ini, inisec,NULL);
	if(inistr==NULL)//�Ҳ�����Ӧ�����򴴽�
	{
		if(def==NULL)
			return -1;
		else
		{
			printf("gt1000.iniû��%s,\t����Ϊ%s\n",inisec,def);
			iniparser_setstr(ini,inisec,def);
			change_flag=1;
			inistr=def;	//shixin changed NULL->def
		}
	}

	confstr=confparser_getstring(conf,confsec,CANT_FIND_KEY);
#if 0 //shixin del these 2006.11.16
	if(strcmp(confstr,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ����
		{
				printf("%sû��%s\t,����ȱʡΪ%s\n",confname,confsec,def);
				confparser_setstr(conf,confsec,def);
				change_flag=1;
				confstr=NULL;
		}
#endif	
	if(strcmp(inistr,confstr)!=0)
	{
		printf("[%s] %s : %s->%s\n",confname,confsec,confstr,inistr);
		gtloginfo("%-30s%s->%s\n",inisec,confstr,inistr);
		change_flag=1;
		confparser_setstr(conf,confsec,inistr);
	}
	return 0;
}

/*��devinfo�ļ���ÿ���ڶ�������gt1000�ļ���Ӧ�ڶ������ݱȽϣ�
  ����ͬ���дgt1000�����ñ��
  ��gt1000.ini�����ڸý��򴴽�*/
int dev_to_gt1000(dictionary *gt1000)
{
	dictionary *devinfo;
	char *gt1000str;
	int i;
	
	if(gt1000==NULL)
		{
			printf("gt1000 dictΪNULL,�˳�dev_to_gt1000\n");
			gtloginfo("gt1000 dictΪNULL,�˳�dev_to_gt1000\n");
			return -1;
		}
	
	devinfo=iniparser_load(DEVINFO_PARA_FILE);
	if (devinfo==NULL) 
        {
             printf("cannot parse ini file [%s]",DEVINFO_PARA_FILE);
			 gtloginfo("����%sʧ���˳�\n",DEVINFO_PARA_FILE);	
             return(-1) ;
		}
	
	for (i=0 ; i<devinfo->size ; i++) 
	{
        if (devinfo->key[i]==NULL)
            continue ;
        if (strchr(devinfo->key[i], ':')==NULL)//û��:,Ϊ���� 
        	continue;
        if (strstr(devinfo->key[i], "disk_capacity") != NULL)//������������ת��
    		continue;
    		
        //��ÿһ������, ��gt1000�Ƿ��У����û���򴴽�
        gt1000str=iniparser_getstring(gt1000,devinfo->key[i],CANT_FIND_KEY);
		if(strcmp(gt1000str,CANT_FIND_KEY)==0)//û��
        	{
        		iniparser_setstr(gt1000,devinfo->key[i],devinfo->val[i]);
				change_flag=1;
				printf("gt1000.ini��%s����Ϊ%s\n",devinfo->key[i],devinfo->val[i]);
				gtloginfo("%-30sNULL->%s\n",devinfo->key[i],devinfo->val[i]);

        	}
        else
        	{
        		
				if(strncmp(gt1000str,devinfo->val[i],strlen(devinfo->val[i]))!=0)//�е�����ͬ
					{
						gtloginfo("%-30s%s->%s\n",devinfo->key[i],gt1000str,devinfo->val[i]);
						iniparser_setstr(gt1000,devinfo->key[i],devinfo->val[i]);
						printf("gt1000.ini��%s�޸�Ϊ%s\n",devinfo->key[i],devinfo->val[i]);
						change_flag=1;
					}
				else
					continue;
				 
        	}
       
	}

	iniparser_freedict(devinfo);
	
	return 0;
}


/*��devinfo�ļ���ָ���ڶ�������/conf/config�ļ���Ӧ�ڶ������ݱȽϣ�
  ����ͬ���дconfig�����ñ��
  ��gt1000.ini�����ڸý��򴴽�*/
int dev_to_conf(confdict *dict, char *devsec,char *cfgsec)
{
	dictionary *devinfo;
	char *devinfostr;
	char *confstr;
	
	if(dict==NULL)
	{
		printf("config dictΪNULL,�˳�dev_to_conf \n");
		gtloginfo("config dictΪNULL,�˳�dev_to_conf \n");
		return -1;
	}
	
	devinfo=iniparser_load(DEVINFO_PARA_FILE);
	if (devinfo==NULL) 
    {
         printf("cannot parse ini file [%s]",DEVINFO_PARA_FILE);
		 gtloginfo("����%sʧ���˳�\n",DEVINFO_PARA_FILE);	
         return(-1) ;
	}


    //��devinfo����Ӧ�ڵ�ֵ
    devinfostr=iniparser_getstring(devinfo,devsec,CANT_FIND_KEY);
	if(strcmp(devinfostr,CANT_FIND_KEY)==0)//û��
	{
		printf("devinfo.iniû��%s��\n",devsec);
		gtloginfo("devinfo.iniû��%s��\n",devsec);
		return 0;
	}
    else
	{
		confstr = confparser_getstring(dict,cfgsec,CANT_FIND_KEY);
		if(strncmp(devinfostr,confstr,strlen(devinfostr))!=0)//�е�����ͬ
		{
			gtloginfo("%-30s%s->%s\n",cfgsec,confstr,devinfostr);
			confparser_setstr(dict,cfgsec,devinfostr);
			printf("/conf/config�ļ���%s�޸�Ϊ%s\n",cfgsec,devinfostr);
		}
	}
	
	iniparser_freedict(devinfo);
	
	return 0;
}




//����chap��ini֮��ת���ĺ���������䶯��д��chap�ļ���

int chap_to_ini(dictionary *ini,char *filename, char *sec1, char *sec2,char *def)
{
	char chapusr[100];
	char chappswd[100];
	char *iniusr;
	char *inipswd;
	FILE *chapfp;
	
	
	if((ini==NULL)||(filename==NULL)||(sec1==NULL)||(sec2==NULL))
	{	
		gtloginfo("chap_to_ini��������Ϊ��,����\n");
		printf("chap_to_ini����Ϊ��,����\n");
		return -1;
	}

	sprintf(chapusr,"%s",def);
	sprintf(chappswd,"%s",def);
	chapfp=fopen(filename,"r+");	
	if(chapfp==NULL)
		{
			gtloginfo("�޷���chap file %s\n",filename);
			printf("�޷���chap file %s\n",filename);
		}
	
	else
		{
			if(get_chap_user_pswd(chapfp,chapusr,chappswd)!=0)
				{
					printf("ȡchap�ļ��е�usr,pswdʧ��,��ȱʡֵ%s\n",def);
					set_chap_user_pswd(chapfp,def,def);	
					change_flag=1;
				}
			fclose(chapfp);
		}

	iniusr=iniparser_getstring(ini, sec1, CANT_FIND_KEY);
	if(strcmp(iniusr,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����Ϊ%s\n",sec1,chapusr);
			gtloginfo("%-30sNULL->%s\n",sec1,chapusr);
			change_flag=1;
			iniparser_setstr(ini,sec1,chapusr);
			iniusr=chapusr;
		}
	inipswd=iniparser_getstring(ini, sec2, CANT_FIND_KEY);
	if(strcmp(inipswd,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����Ϊ****\n",sec2);
			gtloginfo("%-30sNULL->****\n",sec2);
			change_flag=1;
			iniparser_setstr(ini,sec2,chappswd);
			inipswd=chappswd;
		}

	if(strcmp(iniusr,chapusr)!=0)
		{
			printf("gt1000.ini��%s��chap��%s�޸�Ϊ%s\n",sec1,iniusr,chapusr);
			gtloginfo("%-30s%s->%s\n",sec1,iniusr,chapusr);
			change_flag=1;
			iniparser_setstr(ini,sec1,chapusr);
		}
	if(strcmp(inipswd,chappswd)!=0)
		{
			printf("gt1000.ini��%s��chap��****�޸�Ϊ****\n",sec2);
			gtloginfo("%-30s****->****\n",sec2);
			change_flag=1;
			iniparser_setstr(ini,sec2,chappswd);
		}
	return 0;
}

//����ini��chap֮��ת���ĺ������账��䶯��д��chap�ļ���

int ini_to_chap(dictionary *ini,char *filename, char *sec1, char *sec2,char *def)
{


	char chapusr[100];
	char chappswd[100];
	char *iniusr;
	char *inipswd;
	FILE *chapfp;
	
	
	if((ini==NULL)||(filename==NULL)||(sec1==NULL)||(sec2==NULL))
	{	
		gtloginfo("chap_to_ini��������Ϊ��,����\n");
		printf("chap_to_ini��������Ϊ��,����\n");
		return -1;
	}

	iniusr=iniparser_getstring(ini, sec1, CANT_FIND_KEY);
	if(strcmp(iniusr,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����ȱʡΪ%s\n",sec1,def);
			iniparser_setstr(ini,sec1,def);
			change_flag=1;
			iniusr=def;
		}
	inipswd=iniparser_getstring(ini, sec2, CANT_FIND_KEY);
	if(strcmp(inipswd,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����ȱʡΪ****\n",sec2);
			iniparser_setstr(ini,sec2,def);
			inipswd=def;
			change_flag=1;
		}

	
	chapfp=fopen(filename,"r+");
	if(chapfp==NULL)
		{
			gtloginfo("�޷���chap file %s\n",filename);
			printf("�޷���chap file %s\n",filename);
			return 0; 
		}
	else
		{
	
			if(get_chap_user_pswd(chapfp,chapusr,chappswd)!=0)
				{
					printf("ȡchap�ļ��е�usr,pswdʧ��,��ֵ%s,%s\n",iniusr,inipswd);
					gtloginfo("%-30sNULL->%s\n%-30sNULL->****\n",sec1,iniusr,sec2);
					set_chap_user_pswd(chapfp,iniusr,inipswd);
					change_flag=1;
				}
		

	if((strcmp(iniusr,chapusr)!=0)||(strcmp(inipswd,chappswd)!=0))
		{
			printf("chap�ļ�%s�е�%s,��gt1000��Ϊ%s\n",filename,sec1,iniusr);
			printf("chap�ļ�%s�е�%s,��gt1000��Ϊ****\n",filename,sec2);
			gtloginfo("%-30s%s->%s\n",sec1,chapusr,iniusr);
			gtloginfo("%-30s****->****\n",sec2);
			set_chap_user_pswd(chapfp,iniusr,inipswd);
			change_flag=1;
		}

	fclose(chapfp);
		}

	return 0;

}

//����ini��boa֮��ת���ĺ������账��䶯��д��boa�ļ���
int ini_to_boa(dictionary *ini, char *secini, char *secboa, char *def)
{
	char *iniport;
	char *boaport;
	char tmp[100];
	FILE *fpboa;	

	if((ini==NULL)||(secini==NULL)||(secboa==NULL))
	{	
		gtloginfo("ini_to_boa��������Ϊ��,����\n");
		printf("ini_to_boa��������Ϊ��,����\n");
		return -1;
	}

	boaport=tmp;
	iniport=iniparser_getstring(ini, secini, CANT_FIND_KEY);
	if(strcmp(iniport,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����ȱʡΪ%s\n",secini,def);
			iniparser_setstr(ini,secini,def);
			iniport=def;
		}

	fpboa=fopen(BOA_FILE,"r+");
	if(fpboa==NULL)
		{
			gtloginfo("�޷���boa file %s,\n",BOA_FILE);
			printf("�޷���boa file %s\n",BOA_FILE);
			return -1;
		}
	else
		{
			get_boa_str(fpboa,secboa,boaport);
			if(boaport==NULL)
				{
					printf("�޷���ȡboa�ļ�����Ӧֵ,��ֵ%s\n",iniport);
					gtloginfo("%-30sNULL->%s\n",secini,iniport);
					set_boa_str(fpboa,secboa,iniport);
					change_flag=1;
				}
		}

	if((strcmp(iniport,boaport)!=0))
		{
			printf("boa�ļ��е�%s,��gt1000��%s��Ϊ%s\n",secboa,boaport,iniport);
			gtloginfo("%-30s%s->%s\n",secini,boaport,iniport);
			set_boa_str(fpboa,secboa,iniport);
			change_flag=1;
		}
	fclose(fpboa);
	
	
	return 0;

}

//����boa��ini֮��ת���ĺ������账��boa�䶯(if any)��д��boa�ļ���
int boa_to_ini(dictionary *ini, char *secini, char *secboa, char *def)
{
	char *iniport;
	char *boaport;
	FILE *fpboa;
	char tmp[100];

	
	if((ini==NULL)||(secini==NULL)||(secboa==NULL))
	{	
		gtloginfo("boa_to_ini��������Ϊ��,����\n");
		printf("boa_to_ini��������Ϊ��,����\n");
		return -1;
	}
	
	boaport=tmp;
	fpboa=fopen(BOA_FILE,"r+");
	if(fpboa==NULL)
		{
			gtloginfo("�޷���boa file %s\n",BOA_FILE);
			printf("�޷���boa file %s\n",BOA_FILE);
			boaport=def;
		}
	else

		{
			get_boa_str(fpboa,secboa,boaport);
			if(boaport==NULL)
				{
					printf("�޷���ȡboa�ļ�����Ӧֵ,��ȱʡֵ%s\n",def);
					set_boa_str(fpboa,secboa,def);
					change_flag=1;
				}
		
	iniport=iniparser_getstring(ini, secini, CANT_FIND_KEY);
	if(strcmp(iniport,CANT_FIND_KEY)==0)//�Ҳ�����Ӧ�����򴴽�
		{
			printf("gt1000.iniû��%s,\t����Ϊ%s\n",secini,boaport);
			gtloginfo("%-30sNULL->%s\n",secini,boaport);
			iniparser_setstr(ini,secini,boaport);
			change_flag=1;
		}
	if((strcmp(iniport,boaport)!=0))
		{
			printf("gt1000.ini��%s��boa��%s�޸�Ϊ%s\n",secini,iniport,boaport);
			gtloginfo("%-30s%s->%s\n",secini,iniport,boaport);
			iniparser_setstr(ini,secini,boaport);
			change_flag=1;
		}
	

		}
	fclose(fpboa);

	return 0;
}

/*����valueֵ����dhcp�ļ���д����Ӧ���ַ���*/
int set_dhcp_file(int value)
{
	confdict * conf;
	FILE *fp;
	char str[10];
	conf=confparser_load_lockfile(DHCP_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",DHCP_FILE);
			 gtloginfo("����%sʧ���˳�\n",DHCP_FILE);	
             return -2 ;
		}

	if(value == 1)
		sprintf(str,"dhcp");
	else
		sprintf(str,"none");
	confparser_setstr(conf, "BOOTPROTO",str );
	confparser_dump_conf(DHCP_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);
	return 0;

}

//��תһ���˳�
int para_conv_mto1(void)
{
	confdict *conf;
	dictionary *gt1000;
	FILE *fpgt1000;
	FILE *fp;
	int i;
	
	gtloginfo("run para_conv -m,�����ļ���תһ��version:%s\n",VERSION);
	printf("\nrun para_conv -m,�����ļ���תһ��version:%s\n",VERSION);
	
	change_flag=0;
	//����gt1000.ini�ļ�
	gt1000=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fpgt1000);
	if (gt1000==NULL) 
        {
             printf("cannot parse ini file [%s]",IPMAIN_PARA_FILE);
			 gtloginfo("����%sʧ���˳�\n",IPMAIN_PARA_FILE);	
             return(-1) ;
		}
	
	//��devinfoת
	printf("\n------%s ->  %s------\n\n",DEVINFO_PARA_FILE,IPMAIN_PARA_FILE);
	dev_to_gt1000(gt1000);
		
	//��configת
	printf("\n------%s ->  %s------\n\n",CONFIG_FILE,IPMAIN_PARA_FILE);
	
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",CONFIG_FILE);
			 gtloginfo("����%sʧ���˳�\n",CONFIG_FILE);	
             return -2 ;
		}
	
	dev_to_conf(conf,"devinfo:devtypestring","HOST_NAME");
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:ipaddr", 	"ETH0_IPADDR",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:netmask",	"ETH0_NETMASK",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:mac_addr",	"MAC_ADDRESS",	NULL);
	if(get_eth_num()>=2)
	{
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:ipaddr", 	"ETH1_IPADDR",	"");
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:netmask",	"ETH1_NETMASK",	"");
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:mac_addr",	"MAC1_ADDRESS",	"");
	}
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_mac_bind",	"ROUTE_MAC_BIND","0");
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_mac",	"ROUTE_MAC","0");
	
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:internet_mode",	"INTERNET_MODE",NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_default",	"ROUTE_DEFAULT",NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:dns_server",	"DNS_SERVER",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"port:telnet_port",	"LOGIN_PORT",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:use_dhcp", "USE_DHCP", NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:use_upnp", "USE_UPNP", NULL);
	confparser_dump_conf(CONFIG_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//˳���޸�һ��dhcp�ļ�
	set_dhcp_file(iniparser_getint(gt1000, "net_config:use_dhcp",0));

	
	//��vsftpdת
	printf("\n------%s ->  %s------\n\n",VSFTPD_FILE,IPMAIN_PARA_FILE);
	
	conf=confparser_load_lockfile(VSFTPD_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",VSFTPD_FILE);
			 gtloginfo("����%sʧ���˳�\n",VSFTPD_FILE);	
             return -2 ;
		}

	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:ftp_port", 	 "listen_port",	 "21");
	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:pasv_min_port","pasv_min_port","9011");
	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:pasv_max_port","pasv_max_port","9030");
	confparser_dump_conf(VSFTPD_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//��boaת
	printf("\n------%s ->  %s------\n\n",BOA_FILE,IPMAIN_PARA_FILE);
	
	i=boa_to_ini(gt1000,"port:web_port","Port","8094");


	//��chapת
	printf("\n------%s ->  %s------\n\n",CHAP_FILE,IPMAIN_PARA_FILE);
	i=chap_to_ini(gt1000,CHAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

		

	save_inidict_file(IPMAIN_PARA_FILE,gt1000,&fpgt1000);

	iniparser_freedict(gt1000);
	if(change_flag==1)
		{
			gtloginfo("�޸��˲����ļ�(para file modified!)\n");
			change_flag=0;
		}
	printf("\nȫ��ת�����,�˳�����.\n");
	return 0;
}

//һת�ಢ�˳�
int para_conv_1tom(void)
{
	confdict *conf;
	dictionary *gt1000;
	FILE *fpgt1000;
	FILE *fp;

	gtloginfo("run para_conv -s,�����ļ�һת�࣬version:%s\n",VERSION);
	printf("\nrun para_conv -s,�����ļ�һת�࣬version:%s\n",VERSION);
	change_flag=0;
	gt1000=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fpgt1000);
	if (gt1000==NULL) 
        {
             printf("cannot parse ini file [%s]",IPMAIN_PARA_FILE);
			 gtloginfo("����%sʧ���˳�\n",IPMAIN_PARA_FILE);	
             return(-1) ;
		}

	
	//�ȴ���config�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,CONFIG_FILE);
	
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",CONFIG_FILE);
			 gtloginfo("����%sʧ���˳�\n",CONFIG_FILE);	
             return -2 ;
		}
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:ipaddr", 	"ETH0_IPADDR",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:netmask",	"ETH0_NETMASK",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:mac_addr",	"MAC_ADDRESS",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:use_dhcp", "USE_DHCP", NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:use_upnp", "USE_UPNP", NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_mac_bind",	"ROUTE_MAC_BIND",	"0");
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_mac",	"ROUTE_MAC",	"0");
	if(get_eth_num()>=2)
	{
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:ipaddr", 	"ETH1_IPADDR",	NULL);
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:netmask",	"ETH1_NETMASK",	NULL);
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:mac_addr",	"MAC1_ADDRESS",	NULL);
	}
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:internet_mode",	"INTERNET_MODE",NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_default",	"ROUTE_DEFAULT",NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:dns_server",	"DNS_SERVER",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"port:telnet_port",	"LOGIN_PORT",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"devinfo:devtypestring","HOST_NAME",NULL);
	confparser_dump_conf(CONFIG_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);


	//�ٴ���vsftpd�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,VSFTPD_FILE);
	
	conf=confparser_load_lockfile(VSFTPD_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",VSFTPD_FILE);
			 gtloginfo("����%sʧ���˳�\n",VSFTPD_FILE);	
             return -2 ;
		}
	
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:ftp_port", 	 "listen_port",	 "21");
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:pasv_min_port","pasv_min_port","9011");
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:pasv_max_port","pasv_max_port","9030");
	confparser_dump_conf(VSFTPD_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//�ٴ���boa�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,BOA_FILE);
	ini_to_boa(gt1000,"port:web_port","Port","8094");

	//�ٴ���pap�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,PAP_FILE);
	ini_to_chap(gt1000,PAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

	//�ٴ���chap�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,CHAP_FILE);
	ini_to_chap(gt1000,CHAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

	//�ٴ���ppp0�ļ�
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,PPP0_FILE);

	conf=confparser_load_lockfile(PPP0_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",PPP0_FILE);
			 gtloginfo("����%sʧ���˳�\n",PPP0_FILE);	
             return -2 ;
		}
	
	ini_to_conf(PPP0_FILE,gt1000,conf,"net_config:adsl_user","USER","0");
	confparser_dump_conf(PPP0_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//����dhcp�ļ�
	set_dhcp_file(iniparser_getint(gt1000, "net_config:use_dhcp",0));

	

	//�������gt1000.ini
	
	save_inidict_file(IPMAIN_PARA_FILE,gt1000,&fpgt1000);
	iniparser_freedict(gt1000);
	if(change_flag==1)
		{
			gtloginfo("�޸��˲����ļ�(para file modified!)\n");
			change_flag=0;
		}
	printf("\nȫ��ת�����,�˳�����.\n");
	return 0;
}



int main(int argc,char **argv)
{
/*   ���������в���:
 *		����ʱû�д���������ʾ������Ϣ��
 *		-h:���������Ϣ��
 *		-v:���ת�����ߵİ汾��Ϣ��
 *		-m:��ʾ��תһ���� ��������ļ�->gt1000.ini��
 *		-s:��ʾһת�࣬��gt1000.ini->������������ļ���*/
	init_devinfo();		 

//printf("hqdata %d M, update %d M\n",get_disk_total("/hqdata"),get_disk_total("/hqdata/update"));
	if((argc!=2)||(argv[1]==NULL))
		{
			print_helpmsg();
			return 0;
		}
	if(strncmp(argv[1],"-m",2)==0)
		{
			para_conv_mto1();
			return 0;
		}
	if(strncmp(argv[1],"-s",2)==0)
		{
			para_conv_1tom();
			return 0;
		}
	if(strncmp(argv[1],"-v",2)==0)
		{
			print_version();
			return 0;
		}
	else
		{
			print_helpmsg();
			return 0;
		}
	return 0;
}
