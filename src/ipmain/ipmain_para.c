#include "ipmain.h"
#include "netcmdproc.h"
#include "leds_api.h"
#include "ipmain_para.h"
#include "gt_com_api.h"
#include "netinfo.h"
#include "gate_connect.h"
#include "devstat.h"
#include "confparser.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "gate_connect.h"
#include <commonlib.h>


static struct ipmain_para_struct ipmain_para;
static int     serial_debug_mode;
static int     serial_interval;

int GATE_PERIOD_TIME=0;
static int  set_sock_addr(struct sockaddr_in *sock,char *host);


char * devlog(int dev_no)
{
	if(virdev_get_virdev_number()==2)//�����豸
	{
		if(dev_no == 0)
			return "�����豸0 ";
		else
			return "�����豸1 ";
	}
	else
		return "�豸";
}
///�����ļ�����ȡ���ļ��Ĵ��·����������
const char *get_audio_file(char *name)
{
    static const char *def_ack_file="/conf/audio/alarm/ack.adpcm";                 ///Ĭ�ϱ���Ӧ����
    static const char *def_trig_file="/conf/audio/alarm/trigger.adpcm";            ///Ĭ�ϱ���������
   
   static const char *ack_audio="/hqdata/audio_index/alarm/ack.adpcm";       ///�û����ñ���Ӧ����
    static const char *trig_audio="/hqdata/audio_index/alarm/trigger.adpcm";  ///�û����ñ���������

    int trig,ack;
	

    //trig=check_file((char *)trig_audio);
    //ack=check_file((char *)ack_audio);
	
    if(strcasecmp(name,"ack.adpcm")==0)
    {
		/*
        if(trig||ack)
        {///�û�����������
            return ack_audio;
        }
        else
        {
            if(!ack)                        ///û�����������򲥷�Ĭ������
                return def_ack_file;
        }
        */
        return def_ack_file;
    }
    else if(strcasecmp(name,"trigger.adpcm")==0)
    {
    /*
         if(trig||ack)
        {///�û�����������
            return trig_audio;
        }        
        else
        {
            if(!ack)                        ///û�����������򲥷�Ĭ������
                return def_trig_file;
        }
	*/
		return def_trig_file;
    }
    return "no file";

}

