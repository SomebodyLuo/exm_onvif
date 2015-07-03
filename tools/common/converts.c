/*���б�Ҫ��ϵͳ��Ϣת��
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <devinfo.h>
#include <iniparser.h>
#include <confparser.h>
#include <errno.h>
/**********************************************************************************************
 * ������	:check_valid_mac()
 * ����	:���������ڼ��mac��ַ�ĺϷ���
 * ����	:macstr:����mac��ַ���ַ���"AA:BB:CC:DD:EE:FF"
 *			 maclen:�ַ����ĳ���
 * ����ֵ	:1��ʾ�Ϸ� 0��ʾ�Ƿ� -1��ʾ��������
 **********************************************************************************************/
static int check_valid_mac(char *macstr,int maclen)
{
	int i;
	int num;
	if(macstr==NULL)
		return -1;
	num=0;
	for(i=0;i<maclen;i++)
	{
		if(macstr[i]==':')
			num++;
	}
	if(num==5)
		return 1;
	return 0;
}
/**********************************************************************************************
 * ������	:conv_guid2mac()
 * ����	:��guid��Ϣת��Ϊmac��ַ�ַ���
 * ����	:
 *		eth_num:���ڱ��
 *		guid:���guid�Ļ�����
 *		guid_len:��������guid����Ч�ֽ���
 * ���	:
 *	mac_buf:����ʱ���mac��ַ�Ļ��������ַ���"AA:BB:CC:DD:EE:FF"
 * ����ֵ	:��ֵ��ʾ���� ��ֵ��ʾ���ĳ���
 **********************************************************************************************/
static int conv_guid2mac(int eth_num,char *guid,int guid_len,char *mac_buf)
{
	int 	i;
	char gtemp[20];
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
	if((guid==NULL)||(guid_len<=0)||(mac_buf==NULL))
		return -1;
	if(guid_len>sizeof(gtemp))
		guid_len=sizeof(gtemp);
	memset(gtemp,0,sizeof(gtemp));
	for(i=0;i<guid_len;i++)
		gtemp[i]=guid[i];
	if(eth_num==1)
		gtemp[4]+=1;	///����1��mac��ַ�θ�λΪ���λ��1 //shixin 2007.07.04 changed
	sprintf(mac_buf,"%02x:%02x:%02x:%02x:%02x:%02x",(int)gtemp[5],(int)gtemp[4],(int)gtemp[3],(int)gtemp[2],(int)gtemp[1],(int)gtemp[0]);
	return (strlen(mac_buf)+1);	
}
/**********************************************************************************************
 * ������	:get_conf_mac()
 * ����	:�������ļ��л�ȡmac��ַ�ַ���
 * ���� : eth_num:������� 0,1
 * ���	:
 *	  mac_buf:���mac��ַ�ַ����Ļ�����
 * ����ֵ	:��ֵ��ʾ���� ��ֵ��ʾ���ĳ���
 **********************************************************************************************/