/**********************************************************************************************
 * ������	:init_para()
 * ����	:ϵͳ������ʼ��������������ΪĬ��ֵ
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void  init_para(void)
{//
	int i,j;
	struct ipmain_para_struct *p;
	struct gate_struct *gt_list;
	struct video_para_struct *vadc;
	struct quad_dev_struct *quad;
	struct enc_front_struct *front;
	struct motion_struct *motion;
	struct blind_struct *blind;
	char buf[30];
	memset(&ipmain_para,0,sizeof(struct ipmain_para_struct));
	p=&ipmain_para;
	int dev_no;
	dev_para_struct *devpara;
	for(dev_no = 0;dev_no < virdev_get_virdev_number(); dev_no++)
	{	
		devpara = (dev_para_struct *)&(ipmain_para.devpara[dev_no]);
		//MAX_GATE_LIST
		gt_list=get_gate_list(dev_no);
		devpara->rmt_gate0=&gt_list[0].gateaddr;
		devpara->rmt_gate1=&gt_list[1].gateaddr;
		devpara->rmt_gate2=&gt_list[2].gateaddr;
		devpara->rmt_gate3=&gt_list[3].gateaddr;
		devpara->rmt_gate4=&gt_list[4].gateaddr;
		devpara->alarm_gate=&gt_list[5].gateaddr;
		
		
		sprintf(buf,"0.0.0.0 %d",REMOTE_GATE_CMD_PORT);
		set_sock_addr(devpara->rmt_gate0,buf);
		set_sock_addr(devpara->rmt_gate1,buf);
		set_sock_addr(devpara->rmt_gate2,buf);
		set_sock_addr(devpara->rmt_gate3,buf);
		set_sock_addr(devpara->rmt_gate4,buf);
		set_sock_addr(devpara->alarm_gate,buf);

		sprintf(devpara->inst_place,"%s","δ����");
		devpara->cmd_port=DEV_MAIN_CMD_PORT;
		devpara->ip_chg_flag = 0;
		devpara->sendgateini_flag = 1;
		
		if(get_osd_flag()==1)	
		{
			devpara->osd.inst_place_display = 0;
			devpara->osd.inst_place_posx = INSTPLACE_DEF_POSX;
			devpara->osd.inst_place_posy = INSTPLACE_DEF_POSY;
		}
	}

	GATE_PERIOD_TIME=GATE_PERIOD_TIME_DEF;
	p->rmt_env_mode=MSG_AUTH_SSL;
	if(p->rmt_env_mode!=0)
	{
		p->rmt_env_mode=MSG_AUTH_SSL;
		p->rmt_enc_mode=DES_EDE3_CBC;
		if(p->rmt_enc_mode>RC5_32_12_16_CBC)
			p->rmt_enc_mode=DES_EDE3_CBC;
	}

	p->image_port=DEV_MAIN_IMAGE_PORT;
	p->audio_port=DEV_MAIN_AUDIO_PORT;
	p->ftp_port=DEV_DEFAULT_FTP_PORT;
	p->web_port=DEV_DEFAULT_WEB_PORT;
	p->com0_port=COM1_TCP_PORT;
	p->com1_port=COM2_TCP_PORT;
	p->pb_port=8900;
	p->trig_in=0;
	p->tin_mask=ALARMIN_MASK;
	p->alarm_out=0;
	p->alarm_mask=ALARMOUT_MASK;  
	//p->hq_follow_net=1;
	//p->hq_save_ch=4;
	//if(p->hq_save_ch>3)
	//	p->hq_save_ch=4;

	p->net_ch=4;
	if(p->net_ch>3)
		p->net_ch=4;

	p->alarm_playback_ch = -1;

	//video_adc
	vadc = &p->vadc;
	for(j=0;j<get_video_num();j++)
	{
		
	
	
		front=&vadc->enc_front[j];
		front->bright=128;
		front->hue=128;
		front->contrast=128;
		front->saturation=128;
		front->enable=1;
		sprintf(front->name,"��Ƶ%d",j);
		
		if(get_quad_flag()==1)//���ķָ��豸
		{
			quad = &vadc->quad;
			
			quad->current_net_ch = 4;
			//quad->current_local_ch =4;
			
			motion=&quad->motion[j];
			motion->sen=0;
			motion->alarm=0;
			for(i=0;i<12;i++)
				motion->area[i]=0xffff;
			motion->starthour=0;
			motion->startmin=0;
			motion->endhour=0;
			motion->endmin=0;
			
			blind= &quad->blind[j];
			blind->alarm_time = DEF_BLIND_ALARMTIME ;
			blind->cancelalarm_time = DEF_BLIND_UNALARMTIME;
			blind->ch =j;
			blind->sen = DEF_BLIND_SEN;
		}
	}
	p->video_mask=0;
	for(i=0;i<get_video_num();i++)
	{
		if(p->vadc.enc_front[i].enable)
		{
			p->video_mask|=(1<<i);
		}
	}

	
	//quad dev
	if(get_quad_flag()==1)
	{
		//p->vadc.quad.current_local_ch = 4;
		p->vadc.quad.current_net_ch = 4;
	}
	//read video
	p->video_in_ch=get_video_num();
	
	p->inst_ack=0;
	p->reset_modem=1;
	p->reset_modem_time=120;
	p->internet_mode=0;
	//p->log_watch_lost=1;
	//p->snap_file_interval=ALARM_SNAP_INTERVAL;
	//p->snap_file_num=ALARM_SNAP_NUM;
	
}

/**********************************************************************************************
 * ������	:refresh_netinfo()
 * ����	:ˢ������״̬����
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void refresh_netinfo(void)
{
	in_addr_t addr;
	struct ip1004_state_struct *gtstate;
	int i = 0;
	addr=get_net_dev_ip("ppp0");
	if((int)addr==-1)
	{
		memset(&ipmain_para.wan_addr,0,sizeof(ipmain_para.wan_addr));
		//for(i=0;i<virdev_get_virdev_number();i++)
		{	
			gtstate=get_ip1004_state(i);
			pthread_mutex_lock(&gtstate->mutex);
			gtstate->reg_dev_state.link_err=1;
			pthread_mutex_unlock(&gtstate->mutex);	
		}					
 	}
	else
	{
		if(memcmp(&addr,&ipmain_para.wan_addr,sizeof(addr))!=0)
		{
			//for(i=0;i<virdev_get_virdev_number();i++)
			{
				set_regist_flag(i,0);//ip��ַ�ı�
				gtstate=get_ip1004_state(i);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_dev_state.link_err=0;
				pthread_mutex_unlock(&gtstate->mutex);		
				if(get_gate_connect_flag(i))
				ipmain_para.devpara[i].ip_chg_flag=1;
			}
			printf("refresh_netinfo change ip %x->%x\n",ipmain_para.wan_addr,addr);
			memcpy(&ipmain_para.wan_addr,&addr,sizeof(addr));
		}
	}		
	
	addr=get_net_dev_ip("eth0");
	if((int)addr==-1)
	{
		memset(&ipmain_para.lan_addr,0,sizeof(ipmain_para.lan_addr));		
	}
	else
	{
		memcpy(&ipmain_para.lan_addr,&addr,sizeof(addr));
		addr=get_net_dev_mask("eth0");
		if((int)addr==-1)
		{
			memset(&ipmain_para.lan_mask,0,sizeof(ipmain_para.lan_mask));	
		}
		else
		{
			memcpy(&ipmain_para.lan_mask,&addr,sizeof(addr));
		}

	}
}


static int  set_sock_addr(struct sockaddr_in *sock,char *host)
{
	struct hostent *hostPtr = NULL;
	char *begin,*end;
	char buf[100];
	int i,len;
	unsigned short port;
	if((sock==NULL)||(host==NULL))
		return -1;
	sock->sin_family = AF_INET;
	len=strlen(host);
	if((len+1)>100)
		return -1;
	memcpy(buf,host,len+1);
	begin=buf;
	for(i=0;i<len;i++)//ȥ���ַ���ǰ��Ŀո�
	{
		if(*begin!=' ')
			break;
		begin++;
	}
	len-=i;
#if 0
	if(len<=8)	//����8���ֽڿ϶��Ǵ���
	{
		return -1;
	}
#endif
	end=begin;
	for(i=0;i<len;i++)
	{
		if((*end==' ')||(*end=='\0'))
			break;
		end++;
	}
	*end='\0';
	len-=i;
	hostPtr = gethostbyname(begin); /* struct hostent *hostPtr. */ 
       if (NULL == hostPtr)
    	{
        	hostPtr = gethostbyaddr(begin, strlen(begin), AF_INET);
        	if (NULL == hostPtr) 
        	{
        		printf("Error resolving server address %s",host);
        		return -1;
        	}
      }
      memcpy(&sock->sin_addr,hostPtr->h_addr,hostPtr->h_length);	

	if(len>3)
	{
		end++;
		begin=end;
		//printf("begin2=%s,len=%d\n",begin,len);
		for(i=0;i<len;i++)
		{
			if((*begin!=' ')&&(*begin!='\0'))
				break;
			begin++;
		}
		len-=i;
		//printf("begin3=%s,len=%d\n",begin,len);
		if(len>3)
		{
			end=begin;
			for(i=0;i<len;i++)
			{
				if((*end==' ')||(*end=='\0'))
					break;
				end++;
			}
			
			*end='\0';
			//printf("end2=%s,begin=%s,len=%d,i=%d\n",end,begin,len,i);
			if(i>3)
			{
				port=atoi(begin);
				//printf("port=%d\n",port);
				sock->sin_port=htons((port));
			}
		}
	}
	return 0;
}


/**********************************************************************************************
 * ������	:save_setalarm_para()
 * ����		:��������ʽ��ֵ����ini�ṹ�еı�������������������
 * ����		:type:�������ͣ�0Ϊ���Ӵ�����1Ϊ�ƶ����
 *			 ch:��Ӧ��ͨ����
 *			 setalarm:����(1)�򳷷�(0)
 *			 starthour: ������ʼСʱ
 *			 startmin:������ʼ����
 *			 endhour: ��������Сʱ
 *			 endmin: ������������
 * ���	: ini:����ini�ļ��Ľṹָ��,����ʱ�������ֵ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int save_setalarm_para(dictionary *ini,int type, int ch,int setalarm, int starthour, int startmin, int endhour, int endmin)
{
	
	char entry[30];
	
	if(ini==NULL)
		return -EINVAL;
	if(type==0)//����
	{
		if((ch+1)>get_trigin_num())
			return -EINVAL;
		sprintf(entry,"alarm%d:setalarm",ch);
		iniparser_setint(ini,entry,setalarm);
		if(setalarm==1)
		{
			sprintf(entry,"alarm%d:starthour",ch);
			iniparser_setint(ini,entry,starthour);
			sprintf(entry,"alarm%d:startmin",ch);
			iniparser_setint(ini,entry,startmin);
			sprintf(entry,"alarm%d:endhour",ch);
			iniparser_setint(ini,entry,endhour);
			sprintf(entry,"alarm%d:endmin",ch);
			iniparser_setint(ini,entry,endmin);
		}
	}
	if(type==1)//�ƶ����
	{
		if((ch+1)>get_video_num())
			return -EINVAL;
		sprintf(entry,"motion%d:setalarm",ch);
		iniparser_setint(ini,entry,setalarm);
		if(setalarm==1)
		{
			sprintf(entry,"video%d:starthour",ch);
			iniparser_setint(ini,entry,starthour);
			sprintf(entry,"video%d:startmin",ch);
			iniparser_setint(ini,entry,startmin);
			sprintf(entry,"video%d:endhour",ch);
			iniparser_setint(ini,entry,endhour);
			sprintf(entry,"video%d:endmin",ch);
			iniparser_setint(ini,entry,endmin);
		}
	}
	return 0;
}

/**********************************************************************************************
 * ������	:save_video_para()
 * ����		:��������ʽ��ֵ����ini�ṹ�е���Ƶ����������
 * ����		:ch:��Ƶͨ����
 *			 type:���������� "bright"...
 *			 val:����ֵ
 * ���	: ini:����ini�ļ��Ľṹָ��,����ʱ�������ֵ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int save_video_para(dictionary      *ini,int ch,char *type,int val)
{
	char entry[30];
	if((ini==NULL)||(type==NULL))
		return -1;
	if((ch+1)>get_video_num())
		return -1;
	sprintf(entry,"video%d:%s",ch,type);
	return iniparser_setint(ini,entry,val);
}

/**********************************************************************************************
 * ������	:save_video_para_hex()
 * ����	:��16����������ʽ��ֵ����ini�ṹ�е���Ƶ����������
 * ����	:
 *			 ch:��Ƶͨ����
 *			 type:������������ "bright"...
 *			 val:����ֵ
 * ���	: ini:����ini�ļ��Ľṹָ��,����ʱ�������ֵ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int save_video_para_hex(dictionary *ini,int ch,char *type,int val)
{
	char entry[30];
	if((ch+1)>get_video_num())
		return -1;
	if(type==NULL)
		return -1;
	sprintf(entry,"video%d:%s",ch,type);
	return iniparser_sethex(ini,entry,val);
}


#include <netinet/in.h>

//�Ӷ������ַ����н���

int string_to_actions(int *act, char *pstr)
{
	int number=0; //��Ч����
	char string[201];
	char *lp;
	if ((pstr==NULL)||(act==NULL))
		return -1;
	memcpy(string,pstr,200);
	*act=atoi(string);
	//printf("get a int :%d\n",*act);
	number++;
	act++;
	lp=index(string,',');
	while((lp!=NULL) &&(number<=MAX_TRIG_EVENT))
	{	
		lp++;
		//printf("lp is %s\n,int is %d",lp,atoi(lp));
		*act=atoi(lp);
		act++;
		//printf("get a int :%d\n",*act);
		number++;
		
		lp=index(lp,',');
	}
	return number;
}

/**********************************************************************************************
 * ������	:read_config_ini_file()
 * ����	:��config�ļ��еĲ�����Ϣ��ȡ�������ṹ��
 * ����	:filename:�����ļ���
 * ���	:para:����ʱ�������Ľṹָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int read_config_ini_file(char *filename, struct ipmain_para_struct *para)
{
	confdict *ini;
	if((filename==NULL)||(para==NULL))
		return -10;
	ini=confparser_load(filename);
	if (ini==NULL) 
        {
             printf("vsmain  cannot parse ini file [%s]", filename);
             return -1 ;
        }

	para->internet_mode = confparser_getint(ini,"INTERNET_MODE",0);
	confparser_freedict(ini);
	return 0;
}

/**********************************************************************************************
 * ������	:get_internet_mode_str()
 * ����		:��ȡinternet���ӷ�ʽ�ַ���
 * ����		:��
 * ����ֵ	:����internet���ӷ�ʽ���ַ���
 **********************************************************************************************/
char *get_internet_mode_str(void)
{
	switch(ipmain_para.internet_mode)
	{
		case 0:
			return "ADSL����";
		break;
		case 1:
			return "����������";
		break;
		case 2:
			return "ר�߽���";
		break;
	}
	return "��ȷ��";
}

static char alarm_schdule_str[200];
/**********************************************************************************************
 * ������	:get_setalarm_str()
 * ����		:��ȡ�����ı����ṹ�Ĳ�������ʱ������
 * ����		:�����ṹalarm_trigin_struct��ָ�룬�Զ��Ӻ��ƶ���ⶼ����
 * ����ֵ	:���������ı����ṹ�Ĳ�������ʱ���������ַ���
 **********************************************************************************************/