static int get_conf_mac(int eth_num,char *mac_buf)
{
	char *fileconfig="/conf/config";	//���mac��ַ�������ļ�
	confdict *conf;
	char *mac;
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
  	conf=confparser_load(fileconfig);
	if (conf==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
	if(eth_num==0)	
		mac=confparser_getstring(conf,"MAC_ADDRESS","NULL");
	else
		mac=confparser_getstring(conf,"MAC1_ADDRESS","NULL");
	memcpy(mac_buf,mac,strlen(mac)+1);
	confparser_freedict(conf);
	return strlen((mac_buf)+1);
	
}
/**********************************************************************************************
 * ������	:set_conf_mac()
 * ����	:����mac��ַ�������ļ�
 *	  eth_num:�������
 * 	  mac_buf:���mac��ַ�ַ����Ļ�����
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
static int set_conf_mac(int eth_num,char *mac_buf)
{
	char *fileconfig="/conf/config";	//���mac��ַ�������ļ�
	FILE  *fp=NULL;
	confdict *conf;
	int	ret;
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
  	conf=confparser_load(fileconfig);
	if (conf==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
	
	fp = fopen(fileconfig,"w");
	if(fp==NULL)
	{
               fprintf(stderr, "Cannot open output file[%s]",fileconfig);
	 		 confparser_dump_conf(fileconfig,conf,fp);			   
               return -1;
       }		

	if(eth_num==0)
		ret=confparser_setstr(conf,"MAC_ADDRESS",mac_buf);
	else
		ret=confparser_setstr(conf,"MAC1_ADDRESS",mac_buf);
	confparser_dump_conf(fileconfig,conf,fp);			
	fclose(fp);
	return 0;
	
}


/*���������ڼ��mac��ַ�Ƿ���ȷ����
 * ����ȷ���/conf/devinfo�е�devguidֵ��д��mac��ַ��ʽ��д��/conf/config */
//����ֵ <0��ʾ����
//0��ʾ�����ļ�������mac��ַ��ͬ
// 1��ʾ�Ѿ���ini�е�mac���µ�config�ļ���
/**********************************************************************************************
 * ������	:mac_check_conv()
 * ����	:���mac��ַ�Ƿ���ȷ���粻��ȷ���/conf/devinfo�е�devguidֵת��
 *			 ��mac��ַ��ʽ��д��/conf/config 
 * ����:	eth_num:�������0,1,...
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
int mac_check_conv(int eth_num)
{
	const char  GUID_PROTO[8] = {0x0,0x0,0x0,0x0,'t',0x0,'g',0x0};	//���豸guid
	char newmac[32];
	char oldmac[32];
	char guid[32];
	int	guid_len;	
	if((eth_num!=0)&&(eth_num!=1))///ֻ֧����������
		return -EINVAL;
	guid_len=get_devid(guid);
	if(guid_len<0)
		return -1;
	if(memcmp(guid,GUID_PROTO,sizeof(GUID_PROTO))==0)
		return 0;//˵�������豸���ý���mac��ַת��
	
	if(get_conf_mac(eth_num,oldmac)<0)
		return -1;

	if(check_valid_mac(oldmac,strlen(oldmac))<=0)
	{
		if(conv_guid2mac(eth_num,guid,guid_len,newmac)<0)	//��guidת��Ϊmac��ַ
			return -1;
		set_conf_mac(eth_num,newmac);
		printf("eth%d oldmac=%s newmac=%s!!\n",eth_num,oldmac,newmac);
		return 1;
	}
	

	return 0;
	
}

/*���������ڼ��mac��ַ�Ƿ���ȷ����
 * ����ȷ���/conf/devinfo�е�devguidֵ��д��mac��ַ��ʽ��д��/conf/config */
//����ֵ <0��ʾ����
//0��ʾ�����ļ�������mac��ַ��ͬ
// 1��ʾ�Ѿ���ini�е�mac���µ�config�ļ���
#if 0
int mac_check_conv(void)
{

	char parastr[100];
	dictionary *ini1;
	confdict *conf1;	
	int section_len;
	int i;
	char section[10];
	char macaddress[30];
	char *devguid,*oldmac,*newmac;
	char *cat,*lk;
	FILE *fp;
	char *fileini="/conf/devinfo.ini";//guid�ļ�
	char *fileconfig="/conf/config";//�����ű��õ��������ļ�
	
	//��ʼ������/conf/devinfo.ini�ж���devguid��ֵ��devguid��
	sprintf(section,"devinfo");
	if((fileini==NULL)||(fileconfig==NULL))
		return -1;
	section_len=strlen(section);
	ini1=iniparser_load(fileini);
	if (ini1==NULL) 
		{
            printf("cannot parse ini file file [%s]\n", fileini);
            return -1 ;
       }
 	memcpy(parastr,section,section_len);
	parastr[section_len]=':';
	section_len++;
	parastr[section_len]='\0';
	cat=strncat(parastr,"devguid",strlen("devguid"));	
	devguid=iniparser_getstring(ini1,parastr,"NULL");
	printf("devguid= %s\n",devguid);

    //��devguid��д��macaddress
    	section_len=strlen(devguid);
    	if(section_len!=16)
 	{
		printf("devguid length not correct! %d\n",section_len);
		iniparser_freedict(ini1);
		return -2;
    	}
    	lk=devguid+4;
    	for (i=0;i<6;i++)
    	{
			macaddress[3*i]=*lk;
			lk++;
			macaddress[3*i+1]=*lk;
			lk++;
			macaddress[3*i+2]=':';
    	}	
    	macaddress[17]='\0';	
  	 // printf("new macaddress=%s\n",macaddress);
    	newmac=macaddress;
    	iniparser_freedict(ini1);

  	//��/conf/config��ȡ��MAC ADDRESS���бȽ�
	
  	conf1=confparser_load(fileconfig);
	if (conf1==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
		
	oldmac=confparser_getstring(conf1,"MAC_ADDRESS","NULL");
	//printf("old mac=%s\n",oldmac);
	//printf("oldmac= %s\n",oldmac);
	//��������ͬ��ֱ���˳�
	//��mac��ַ�Ϸ���ֱ���˳�
	//remed by shixin if(check_valid_mac(oldmac,strlen(oldmac)))//strcmp(oldmac,newmac)==0)
	if(memcmp(oldmac,newmac,strlen(newmac))==0)
	{
		//printf("no need to modify! correct MAC_ADDRESS\n");
		confparser_freedict(conf1);
		return 0;
	}
	//����ͬ������޸ģ���macaddressд��/conf/config
	else
	{	
			
		fp = fopen(fileconfig,"w");
		if(fp==NULL)
              	{
                      fprintf(stderr, "Cannot open output file[%s]",fileconfig);
                      return -3;
              	}		

		i=confparser_setstr(conf1,"MAC_ADDRESS",newmac);
		//iniparser_dump_ini(conf1,stdout);
		confparser_dump_conf(conf1,fp);			
		fclose(fp);
			
	}
    

	//convert_file2ini(fileconfig);	
	confparser_freedict(conf1);
 	return 1;
}
#endif