char *get_setalarm_str(struct alarm_trigin_struct *trigin)
{
	
	int starttime=0;
	int endtime=0;
	
	if(trigin==NULL)
		return "��ȷ��";
	if(trigin->enable==0)
		return "��Ч";
	if(trigin->setalarm==0)
		return "����";

	starttime=trigin->starthour*60+trigin->startmin;
	endtime=trigin->endhour*60+trigin->endmin;

	if(starttime==endtime)	//��ʼʱ��=����ʱ��
		return "ȫ�첼��";

	sprintf(alarm_schdule_str,"%02d:%02d~%02d:%02d����",trigin->starthour,trigin->startmin,trigin->endhour,trigin->endmin);
	return alarm_schdule_str;
}
//��ȡ��Ƶǰ�˵Ĳ���
static int read_video_para(dictionary  *ini,struct enc_front_struct *front, int number)
{
	char sec[50];
	int val;
	int len;
	char *p;
	char *name;
	if((ini==NULL)||(front==NULL))
		return -1;
	sprintf(sec,"video%d",number);	
	len=strlen(sec);
	if(len>30)
		return -1;
	p=sec+len;
	*p=':';
	p++;
	
	
	sprintf(p,"%s","bright");
	val=iniparser_getint(ini,sec,51);
	front->bright=val;

	sprintf(p,"%s","color");
	val=iniparser_getint(ini,sec,51);
	front->hue=val;

	sprintf(p,"%s","contrast");
	val=iniparser_getint(ini,sec,51);
	front->contrast=val;

	sprintf(p,"%s","saturation");
	val=iniparser_getint(ini,sec,61);
	front->saturation=val;
	
	sprintf(p,"%s","enable");
	front->enable=iniparser_getint(ini,sec,1);


	//��ȡosd���:name
	sprintf(p,"%s","name");
	name=iniparser_getstr(ini,sec);
	if(name != NULL)
	{
		strncpy(front->name,name,MAX_CAMERANAME_LENGTH); //���������20����!
		front->name[MAX_CAMERANAME_LENGTH+1] = '\0';
	}
	else
	{
		sprintf(front->name, "��Ƶ%d",number);
	}
	//gtloginfo("bright=%d hue=%d contrast=%d saturation=%d\n",front->bright,front->hue,front->contrast,front->saturation);
	
	return 0;
}

//��ȡ�ƶ����Ĳ���
static int read_motion_para(dictionary  *ini,char *section,struct motion_struct *motion)
{
	char sec[50];
	char result[100];
	char *res;
	int len;
	int i;
	char *p;
	if((ini==NULL)||(section==NULL)||(motion==NULL))
		return -1;
	len=strlen(section);
	if(len>30)
		return -1;
	memcpy(sec,section,len+1);
	p=sec+len;
	*p=':';
	p++;
  
	sprintf(p,"%s","motion_sen");
	motion->sen=iniparser_getint(ini,sec,0);

	sprintf(p,"%s","motion_alarm");
	motion->alarm=iniparser_getint(ini,sec,0);

	//����area��Ϣ
	for(i=0;i<12;i++)
	{
		sprintf(p,"area%d",i);
		res=iniparser_getstring(ini,sec,"0xffff");
		if(res!=NULL)
		{
			sprintf(result,"%s",res);
			//printf("\n\n\n\nresult is %s, convert to %d\n",result,atohex(result));
			motion->area[i]=atohex(result);
		}
		else
			motion->area[i]=0xffff;

	}
	sprintf(p,"%s","starthour");
	motion->starthour=iniparser_getint(ini,sec,0);
	sprintf(p,"%s","startmin");
	motion->startmin=iniparser_getint(ini,sec,0);
	sprintf(p,"%s","endhour");
	motion->endhour=iniparser_getint(ini,sec,0);
	sprintf(p,"%s","endmin");
	motion->endmin=iniparser_getint(ini,sec,0);
	
	return 0;
}

static int read_blind_para(dictionary  *ini,char *section,struct blind_struct *blind)
{
	char sec[50];
	//int val;
	int len;
	char *p;
	if((ini==NULL)||(section==NULL)||(blind==NULL))
		return -1;
	len=strlen(section);
	if(len>30)
		return -1;
	memcpy(sec,section,len+1);
	p=sec+len;
	*p=':';
	p++;

	sprintf(p,"%s","blind_sen");
	blind->sen=iniparser_getint(ini,sec,0);	
	sprintf(p,"%s","blind_alarm_time");
	blind->alarm_time=iniparser_getint(ini,sec,3); 
	sprintf(p,"%s","blind_cancelalarm_time");
	blind->cancelalarm_time=iniparser_getint(ini,sec,10);//3);wsy changed
	return 0;
}

//��ȡ����ָ�������
static int read_quad_para(dictionary  *ini,struct quad_dev_struct *quad)
{
	int i;
	char sec[20];
	if((ini==NULL)||(quad==NULL))
		return -1;
	
	//quad->current_local_ch=iniparser_getint(ini,"hqpara:rec_ch",4);
	//if(quad->current_local_ch>3)
	//	quad->current_local_ch=4;

	//quad->current_net_ch=iniparser_getint(ini,"netencoder:net_ch",4);
	//if(quad->current_net_ch>3)
		quad->current_net_ch=4;	
		
	for(i=0;i<get_video_num();i++)
	{
		sprintf(sec,"video%d",i);
		read_motion_para(ini,sec,&quad->motion[i]);
		read_blind_para(ini,sec,&quad->blind[i]);	
	}	
	return 0;
}

typedef struct{
	int chan_enable[4];
}CHANS_ENABLE;

CHANS_ENABLE channels_enable;

//lc do 2013-11-1 ���ظ÷�����Ӧ����Ƶͨ����
int parse_alarm_trigin_video_ch(char* channelstr)
{
	memset((void*)&channels_enable,0,sizeof(CHANS_ENABLE));
	int i = 0; int j = 0;
	int count  = 0;
	int chnidx = -1;
	while(*(channelstr+i) != 0)
	{
		//printf("channelstr %d char is %c\n",i,*(channelstr+i));
		//channels_enable.chan_enable[i] = 0;
		
		if( *(channelstr+i) == ',' )
		{
			i++;
			continue;
		}
		else
		{
			j= *(channelstr+i)-'0';
			channels_enable.chan_enable[j] = 1;
			chnidx = j;
			//printf("channels_enable chan_enable %d is 1!!\n",j);
			i++;
			count++;
		}
	}

	if(count > 1)
		chnidx = 4;

	return chnidx;
}





//������ini�ж�ȡ�������������õ���Ӧ����
int read_motion_ini_file(char *filename,struct alarm_motion_struct *alarmmotion)
{
	dictionary      *ini;
	int i;
	char *pstr;
	char sec[50]; //��������"alarm0"
	char string[200];
	int mask=0;
	int num=0;
	int intrig=0;
	char* alarm_video=NULL;
	
	struct alarm_trigin_struct *trigin;
	struct ipmain_para_struct *p;
	
	if((filename==NULL)||(alarmmotion==NULL))
		return -10;
		
	ini=iniparser_load(filename);
        if (ini==NULL) 
        {
             printf("vsmain  cannot parse ini file file [%s]", filename);
             return -1 ;
        }

    p=get_mainpara();
   // gtloginfo("trigin 0x%x,inmask 0x%x,out 0x%x, outmask 0x%x\n",p->trig_in,p->tin_mask,p->alarm_out,p->alarm_mask);

    alarmmotion->audioalarm=0;

	pthread_mutex_init(&pb_Tag.mutex,NULL);
	
	for(i=get_trigin_num()-1;i>=0;i--)
	{
		pb_Tag.pb[i] = 0;
		trigin=&alarmmotion->trigin[i];
		trigin->trigin=i; 
		sprintf(sec,"alarm%d:enable",i);	
		trigin->enable=iniparser_getint(ini,sec,1);
		mask|=trigin->enable;
		mask<<=1;
		sprintf(sec,"alarm%d:setalarm",i);	
		trigin->setalarm=iniparser_getint(ini,sec,1);
		//gtloginfo("%d·enable=%d,setalarm=%d\n",i,trigin->enable,trigin->setalarm);
		sprintf(sec,"alarm%d:attrib",i);	//��ȡ������������
		intrig|=iniparser_getint(ini,sec,0);
		intrig<<=1;
		//lc do ��[alarmx]���У���ÿ�����Ӽ����Ӧ��playback_ch�ֶΣ���ʾ��Ӧ�Ļط�ch
		sprintf(sec,"alarm%d:playback_ch",i);
//lc do 2013-11-1 ����һ�Զ�֧��
		alarmmotion->alarm_trigin_video_ch[i] = iniparser_getint(ini,sec,-1);		
/*
		alarm_video = iniparser_getstr(ini,sec);
		if(alarm_video != NULL)
		{
			alarmmotion->alarm_trigin_video_ch[i] = parse_alarm_trigin_video_ch(alarm_video);
			printf("alarmmotion->alarm_trigin_video_ch[%d] is %d\n",i,alarmmotion->alarm_trigin_video_ch[i]);
		}
		else
			alarmmotion->alarm_trigin_video_ch[i] = -1;
*/
		sprintf(sec,"alarm%d:imact",i);	
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->imact,string);
			if(strstr(string,"40")!=NULL) //��Ҫ������ʾ
				alarmmotion->audioalarm=1;
			//printf("trigin%d-imact��%d��\n",i,num);
		}
		sprintf(sec,"alarm%d:ackact",i);
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->ackact,string);
			//printf("trigin%d-ackact��%d��\n",i,num);
		}
		sprintf(sec,"alarm%d:rstact",i);	
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->rstact,string);
			//printf("trigin%d-rstact��%d��\n",i,num);
		}	

		//wsy add below for alarm-schedule
		sprintf(sec,"alarm%d:starthour",i);	
		trigin->starthour=iniparser_getint(ini,sec,0);
		sprintf(sec,"alarm%d:startmin",i);	
		trigin->startmin=iniparser_getint(ini,sec,0);
		sprintf(sec,"alarm%d:endhour",i);	
		trigin->endhour=iniparser_getint(ini,sec,0);
		sprintf(sec,"alarm%d:endmin",i);	
		trigin->endmin=iniparser_getint(ini,sec,0);
		
	}	
	p->tin_mask&=(mask>>1);
	p->trig_in|=(intrig>>1); //��tin_mask��trig_in����


	for(i=get_video_num()-1;i>=0;i--)
	{
		trigin=&alarmmotion->motion[i];
		trigin->trigin=i+get_trigin_num();
		//sprintf(sec,"motion%d:enable",i);	
		//trigin->enable=iniparser_getint(ini,sec,0);
		
		sprintf(sec,"video%d:motion_sen",i);	
		if(iniparser_getint(ini,sec,0)==0)
			trigin->enable=0;
		else
			trigin->enable=1;
		sprintf(sec,"motion%d:setalarm",i);	
		trigin->setalarm=iniparser_getint(ini,sec,1);	
		sprintf(sec,"motion%d:imact",i);		
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->imact,string);
		//	printf("motion%d-imact��%d��\n",i,num);
		}
		sprintf(sec,"motion%d:ackact",i);		
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->ackact,string);
		//	printf("motion%dackact��%d��\n",i,num);
		}
		sprintf(sec,"motion%d:rstact",i);	
		pstr=iniparser_getstring(ini,sec,NULL);
		if(pstr!=NULL)
		{
			
			num=strlen(pstr);
			memcpy(string,pstr,(num+1));
			num=string_to_actions(trigin->rstact,string);
		//	printf("motion%drstact��%d��\n",i,num);
		}
		
		
	}
	iniparser_freedict(ini);


	//��¼��Ӧ��־
	for(i=0;i<get_trigin_num();i++)
	{
		trigin=&alarmmotion->trigin[i];
		printf("���Ӵ���%d ����: %s\n",i,get_setalarm_str(trigin));
		gtloginfo("���Ӵ���%d ����: %s\n",i,get_setalarm_str(trigin));	
	}
	#if 0	//��ʱע������Ϊ���и�motion_alarm������
	for(i=0;i<get_video_num();i++)
	{
		trigin=&alarmmotion->motion[i];
		printf("�ƶ����%d ����: %s\n",i,get_setalarm_str(trigin));
		gtloginfo("�ƶ����%d ����: %s\n",i,get_setalarm_str(trigin));	
	}
	#endif
	return 0;	

			
}

/**********************************************************************************************
 * ������	:readmain_para_file()
 * ����	:��ip1004.ini�ļ��еĲ�����Ϣ��ȡ�������ṹ��
 * ����	:filename:�����ļ���
 * ���	:para:����ʱ�������Ľṹָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int  readmain_para_file(char *filename,struct ipmain_para_struct *para)
{//

	struct ipmain_para_struct *p;
	//struct hostent *hostPtr = NULL;
	dictionary      *ini;	
	FILE *lock_fp=NULL;
	struct video_para_struct *vadc;
	int i;
	char *pstr;
	//int status;
	int num;
	int ret;
	refresh_netinfo();	//��ȡ������Ϣ
	if(filename==NULL)
		return -10;
	p=para;
	if(p==NULL)
		return -11;
	//ini=iniparser_load(filename);
	ini=iniparser_load_lockfile(filename,1,&lock_fp);
	if (ini==NULL) {
                printf("vsmain  cannot parse ini file file [%s]", filename);
                return -1 ;

        }
		
	//iniparser_dump_ini(ini,stdout);	//��ini�ļ�������ʾ����Ļ�ϣ�ʵ��ʹ��ʱû���ã�Ӧȥ��
	pstr=iniparser_getstring(ini,"install:inst_place",NULL);
	if(pstr!=NULL)
	{
		num=strlen(pstr);
		if(num>99)
			num=99;
		p->devpara[0].inst_place[num]='\0';
		memcpy(p->devpara[0].inst_place,pstr,(num+1));
	}
	
	pstr=iniparser_getstring(ini,"install_dev2:inst_place",NULL);
	if(pstr!=NULL)
	{
		num=strlen(pstr);
		if(num>99)
			num=99;
		p->devpara[1].inst_place[num]='\0';
		memcpy(p->devpara[1].inst_place,pstr,(num+1));
	}
	
	GATE_PERIOD_TIME=iniparser_getint(ini,"remote_gate:timeout",GATE_PERIOD_TIME_DEF);
	if(GATE_PERIOD_TIME<=0)
		GATE_PERIOD_TIME=0x7fffffff;//���С��0���ʾ����Ҫ�Ͽ�����

	p->rmt_env_mode=iniparser_getint(ini,"remote_gate:env",MSG_AUTH_SSL);
	if(p->rmt_env_mode!=0)
	{
		p->rmt_env_mode=MSG_AUTH_SSL;
		p->rmt_enc_mode=iniparser_getint(ini,"remote_gate:enc",DES_EDE3_CBC);
		if(p->rmt_enc_mode>RC5_32_12_16_CBC)
			p->rmt_enc_mode=DES_EDE3_CBC;
	}


	//��ȡ����˿ڲ���
	p->devpara[0].cmd_port = iniparser_getint(ini,"port:cmd_port",DEV_MAIN_CMD_PORT);
	p->devpara[1].cmd_port = iniparser_getint(ini,"port_dev2:cmd_port",DEV_MAIN_CMD_PORT+1000);

	p->image_port=iniparser_getint(ini,"port:image_port",DEV_MAIN_IMAGE_PORT);
	p->audio_port=iniparser_getint(ini,"port:audio_port",DEV_MAIN_AUDIO_PORT);
	p->ftp_port=iniparser_getint(ini,"port:ftp_port",DEV_DEFAULT_FTP_PORT);
	p->web_port=iniparser_getint(ini,"port:web_port",DEV_DEFAULT_WEB_PORT);
	p->telnet_port=iniparser_getint(ini,"port:telnet_port",DEV_DEFAULT_TELNET_PORT);
	p->com0_port=iniparser_getint(ini,"port:com0_port",COM1_TCP_PORT);
	p->com1_port=iniparser_getint(ini,"port:com1_port",COM2_TCP_PORT);
	p->pb_port=iniparser_getint(ini,"port:playback_port",8900);

	pstr=iniparser_getstring(ini,"remote_gate:rmt_gate1",NULL);
	ret = set_sock_addr(p->devpara[0].rmt_gate1,pstr);
	pstr=iniparser_getstring(ini,"remote_gate:rmt_gate2",NULL);
	ret = set_sock_addr(p->devpara[0].rmt_gate2,pstr);
	pstr=iniparser_getstring(ini,"remote_gate:rmt_gate3",NULL);
	ret = set_sock_addr(p->devpara[0].rmt_gate3,pstr);
	pstr=iniparser_getstring(ini,"remote_gate:rmt_gate4",NULL);
	ret = set_sock_addr(p->devpara[0].rmt_gate4,pstr);
	pstr=iniparser_getstring(ini,"remote_gate:alarm_gate",NULL);
	set_sock_addr(p->devpara[0].alarm_gate,pstr);
	
	pstr=iniparser_getstring(ini,"remote_gate_dev2:rmt_gate1",NULL);
	ret = set_sock_addr(p->devpara[1].rmt_gate1,pstr);
	pstr=iniparser_getstring(ini,"remote_gate_dev2:rmt_gate2",NULL);
	ret = set_sock_addr(p->devpara[1].rmt_gate2,pstr);
	pstr=iniparser_getstring(ini,"remote_gate_dev2:rmt_gate3",NULL);
	ret = set_sock_addr(p->devpara[1].rmt_gate3,pstr);
	pstr=iniparser_getstring(ini,"remote_gate_dev2:rmt_gate4",NULL);
	ret = set_sock_addr(p->devpara[1].rmt_gate4,pstr);
	pstr=iniparser_getstring(ini,"remote_gate_dev2:alarm_gate",NULL);
	set_sock_addr(p->devpara[1].alarm_gate,pstr);
	
	read_motion_ini_file(MOTION_ALARM_PARA_FILE,&p->alarm_motion);
	
	//p->hq_follow_net=iniparser_getint(ini,"hqpara:follow_net",1);

	
	//p->hq_save_ch=iniparser_getint(ini,"hqpara:rec_ch",4);
	//if(p->hq_save_ch>3)
	//	p->hq_save_ch=4;

	p->net_ch=iniparser_getint(ini,"netencoder:net_ch",4);
	if(p->net_ch>3)
		p->net_ch=4;
	
//osd����
	p->devpara[0].osd.inst_place_display = iniparser_getint(ini,"install:inst_place_enable",0);
	
	p->devpara[0].osd.inst_place_posx	= iniparser_getint(ini,"install:inst_place_posx",INSTPLACE_DEF_POSX);
	if(p->devpara[0].osd.inst_place_posx > OSD_POSX_MAX)
		p->devpara[0].osd.inst_place_posx = OSD_POSX_MAX;
	if(p->devpara[0].osd.inst_place_posx < 0)
		p->devpara[0].osd.inst_place_posx = 0;
		
	p->devpara[0].osd.inst_place_posy	= iniparser_getint(ini,"install:inst_place_posy",INSTPLACE_DEF_POSY);
	if(p->devpara[0].osd.inst_place_posy > OSD_POSY_MAX)
		p->devpara[0].osd.inst_place_posy = OSD_POSY_MAX;
	if(p->devpara[0].osd.inst_place_posy < 0)
		p->devpara[0].osd.inst_place_posy = 0;

	if(virdev_get_virdev_number()==2)//�����豸
	{
		p->devpara[1].osd.inst_place_display = iniparser_getint(ini,"install_dev2:inst_place_enable",0);
		
		p->devpara[1].osd.inst_place_posx	= iniparser_getint(ini,"install_dev2:inst_place_posx",INSTPLACE_DEF_POSX);
		if(p->devpara[1].osd.inst_place_posx > OSD_POSX_MAX)
			p->devpara[1].osd.inst_place_posx = OSD_POSX_MAX;
		if(p->devpara[1].osd.inst_place_posx < 0)
			p->devpara[1].osd.inst_place_posx = 0;
			
		p->devpara[1].osd.inst_place_posy	= iniparser_getint(ini,"install_dev2:inst_place_posy",INSTPLACE_DEF_POSY);
		if(p->devpara[1].osd.inst_place_posy > OSD_POSY_MAX)
			p->devpara[1].osd.inst_place_posy = OSD_POSY_MAX;
		if(p->devpara[1].osd.inst_place_posy < 0)
			p->devpara[1].osd.inst_place_posy = 0;	
	}

	p->inst_ack=iniparser_getint(ini,"product:inst_ack",0);
	p->reset_modem=iniparser_getint(ini,"product:reset_modem",1);
	p->reset_modem_time=iniparser_getint(ini,"product:reset_modem_time",120);
	//p->log_watch_lost=iniparser_getint(ini,"product:log_watch_lost",1);

	p->ini_version=iniparser_getint(ini,"alarmmotion:version",0);

//vadc ����
	vadc = &p->vadc;
	for(i=0;i<get_video_num();i++)
	{
		read_video_para(ini,&vadc->enc_front[i],i);
	}
	
	read_quad_para(ini, &vadc->quad);


//����ץͼ����
	//p->snap_file_num=iniparser_getint(ini,"alarm:snap_pic_num",ALARM_SNAP_NUM);
   // p->snap_file_interval=iniparser_getint(ini,"alarm:snap_pic_interval",ALARM_SNAP_INTERVAL);
	//pstr=iniparser_getstring(ini,"alarm:snap_pic_path","/picindex/alarmpic.txt");
	//sprintf(p->snap_file_path,"%s",pstr);
	//gtloginfo("test,path %s\n",p->snap_file_path);
	
	p->net_ch_osd_picsize=iniparser_getint(ini,"netencoder:vidpicsize",1);
  	//p->local_ch_osd_picsize=iniparser_getint(ini,"hqenc0:vidpicsize",1);
  
  //zw-add 2012-04-24-->
  //��ȡ���Ӵ����˾���¼��طŹ��ܱ�־Ϊ��0Ϊ���ã�1Ϊ����
  p->alarm_motion.playback_enable=iniparser_getint(ini,"netencoder:playback_enable",1); //Ĭ��Ϊ��Ҫ
  //zw-add 2012-04-24<--

	//2013-6-3 lc do ͨ���˲����رմ���ttyAMA0���������ܣ���Ϊ����ʹ��
    serial_debug_mode = iniparser_getint(ini,"serial:serial_mode",0);
    serial_interval = iniparser_getint(ini,"serial:serial_interval",1);

	p->multi_channel_enable = iniparser_getint(ini,"multichannel:enable",0);
	p->power_monitor_enable = iniparser_getint(ini,"power_mon:enable",0);
	p->current_audio_down_channel = -1;

	p->buzzer_alarm_enable = iniparser_getint(ini,"buzalarm:enable",0);
	//lc add 2014-10-20
	p->gatedown_relay_count = iniparser_getint(ini,"gatedown_relay:count",0);
	p->gatedown_relay_chn = iniparser_getint(ini,"gatedown_relay:chn",0);

	save_inidict_file(filename,ini,&lock_fp);	//zw-add �������ݵ�ip1004.ini��
  	iniparser_freedict(ini);
	return 0;	
}

/**********************************************************************************************
 * ������	:get_mainpara()
 * ����	:��ȡ�豸�����ṹָ��
 * ����	:��
 * ����ֵ	:�豸�����ṹָ��
 **********************************************************************************************/
struct ipmain_para_struct * get_mainpara(void)
{//
	return &ipmain_para;
}

int get_serial_mode()
{
	return serial_debug_mode;
}

int get_serial_interval()
{
	return serial_interval;
}

/**********************************************************************************************
 * ������	:AddParaFileVersion()
 * ����	:�������ļ��İ汾�ż�1
 * ����	:type��ʾ�����ļ�����  2:ip1004.ini 3:alarm.ini
 * ����ֵ	:�µİ汾��,��ֵ��ʾʧ��
 **********************************************************************************************/
int AddParaFileVersion(int dev_no,int type)
{
	char *filename;
	dictionary      *ini;	
	int version;
	char vbuf[30];
	if(type==2)
	{
		filename = IPMAIN_PARA_FILE;
	}
	else if(type==3)
	{
		filename=MOTION_ALARM_PARA_FILE;
	}
	else
	{
		return -1;
	}	
	ini=iniparser_load(filename);
        if (ini==NULL) {
                printf("vsmain  cannot parse ini file file [%s]", filename);
                return -1 ;

        }
	version=iniparser_getint(ini,"alarmversion:version",-1);
		
	if(version<0)
	{//���ڽ� alarmversion
		if(iniparser_find_entry(ini,"alarmversion")==1)
			version=0;
	}
	iniparser_freedict(ini);
	if(version>=0)
	{
		version++;
		sprintf(vbuf,"%d",version);
		save2para_file(filename,"alarmversion:version",vbuf);
	}
	return version;
}

/**********************************************************************************************
 * ������	:CheckParaFileChange()
 * ����	:��������ļ��Ƿ��б仯
 * ����	:type��ʾ�����ļ�����  2:ip1004.ini 3:alarm.ini
 * ����ֵ	:����0��ʾ�ޱ仯  1��ʾ�б仯 -1��ʾ���ʹ���
 **********************************************************************************************/
int CheckParaFileChange(int dev_no,int type)
{
	char *filename;
	char *bakfile;
	int ret;
	if(type==2)
	{
		filename=IPMAIN_PARA_FILE;
		bakfile=IPMAIN_PARA_GATE_BAK;
	}
	else if(type==3)
	{
		filename=MOTION_ALARM_PARA_FILE;
		bakfile=MOTION_ALARM_GATE_BAK;
	}
	else
	{
		return -1;
	}

	//�Ƚ������ļ�
	ret=ini_diff(bakfile,filename);
	if(ret==0)
		return 0;
	else
		return 1;	
}


/**********************************************************************************************
 * ������	:CopyPara2Bak()
 * ����	:�������ļ����µ���Ӧ�ı���
 * ����	:type:�����ļ�����2:ip1004.ini 3:alarm.ini
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/

int CopyPara2Bak(int dev_no,int type)
{
	char *filename;
	char *bakfile;
	int ret;
	char tbuf[200];
	if(type==2)
	{
		filename=IPMAIN_PARA_FILE;
		bakfile=IPMAIN_PARA_GATE_BAK;

	}
	else if(type==3)
	{
		filename=MOTION_ALARM_PARA_FILE;
		bakfile=MOTION_ALARM_GATE_BAK;
	}
	else
	{
		return -1;
	}
	gtloginfo("�����ļ��ı���µ�����:%s->%s\n",filename,bakfile);
	sprintf(tbuf,"cp -f %s %s",filename,bakfile);
	ret=system(tbuf);	
	
	return 0;
}

/**********************************************************************************************
 * ������	:dump_para_to_file()
 * ����	:���ڴ��д洢�Ĳ������ı��ķ�ʽ�����һ���Ѵ򿪵��ļ���
 * ����	:��
 * ���	:�Ѵ򿪵��ļ�ָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int dump_para_to_file(FILE *fp)//FIXMEWSY,at last
{//���������������һ���Ѿ��򿪵��ļ�ָ��
	struct ipmain_para_struct *p;
	struct sockaddr_in rmt;
	if(fp==NULL)
		return -1;
	p=&ipmain_para;
	fprintf(fp,"ipmain_para struct \n");
	fprintf(fp,"{\n");
	fprintf(fp,"\ttotal_com=%d\n",p->total_com);		//����ʹ�õ�͸��������
	memcpy((void*)&rmt.sin_addr.s_addr,(void*)&p->wan_addr,sizeof(p->wan_addr));
	fprintf(fp,"\twan_addr=%s\n",inet_ntoa(rmt.sin_addr));
	fprintf(fp,"\trmt_env_mode=0x%x\n",p->rmt_env_mode);				//��Զ�̷�����ͨѶʹ�õ�����ǩ������
	fprintf(fp,"\trmt_enc_mode=0x%x\n",p->rmt_enc_mode);				//��Զ�̷�����ͨѶ����ʹ�õļ�������
	fprintf(fp,"\ttrig_in=0x%02x\n",p->trig_in);
	fprintf(fp,"\ttin_mask=0x%02x\n",p->tin_mask);
	fprintf(fp,"\talarm_out=0x%02x\n",p->alarm_out);
	fprintf(fp,"\talarm_mask=0x%02x\n",p->alarm_mask);
	//fprintf(fp,"\thq_follow_net=%d\n",p->hq_follow_net);
	//fprintf(fp,"\thq_save_ch=%d\n",p->hq_save_ch);

//FIXME quad??	struct tw2824_init_struct quad;			//tw2824�Ĳ���
	fprintf(fp,"\tnet_ch=%d\n",p->net_ch);

	fprintf(fp,"\tvideo_in_ch=%d\n",p->video_in_ch);
	fprintf(fp,"\tvideo_mask=0x%02x\n",p->video_mask);

	fprintf(fp,"\timage_port=%d\n",p->image_port);
	fprintf(fp,"\taudio_port=%d\n",p->audio_port);
	fprintf(fp,"\tftp_port=%d\n",p->ftp_port);
	fprintf(fp,"\tweb_port=%d\n",p->web_port);
	fprintf(fp,"\tcom0_port=%d\n",p->com0_port);
	fprintf(fp,"\tcom1_port=%d\n",p->com1_port);
	fprintf(fp,"\tdev_type=%d\n",p->dev_type);
	fprintf(fp,"\tinst_ack=%d\n",p->inst_ack);
	fprintf(fp,"\tvalid_cert=%d\n",p->valid_cert);
	fprintf(fp,"\treset_modem=%d\n",p->reset_modem); 
	fprintf(fp,"\treset_modem_time=%d\n",p->reset_modem_time); 
	fprintf(fp,"\tinternet_mode=%d\n",p->internet_mode); 
	//fprintf(fp,"\tlog_watch_lost=%d\n",p->log_watch_lost);
	fprintf(fp,"}\n");
	return 0;
}



