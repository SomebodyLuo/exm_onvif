/* �����������
*/


#include "ipmain.h"
#include "trans_com.h"
#include <commonlib.h>
#include "watch_process.h"
#include "netcmdproc.h"
#include "ipmain_para.h"
#include "alarm_process.h"
#include "process_rtimg.h"
#include <sys/types.h>
#include "netinfo.h"
#include "leds_api.h"
#include "gate_connect.h"
#include "maincmdproc.h"
#include "watch_board.h"
#include "devstat.h"
#include "infodump.h"
#include "hdmodapi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "update.h"
#include "remote_file.h"
#include <hdctl.h>
#include "xmlparser.h"
//#include "gtvs_io_api.h"
#include "diskinfo.h"
#define bypass_videoenc_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,VIDEOENC_MOD_ID,env,enc,dev_no) 
#define bypass_rtimg_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,RTIMAGE_PROCESS_ID,env,enc,dev_no) 
#define bypass_rtaudio_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,RTIMAGE_PROCESS_ID,env,enc,dev_no)//�������н����Ѿ���ͼ����̺ϲ�RTAUDIO_PROCESS_ID)
#define bypass_hqsave_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,HQSAVE_PROCESS_ID,env,enc,dev_no)
#define bypass_diskman_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,DISKMAN_ID,env,enc,dev_no)
#define bypass_hdpb_cmd(fd,cmd,env,enc,dev_no) bypass_gatecmd2mod(fd,cmd,PLAYBACK_PROCESS_ID,env,enc,dev_no)
static pthread_mutex_t update_mutex = PTHREAD_MUTEX_INITIALIZER;
static int update_waiting_no=0;	//��¼����������Ҫ���������Ŀ
static gateinfo updatesw_gate_list[10];//��ŷ���������Ҫ���������Ϣ

static int updating_flag=0;

static struct send_dev_trig_state_struct last_dev_trig={0,0,0,0,0,0,0,0};

#define UPDATE_UNRESPONED_INTERVAL 300

//static int usr_update_software_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc);
//static int usr_get_hq_pic_cmd(int fd,struct gt_usr_cmd_struct *netcmd);
//static int usr_query_index_cmd(int fd,struct gt_usr_cmd_struct *netcmd);

/**********************************************************************************************
 * ������	:init_netcmd_var()
 * ����	:��ʼ���������������ر���
 * ����	:��
 *����ֵ	:��
 **********************************************************************************************/
void init_netcmd_var(void)
{
	int i = 0;

		set_regist_flag(i,0);
		set_alarm_flag(i,1);
		set_reportstate_flag(i,0);
		set_trigin_flag(i,1);

	memset(updatesw_gate_list,0,sizeof(updatesw_gate_list));
}

int print_devid(BYTE *buf,int len)
{
        int i;
        for(i=0;i<(len-1);i++)
                printf("%02x-",buf[len-i-1]);
        printf("%02x ",buf[0]);
        return 0;
}

//Զ��ϵͳ�����豸ʱ��
//������������߳�
int rmt_set_time(struct tm *ntime)
{
	int ret;
	//int ret2;
	time_t curtime,newt;
	double diff;
	//struct tm tmt;
	if(ntime==NULL)
		return -1;
	newt = mktime(ntime);
	curtime=time(NULL);
	diff=difftime(curtime,newt);
	printf("time on dev is %d seconds later than server time\n",(int)diff);
	gtloginfo("�豸ʱ��ȷ�����ʱ���%d��\n",(int)diff);
#if 0	//removed by shixin
	if(((newt+10)<curtime)||((curtime+10)<newt))//��10������������������¼�����
	{
		ret=stop_record_file_thread(get_hdch0());
		sleep(1);
	}
#endif
	//printf("vsmain recv a set rmt_set_time cmd new time:%d-%02d-%02d %02d:%02d:%02d\n",
	//	ntime->tm_year,ntime->tm_mon,ntime->tm_mday,ntime->tm_hour,ntime->tm_min,ntime->tm_sec);
	
	//gtloginfo("%04d-%02d-%02d %02d:%02d:%02d\n",
	//	tmt.tm_year+1900,tmt.tm_mon+1,tmt.tm_mday,(int)tmt.tm_hour+(int)tmt.tm_min+(int)tmt.tm_sec);
#ifdef ARCH_3520A
	ret=set_dev_time(ntime);
#endif
#ifdef ARCH_3520D
	ret=set_dev_time_d(ntime);
#endif


	if(ret==0)
	{
		printf("ʱ�����óɹ�\n");
		gtloginfo("ʱ�����óɹ�\n");
	}
	else
	{
		printf("ʱ������ʧ�� err=%d\n",ret);
		gtloginfo("ʱ������ʧ�� err=%d\n",ret);
	}
	return ret;
}



int get_dev_sitename(int dev_no,BYTE *devsite)

{
	int len;
	struct ipmain_para_struct *main_para;
	main_para=get_mainpara();
	memset(devsite,0,40);
	len=strlen(main_para->devpara[dev_no].inst_place);

	if(len>40)
		len=38; //�����Ǻ���
	main_para->devpara[dev_no].inst_place[len]='\0';
	memcpy(devsite,main_para->devpara[dev_no].inst_place,len+1);
	return 40;
}
//����豸������ip��ַ(���Ǿ�������ַ)


//����豸������ip��ַ(���Ǿ�������ַ)
static  int get_sys_ip(DWORD *ip)
{
	in_addr_t *addr;
	//int ad;
	struct ipmain_para_struct *main_para;
	main_para=get_mainpara();
	if(main_para->wan_addr!=0)
	{
		addr=&main_para->wan_addr;
	}
	else
		addr=&main_para->lan_addr;

	memcpy(ip,addr,4);
	
	return 16;
}	

#ifdef TEST_FROM_TERM
void set_reg_per_state(DWORD state)
{
	memcpy((char*)&ip1004_state.reg_per_state,&state,4);
}
void set_reg_dev_state(DWORD state)
{
	memcpy((char*)&ip1004_state.reg_dev_state,&state,4);
}
void set_reg_trig_state(DWORD state)
{
	memcpy((char*)&ip1004_state.reg_trig_state,(char*)&state,4);
}
#endif

#define BUZ_TEMP "/tmp/buzgrep"
int getgrepcmd(const char *cmd)
{
	int found;
	system(cmd);
	found = get_file_lines(BUZ_TEMP);
	return found;
}


void active_buz(void)
{
	printf("active buz!\n");
		char pbuf[200];
	    int  buz_playnum = 0;
	    sprintf(pbuf,"ps | grep alarmbuz > %s",BUZ_TEMP);
	    buz_playnum = getgrepcmd(pbuf);
	    printf("buz num now is %d\n",buz_playnum);
	    if(!buz_playnum)
	    {		
			gtloginfo("������ʾbuz�����\n");
			system("/conf/alarmbuz.sh & >/dev/null");	
	    }
}

void kill_buz(void)
{
	char  cmdline[100];
	char buff[50]; 
	FILE *fstream=NULL;
	int  buzpid=0;

	printf("kill buz\n");

	memset(cmdline,0,100);
	memset(buff,0,sizeof(buff));  
	sprintf(cmdline,"ps | grep alarmbuz | grep sh | awk '{print $1}' ");

	if(NULL==(fstream=popen(cmdline,"r")))       
    {      
        printf("execute command failed: %s",strerror(errno));       
        return -1;       
    } 

	if(NULL!=fgets(buff, sizeof(buff), fstream))      
    {      
        printf("alarmbuz's pid is %s!\n",buff);
    }      
    else     
    {     
        pclose(fstream);     
		return;
    } 
	
	memset(cmdline,0,100);
	sprintf(cmdline,"kill %s\n",buff);
	system(cmdline);
	system("/conf/buzzer.sh");
}


/**********************************************************************************************
 * ������	:system_regist()
 * ����	:��ϵͳ��Ϣע�ᵽԶ�����ط�����ϵͳע��
 * ����	:fd:socket���������� >0��ʾ�Ѿ����������� -1��ʾ��Ҫ��������
 *				���߳̽���һ������
 *			  ack:�Ƿ���Ҫ����ȷ�� 1��ʾ��Ҫ  0��ʾ����Ҫ
 *			env:ǩ������,fdΪ��ʱ��������
 *			enc:�����㷨
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/

int  system_regist(int fd,int ack, int env, int enc,int dev_no)
{//
	int rc;
	DWORD send_buf[150];
//	char rtversion[30];
//	char hdversion[30];

//	char tbuf[256];

	struct tm *lvtime;
	//time_t ctime;
	struct mod_com_type *modcom;
	struct gt_pkt_struct *send;
	struct gt_usr_cmd_struct *cmd;
	struct ip1004_info_struct *info;
	struct ipmain_para_struct * para;
	int i;
	int *ppb;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(fd>0)//����������ע��
	{
		send=(struct gt_pkt_struct *)send_buf;
		cmd=(struct gt_usr_cmd_struct *)send->msg;

	}
	else
	{
		modcom=(struct mod_com_type *)send_buf;
		modcom->env = env;
		modcom->enc = enc;
		cmd=(struct gt_usr_cmd_struct *)modcom->para;
	}
	
	cmd->cmd=DEV_REGISTER;
	cmd->en_ack=ack;
	cmd->reserve0=0;
	cmd->reserve1=0;
	para=get_mainpara();
	rc=virdev_get_devid(dev_no,cmd->para);
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_IP1004_INFO_STRUCT;
	info=(struct ip1004_info_struct *)(&cmd->para[rc]);
	info-> vendor	=DEVICE_VENDOR;			//�豸�����̱�ʶ(4) +�豸�ͺű�ʶ(4)
	info->device_type=get_devtype();			//�豸�ͺ�
	info->protocal_version=INTERNAL_PROTOCAL;	// ���غ�ǰ���豸ͨѶ��Э���ڲ��汾��(�޷�������)
	rc=get_dev_sitename(dev_no,info->site_name);	//��װ�ص�����
	rc=get_sys_ip(&info->dev_ip);				//�豸�Ķ�̬ip��ַ
	//	BYTE reserve[12];		//Ϊipv6��ַ��չ����
	info->dev_ip=ntohl(info->dev_ip);
 	info->boot_type= 0 ; //û��51���޷�get_regist_reason();		//�豸��ע��ԭ��,16λ����λ��ʾ����


	//��������Ӧ��������Ӧ�Ļ�ȡ����

	info->video_num=virdev_get_video_num(dev_no);		//�豸��Ƶ����ͨ������
	printf("info->video_num is %d\n",info->video_num);
	info->com_num=virdev_get_total_com(dev_no);			// ������
	info->storage_type=get_hd_type()+1;// HDTYPE;		//�豸�Ĵ洢�������� 0��û�� 1��cf�� 2��Ӳ��

#ifndef FOR_PC_MUTI_TEST
	info->storage_room=get_hd_capacity();
#else
	info->storage_room=1024*1024*80;
#endif
	info->compress_num=virdev_get_videoenc_num(dev_no);	//ѹ��ͨ����	ĿǰΪ1��2��5
	info->compress_type=1;	//ѹ���������ͣ�(ѹ��оƬ��ѹ����ʽ)
	info->audio_compress=1;	//��������ѹ������
	info->audio_in=virdev_get_audio_num(dev_no);			//��������ͨ������ĿǰΪ1
    info->audio_in_act=1;		//��������ͨ����Чλ����������1-8��0-ͨ����Ч��1-��Ч
	info->switch_in_num=virdev_get_trigin_num(dev_no);	//����������ͨ����
	if(info->switch_in_num != 0)
		info->switch_in_act=para->tin_mask;//����������ͨ����Чλ����������1-8��0-ͨ����Ч��1-��Ч
	info->switch_out_num=virdev_get_alarmout_num(dev_no);	//���������ͨ����
	if(info->switch_out_num != 0)
		info->switch_out=para->alarm_mask;		//���������ͨ����Чλ����������1-8��0-ͨ����Ч��1-��Ч
	info->max_pre_rec=1800;					//�豸���Ԥ¼ʱ�䣬����Ϊ��λ
	info->max_dly_rec=1800;					//�豸�����ʱ¼��ʱ��

	//�豸����ʱ��
	//ctime=time(NULL);
	//ptime=localtime(&ctime);
	lvtime=get_lvfac_time();
	info->year=lvtime->tm_year+1900;	//��
	info->month=lvtime->tm_mon+1;	//��
	info->day=lvtime->tm_mday; 		//��
	info->hour=lvtime->tm_hour;		//ʱ
	info->minute=lvtime->tm_min;		//��
	info->second=lvtime->tm_sec;		//��
	info->reserve1=0; 
	
	info->reserve2=0;

	//����˿�
	info->cmd_port=para->devpara[dev_no].cmd_port;
	//printf("send regist %d %d %d\n",info->cmd_port,info->image_port,info->audio_port);
	info->image_port=para->image_port;
	info->audio_port=para->audio_port;
	//ppb=(int*)info->ex_info;
	//*ppb = para->pb_port;
	memset(info->ex_info,0,sizeof(info->ex_info));
	sprintf(info->ex_info,"%d",para->pb_port);
	printf("system regist pb_port is %s\n",info->ex_info);
	
	memset(info->firmware,0,sizeof(info->firmware));
	sprintf(info->firmware,"%s",get_prog_ver());		
	memset(info->dev_info,0,sizeof(info->dev_info));
	if(get_quad_flag())
		sprintf(info->dev_info,"n:%s q:%s t:%d f:%d w:%d c:%d",get_internet_mode_str(),"tw2835",(int)para->telnet_port,(int)para->ftp_port,(int)para->web_port,(int)para->devpara[dev_no].cmd_port);
	else
		sprintf(info->dev_info,"n:%s t:%d f:%d w:%d c:%d",get_internet_mode_str(),(int)para->telnet_port,(int)para->ftp_port,(int)para->web_port,(int)para->devpara[dev_no].cmd_port);
	if(fd>0)
	{
		rc = gt_cmd_pkt_send(fd,send,cmd->len+2,NULL,0,env,enc);
		if(rc>0)
			rc = 0;
	}
	else	
		rc=send_gate_pkt(fd,modcom,cmd->len+2,dev_no);
	//gtloginfo("��Զ�̷�����ע��\n");
	if(rc==0)
	{
#ifdef SHOW_WORK_INFO
	printf("ipmain %s send a regist info\n",devlog(dev_no));
#endif
		return 0;
	}	
#ifdef SHOW_WORK_INFO
	printf("ipmain %s use send_gate_pkt()=%d send a regist info failed!\n",devlog(dev_no),rc);
#endif
	
	return -1;

}


static DWORD dev_state_old[MAX_DEV_NO]={0};
static DWORD per_state_old[MAX_DEV_NO]={0};
static DWORD alarmin_state_old[MAX_DEV_NO] = {0};

/**********************************************************************************************
 * ������	:send_dev_state()
 * ����	:�����豸״̬��Զ�̷�����
 * ����	:fd:socket���������� >0��ʾ�Ѿ����������� -1��ʾ��Ҫ��������
 *				���߳̽���һ������
 *			  ack:�Ƿ���Ҫ����ȷ�� 1��ʾ��Ҫ  0��ʾ����Ҫ
 *			  required:�Ƿ�ǿ�Ʒ��� 1��ʾǿ�Ʒ��� 0��ʾֻ��״̬���˲ŷ��� 
 *			env:ǩ������,fdΪ��ʱ��������
 *			enc:�����㷨
 *����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/

int send_dev_state(int fd,int ack,int required, int env, int enc,int dev_no)

{
	DWORD send_buf[25];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	struct ip1004_state_struct * gtstate;
	int rc;
	int i;
	DWORD *per_state;
	DWORD *dev_state;
	DWORD *alarmin_state;
	struct sockaddr_in peeraddr;
	int link_err;	
	int addrlen=sizeof(struct sockaddr);	
	
	//���û��ע��ɹ�,��fd<0�򲻷���״̬
	if((!get_regist_flag(dev_no))&&(fd<0))
	{	
		return 0;
	}	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_STATE_RETURN;
	cmd->en_ack=ack;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc=virdev_get_devid(dev_no,cmd->para);

	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para) -2+4+4+4;
	gtstate=get_ip1004_state(dev_no);
	
	pthread_mutex_lock(&gtstate->mutex);
	memcpy(&cmd->para[rc],(char *)&gtstate->reg_per_state,4);
	link_err=gtstate->reg_dev_state.link_err;
	gtstate->reg_dev_state.link_err=0;
//	gtstate->reg_dev_state.cf_err=0;//test!!!
	memcpy(&cmd->para[rc+4],(char *)&gtstate->reg_dev_state,4);
	gtstate->reg_dev_state.link_err=link_err;//���ӹ��ϲ�Ӧ���͸�����
	memcpy(&cmd->para[rc+8],(char *)&gtstate->alarmin_state,4);
	pthread_mutex_unlock(&gtstate->mutex);
	per_state=(DWORD*)&cmd->para[rc];
	dev_state=(DWORD*)&cmd->para[rc+4];
	alarmin_state = (DWORD*)&cmd->para[rc+8];

	//gtlogdebug("%d,%d,%08x,%08x,%08x,%08x,%d,%d\n",ack,required,dev_state_old,*dev_state,per_state_old,*per_state,(dev_state_old!=*dev_state),(per_state_old!=*per_state));

	
	if((required==1)||(dev_state_old[dev_no]!=*dev_state)||(per_state_old[dev_no]!=*per_state)||(alarmin_state_old[dev_no]!= *alarmin_state))
	{
		if(ack==1)//����ҪACK,�����־λ
			set_reportstate_flag(dev_no,0);

#ifdef SHOW_WORK_INFO
		printf("%s��Զ�˷���������״̬%08x,%08x,%08x\n",devlog(dev_no),(int)*per_state,(int)*dev_state,(int)*alarmin_state);
#endif
		gtloginfo("%s��Զ�˷���������״̬%08x,%08x,%08x\n",devlog(dev_no),(int)*per_state,(int)*dev_state,(int)*alarmin_state);
		rc=send_gate_pkt(fd,modcom,cmd->len+2,dev_no);

	}
	else
		rc=0;
		 
	dev_state_old[dev_no]=*dev_state;
	per_state_old[dev_no]=*per_state;
	alarmin_state_old[dev_no] = *alarmin_state;
	return rc;
}


/**********************************************************************************************
 * ������	:send_dev_req_sync_time()
 * ����	:���������ʱ�������
 * ����	:fd:�Ѿ�������Ŀ��socket
 *		env:ǩ��
 *		enc:����
 *����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/

int send_dev_req_sync_time(int fd,int env,int enc,int dev_no)
{	
	DWORD send_buf[25];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	int rc;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_REQ_SYNC_TIME;
	cmd->en_ack=1;
	cmd->reserve0=0;
	cmd->reserve1=0;

	rc=virdev_get_devid(dev_no,cmd->para);
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2;//+sizeof(struct send_dev_trig_state_struct);
	gtloginfo("%s����ʱ��ͬ������!\n",devlog(dev_no));
	return send_gate_pkt(fd,modcom,cmd->len+2,dev_no);			
}

/**********************************************************************************************
 * ������	:send_dev_trig_state()
 * ����	:�����豸������Ϣ��Զ�̷�����
 * ����	:fd:socket���������� >0��ʾ�Ѿ����������� -1��ʾ��Ҫ��������
 *				���߳̽���һ������,-2��ʾ������Ӳ�����������������
 *				��������������
 *			  dev_rtig:������Ϣ�ṹ
 *			  ack:�Ƿ���Ҫȷ�� 1��ʾ��Ҫ 0��ʾ����Ҫ
 *				env:ǩ��
 *				enc:���ܣ���������fd>0ʱ������
 *����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int send_dev_trig_state(int fd,struct send_dev_trig_state_struct *dev_trig,int ack, int env, int enc,int dev_no)
{	
	DWORD send_buf[25];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	int rc;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_ALARM_RETURN;
	cmd->en_ack=ack;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc=virdev_get_devid(dev_no,cmd->para);

	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+sizeof(struct send_dev_trig_state_struct);
	memcpy(&cmd->para[rc],dev_trig,sizeof(struct send_dev_trig_state_struct));
	
	gtloginfo("%s���ͱ���״̬0x%04x,ʱ��%04d-%02d-%02d-%02d-%02d-%02d������\n",devlog(dev_no),(int)dev_trig->alarmstate,dev_trig->year,dev_trig->month,dev_trig->day,dev_trig->hour,dev_trig->minute,dev_trig->second);
	memcpy(&last_dev_trig,dev_trig,sizeof(struct send_dev_trig_state_struct));
	//if(dev_no == 0)
	//	set_state_led_state(2);
	if(ack==1)//����ҪACK,�����־λ
		set_alarm_flag(dev_no,0);
	return send_gate_pkt(fd,modcom,cmd->len+2,dev_no);	

}

/**********************************************************************************************
 * ������	:send_gate_ack()
 * ����	:�����ط���һ���������Ӧ
 * ����	:fd:�Ѿ�������Ŀ��socket
 *			 rec_cmd:Ҫ��Ӧ������
 *			 result:ִ������Ľ����
 *				env:ǩ��
 *				enc:���ܣ���������fd>0ʱ������
 *����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
int send_gate_ack(int fd,WORD rec_cmd,WORD result,int env, int enc,int dev_no)
{
	DWORD send_buf[25];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	int rc;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_CMD_ACK;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc= virdev_get_devid(dev_no, cmd->para);
	memcpy(&cmd->para[rc],(char *)&result,2);
	memcpy(&cmd->para[rc+2],(char *)&rec_cmd,2);
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para) -2+4;
	send_gate_pkt(fd,modcom,cmd->len+2,dev_no);

	gtloginfo("%s��%s��������%04x�Ľ��%04x",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),rec_cmd,result);


	return 0;
}


//͸�����ڿ��Ƶ������
static int send_gate_serial_return(int fd,WORD ch,WORD result,WORD port, int env,int enc, int dev_no)
{
	DWORD send_buf[25];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	struct dev_com_ret_struct *serialrt;
	int rc;
	
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_COM_PORT_RET;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc=virdev_get_devid(dev_no,cmd->para);
	serialrt=(struct dev_com_ret_struct *)((char *)cmd->para+rc);
	serialrt->ch=ch;
	serialrt->result=result;
	serialrt->port=port;
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_DEV_COM_RET_STRUCT;
	send_gate_pkt(fd,modcom,cmd->len+2,dev_no);

#ifdef SHOW_WORK_INFO
	printf("%s��Զ�˷���������͸�����ڿ��������%d\n",devlog(dev_no),result);
#endif
	gtloginfo("%s��Զ�˷���������͸�����ڿ��������%d\n",devlog(dev_no),result);
	return 0;
}

static int process_serial_ctrl(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no)

{
	struct usr_serial_ctl_struct *serial_cmd;
	struct trans_com_struct *com;
	struct ipmain_para_struct *main_para;
	WORD com_port;
	//struct in_add *addr;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if(cmd->cmd!=USR_SEIRAL_PORT_CONTROL)
		return -1;
#ifdef SHOW_WORK_INFO
	printf("vsmain dev %d recv  USR_SEIRAL_PORT_CONTROL cmd!\n",dev_no);
#endif
	/*if(cmd->len!=(6+sizeof(struct usr_serial_ctl_struct)))
	{
		printf("vsmain recv USR_SEIRAL_PORT_CONTROL cmd len err %d!=%d",cmd->len,(6+SIZEOF_USR_SERIAL_CTL_STRUCT));
				return -1;
	}*/
	main_para=get_mainpara();
	serial_cmd=(struct usr_serial_ctl_struct *)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����%s���пڿ���0x0218,���ں�%d,ģʽ%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),serial_cmd->ch,serial_cmd->mode);
#endif
	gtloginfo("%s(fd=%d)����%s���пڿ���0x0218,���ں�%d,ģʽ%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),serial_cmd->ch,serial_cmd->mode);
	if((com==NULL)||((serial_cmd->ch+1)>virdev_get_total_com(dev_no)))
	{
		//������Χ
	#ifdef SHOW_WORK_INFO
		printf("%s����:%d������Χ\n",devlog(dev_no),serial_cmd->ch);
	#endif
		gtloginfo("%s����:%d������Χ\n",devlog(dev_no),serial_cmd->ch);
		send_gate_serial_return(fd,serial_cmd->ch,ERR_ENC_NOT_ALLOW,0,env,enc,dev_no);

		return 0;
	}
	/*
	if(virdev_get_virdev_number()==2)//�������豸
	{
		if(dev_no == 0)
			serial_cmd->ch = 0;
		if(dev_no == 1)
			serial_cmd->ch = 1;
	}
	*/
	com=get_trans_com_info(serial_cmd->ch);
	if(serial_cmd->ch==0)
	{
		com_port=main_para->com0_port;
	}
	else
	{
		com_port=main_para->com1_port;
	}
	pthread_mutex_lock(&com->mutex);

	if(serial_cmd->mode==0)
	{
		//ȡ������
	#ifdef SHOW_WORK_INFO
		printf("����ȡ������\n");
	#endif
		gtloginfo("����ȡ������\n");
		send_gate_serial_return(fd,serial_cmd->ch,RESULT_SUCCESS,0,env,enc,dev_no);

		memset((char *)(&com->allow_addr.sin_addr.s_addr),0,sizeof(struct in_addr));
		com->flag = 0;	
		pthread_mutex_unlock(&com->mutex);
	
		return  0;
	}
	if(com->flag!=0)
	{
		//����ʹ��
	#ifdef SHOW_WORK_INFO
		printf("��������ʹ��\n");
	#endif
		gtloginfo("��������ʹ��\n");
		send_gate_serial_return(fd,serial_cmd->ch,ERR_DVC_BUSY,0,env,enc,dev_no);
		pthread_mutex_unlock(&com->mutex);
		return 0;
	}


	serial_cmd->remoteip=htonl(serial_cmd->remoteip);
	memcpy((char *)(&com->allow_addr.sin_addr.s_addr),(char *)(&serial_cmd->remoteip),sizeof(struct in_addr));
	com->baud=serial_cmd->baud;
	com->databit=serial_cmd->databit;
	com->parity=serial_cmd->parity;
	com->stopbit=serial_cmd->stopbit;
	com->flow=serial_cmd->flow;
#ifdef SHOW_WORK_INFO
	printf("%s will connect com%d at port %d\n",inet_ntoa(com->allow_addr.sin_addr),serial_cmd->ch,com_port);
#endif
	send_gate_serial_return(fd,serial_cmd->ch,RESULT_SUCCESS,com_port,env,enc,dev_no);

	pthread_mutex_unlock(&com->mutex);
	return 0;
}

/**********************************************************************************************
 * ������	:bypass_gatecmd2mod()
 * ����	:���������յ�������ת��������ģ��
 * ����	:fd:�յ����������������
 *			cmd:�յ������ط������������
 *			target:Ŀ��ģ��ĵ�ַ
 *			env:
 *			enc:�����㷨
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
static int bypass_gatecmd2mod(int fd,struct gt_usr_cmd_struct *cmd,long int target,int env, int enc, int dev_no)
{
	mod_socket_cmd_type *send;
	

	//struct gt_usr_cmd_struct *bypass;
	int rc;
	BYTE sendbuf[300];
/*	if(cmd->cmd!=USR_REQUIRE_RT_IMAGE)
		return -1;
*/
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if(fd<0)
		return -1;
	if((cmd->len+30)>(sizeof(sendbuf)))
	{
		showbug();
	}

	send=(mod_socket_cmd_type *)sendbuf;
	send->gate.env = env;
	send->gate.enc = enc;
	send->gate.gatefd = fd;
	send->cmd = GATE_BYPASSTO_MOD_CMD;
	send->gate.dev_no = dev_no;
	send->len = cmd->len+2;
	memcpy((BYTE *)&send->para,(BYTE *)cmd,send->len);	
	
	rc=main_send_cmd(send,target,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
	
#ifdef		SHOW_WORK_INFO
	printf("%s ��������%s���յ�������ת��cmd=0x%04x to ģ��%d fd=%d,rc=%d\n",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),(int)cmd->cmd,(int)target,(int)fd,rc);
#endif
	gtloginfo("%s ��������%s���յ�������ת��cmd=0x%04x to ģ��%d fd=%d,rc=%d\n",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),(int)cmd->cmd,(int)target,(int)fd,rc);
	
	if(rc<0)
	{
		printf("ipmain dev %d bypass a cmd Error:%s\n\a",dev_no,strerror(errno)); 
		//gtlogerr("ת������ʧ��%s\n",strerror(errno));
	}	
	else 
	{
		printf("ipmain dev %d bypass a cmd succeed\n\a",dev_no,strerror(errno));
		gtloginfo("ת������ɹ�\n");
	}
	return rc;
	
}



//��������ip��ַ
static int usr_set_gate_ip(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	struct usr_set_auth_ip_struct *gatecmd;
	struct in_addr addr;
	DWORD rmt;
	WORD	result;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	
	if((fd<0)||(cmd->cmd!=USR_SET_AUTH_IP))
		return -1;
	gatecmd=(struct usr_set_auth_ip_struct*)cmd->para;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
//	
	printf("%s(fd=%d)��������%s����ip��ַ����0x0101\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
	gtloginfo("%s(fd=%d)��������%s����ip��ַ����0x0101\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));

	if(set_gate_ip(dev_no,gatecmd->server_sn,&gatecmd->ip,REMOTE_GATE_CMD_PORT)<0)
	{
		result=ERR_ENC_NOT_ALLOW;
	}
	else
	{
		result=RESULT_SUCCESS;
		if(gatecmd->save_flag)
		{
			//����flash
			rmt=htonl(gatecmd->ip);
			memcpy(&addr,&rmt,4);
			switch(gatecmd->server_sn)
			{
				case 1:
				{
					if(dev_no == 0)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate:rmt_gate1",inet_ntoa(addr));	
					if(dev_no == 1)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate_dev2:rmt_gate1",inet_ntoa(addr));	
				}
				break;
				case 2:
				{
					if(dev_no == 0)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate:rmt_gate2",inet_ntoa(addr));	
					if(dev_no == 1)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate_dev2:rmt_gate2",inet_ntoa(addr));	
				}
				break;
				case 3:
				{
					if(dev_no == 0)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate:rmt_gate3",inet_ntoa(addr));	
					if(dev_no == 1)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate_dev2:rmt_gate3",inet_ntoa(addr));	
				}
				break;
				case 4:
				{
					if(dev_no == 0)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate:rmt_gate4",inet_ntoa(addr));	
					if(dev_no == 1)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate_dev2:rmt_gate4",inet_ntoa(addr));	
				}
				break;
				case 5:
				{
					if(dev_no == 0)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate:alarm_gate",inet_ntoa(addr));	
					if(dev_no == 1)
						save2para_file(IPMAIN_PARA_FILE,"remote_gate_dev2:alarm_gate",inet_ntoa(addr));	
				}
				break;
				
			}
		}
	}
	//if(virdev_get_virdev_number()==2)//�����豸
	//	system("/ip1004/ini_conv -s");
	gtloginfo("%s����ip��ַ������ϣ����Ϊ%d\n",devlog(dev_no),result);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SET_AUTH_IP, result,env,enc,dev_no);
	return 0;

}

static int usr_set_clock(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{	//
	WORD	result;
	struct usr_set_time_struct *timeval;
	//time_t systime;
	struct tm tmt;
	//int inttime;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	
	if((fd<0)||(cmd->cmd!=USR_CLOCK_SETING))
		return -1;
	//date MMDDHHMMYYYY.SS	//����ʱ��
	//clock -w��ϵͳʱ��д�� CMOS ��
	timeval=(struct usr_set_time_struct*)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("vsmain recv a set time cmd new time:%d-%02d-%02d %02d:%02d:%02d\n",
		timeval->year,timeval->month,timeval->day,timeval->hour,timeval->minute,timeval->second);
#endif	

	tmt.tm_year=timeval->year-1900;
	tmt.tm_mon=timeval->month-1;
	tmt.tm_mday=timeval->day;
	tmt.tm_hour=timeval->hour;
	tmt.tm_min=timeval->minute;
	tmt.tm_sec=timeval->second;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	gtloginfo("%s(fd=%d)����ʱ��0x0200��ʱ��%d-%d-%d-%d-%d-%d\n",inet_ntoa(peeraddr.sin_addr),fd,timeval->year,timeval->month,timeval->day,timeval->hour,timeval->minute,timeval->second);

	//����rtcʱ��
	if(rmt_set_time(&tmt)<0)
	{
#ifdef SHOW_WORK_INFO	
		printf("ipmain recv a bad time\n");
#endif
		result=ERR_DVC_INVALID_REQ;
		cmd->en_ack=1;
	}
	else
	{
		result=RESULT_SUCCESS;
	}
	
#ifdef SHOW_WORK_INFO
	printf("usr_set_clock result is d\n",result);
#endif
	gtloginfo("����ʱ����ϣ����Ϊ%d\n",result);

	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_CLOCK_SETING, result,env,enc,dev_no);	
	return 0;
}


//���ô������������
static int usr_set_trigger_in(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{//
	WORD	result;
	//struct ipmain_para_struct * para;
	BYTE en_ack;
	struct trig_in_attrib_struct *attrib;
	int i,ch;
	//char entry[30],vstr[30];
	//DWORD *act;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	//dictionary *ini;	
	//FILE *fp;
	if((fd<0)||(cmd->cmd!=USR_SET_SWITCH_IN))
		return -1;

	attrib=(struct trig_in_attrib_struct*)cmd->para;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO	
	printf("%s(fd=%d)����%s������������������0x0213,ͨ��ģʽ%x,����0%x,1%x,2%x,3%x\n",inet_ntoa(peeraddr.sin_addr),(int)fd,devlog(dev_no),(int)attrib->ch_mode,(int)attrib->action0,(int)attrib->action1,(int)attrib->action2,(int)attrib->action3);
#endif
	gtloginfo("%s(fd=%d)����%s������������������0x0213,ͨ��ģʽ%x,����0%x,1%x,2%x,3%x\n",inet_ntoa(peeraddr.sin_addr),(int)fd,devlog(dev_no),(int)attrib->ch_mode,(int)attrib->action0,(int)attrib->action1,(int)attrib->action2,(int)attrib->action3);
	
	if(virdev_get_trigin_num(dev_no)==0)
	{
		result=ERR_ENC_NOT_ALLOW;
	}
	//lc do
#ifdef USE_IO	
	if(set_trigin_attrib_perbit((DWORD)(attrib->ch_mode))<0)
		result=ERR_ENC_NOT_ALLOW;
	else
	{
		en_ack=cmd->en_ack;
		
//#ifdef HQMODULE_USE
		//cmd->en_ack=0;
		//bypass_hqsave_cmd(fd,cmd);	//ת����������¼��ģ��
//#endif
		cmd->en_ack=en_ack;
		ch=attrib->ch_mode&0xff;

	if((ch)<get_trigin_num())
		{
			result=RESULT_SUCCESS;
			if(attrib->save_flag!=0)
			{//dont save for now,lc
			/*
				ini=iniparser_load(IPMAIN_PARA_FILE);
				if(ini==NULL)
				{
					//retflag=-3;
				}
				else
				{				
					para=get_mainpara();
					act=&attrib->action0;
					for(i=0;i<MAX_TRIG_EVENT;i++)
					{					
						sprintf(entry,"alarm:tin%dact%d",(attrib->ch_mode&0xff),i);
						para->tinact[ch][i]=*act;
						sprintf(vstr,"%08x",htonl(*act));
						save2para(ini,entry,vstr);
					}	
					fp=fopen(IPMAIN_PARA_FILE,"w");
					if(fp!=NULL)
					{
						iniparser_dump_ini(ini,fp);
						fclose(fp);
					}
					iniparser_freedict(ini);
					
				}	*/
			}


		}
		else
			result=ERR_ENC_NOT_ALLOW;
	}
#endif	
	gtloginfo("%s���ô�����������Խ��Ϊ%d\n",devlog(dev_no),result);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SET_SWITCH_IN, result,env,enc,dev_no);	
	return 0;
}
//���ñ������������
static int usr_set_alarm_out(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{//
	WORD	result=0;
	WORD	*attrib;
	WORD	*saveflag;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_SET_SWITCH_OUT))
		return -1;
	saveflag=(WORD*)cmd->para;
	attrib=(WORD*)&cmd->para[2];
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO
	printf("ipmain recv a set_alarm_attrib cmd new attrib is:%04x\n",*attrib);
#endif
	gtloginfo("%s(fd=%d)����%s���ñ������������0x0214,����%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),*attrib);
	//if(set_alarm_out_attrib(*attrib,*saveflag)<0)
	if(virdev_get_alarmout_num(dev_no)==0)
	{
		result=ERR_ENC_NOT_ALLOW;
	}
	gtloginfo("�������%s������������Խ��Ϊ%d\n",devlog(dev_no),result);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SET_SWITCH_OUT, result,env,enc,dev_no);	
	return 0;
}
//���Ʊ��������
static int usr_alarm_ctl(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
//
	WORD	result;
	WORD	*output;
	WORD	*saveflag;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_SWITCH_OUT))
		return -1;
	saveflag=(WORD*)cmd->para;
	output=(WORD*)&cmd->para[2];
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO
	printf("vsmain recv a usr_alarm_ctl cmd to set output%d to %d\n",(*output&0xff),(*output>>8&0xff));
#endif
	gtloginfo("%s(fd=%d)��������%s���Ʊ��������0x0215,%d �� %d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),(*output&0xff),(*output>>8&0xff));
	//if(set_alarm_state_bit((*output)>>8,*output&0xff)<0)
	if(virdev_get_alarmout_num(dev_no)==0)
	{
		result=ERR_ENC_NOT_ALLOW;
	}
	//lc do ��io��װ����ɺ�ʵ��
	if(set_relay_output(*output&0xff,(*output)>>8)<0)
		result=ERR_ENC_NOT_ALLOW;
	else
	{
		result=RESULT_SUCCESS;	//���ܴ洢		
	}
	
	result=RESULT_SUCCESS;
#ifdef SHOW_WORK_INFO
	printf("%s���ÿ��Ʊ�������ڽ��%d\n",devlog(dev_no),result);
#endif
	gtloginfo("%s���ÿ��Ʊ�������ڽ��%d\n",devlog(dev_no),result);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SWITCH_OUT, result,env,enc,dev_no);
	return 0;
}



static int usr_query_index_cmd(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no)
{//
	DWORD	buf[200];
	int i;
	mod_socket_cmd_type *modsocket;
	struct query_index_with_channel *qindexch;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	
	if((fd<0)||(cmd->cmd!=USR_QUERY_INDEX))
		return -1;	
	
	gtloginfo("%s(fd=%d)������ѯ%s¼����������0x0105\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
	modsocket = (mod_socket_cmd_type *)buf;
	qindexch = (struct query_index_with_channel *)modsocket->para;
	memcpy(&qindexch->queryindex,cmd->para,sizeof(struct query_index_struct));

	//lc todo 2014-2-11 ��ƽ̨Ӧ�ô��ݸ�ͨ����
	//qindexch->channel = dev_no; 
	
	modsocket->gate.gatefd	=	fd;
	modsocket->gate.env		=	env;
	modsocket->gate.enc		=	enc;
	modsocket->gate.dev_no	=	dev_no;
	modsocket->cmd			=	USR_QUERY_INDEX;
	modsocket->len			=	sizeof(struct query_index_with_channel);
	
	return main_send_cmd(modsocket,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);
	
}


static int usr_query_state_cmd(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no)
{//
	time_t rtime,curtime;
	struct tm tm,*pctime=NULL;
	int inttime;
	struct gt_time_struct *rmttime;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if((fd<0)||(cmd->cmd!=USR_QUERY_STATE))
		return -1;	
	rmttime=(struct gt_time_struct *)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)������ѯ%s�豸״̬����0x0103\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
#endif
	gtloginfo("%s(fd=%d)������ѯ%s�豸״̬����0x0103\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
	gttime2tm(rmttime,&tm);

	rtime = mktime(&tm);
	inttime=rtime;
	if(inttime>0)
	{
		curtime=time(NULL);
		pctime=localtime(&curtime);	
		if(pctime!=NULL)
		{
			if((pctime->tm_year+1900)<2005)
			{//����豸��ǰʱ��С��2005��ʾ���ܳ�����Ҫ��ʱ
				gtloginfo("ʹ��Զ�̶�ʱ :\n");
				gtloginfo("oldtime:%d%d%d-%d:%d:%d\n",(pctime->tm_year+1900),
													    (pctime->tm_mon+1),
													    (pctime->tm_mday),
													    (pctime->tm_hour),
													    pctime->tm_min,
													    pctime->tm_sec);
				
				gtloginfo("newtime:%d%d%d-%d:%d:%d\n",(pctime->tm_year+1900),
													    (pctime->tm_mon+1),
													    (pctime->tm_mday),
													    (pctime->tm_hour),
													    pctime->tm_min,
													    pctime->tm_sec);
													    
				//lc do �Զ�Ҫ���ʱ
				rmt_set_time(&tm);
			}
		}

	}
	//gtloginfo("����ѯ״̬�¼�\n");
	send_dev_state(fd,0,1,env,enc,dev_no);	
	return 0;
}


static int usr_set_net_avstream(int fd,struct gt_usr_cmd_struct *cmd, int env,int enc, int dev_no)
{//
	struct usr_net_avstream_set *stream;
	int val;
	int i;
	struct sockaddr_in peeraddr;
	dictionary      *ini;	
	FILE *fp;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_NET_STREAM_SETING))
		return -1;
	stream=(struct usr_net_avstream_set *)cmd->para;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)��������%s���紫��ͼ����������0x0205,ͼ����%d,����%d,����%d,��Ƶ���뷽ʽ%d,��Ƶ������%d,��Ƶת������%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),stream->picsize,stream->vbitrate,stream->frame,stream->aud_mode,stream->aud_samrate,stream->aud_sambit);
#endif
	gtloginfo("%s(fd=%d)��������%s���紫��ͼ����������0x0205,ͼ����%d,����%d,����%d,��Ƶ���뷽ʽ%d,��Ƶ������%d,��Ƶת������%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),stream->picsize,stream->vbitrate,stream->frame,stream->aud_mode,stream->aud_samrate,stream->aud_sambit);	
	if(stream->save_flag!=0)
	{
	
		switch(stream->picsize)
		{
			case 1://D1
				val=0;
			break;
			case 2://CIF
				val=1;
			break;
			default:
				val=-1;
			break;
		}
		ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
		//ini=iniparser_load(IPMAIN_PARA_FILE);
		if(ini==NULL)
		{
			//retflag=-3;
		}
		else
		{	
			if(dev_no == 0)
			{
				if(val>=0)
				{
					iniparser_setint(ini,"netencoder:vidpicsize",val);
				}
				if(stream->vbitrate!=0)
				{
					iniparser_setint(ini,"netencoder:TargetBitRate",stream->vbitrate);
				}
				if(stream->frame<5)
				{
					iniparser_setint(ini,"netencoder:FrameRate",stream->frame);
				}
				if((stream->aud_mode>0)&&(stream->aud_mode<3))
				{
					iniparser_setint(ini,"netencoder:AudEncMode",stream->aud_mode);
				}
			}
			/*
			else
			{
				if(val>=0)
				{
					iniparser_setint(ini,"hqenc0:vidpicsize",val);
				}
				if(stream->vbitrate!=0)
				{
					iniparser_setint(ini,"hqenc0:TargetBitRate",stream->vbitrate);
				}
				if(stream->frame<5)
				{
					iniparser_setint(ini,"hqenc0:FrameRate",stream->frame);
				}
				if((stream->aud_mode>0)&&(stream->aud_mode<3))
				{
					iniparser_setint(ini,"hqenc0:AudEncMode",stream->aud_mode);
				}
			}
			*/
			save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
			//fp=fopen(IPMAIN_PARA_FILE,"w");
			iniparser_freedict(ini);
		}
		//stream->aud_sambit;//������������ʱ��Ч
		//stream->aud_samrate;
	}
	//bypass_rtimg_cmd(fd,cmd);	//��ͼ��ģ�鴦��ʵ�ʹ���
	//if(virdev_get_virdev_number()==2)
	//	system("/ip1004/ini_conv -s");
	bypass_videoenc_cmd(fd, cmd,env,enc,dev_no);//������videoencģ�鴦��ʵ�ʹ���
	return 0;
}



#ifdef QUAD_CTRL


static int usr_net_scr_ctl(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{	
	struct scr_ctrl_struct *scr;
	WORD result;
	int retflag;
	
	int i;
	int ch;
	char buf[12];
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	
	if((fd<0)||(cmd->cmd!=USR_SCREEN_SETING))
		return -1;
	scr=(struct scr_ctrl_struct*)cmd->para;
	retflag=0;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if(get_quad_flag()==0)
		{
			gtloginfo("%s(fd=%d) �����л�%s������Ƶ�˿�����,���豸����Ƶ�ָ���\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
			if(cmd->en_ack!=0)
				return send_gate_ack(fd,USR_SCREEN_SETING,ERR_EVC_NOT_SUPPORT,env,enc,dev_no);
			else				
				return 0;	
		}
	if(scr->disp_type==SCR_FULL)
	{
#ifdef SHOW_WORK_INFO
		printf("%s(fd=%d) �����л�%s������Ƶ�˿�����,Ҫ���л�ͨ��%dΪȫ��ģʽ\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),scr->ch);
#endif
		gtloginfo("%s(fd=%d) �����л�%s������Ƶ�˿�����,Ҫ���л�ͨ��%dΪȫ��ģʽ\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),scr->ch);
		
		//if(get_mainpara()->vadc.quad.current_net_ch != scr->ch)
		{
		
			if(scr->ch<4)
			{
				retflag=set_net_scr_full(scr->ch,(struct sockaddr *)&peeraddr,NULL);
			}
			else
				retflag=-2;
		}
		
	}
	else if(scr->disp_type==SCR_QUAD)
	{
#ifdef SHOW_WORK_INFO
		printf("%s(fd=%d) �����л�������Ƶ�˿�����0x0204,Ҫ���л�Ϊ4����ģʽ\n",inet_ntoa(peeraddr.sin_addr),fd);
#endif
		gtloginfo("%s(fd=%d) �����л�������Ƶ�˿�����0x0204,Ҫ���л�Ϊ4����ģʽ\n",inet_ntoa(peeraddr.sin_addr),fd);
		//if(get_mainpara()->vadc.quad.current_net_ch != 4)
		//{
			retflag=set_net_scr_quad((struct sockaddr *)&peeraddr,NULL);
		//}
		
	}
	else
		retflag=-2;
	if(retflag>=0)
	{
		if(scr->save_flag)
		{
			if(scr->disp_type==SCR_QUAD)
				ch=4;
			else
				ch=scr->ch;
			sprintf(buf,"%d",ch);		
			save2para_file(IPMAIN_PARA_FILE,"netencoder:net_ch",buf);
			//if(virdev_get_virdev_number()==2)
			//	system("/ip1004/ini_conv -s");		
		}
		
	}	
	if(cmd->en_ack!=0)
	{
		if(retflag>=0)
			result=RESULT_SUCCESS;
		else if(retflag==-2)
			result=ERR_ENC_NOT_ALLOW;
		else
			result=ERR_DVC_INTERNAL;
		gtloginfo("����л�������Ƶ�˿�,���Ϊ:%d\n",retflag);
		return send_gate_ack(fd,USR_SCREEN_SETING, result,env,enc,dev_no);
	}
	gtloginfo("����л�������Ƶ�˿�,���Ϊ%d\n",retflag);
	return 0;
	
}
//�����˶����������Чʱ��
static int usr_set_senser_array(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{

	WORD result=0;
	int ret;
	int ch,sen;
	WORD *area;
	int i;
	dictionary      *ini;	
	FILE *fp;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	struct usr_set_motion_para_struct* motion_para;
	struct ipmain_para_struct *main_para;
	struct quad_dev_struct *quad;
	struct ip1004_state_struct *ip1004state;
	struct motion_struct *motion[MAX_VIDEO_IN];
	main_para=get_mainpara();	
	ip1004state=get_ip1004_state(dev_no);
	quad=(struct quad_dev_struct *)(&main_para->vadc.quad);
	for(i=0;i<get_video_num();i++)
		motion[i]=(struct motion_struct *)&quad->motion[i];
	
	if((fd<0)||(cmd->cmd!=USR_SET_SENSER_ARRAY))
		return -1;	
	motion_para=(struct usr_set_motion_para_struct*)cmd->para;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)���������˶�����������0x0208,������%d,ʱ��%d%d-%d%d,����%d\n",inet_ntoa(peeraddr.sin_addr),fd,motion_para->sen,motion_para->beginhour,motion_para->beginmin,motion_para->endhour,motion_para->endmin,motion_para->alarm);
#endif
	gtloginfo("%s(fd=%d)���������˶�����������0x0208,������%d,ʱ��%d%d-%d%d,����%d\n",inet_ntoa(peeraddr.sin_addr),fd,motion_para->sen,motion_para->beginhour,motion_para->beginmin,motion_para->endhour,motion_para->endmin,motion_para->alarm);
	if(motion_para->channel<get_video_num())//ֻ��4·
	{
		ch=motion_para->channel;
		sen=motion_para->sen;
		area=(WORD *)motion_para->area;
        //printf("area[1]=%d,begin at %d:%d\n",area[1],motion_para->beginhour,motion_para->beginmin);

		//lc do �����ƶ���ⱨ����������3520
		ret=set_motion_vda_sen(ch,sen,area);	
					
		motion[ch]->starthour= motion_para->beginhour;
		motion[ch]->startmin= motion_para->beginmin;
		motion[ch]->endhour= motion_para->endhour;
		motion[ch]->endmin= motion_para->endmin;
		motion[ch]->alarm=motion_para->alarm;
	
		if((motion_para->save_flag)!=0)//�浽�����ļ�
		{
			//ini=iniparser_load(IPMAIN_PARA_FILE);
			ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
			if(ini==NULL)
			{
				result=ERR_DVC_INTERNAL;	
			}
			else
			{
			
				save_video_para(ini,ch,"motion_sen",motion_para->sen);
				save_video_para(ini,ch,"motion_alarm",motion_para->alarm);
				save_video_para_hex(ini,ch,"area0",motion_para->area[0]);
				save_video_para_hex(ini,ch,"area1",motion_para->area[1]);	
				save_video_para_hex(ini,ch,"area2",motion_para->area[2]);
				save_video_para_hex(ini,ch,"area3",motion_para->area[3]);
				save_video_para_hex(ini,ch,"area4",motion_para->area[4]);
				save_video_para_hex(ini,ch,"area5",motion_para->area[5]);
				save_video_para_hex(ini,ch,"area6",motion_para->area[6]);
				save_video_para_hex(ini,ch,"area7",motion_para->area[7]);
				save_video_para_hex(ini,ch,"area8",motion_para->area[8]);
				save_video_para_hex(ini,ch,"area9",motion_para->area[9]);
				save_video_para_hex(ini,ch,"area10",motion_para->area[10]);
				save_video_para_hex(ini,ch,"area11",motion_para->area[11]);
				save_video_para(ini,ch,"starthour",motion_para->beginhour);
				save_video_para(ini,ch,"startmin",motion_para->beginmin);
				save_video_para(ini,ch,"endhour",motion_para->endhour);
				save_video_para(ini,ch,"endmin",motion_para->endmin);

				save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);//changed by shixin
				iniparser_freedict(ini);
			}
			
		}
	}
	else
		result=ERR_ENC_NOT_ALLOW;
#ifdef SHOW_WORK_INFO
	printf("�������˶������������Ϊ0x%04x\n",result);
#endif
	gtloginfo("�������˶������������Ϊ0x%04x\n",result);
	//if(virdev_get_virdev_number()==2)
	//	system("/ip1004/ini_conv -s");
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SET_SENSER_ARRAY,result,env,enc,dev_no);
		
	return 0;
}

//�������ȣ�ɫ�ȵȲ���
static int usr_set_video_para(int fd,struct gt_usr_cmd_struct *cmd,  int env, int enc,int dev_no)
{
	WORD result;
	int retflag;
	FILE *fp=NULL;
	struct usr_video_para_struct *video_para;
	struct ipmain_para_struct *main_para;
	struct enc_front_struct *front;
	int i;
	struct sockaddr_in peeraddr;
	dictionary      *ini;	
	int addrlen=sizeof(struct sockaddr);
	
	int ch;
	if((fd<0)||(cmd->cmd!=USR_SET_VIDEO_AD_PARAM))
		return -1;	
	video_para=(struct usr_video_para_struct*)cmd->para;

	main_para=get_mainpara();	
	
	retflag=0;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����������Ƶ�����������0x0209.ͨ��%d,����%d,ɫ��%d,�Աȶ�%d,���Ͷ�%d\n",inet_ntoa(peeraddr.sin_addr),fd,video_para->channel,video_para->bright,video_para->hue,video_para->contrast,video_para->saturation);
#endif
	gtloginfo("%s(fd=%d)����������Ƶ�����������0x0209.ͨ��%d,����%d,ɫ��%d,�Աȶ�%d,���Ͷ�%d\n",inet_ntoa(peeraddr.sin_addr),fd,video_para->channel,video_para->bright,video_para->hue,video_para->contrast,video_para->saturation);
	//gtloginfo("��Ƶ�ָ���%d\n",get_quad_flag());
	if(video_para->channel<virdev_get_video_num(dev_no))
	{
		ch=video_para->channel;
		front=&main_para->vadc.enc_front[ch];
		
		if(video_para->save_flag!=0)//����б�Ҫ���ȴ�������ļ�
		{
			ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
			//ini=iniparser_load(IPMAIN_PARA_FILE);
			if(ini==NULL)
			{
				retflag=-3;
			}
			else
			{
				save_video_para(ini,ch,"bright",video_para->bright);
				save_video_para(ini,ch,"hue",video_para->hue);
				save_video_para(ini,ch,"contrast",video_para->contrast);
				save_video_para(ini,ch,"saturation",video_para->saturation);
				//��ini�ṹ����ָ���ļ�
				save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
				iniparser_freedict(ini);
			}
		}
		//if(virdev_get_virdev_number()==2)
		//	system("/ip1004/ini_conv -s");
		//gtloginfo("quad is %d\n",get_quad_flag());
			
		//���µ���ǰ����
		if(video_para->bright!=-1000)
		{
			front->bright=video_para->bright;
			//set_video_bright(ch,front->bright);
		}
		if(video_para->hue!=-1000)
		{
			front->hue=video_para->hue;
			//set_video_hue(ch,front->hue);
		}
		if(video_para->contrast!=-1000)
		{
			front->contrast=video_para->contrast;
			//set_video_contrast(ch,front->contrast);
		}
		if(video_para->saturation!=-1000)
		{
			front->saturation=video_para->saturation;
			//set_video_saturation(ch,front->saturation);
		}
			
		bypass_videoenc_cmd(fd, cmd,env,enc,dev_no);
		
	}
	else
	{
		retflag=-2;
		result=ERR_ENC_NOT_ALLOW;
	}
       if(retflag>=0)
            result=RESULT_SUCCESS;
	if(cmd->en_ack!=0)
	{
		if(retflag>=0)
			result=RESULT_SUCCESS;
		else if(retflag==-2)
			result=ERR_ENC_NOT_ALLOW;
		else
			result=ERR_DVC_INTERNAL;
#ifdef SHOW_WORK_INFO
		printf("%s���������Ƶ�������,���0x%x\n",devlog(dev_no),result);
#endif
		gtloginfo("%s���������Ƶ�������,���0x%x\n",devlog(dev_no),result);
		return send_gate_ack(fd,USR_SET_VIDEO_AD_PARAM, result,env,enc,dev_no);
	}
#ifdef SHOW_WORK_INFO
	printf("%s���������Ƶ�������,���0x%x\n",devlog(dev_no),result);
#endif
	gtloginfo("%s���������Ƶ�������,���0x%x\n",devlog(dev_no),result);
	return 0;
}


static int usr_local_avstream_set(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	
	struct usr_local_avstream_set *stream;
	WORD result=0;
	#if 0
	int val;
	dictionary      *ini;
	FILE *fp;
	#endif
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_LOCAL_STREAM_SETING))
		return -1;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	stream=(struct usr_local_avstream_set*)cmd->para;

	gtloginfo("%s(fd=%d)�����趨%s������¼����������,ͨ��%d,ͼ����%d,����%d,����%d,��Ƶ���뷽ʽ%d,��Ƶ������%d,��Ƶת������%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),stream->channel,stream->picsize,stream->vbitrate,stream->frame,stream->aud_mode,stream->aud_samrate,stream->aud_sambit);

#if 0
	if(stream->channel>(get_total_hqenc_num()))//Э���еĸ�����������1,2,3,4 //changed by shixin

	{
		result=ERR_EVC_NOT_SUPPORT;
	}
	else
	{
		//�ȴ浽�����ļ�
		if(stream->save_flag!=0)
		{
		
			switch(stream->picsize)
			{
				case 1:
					val=0;
				break;
				case 2:
					val=1;
				break;
				case 3:
					val=2;
				break;
				case 4:
					val=3;
				break;
				default:
					val=-1;
				break;
			}
			
			ini=iniparser_load_lockfile(VSMAIN_PARA_FILE,1,&fp);
			//ini=iniparser_load(VSMAIN_PARA_FILE);
			if(ini==NULL)
			{
				result=ERR_DVC_INTERNAL;
			}
			else
			{//FIXME ��·ʱ��Ҫ�޸�
				if(val>=0)
				{
					iniparser_setint(ini,"hqenc0:vidpicsize",val);
				}
				if(stream->vbitrate!=0)
				{
					iniparser_setint(ini,"hqenc0:TargetBitRate",stream->vbitrate);
				}
				if(stream->frame<5)
				{
					iniparser_setint(ini,"hqenc0:FrameRate",stream->frame);
				}
				if((stream->aud_mode>0)&&(stream->aud_mode<3))
				{
					iniparser_setint(ini,"hqenc0:AudEncMode",stream->aud_mode);
				}
				save_inidict_file(VSMAIN_PARA_FILE,ini,&fp);
				iniparser_freedict(ini);
			//stream->aud_sambit;//������������ʱ��Ч
			//stream->aud_samrate;
			}
		}
	}

		//ret=get_hdch(0);
		//read_enc_para_file(HDSAVE_PARA_FILE,"hqenc0",&hd->encoder);//��ȡ�����ļ�
		/*ret=refresh_hdmodule_para();
		ret=restart_hd_record(0);
		//ret=restart_recordfilethread(hd);//��·
		if(ret==0)
			result=RESULT_SUCCESS;
		else
			result=ERR_DVC_INTERNAL;*/

		//���֮ǰ���д�����
	if(result!=RESULT_SUCCESS)
	{
		gtloginfo("%s�趨������¼���������0x%x\n",devlog(dev_no),result);
		if(cmd->en_ack!=0)
			return send_gate_ack(fd,USR_LOCAL_STREAM_SETING, result,env,enc,dev_no);
		else
			return 0;
	}
	else//֮ǰ���޴���,ת����videoenc���ʵ������
	{
		bypass_videoenc_cmd(fd, cmd,env,enc,dev_no);
		return 0;
	}
#endif	
	gtloginfo("%s�趨������¼���������0x%x\n",devlog(dev_no),result);
		if(cmd->en_ack!=0)
			return send_gate_ack(fd,USR_LOCAL_STREAM_SETING, result,env,enc,dev_no);
		else
			return 0;
}






#endif



/**********************************************************************************************
 * ������	:remote_cancel_alarm()
 * ����	:����Զ�̷����Ľ������ָ��
 * ����	:trig:��λ��ʾ�Ĵ���״̬������λ����������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int remote_cancel_alarm(DWORD trig)
{
	//struct hd_enc_struct *hd;
	struct ipmain_para_struct *para;
	struct alarm_motion_struct *alarm_motion;
	int i,j,k,video_ch;
	struct ip1004_state_struct *ip1004state;
	DWORD temptrig=0;
	DWORD tempatrig=0;
	int event=0;
	int temptag=0;
	int count=0;
	unsigned int video_trig = 0;
	int cancel_ch[MAX_VIDEO_IN]={0};
	DWORD actual_trig=0;
	para=get_mainpara();
	alarm_motion=&para->alarm_motion;
	
	#if 0
	for(i=0;i<MAX_VIDEO+IN;i++)
	{
		hd=get_hdch(i);
		hd->trig=0;
    	hd->state=1;
	}
	#endif
	
	//�������ᵥ��������������
	//lc 2014-2-11
	if(para->multi_channel_enable)
	{
		pthread_mutex_lock(&pb_Tag.mutex);
		for(i=0;i<(get_trigin_num());i++)
		{
			k=(i<10)?i:i+16;
			if((trig&(1<<k))!=0)
			{
				temptag=0;
				pb_Tag.pb[i] = 0;
				video_ch=alarm_motion->alarm_trigin_video_ch[i];
				for(j=0;j<(get_trigin_num());j++)
				{
					if(pb_Tag.pb[j] == 1 && alarm_motion->alarm_trigin_video_ch[j]==video_ch)
					{
						temptag=1;
						break;
					}
				}
				if(temptag==0)
				{
					gtloginfo("��Ƶͨ��%d�ڸ�λ���������н���\n",video_ch);
					clear_hdmod_trig_flag(video_ch);
				}
				continue;
			}
		}
		pthread_mutex_unlock(&pb_Tag.mutex);

		for(i=0;i<(get_trigin_num());i++)
		{
			if(pb_Tag.pb[i]==1)
			  count++;
		}
		if(count==0)
		{
//lc do ���Զ�ȷ�ϻ�ȡ������ʱ�����ܵƻָ�����
#ifdef USE_LED
		  set_net_led_state(7);
		//set_alarm_led_state(0);
#endif
		if(para->buzzer_alarm_enable)
			kill_buz();
		}
	}
	else
	  clear_hdmod_trig_flag(0);
#if 0		
		for(i=0;i<(get_trigin_num());i++)
		{
			temptrig=trig&(1<<i);
			tempatrig=actual_trig&(1<<i);
			printf("temptrig is %d,actualtrig is %d\n",temptrig,tempatrig);
			video_ch = alarm_motion->alarm_trigin_video_ch[i];
			if((temptrig==0)&&(tempatrig!=0))
			{
				printf("0/1 cancel chn at video chn %d alarm ch %d is %d\n",video_ch,i,cancel_ch[video_ch]);
				cancel_ch[video_ch]=-1;
				printf("alarm %d at chn %d set to -1\n",i,video_ch);
				continue;
			}
			else if((temptrig==0)&&(tempatrig==0))
			{
				printf("0/0 cancel chn at video chn %d alarm ch %d is %d\n",video_ch,i,cancel_ch[video_ch]);
				if(cancel_ch[video_ch]<0)
				  continue;
			}
			else if((temptrig!=0)&&(tempatrig!=0))
			{
				
				printf("1/1 cancel chn at video chn %d alarm ch %d is %d\n",video_ch,i,cancel_ch[video_ch]);
				if(cancel_ch[video_ch]<0)
				  continue;
				printf("alarm %d at chn %d set to 1\n",i,video_ch);
				cancel_ch[video_ch]=1;
			}
			else if((temptrig!=0)&&(tempatrig==0))
			{
				printf("1/0 cancel chn at video chn %d alarm ch %d is %d\n",video_ch,i,cancel_ch[video_ch]);
				if(cancel_ch[video_ch]<0)
				  continue;
				printf("alarm %d at chn %d set to 1\n",i,video_ch);
				cancel_ch[video_ch]=1;
			}
			video_trig = video_trig|(1<<video_ch);
			//clear_hdmod_trig_flag(video_ch);
		}

		for(i=0;i<(get_video_num());i++)
		{
			if(cancel_ch[i]==1)
				clear_hdmod_trig_flag(i);
		}

		//clear_hdmod_trig_flag(trig);	
	}
	else
		clear_hdmod_trig_flag(0);	
#endif	

	//ִ����Щresetִ�еı�������
	for(i=0;i<(get_trigin_num());i++)
	{
		k=(i<10)?i:i+16;
		if((trig&(1<<k))==0)
			continue;
		for(j=0;j<MAX_TRIG_EVENT;j++)
		{
			event=get_alarm_event(i,2,j); 
#ifdef SHOW_WORK_INFO
			printf("��λʱeventΪ%d\n",event);
#endif
			take_alarm_action(i,event);
		}
	}


	/*��Ϊtrig�ĵ�λ10�ֽ��Ǹ�������reserve�ģ���bit10��ʼ����motion_alarmλ��
	  ���Դ�bit10��ʼ�Ƚ���ȷ������·�ƶ���ⱨ����
	 */
	for(i=10;i<((get_video_num())+10);i++)
	{
		if((trig&(1<<i))==0)
			continue;
		for(j=0;j<MAX_TRIG_EVENT;j++)
		{
			
			//��Ϊget_alarm_event������ƶ����Ŀǰ��6-9������Ӧ��ѭ������i��10-14������������i-4  
			event=get_alarm_event(i-4,2,j); 
			//printf("��λʱeventΪ%d\n",event);
			take_alarm_action(i,event);
		}
	}

	return 0;
}

int remote_cancel_alarm_playback(int trig)
{
	//lc 2013-11-04
	struct alarm_motion_struct *alarm_motion;
	struct ipmain_para_struct *para;
	int count = 0;
	int i,j,k;
	int new_ch,old_ch;

	para = get_mainpara();
	alarm_motion = &para->alarm_motion;
	
	if(para->multi_channel_enable == 0)
	{
		pthread_mutex_lock(&pb_Tag.mutex);
		para->alarm_playback_ch = -1;
		for(i=0;i<(get_trigin_num());i++)
		{
			k=(i<10)?i:i+16;
			if((trig&(1<<k))==0)
				continue;
			pb_Tag.pb[i] = 0;
		}


		for(i=0;i<(get_trigin_num());i++)
		{
			if(pb_Tag.pb[i] == 1)
			{
				new_ch=alarm_motion->alarm_trigin_video_ch[i];
				if(new_ch!=-1)
				{
					count++;
					for(j=0;j<(get_trigin_num());j++)
					{
						if(pb_Tag.pb[j]==1)
						{
							old_ch=alarm_motion->alarm_trigin_video_ch[j];
							if((old_ch!=-1)&&(old_ch!=new_ch))
								count++;
						}
					}
					if(count > 1)
					  para->alarm_playback_ch = 4;
					else
					  para->alarm_playback_ch = new_ch;

					break;
				}
				else
					continue;
			}
		}

		if(count==0)
		{
				//lc do ���Զ�ȷ�ϻ�ȡ������ʱ�����ܵƻָ�����
#ifdef USE_LED
				set_net_led_state(7);
				//set_alarm_led_state(0);
#endif
				if(para->buzzer_alarm_enable)
				  kill_buz();

		}
			
		//�ڱ�����λ�����з���ֹͣ¼��ط������tcprtimg
		//lc 2013-11-4
		if(para->alarm_motion.playback_enable)
		{
			if(count == 0)
			{
				#ifdef SHOW_WORK_INFO
				printf("�ڸ�λ���������и�rtimage����ֹͣplayback����\n");
				#endif
				gtloginfo("�ڸ�λ���������и�rtimage����ֹͣplayback����\n");
				alarm_cancel_playback_rtimg(para->vadc.quad.current_net_ch);
			}
			else
			{
				#ifdef SHOW_WORK_INFO
				printf("�ڸ�λ���������и�rtimage����playback����ͨ��%d\n",para->alarm_playback_ch);
				#endif
				gtloginfo("�ڸ�λ���������и�rtimage����playback����ͨ��%d\n",para->alarm_playback_ch);
				alarm_playback(para->alarm_playback_ch);
			}
		}
		pthread_mutex_unlock(&pb_Tag.mutex);
	}
#if 0		
	else
	{
		for(i=0;i<(get_trigin_num());i++)
		{
			if((trig&(1<<i))==0)
			{
				pb_Tag.pb[i] = 0;
				continue;
			}
		}
		for(i=0;i<(get_trigin_num());i++)
		{
			if(pb_Tag.pb[i] == 1 && para->alarm_motion.playback_enable)
			{
				gtloginfo("ͨ��%d�ڸ�λ���������и�rtimage����ֹͣplayback����\n",i);
				alarm_cancel_playback_rtimg(alarm_motion->alarm_trigin_video_ch[i]);
			}
		}
	}
#endif	

	return 0;
}

//�û�ȡ������
static int usr_cancel_alarm_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	int rc;
	WORD	result;
	int i;
	struct sockaddr_in peeraddr;
	struct usr_start_alarm_actions_yes_struct *action;
	struct ipmain_para_struct *para;
	
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	
	if((fd<0)||(cmd->cmd!=USR_CANCEL_ALARM))
		return -1;	
	action=(struct usr_start_alarm_actions_yes_struct *)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����%s������λ����0x0219,����״̬0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),(int)action->trig);
#endif
	gtloginfo("%s(fd=%d)����%s������λ����0x0219,����״̬0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),(int)action->trig);

	remote_cancel_alarm_playback(action->trig);

	rc=remote_cancel_alarm(action->trig);

	para = get_mainpara();
	
	if(rc>=0)
		result=RESULT_SUCCESS;
	else
		result=ERR_DVC_INTERNAL;
	
#ifdef SHOW_WORK_INFO
	printf("������λ���%d\n",result);
#endif
	gtloginfo("������λ���%d\n",result);

	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_CANCEL_ALARM, result,env,enc,dev_no);		

	
	return 0;
}

static int usr_local_record_set(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{//
	struct local_record_set_struct *set;
	char pbuf[15];
	WORD result;
	int rc=0;
	int i;
	struct sockaddr_in peeraddr;
	dictionary      *ini;	
	FILE *fp;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if((fd<0)||(cmd->cmd!=USR_LOCAL_RECORDER_SETING))
		return -1;
	set=(struct local_record_set_struct*)cmd->para;
	gtloginfo("%s(fd=%d)����%s������¼����������0x0217,Ԥ¼%d,��ʱ¼%d,�зֳ���%d\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),set->pre_rec,set->dly_rec,set->file_len);
	

	if(set->save_flag!=0)
	{
		ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
		if(ini==NULL)
		{
		//	ret=-3;
		}
		else
		{
			if(dev_no == 0)
			{
				sprintf(pbuf,"%d",set->pre_rec);
				save2para(ini,"hqpara:pre_rec",pbuf);

				sprintf(pbuf,"%d",set->dly_rec);
				save2para(ini,"hqpara:dly_rec",pbuf);

				sprintf(pbuf,"%d",set->file_len);
				save2para(ini,"hqpara:cut_len",pbuf);
			}
			else
			{
				sprintf(pbuf,"%d",set->pre_rec);
				save2para(ini,"hqpara1:pre_rec",pbuf);

				sprintf(pbuf,"%d",set->dly_rec);
				save2para(ini,"hqpara1:dly_rec",pbuf);

				sprintf(pbuf,"%d",set->file_len);
				save2para(ini,"hqpara1:cut_len",pbuf);
			}
			
			save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
			
			
			iniparser_freedict(ini);
		}
		
	}
	if(rc>=0)
		result=RESULT_SUCCESS;
	else
		result=ERR_DVC_INTERNAL;
	//system("killhd");
	if(virdev_get_virdev_number()==2)
		system("/ip1004/ini_conv -s");
	sleep(1);
	refresh_hdmodule_para();

	//lc todo 2014-2-11 ƽ̨���͵�ԭ�и�ʽ�в�����ͨ���ţ�������չҪ��
	restart_hd_record(dev_no);
	gtloginfo("%s������¼���������ý��%d\n",devlog(dev_no),result);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_LOCAL_RECORDER_CONTROL, result,env,enc,dev_no);
	return 0;
}

//���������趨������Чʱ�������
static int usr_set_alarm_schedule(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	int i;
	struct sockaddr_in peeraddr;
	FILE *fp=NULL;
	dictionary *ini;
	char typestr[20];
	char statustr[20];
	struct ipmain_para_struct *mainpara;
	struct alarm_trigin_struct *trigin;
	struct usr_set_alarm_schedule_struct *schedule;
	struct motion_struct *motion;
	
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	
	
	if((fd<0)||(cmd->cmd!=USR_SET_ALARM_SCHEDULE))
		return -1;	
	schedule=(struct usr_set_alarm_schedule_struct *)cmd->para;


	if(schedule->alarm_type==1)
		sprintf(typestr,"�ƶ����");
	else
		sprintf(typestr,"���Ӵ���");

	if(schedule->setalarm==0)
		sprintf(statustr,"����");
	else
		sprintf(statustr,"����");

#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����%s���������趨������Чʱ�������0x0226\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
	printf("Ҫ������%s %d· %s\n",typestr,schedule->channel,statustr);	
#endif
	gtloginfo("%s(fd=%d)����%s���������趨������Чʱ�������0x0226\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
	gtloginfo("Ҫ������%s %d· %s\n",typestr,schedule->channel,statustr);	
	
	if(schedule->setalarm==1)
	{
#ifdef SHOW_WORK_INFO
	printf("����ʱ��%02d:%02d-%02d:%02d\n",schedule->begin_hour,schedule->begin_min,schedule->end_hour,schedule->end_min);
#endif
	gtloginfo("����ʱ��%02d:%02d-%02d:%02d\n",schedule->begin_hour,schedule->begin_min,schedule->end_hour,schedule->end_min);
	}
	
	//����flash
	if(schedule->save_flag!=0)//����б�Ҫ���ȴ�������ļ�
	{
		ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
		//ini=iniparser_load(IPMAIN_PARA_FILE);
		if(ini!=NULL)
		{
			save_setalarm_para(ini,schedule->alarm_type,schedule->channel,schedule->setalarm,schedule->begin_hour,schedule->begin_min,schedule->end_hour,schedule->end_min);
			//��ini�ṹ����ָ���ļ�
			save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
			iniparser_freedict(ini);
			//if(virdev_get_virdev_number()==2)
			//	system("/ip1004/ini_conv -s");
		}
	}

	//ȡ�ṹ,���˸���
	mainpara=get_mainpara();
	if(schedule->alarm_type==0)
	{
		trigin=&mainpara->alarm_motion.trigin[schedule->channel];
		trigin->setalarm=schedule->setalarm;
		if(trigin->setalarm==1)
		{
			{
				trigin->starthour=schedule->begin_hour;
				trigin->startmin=schedule->begin_min;
				trigin->endhour=schedule->end_hour;
				trigin->endmin=schedule->end_min;
			}
		}
	}
	
	else
	{
		motion = &mainpara->vadc.quad.motion[schedule->channel];
		trigin= &mainpara->alarm_motion.trigin[schedule->channel]; 

		
		trigin->setalarm=schedule->setalarm;
		if(trigin->setalarm==1)
		{
			{
				motion->starthour=schedule->begin_hour;
				motion->startmin=schedule->begin_min;
				motion->endhour=schedule->end_hour;
				motion->endmin=schedule->end_min;
			}
			
		}
	}
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_SET_ALARM_SCHEDULE, 0,env,enc,dev_no);		
	return 0;
}


//����ִ�б�����������
static int usr_start_alarm_actions_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env, int enc,int dev_no)
{
	int i,event;
	struct sockaddr_in peeraddr;
	struct usr_start_alarm_actions_struct *action;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if((fd<0)||(cmd->cmd!=START_ALARM_ACTIONS))
		return -1;	
	action=(struct usr_start_alarm_actions_struct *)cmd->para;
	event=action->event;
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)��������ִ�б�����������0x0223,Ҫ��ִ�ж�����%d\n",inet_ntoa(peeraddr.sin_addr),fd,event);
#endif
	gtloginfo("%s(fd=%d)��������ִ�б�����������0x0223,Ҫ��ִ�ж�����%d\n",inet_ntoa(peeraddr.sin_addr),fd,event);
	take_alarm_action(-1,event);
	if(cmd->en_ack!=0)
		return send_gate_ack(fd,START_ALARM_ACTIONS, 0,env,enc,dev_no);	
	return 0;
}

//ȷ�ϱ���
static int usr_start_alarm_actions_yes_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	
	int i,j,k,event;
	struct sockaddr_in peeraddr;
	struct usr_start_alarm_actions_yes_struct *action;
	struct ipmain_para_struct *para;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	if((fd<0)||(cmd->cmd!=START_ALARM_ACTIONS_YES))
		return -1;	
	action=(struct usr_start_alarm_actions_yes_struct *)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����ȷ�ϱ�����������0x0222,����״̬0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,(int)action->trig);
#endif
	gtloginfo("%s(fd=%d)����ȷ�ϱ�����������0x0222,����״̬0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,(int)action->trig);

	para = get_mainpara();
	
#if 0
	for(i=0;i<get_trigin_num()+get_video_num();i++)
	{
		if((action->trig&(1<<i))==0)
			continue;
		for(j=0;j<MAX_TRIG_EVENT;j++)
			{
				event=get_alarm_event(i,1,j);
				take_alarm_action(event);
			}
	}
#endif

	remote_cancel_alarm_playback(action->trig);

	//ִ����Щresetִ�еı�������
	for(i=0;i<(get_trigin_num());i++)
	{
		k=(i<10)?i:i+16;
		if((action->trig&(1<<k))==0)
			continue;
		for(j=0;j<MAX_TRIG_EVENT;j++)
		{
			event=get_alarm_event(i,1,j); 
			//printf("��λʱeventΪ%d\n",event);
			take_alarm_action(i,event);
		}
	}

	/*��Ϊtrig�ĵ�λ10�ֽ��Ǹ�������reserve�ģ���bit10��ʼ����motion_alarmλ��
	  ���Դ�bit10��ʼ�Ƚ���ȷ������·�ƶ���ⱨ����
	 */
	for(i=10;i<((get_video_num())+10);i++)
	{
		if((action->trig&(1<<i))==0)
			continue;
		for(j=0;j<MAX_TRIG_EVENT;j++)
		{
			
			//��Ϊget_alarm_event������ƶ����Ŀǰ��6-9������Ӧ��ѭ������i��10-14������������i-4  
			event=get_alarm_event(i-4,1,j); 
			//printf("��λʱeventΪ%d\n",event);
			take_alarm_action(i,event);
		}
	}
#if 0	
	alarm_cancel_playback_rtimg(para->vadc.quad.current_net_ch);

#ifdef SHOW_WORK_INFO
	printf("��tcprtimg����ֹͣplayback����\n");
#endif
	gtloginfo("��tcprtimg����ֹͣplayback����\n");
#endif

	if(cmd->en_ack!=0)
		return send_gate_ack(fd,START_ALARM_ACTIONS_YES, 0,env,enc,dev_no);
	return 0;
}

//#endif


//����ftp�ʺŵ���Ϣ
static int send_ftp_info(int fd,WORD result,int env, int enc,int dev_no)
{	
	DWORD send_buf[20];//��Ӧ��������ᳬ��100�ֽ�
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	struct ftp_info_struct *info;	
	struct ipmain_para_struct *main_para;
	int rc;

	if(fd<0)
		return -1;
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;	
	cmd->cmd=USR_QUERY_FTP_ANSWER;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc=virdev_get_devid(dev_no,cmd->para);

	info=(struct ftp_info_struct *)((char *)cmd->para+rc);
	info->result=result;
	info->reserve=0;

	memset(	info->user,0,sizeof(info->user));
	memset(	info->password,0,sizeof(info->password));
#if EMBEDED==0
	memcpy(info->user,"anonymous",10);
	memcpy(info->password,"123@abc.com",12);
#else
	memcpy(info->user,"gtftp",6);
	memcpy(info->password,"gtalarm",8);

#endif
	info->expired_time=0;
	main_para=get_mainpara();
	info->ftp_port=main_para->ftp_port;
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_FTP_INFO_STRUCT;
	return send_gate_pkt(fd,modcom,cmd->len+2,dev_no);	


}
/*
	��ѯftp�ʺ�����
*/

static int usr_query_ftp_info_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)

{
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_QUERY_FTP))
		return -1;	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d) Ҫ���ѯFTP�˺���Ϣ,�ѷ���\n",inet_ntoa(peeraddr.sin_addr),fd);
#endif
	gtloginfo("%s(fd=%d) Ҫ���ѯFTP�˺���Ϣ,�ѷ���\n",inet_ntoa(peeraddr.sin_addr),fd);
	return send_ftp_info(fd,RESULT_SUCCESS,env,enc,dev_no);

}

static int usr_reboot_device_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	int i;
	int *boot_mode=NULL;
	struct sockaddr_in peeraddr;
	int rc;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_REBOOT_DEVICE))
		return -1;	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	boot_mode=(int *)cmd->para;

#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d) ����%s�����豸����\n",inet_ntoa(peeraddr.sin_addr),fd,*boot_mode?"Ӳ":"��");
#endif
	gtloginfo("%s(fd=%d) ����%s�����豸����\n",inet_ntoa(peeraddr.sin_addr),fd,*boot_mode?"Ӳ":"��");

	if(*boot_mode)
	{
		if(access("/tmp/hwrbt",R_OK|W_OK) == 0)
			rc = system("/tmp/hwrbt 3");
		else
			rc=system("/ip1004/hwrbt 3");
	}
	else
	{
		if(access("/tmp/swrbt",R_OK|W_OK) == 0)
			rc = system("/tmp/swrbt 3");
		else
			rc=system("/ip1004/swrbt 3");
	}
	
#ifdef SHOW_WORK_INFO
	printf("ִ�и�λ�ű�rc=%d\n",rc);
#endif
	gtloginfo("ִ�и�λ�ű�rc=%d\n",rc);

	if(cmd->en_ack!=0)
		return send_gate_ack(fd,USR_REBOOT_DEVICE, RESULT_SUCCESS,env,enc,dev_no);


	return 0;
}	


//���������Ϻ���Ӧ
int update_answer(int fd,WORD result,BYTE *resultinfo,int env, int enc,int dev_no)

{
	DWORD send_buf[50];//��Ӧ��������ᳬ��200�ֽ�
	//BYTE  dev_buf[20];
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
	struct user_upd_ack_struct *upack;
	int state;
	int rc;
	int i;
	BYTE *info;
	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=UPDATE_ANSWER;
	cmd->en_ack=0;
	
	cmd->reserve0=0;
	cmd->reserve1=0;
	upack=(struct user_upd_ack_struct*)cmd->para;
	rc=virdev_get_devid(dev_no,upack->dev_id);// cmd->para);

	print_devid(upack->dev_id,8);

	upack->state=result;
	upack->reserve=0;
	state=result;
	info=(BYTE*)upack->info;

	//����state�ж�info�����ݲ�д��

	sprintf(info,"%s",get_gt_errname(state));
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para) -2+28;
	i=send_gate_pkt(fd,modcom,cmd->len+2,dev_no);

	sprintf(resultinfo,info);
	gtloginfo("%s����������Ӧ�����0x%04x\n",devlog(dev_no),result);
	return 0;
}



//�û���¼�������Ӧ����ָ����env��enc����ָ����fd
int send_dev_login_return(int fd, WORD result, int env, int enc,int dev_no)
{
	DWORD send_buf[50];
	int rc;
	struct dev_login_return *login_return;
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *cmd;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	
	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;
	cmd->cmd=DEV_LOGIN_RETURN;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	login_return=(struct dev_login_return *)cmd->para;
	rc=virdev_get_devid(dev_no,login_return->dev_id);// cmd->para);

	login_return->result = result;
	login_return->reserved=0;
	login_return->login_magic = lrand48();//�����
	cmd->len=sizeof(struct gt_usr_cmd_struct)+rc+sizeof(struct dev_login_return)-sizeof(cmd->para);
	
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
#ifdef SHOW_WORK_INFO
	printf("%s��%s(fd=%d)�����û���¼��Ӧ����(result=0x%04x),magic=%d\n",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),fd,result,login_return->login_magic);
#endif
	gtloginfo("%s��%s(fd=%d)�����û���¼��Ӧ����(result=0x%04x),magic=%d\n",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),fd,result,login_return->login_magic);
	return 0;
	


}
//ֱ������������Ӧ����ָ����env��enc����ָ����fd
int direct_update_answer(int fd,WORD result,BYTE *resultinfo, int env, int enc,int dev_no)
{
	
	DWORD send_buf[50];//��Ӧ��������ᳬ��200�ֽ�
	struct user_upd_ack_struct *upack;
	int rc;
	BYTE *info;
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *cmd;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);

	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	
	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;
	cmd->cmd=UPDATE_ANSWER;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	upack=(struct user_upd_ack_struct*)cmd->para;
	rc=virdev_get_devid(dev_no,upack->dev_id);// cmd->para);

	upack->state=result;
	upack->reserve=0;
	info=(BYTE*)upack->info;
	//����state�ж�info�����ݲ�д��
	sprintf(info,"%s",get_gt_errname(result));
	cmd->len=sizeof(struct gt_usr_cmd_struct)+rc+sizeof(struct user_upd_ack_struct)-sizeof(cmd->para);	
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
	gtloginfo("%s��%s(fd = %d)����ֱ�������������Ӧ0x%04x:%s ",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),fd,result,get_gt_errname(result));
	return 0;	

}

//ֱ���������(non-ftp)
static int update_sw_direct(int fd,struct gt_usr_cmd_struct *netcmd, int env, int enc,int dev_no)

{
	struct update_direct_struct *update_direct;
	int result=0;
	int i;
	int read_cnt = 0 ;
	int read_total = 0;
	int stateled;
	char updatedir[50];
	char gzfilename[200];
	BYTE buf[8192];
	time_t timep;
	int fail_cnt =0;
	struct tm *p;
	int write_cnt=0;
	FILE *fp;
	char updateinfo[20];
	struct sockaddr_in peeraddr;
	
	char cmd[100];
	int addrlen=sizeof(struct sockaddr);
		
	if((fd<0)||(netcmd->cmd!=UPDATE_SOFTWARE_DIRECT))
		return -1;	
	//ȡ������
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);//��ȡ���ӶԷ���ip��ַ
	update_direct = (struct update_direct_struct*)netcmd->para;
	gtloginfo("%s ����Ҫ��%sֱ����������,����Ϊ%d,�ļ���С%d\n",inet_ntoa(peeraddr.sin_addr),devlog(dev_no),update_direct->type, update_direct->filesize);

	//////lc do 2013-7-24 ��ɾ��ip1004�����ݣ����жϿռ��Ƿ��㹻
	//lc do 20131107 �ı�������ʽ��ip1004Ŀ¼������ʹ��ʣ�µ�36M�ռ��е�32M������һ����ռ���ڴ��ļ�ϵͳ����������
#if 1	
	sprintf(cmd,"mkdir -p %s","/ip1004/");
	i=system(cmd);   
	sprintf(cmd,"mkdir -p %s/temp","/ip1004");  
	i=system(cmd);
	i=chdir("/ip1004/");
	sprintf(cmd,"cp /ip1004/hardreboot /tmp/ -frv");
	system(cmd);
	sprintf(cmd,"cp /ip1004/hwrbt /tmp/ -frv");
	system(cmd);
	sprintf(cmd,"rm -rf %s/*","/ip1004");
	system(cmd);
#else
	kill_apps4update();

	sleep(5);
	
	printf("0\n");
	sprintf(cmd,"/conf/updatelc");
	i=system(cmd);
	printf("1");
/*	
	sprintf(cmd,"rm -rf %s","/hqdata/firmup/");
	i=system(cmd);
	printf("1");
	sprintf(cmd,"mkdir -p %s","/hqdata/firmup/");
	i=system(cmd);
	printf("2");
	sprintf(cmd,"mkdir /var/upimg");
	i=system(cmd);
	printf("3");
	sprintf(cmd,"mount -t ramfs -o size=40M,maxsize=40M ramfs %s","/var/upimg");
	i=system(cmd);
	printf("4");
	chdir("/var/upimg");
	sprintf(cmd,"dd if=/dev/zero of=up.img bs=40M count=1");
	i=system(cmd);
	printf("5");
	sprintf(cmd,"echo 'y' | mkfs.ext3 up.img > /dev/null");
	i=system(cmd);
	printf("6");
	sprintf(cmd,"mount %s/up.img %s","/var/upimg","/hqdata/firmup/");
	i=system(cmd);
	printf("7");
*/	
#endif
	

	//����Ƿ�ռ��㹻
	result = -check_update_space(update_direct->filesize, updatedir); //result����Ϊ��ֵ
	if(result != RESULT_SUCCESS)
	{
		if(netcmd->en_ack!=0)
		i=direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);

		return 0;
	}


	
	//����Ƿ�������
	if(updating_flag == 0 )
	{	
		updating_flag =1;
		
	}
	else
	{
		if(update_waiting_no >= 10)
		{
			gtloginfo("%s:ͬʱҪ������������̫��,����BUSY\n",devlog(dev_no));
			result = ERR_DVC_BUSY;
			return direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);
		}
		
		pthread_mutex_lock(&update_mutex);
		updatesw_gate_list[update_waiting_no].dev_no = dev_no;
		updatesw_gate_list[update_waiting_no].gatefd = fd;
		updatesw_gate_list[update_waiting_no].env = env;
		updatesw_gate_list[update_waiting_no].enc = enc;
		update_waiting_no++;
		pthread_mutex_unlock(&update_mutex);
		printf("%s: direct updating !\n",devlog(dev_no));
		gtloginfo("%s:��������,������Ϻ�ͳһ����\n",devlog(dev_no));
		result = 0;
		i=direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);
		return 0;
	}	

	
	
	
	if(netcmd->en_ack!=0)
		i=direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);

	
	if(result !=  RESULT_SUCCESS)
		return 0;
	
    //��ʼ׼��������
   	net_set_recv_timeout(fd,10);
    time(&timep);
    p=localtime(&timep);
    sprintf(gzfilename,"%s/up_direct_%04d%02d%02d.tar.gz",updatedir,1900+p->tm_year,1+p->tm_mon,p->tm_mday);

	gtloginfo("ֱ������:׼����ʼ�������ݴ浽%s\n",gzfilename);
	fp = fopen(gzfilename,"w+");
	while(read_total < update_direct->filesize)
	{
		read_cnt = net_read_buf(fd, buf,8192);
		if(read_cnt > 0)
		{
			fail_cnt=0;				//zw-20071204��ɾ�����������
			read_total += read_cnt;
			write_cnt = fwrite(buf,1,read_cnt,fp);
			if(write_cnt== read_cnt)
				continue;
			else
			{
				gtloginfo("ֱ��������,д���ļ�%sʧ��,ferrno %d,�˳�\n",gzfilename,ferror(fp));
				result = ERR_DVC_INTERNAL;
				goto cleanup_and_return;
			}
		}
		//zw-20071204��ɾ�����������---->
		else
		{
			if(read_cnt==-EAGAIN)
			{///���ճ�ʱ
				if(++fail_cnt < 30) //shixin changed from 3
					continue;
				//����6����������
			}
			//��������ֱ���˳�
		}
		//zw-20071204��ɾ�����������<----
		
	
		
		printf("ֱ��������,��%d�ֽں�read_buf����%d,�˳�\n",read_total,read_cnt);
		gtlogerr("ֱ��������,��%d�ֽں�read_buf����%d,�˳�\n",read_total,read_cnt);
		result = ERR_DVC_WRONG_SIZE;

cleanup_and_return://�����ƺ����󣬸�֪���ز��˳�
		fclose(fp);
		sprintf(cmd,"rm -rf %s",gzfilename);
		system(cmd);
		if(netcmd->en_ack!=0) //��֪����
			i=direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);
		pthread_mutex_lock(&update_mutex);
		for(i=0;i<update_waiting_no;i++)//�������֮ǰ����busyû���ص��������
		{
			if(updatesw_gate_list[i].gatefd <= 0)
				update_answer(updatesw_gate_list[i].gatefd,result,updateinfo,updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
			else
				direct_update_answer(updatesw_gate_list[i].gatefd,result,get_gt_errname(result),updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
		}
		update_waiting_no = 0;
		pthread_mutex_unlock(&update_mutex);
		updating_flag=0;
		//lc add
		system("/tmp/hardreboot");
		return -result;
	}
	
	fclose(fp);
	printf("%sֱ�����������ļ�%s�ɹ�\n",devlog(dev_no),gzfilename);
	gtloginfo("%sֱ�����������ļ�%s�ɹ�\n",devlog(dev_no),gzfilename);

	 //�õ��˴�С�����.tar.gz�ļ�
	//stateled=get_current_stateled();//��ʼ��˸state�ƣ��Է����ε�
	//set_state_led_state(1);	

	//lc 2014-2-21 �ӽ����ļ���ʼ��ʱ��ֹͣ����������5���ӱ�Ϊ���
	//ʹ����������
#ifdef ARCH_3520A	
	update_set_com_mode(1,UPDATE_UNRESPONED_INTERVAL);
#endif	 
	result = direct_update_software(gzfilename,updatedir);
	//set_state_led_state(stateled);
	i=direct_update_answer(fd,result,get_gt_errname(result),env,enc,dev_no);

	pthread_mutex_lock(&update_mutex);
	for(i=0;i<update_waiting_no;i++)//�������֮ǰ����busyû���ص��������
	{
		if(updatesw_gate_list[i].gatefd <= 0)
			update_answer(updatesw_gate_list[i].gatefd,result,updateinfo,updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
		else
			direct_update_answer(updatesw_gate_list[i].gatefd,result,get_gt_errname(result),updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
	}
	update_waiting_no = 0;
	pthread_mutex_unlock(&update_mutex);

#ifdef SHOW_WORK_INFO
    printf("%s sending update_answer %d\n",devlog(dev_no),result);
#endif    
   //����־

   	gtloginfo("%sֱ����������������Ϊ 0x%04x, ���Ϊ%s",devlog(dev_no),result,get_gt_errname(result));
	updating_flag = 0;
	return 0;
}

//�û���ʽ��Ӳ�̵���Ӧ����ָ��env,enc����ָ����fd
int format_disk_answer(int fd, int result,int env,int enc,int dev_no,int approxtime)
{
	DWORD send_buf[50];//��Ӧ��������ᳬ��200�ֽ�
	format_disk_answer_struct *fmtanswer;
	int rc;
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *cmd;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);

	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	
	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;
	cmd->cmd=FORMAT_DISK_ANSWER;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	fmtanswer=(format_disk_answer_struct *)cmd->para;
	rc=virdev_get_devid(dev_no,fmtanswer->dev_id);// cmd->para);

	fmtanswer->result 	= result;
	fmtanswer->reserve1 = 0;
	fmtanswer->approxtime = approxtime;
	cmd->len=sizeof(struct gt_usr_cmd_struct)+rc+sizeof(format_disk_answer_struct)-sizeof(cmd->para);	
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
	if(approxtime == 0)
		gtloginfo("%s��%s(fd = %d)���͸�ʽ����������Ľ��0x%04x",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),fd,result);
	else
		gtloginfo("%s��%s(fd = %d)���͸�ʽ�����������ACK 0x%04x,Ԥ��ʱ��%d��",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),fd,result,approxtime);
	return 0;	

}

//����Ӳ�̱��(0��ʾ����Ӳ�̣�1,2...�ֱ��ʾ��1,2��Ӳ��)�����ʽ������ʱ��
int get_fmt_time_by_disknum(int diskno)
{
	int i;
	int volume_in_G = 0;
	
	if(diskno == 0)//����Ӳ��
	{
		for(i=0;i< get_sys_disk_num();i++)
			volume_in_G += get_sys_disk_capacity(get_sys_disk_name(i));
	}
	else
		volume_in_G = get_sys_disk_capacity(get_sys_disk_name(i-1));

	return (50 + (volume_in_G>>13) ); //ʱ��:250G 66��,500G 86�룬��ʽ�ݶ�Ϊ50+v/8
}

static int formating_flag = 0; //�Ƿ��ڸ�ʽ�����̱�־��1Ϊ�ڸ�ʽ�� 
static pthread_mutex_t format_mutex = PTHREAD_MUTEX_INITIALIZER;
static int format_waiting_no=0;	//��¼��������ʽ������Ҫ���������Ŀ
static gateinfo formathd_gate_list[10];//��ŷ�������ʽ������Ҫ���������Ϣ

//�û���ʽ������
static int usr_format_hd(int fd,struct gt_usr_cmd_struct *netcmd, int env, int enc,int dev_no)
{
//lc

	usr_format_hd_struct *fmt;
	int result=0;
	int i;
	int disknum;
	struct sockaddr_in peeraddr;
	char cmd[100];
	int ret;
	int addrlen=sizeof(struct sockaddr);
		
	if((fd<0)||(netcmd->cmd!=USR_FORMAT_HARDDISK))
		return -1;	

	//ȡ������
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);//��ȡ���ӶԷ���ip��ַ
	fmt = (usr_format_hd_struct *)netcmd->para;
	disknum = get_sys_disk_num();

	if(fmt->diskno == 0)
	{
		gtloginfo("%s ����Ҫ��%s��ʽ����������,��ʽ�����д���\n",inet_ntoa(peeraddr.sin_addr),devlog(dev_no));
		gtloginfo("��ʽ��������:%s",fmt->usr_info);
		if(disknum <= 0)//�޿��ô���
		{
			gtloginfo("�豸�޿��ô���,�޷���ʽ��\n");
			return format_disk_answer(fd,ERR_DVC_NO_DISK,env,enc,dev_no,0);
		}
	}
	else
	{
		gtloginfo("%s ����Ҫ��%s��ʽ����������,���̺�Ϊ%d\n",inet_ntoa(peeraddr.sin_addr),devlog(dev_no),fmt->diskno);
		gtloginfo("��ʽ��������:%s",fmt->usr_info);
		//����Ƿ��д���
		if(get_sys_disk_num()<fmt->diskno)
		{
			gtloginfo("�豸������%d,�޷���ʽ����%d������\n",get_sys_disk_num(),fmt->diskno);
			return format_disk_answer(fd,ERR_DVC_NO_DISK,env,enc,dev_no,0);
		}
	}
	
	format_disk_answer(fd,RESULT_SUCCESS,env,enc,dev_no,get_fmt_time_by_disknum(fmt->diskno));

	//�鿴��ǰ�Ƿ��Ѿ��ڸ�ʽ�����̣������ظ�ִ�У�֮��ͳһ����
	if(formating_flag == 0 )
	{	
		formating_flag =1;
	}
	else
	{
		if(format_waiting_no >= 10)
		{
			gtloginfo("%s:ͬʱҪ���ʽ�����̵�����̫��,����BUSY\n",devlog(dev_no));
			return format_disk_answer(fd,ERR_DVC_BUSY,env,enc,dev_no,0);
		}
		
		pthread_mutex_lock(&format_mutex);
		formathd_gate_list[format_waiting_no].dev_no = dev_no;
		formathd_gate_list[format_waiting_no].gatefd = fd;
		formathd_gate_list[format_waiting_no].env = env;
		formathd_gate_list[format_waiting_no].enc = enc;
		format_waiting_no++;
		pthread_mutex_unlock(&format_mutex);
		gtloginfo("%s:���ڸ�ʽ��Ӳ��,��ʽ����Ϻ�ͳһ����\n",devlog(dev_no));
		i=format_disk_answer(fd,RESULT_SUCCESS,env,enc,dev_no,get_fmt_time_by_disknum(0));
		return 0;
	}	

	//ִ�и�ʽ������
	
	sprintf(cmd,"/ip1004/initdisk -B 1");//�������ĸ�ʽ��,fixme,�������ܴ�����
	ret = system(cmd);
	if(ret == -1)
	{
		result = ERR_DVC_INTERNAL;
	}
	format_disk_answer(fd,result,env,enc,dev_no,0);		

	//ͳһ���������Ŷ��еĸ�ʽ������
	pthread_mutex_lock(&format_mutex);
	for(i=0;i<format_waiting_no;i++)//�������֮ǰ����busyû���صĸ�ʽ�����
	{
		format_disk_answer(formathd_gate_list[i].gatefd,result,formathd_gate_list[i].env,formathd_gate_list[i].enc,formathd_gate_list[i].dev_no,0);
	}
	format_waiting_no = 0;
	pthread_mutex_unlock(&format_mutex);
	formating_flag=0;

	//�������豸
	if(result == 0)
	{
		gtloginfo("��ʽ���ɹ��������豸�Ա�֤��������");
		system("/ip1004/hwrbt");
	}
	
	return 0;
}




//�û���¼�豸
static int usr_login_device_cmd(int fd,struct gt_usr_cmd_struct *netcmd, int env, int enc,int dev_no)

{
	struct usr_login_device *usr_login;
	
	int result=0;
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
		
	if((fd<0)||(netcmd->cmd!=USR_LOGIN_DEVICE))
		return -1;	
	//ȡ������
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);//��ȡ���ӶԷ���ip��ַ
	usr_login = (struct usr_login_device*)netcmd->para;
#ifdef SHOW_WORK_INFO
	printf("%s ��¼%s,�û���Ϣ:%s\n",inet_ntoa(peeraddr.sin_addr),devlog(dev_no),usr_login->username);
#endif
	gtloginfo("%s ��¼%s,�û���Ϣ:%s\n",inet_ntoa(peeraddr.sin_addr),devlog(dev_no),usr_login->username);


	if(netcmd->en_ack!=0)
		i=send_dev_login_return(fd,result,env,enc,dev_no);

	return 0;
	
}


//�û���¼�������Ӧ����ָ����env��enc����ָ����fd
int send_dev_ip_return(int fd, WORD result, int env, int enc,int dev_no)
{

	DWORD send_buf[50];
	int rc;
       struct dev_ip_return *ip_return=NULL;
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *cmd;
	struct sockaddr_in peeraddr;
       struct sockaddr_in localaddr;

       
	int addrlen=sizeof(struct sockaddr);
	
	rc=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

       addrlen=sizeof(struct sockaddr);
       rc=getsockname(fd,(struct sockaddr *)&localaddr,&addrlen);
       memset(send_buf,0,sizeof(send_buf));
	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;
	cmd->cmd=DEV_IP_RETURN;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	ip_return=(struct dev_ip_return *)cmd->para;
	rc=virdev_get_devid(dev_no,ip_return->dev_id);// cmd->para);
	ip_return->result = result;
	ip_return->reserved1=0;

       memcpy(&ip_return->client_ip,&peeraddr.sin_addr,sizeof(ip_return->client_ip));
       memcpy(&ip_return->device_ip,&localaddr.sin_addr,sizeof(ip_return->device_ip));
       ip_return->client_ip=htonl(ip_return->client_ip);///ת����������
       ip_return->device_ip=htonl(ip_return->device_ip);

    
	cmd->len=sizeof(struct gt_usr_cmd_struct)+rc+sizeof(struct dev_ip_return)-sizeof(cmd->para);
	
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);

	return rc;
	


}

//�û���ѯ�Լ���ip��ַ
static int usr_require_self_ip_cmd(int fd,struct gt_usr_cmd_struct *netcmd, int env, int enc,int dev_no)

{
	
	int result=0;
	
	if((fd<0)||(netcmd->cmd!=USR_REQUIRE_SELF_IP))
		return -1;	
	return send_dev_ip_return(fd,result,env,enc,dev_no);


}

//�������
static int usr_update_software_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc,int dev_no)
{
	struct update_software_struct *update;
	char msg[300];
	char userid[13];
	char password[13];
	char FTPip[50],*FTPipPtr;
	char rebootcmd[100];
	unsigned long int FTPipaddress;
	char *filepath;
	int result;
	int i;
	int stateled;
	struct in_addr *addr;
	struct sockaddr_in peeraddr;
	char updateinfo[20];
	int addrlen=sizeof(struct sockaddr);
		
	if((fd<0)||(netcmd->cmd!=USER_UPDATE))
		return -1;	

	memset(msg,0,300);
	//ȡ������
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);//��ȡ���ӶԷ���ip��ַ
	printf("usr_update_software_cmd remote rc=%d %s\n",i,inet_ntoa(peeraddr.sin_addr));
	
	update=(struct update_software_struct*)netcmd->para;
	memcpy(userid,update->userid,12);
	memcpy(password,update->password,12);
	userid[12]='\0';
	password[12]='\0';
	FTPipaddress=(unsigned long int)update->FTPip;

	FTPipaddress=htonl(FTPipaddress);
	addr=(struct in_addr *)&FTPipaddress;
	FTPipPtr=inet_ntoa(*addr);
	memcpy(FTPip,FTPipPtr,sizeof(FTPip));
	


	filepath=(char *)update->filepath;

	gtloginfo("%s(fd=%d) ����%s��������,����Ϊ %d,·��Ϊ%s:%d/%s\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),update->type,FTPip,update->ftpport,filepath);
//add by wsy 05.09.26,�ж�filepath�Ƿ���"/"
//13.06.21 lc do ���豸��wget��֧��-T��ʱ��ȥ��
	if(index(filepath,'/')==filepath)
		sprintf(msg,"wget -c ftp://%s:%s@%s:%d%s",userid,password,FTPip,update->ftpport,filepath);
	else
		sprintf(msg,"wget -c ftp://%s:%s@%s:%d/%s",userid,password,FTPip,update->ftpport,filepath);

	printf("usr_update_software_cmd msg is %s\n",msg);

	if(netcmd->en_ack!=0)
		i=send_gate_ack(fd,USER_UPDATE,0,env,enc,dev_no);

	if(updating_flag==0)
	{
		updating_flag=1;

		//lc to do ����������״̬��
//		stateled=get_current_stateled();
		//��ʼ��˸state�ƣ��Է����ε�
		//set_state_led_state(1);
#ifdef ARCH_3520A		
		result=-update_software(update->filesize,msg,UPDATE_UNRESPONED_INTERVAL);
#else  
		result=-update_software(update->filesize,msg,-1);
#endif
		//set_state_led_state(stateled);
		updating_flag=0;
		i=update_answer(-1,result,updateinfo,env,enc,dev_no);
		pthread_mutex_lock(&update_mutex);
		for(i=0;i<update_waiting_no;i++)//�������֮ǰ����busyû���ص��������
		{
			if((updatesw_gate_list[i].gatefd) <= 0)
			{
				update_answer(updatesw_gate_list[i].gatefd,result,updateinfo,updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
				
			}
			else
			{
				direct_update_answer(updatesw_gate_list[i].gatefd,result,get_gt_errname(result),updatesw_gate_list[i].env,updatesw_gate_list[i].enc,updatesw_gate_list[i].dev_no);
			}
		}
		update_waiting_no = 0;
		pthread_mutex_unlock(&update_mutex);
	}	
	else
	{	
		if(update_waiting_no >= 10)
		{
			gtloginfo("ͬʱҪ������������̫��,����BUSY\n");
			return update_answer(-1,ERR_DVC_BUSY,updateinfo,env,enc,dev_no);
		}
		
		pthread_mutex_lock(&update_mutex);
		updatesw_gate_list[update_waiting_no].dev_no = dev_no;
		updatesw_gate_list[update_waiting_no].gatefd = -1;
		updatesw_gate_list[update_waiting_no].env = env;
		updatesw_gate_list[update_waiting_no].enc = enc;
		update_waiting_no++;
		pthread_mutex_unlock(&update_mutex);
		gtloginfo("��������,������Ϻ�ͳһ����\n");
		return 0;
	}
	
#ifdef SHOW_WORK_INFO
    printf("sending update_answer %d\n",result);
#endif    
   //����־
   	gtloginfo("��������������Ϊ 0x%04x, ���Ϊ%s",result,updateinfo);
	if((result==0))//remed by shixin &&(i==0))
	{
		if (update->reset_flag==0) 
		{
				gtloginfo("������ɺ��˳�ipmain\n");
				sleep(30);
				exit(0);
		}
		else if (update->reset_flag==1) 
		{
			sleep(2);
			//lc do
			system("/tmp/hardreboot &");
			printf("/tmp/hardreboot & done!\n");
			//send_require_reset();
		}
	}
	else
	{
		gtlogerr("����ʧ�ܣ��豸����\n");
		system("/tmp/hardreboot &");
	}

	return 0;
}


//ץͼ��Ϊ�������豸����Ҫ�ȴ���һ��ͨ����,��ת��
static int usr_take_hq_pic_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc,int dev_no)
{
	struct user_get_hq_pic_struct *hqpic;
	if(virdev_get_virdev_number()==2)//�������豸
	{
		hqpic = (struct user_get_hq_pic_struct *)netcmd->para;
		hqpic->rec_ch = dev_no;	
	}
	
	return bypass_hqsave_cmd(fd, netcmd, env, enc, dev_no);
}

//����¼��Ϊ�������豸����Ҫ�ȴ���һ��ͨ����,��ת��
static int usr_local_recorder_ctl_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc,int dev_no)
{
	struct local_record_ctl_struct *recctl;
	if(virdev_get_virdev_number()==2)//�������豸
	{
		recctl = (struct local_record_ctl_struct *)netcmd->para;
		recctl->rec_ch = dev_no;
	}
	
	return bypass_hqsave_cmd(fd, netcmd, env, enc, dev_no);
}

//�ӽ�����Ϊ�������豸����Ҫ�ȴ���һ��ͨ����,��ת��
static int usr_lock_file_time_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc,int dev_no)
{
	struct usr_lock_file_time_struct *locktime;
	struct ipmain_para_struct *para;
	struct alarm_motion_struct *alarm_motion;
	int i,j,video_ch;
	int temptag=0;
	DWORD trig = 0;	
	DWORD videotrig=0;

	para=get_mainpara();
	alarm_motion=&para->alarm_motion;	
	locktime  = (struct usr_lock_file_time_struct *)netcmd->para;
	trig=locktime->lock_ch;



	//�������ᵥ��������������
	//	//lc 2014-2-11
	if(para->multi_channel_enable)
	{
		pthread_mutex_lock(&pb_Tag.mutex);
		for(i=0;i<(get_trigin_num());i++)
		{
			if((trig&(1<<i))!=0)
			{
				temptag=0;
				pb_Tag.pb[i] = 0;
				video_ch=alarm_motion->alarm_trigin_video_ch[i];
				for(j=0;j<(get_trigin_num());j++)
				{
					if(pb_Tag.pb[j] == 1 && alarm_motion->alarm_trigin_video_ch[j]==video_ch)
					{
						temptag=1;
						break;
					}
				}
				if(temptag==0)
				{
					gtloginfo("video ch %d cancel lock \n",video_ch);
					videotrig|=(1<<video_ch);
				}

				continue;
			}
		}
		pthread_mutex_unlock(&pb_Tag.mutex);	
		locktime->lock_ch=videotrig;
	}	
	
	return bypass_diskman_cmd(fd, netcmd, env, enc, dev_no);
}

//Ҫ����Ƶ��Ϊ�������豸����Ҫ�ȴ���һ��ͨ����,��ת��
static int usr_require_speak_cmd(int fd,struct gt_usr_cmd_struct *netcmd,int env,int enc,int dev_no)
{
	struct sockaddr_in peeraddr;
	int i;
	int addrlen=sizeof(struct sockaddr);
		
	if((fd<0)||(netcmd->cmd!=USR_REQUIRE_SPEAK))
		return -1;	
	
	if(virdev_get_audio_num(dev_no)==0)//����Ƶ��Դ
	{
		
		//ȡ������
		i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);//��ȡ���ӶԷ���ip��ַ		
		gtloginfo("%s(fd=%d) ����Ҫ��������������,��%s����Ƶ�ӿ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
		return send_gate_ack(fd,netcmd->cmd,ERR_DVC_NO_AUDIO,env,enc,dev_no);
	}
	else
		return bypass_rtaudio_cmd(fd, netcmd, env, enc, dev_no);
}


//static int regist_cnt=0;//shixintest
/**********************************************************************************************
 * ������	:netcmd_second_proc()
 * ����	:����Զ���������ӵ��봦�����
 * ����	:��
 *����ֵ	:��
 **********************************************************************************************/

void netcmd_second_proc(void)
{
	struct ip1004_state_struct *stat;
	int dev_no = 0;
	
	//struct send_dev_trig_state_struct *dev_trig;
	//for(dev_no=0;dev_no <virdev_get_virdev_number();dev_no++)
	{
	
		stat=get_ip1004_state(dev_no);	
		//gtloginfo("second_proc\n");
		if(get_regist_flag(dev_no)==0)
		{//��û��ע����
			if(++stat->regist_timer>REGIST_TIMEOUT)
			{
				stat->regist_timer=0;
				//gtloginfo("��ͼ����ע����Ϣ\n");
				system_regist(-1,1,0,0,dev_no);		//�����Լ���ע����Ϣ
				//send_dev_state(-1,1);	//�����Լ���״̬
				//send_dev_trig_state(-1,1);//���ͱ���״̬
			}
		}
		else
		{//�Ѿ�ע����
			if(get_reportstate_flag(dev_no)==0)
			{				
					if(++stat->reportstate_timer>REPORTSTATE_TIMEOUT)
					{
						stat->reportstate_timer=0;
						//gtloginfo("�ٴ���ͼ�����Լ���״̬��Ϣ�¼�\n");
						send_dev_state(-1,1,1,0,0,dev_no);	//�����Լ���״̬
					}
			}

			if(get_alarm_flag(dev_no)==0)
			{
				if(++stat->alarm_timer>ALARM_TIMEOUT)
				{
					stat->alarm_timer=0;
						
					//gtloginfo("�ٴ���ͼ���ͱ�����Ϣ\n");
					send_dev_trig_state(-1,&last_dev_trig,1,0,0,dev_no);	//���ͱ���״̬
				}	
			}
			
			if(get_trigin_flag(dev_no)==0)
			{
				//gtloginfo("stat->trigin_timer %d\n", stat->trigin_timer);
				if(++stat->trigin_timer>TRIGIN_TIMEOUT)
				{
					stat->trigin_timer=0;
					send_alarmin_state_change(stat->old_alarmin_state, stat->alarmin_state, stat->alarmin_change_time,dev_no);
				}	
			}
		}
	}
}



static int usr_ack_cmd(int fd,struct gt_usr_cmd_struct *cmd,int env, int enc,int dev_no)
{
	struct usr_cmd_ack_struct *ack;
	int i;
	char ackinfo[200];
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	struct ipmain_para_struct *mainpara;
	struct alarm_motion_struct *alarmmotion;
	
	if((fd<0)||(cmd->cmd!=USR_CMD_ACK))
		return -1;
	ack=(struct usr_cmd_ack_struct*)cmd->para;
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	switch(ack->rec_cmd)
	{
		case DEV_REGISTER:
			
		if(ack->result==RESULT_SUCCESS)
		{
			set_regist_flag(dev_no,1);//ע��ɹ�
#ifdef SHOW_WORK_INFO
			printf("%s(fd=%d) ����ACK��֪%sע��ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
#endif
			gtloginfo("%s(fd=%d) ����ACK��֪%sע��ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
			if(dev_no == 0)
			{
				//lc do ����״̬��--ע��ɹ�������Ƴ���
//				set_net_led_state(NET_REGISTERED);
#ifdef USE_LED
				set_net_led_state(7);
#endif
			}
		}
		else 
		{
#ifdef SHOW_WORK_INFO
			printf("%s(fd=%d) ����ACK��֪%sע��ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
#endif
			gtloginfo("%s(fd=%d) ����ACK��֪%sע��ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
		}
		break;
		case DEV_STATE_RETURN: //����״̬
			if(ack->result==RESULT_SUCCESS)
			{
				set_reportstate_flag(dev_no,1);
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ����ACK��֪%s����״̬�ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
#endif
				gtloginfo("%s(fd=%d) ����ACK��֪%s����״̬�ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
			}
			else 
			{
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ����ACK��֪%s����״̬ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
#endif
				gtloginfo("%s(fd=%d) ����ACK��֪%s����״̬ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
			}
		break;
		case DEV_ALARM_RETURN:	//��������
			//lc do ��������Ϣ�������ݿ�
#ifdef DUMP_ALARM			
			dump_alarmack_to_log(ack->result, "--");
#endif
			if(ack->result==RESULT_SUCCESS)
			{	
				mainpara=get_mainpara();
				if(dev_no == 0)
				{
					//lc do �������ص�Ϊ��˸
				#ifdef USE_LED
					set_net_led_state(2);
					//set_alarm_led_state(4);
				#endif
					if(mainpara->buzzer_alarm_enable)
						active_buz();
				}
				set_alarm_flag(dev_no,1);
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ����ACK��֪%sԶ�̱����ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
#endif
				gtloginfo("%s(fd=%d) ����ACK��֪%sԶ�̱����ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));

				if(mainpara!=NULL)
				{
					alarmmotion=&(mainpara->alarm_motion);
					if(alarmmotion!=NULL)
					{
						if(alarmmotion->audioalarm==1)//��Ҫ������ʾ
						{
						        //system("/ip1004/aplay /conf/audio/alarm/ack.wav &");
						#ifdef AUDIO_OUTPUT        
								//system("killall -15 aplay");
								//sleep(1);
								//sprintf(ackinfo,"/ip1004/aplay 49 %s &",get_audio_file("ack.adpcm")); //changed by shixin
                                                 system(ackinfo);
						#endif						 
							
						}
					}
				}
			}
			else 
				{
					//if(dev_no == 0)
					//lc to do ���ñ�����
						//set_state_led_state(3);
					gtloginfo("%s(fd=%d) ����ACK��֪%s����ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
				}
		break;
		case DEV_QUERY_TRIG_RETURN:	//����������״̬�仯
			set_trigin_flag(dev_no,1);
			//lc do ��������Ϣ�������ݿ�
#ifdef DUMP_ALARM			
			dump_triginack_to_log(ack->result,"--");
#endif
			if(ack->result==RESULT_SUCCESS)
			{
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ����ACK��֪%s����������״̬�ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
#endif
				gtloginfo("%s(fd=%d) ����ACK��֪%s����������״̬�ɹ�\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no));
			}
			else 
			{
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ����ACK��֪%s����������״̬ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
#endif
				gtloginfo("%s(fd=%d) ����ACK��֪%s����������״̬ʧ��,������0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
			}
		break;
		case	USR_RW_DEV_PARA:
			{
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ������%sUSR_RW_DEV_PARA ��ACK result=0x%x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
#endif
				gtloginfo("%s(fd=%d) ������%sUSR_RW_DEV_PARA ��ACK result=0x%x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
			}
		break;
		case 	DEV_POSITION_RETURN:
//				gtloginfo("%s(fd=%d) ������%DEV_POSITION_RETURN ��ACK result=0x%x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
		break;
		case 	DEV_PARA_RETURN:
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ������%sDEV_PARA_RETURN ��ACK result=0x%x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
#endif
				gtloginfo("%s(fd=%d) ������%sDEV_PARA_RETURN ��ACK result=0x%x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->result);
		break;
		default:
#ifdef SHOW_WORK_INFO
				printf("%s(fd=%d) ������%s��֧�ֵ�ACK cmd=0x%x,result 0x%x(%s)\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->rec_cmd,ack->result,get_gt_errname(ack->result));
#endif
				gtloginfo("%s(fd=%d) ������%s��֧�ֵ�ACK cmd=0x%x,result 0x%x(%s)\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),ack->rec_cmd,ack->result,get_gt_errname(ack->result));
		break;
	}
	return 0;
}


/**********************************************************************************
 *      ������: send_gate_query_adparam_return()
 *      ����:   �����ط����û�ͼ��A/D������ѯ����Ľ��
 *      ����:	fd			�û����ӵ�tcp���
 *				channel		��ѯͼ���ͨ��
 *				result		�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int send_gate_query_adparam_return(int fd,WORD channel,DWORD result,int env,int enc,int dev_no)
{

	DWORD send_buf[50];							//��Ӧ��������ᳬ��100�ֽ�
	struct ipmain_para_struct *main_para;
	struct dev_query_adparam_ret *ad_para;
	struct gt_usr_cmd_struct *cmd;
	struct gt_pkt_struct *send=NULL;	
	int rc;
	char *buf_p=NULL;

	memset(send_buf,0,sizeof(send_buf));
	buf_p =(char *)send_buf;	
	if(fd<0)
	{
		return -1;
	}
	
	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;	
	cmd->cmd=DEV_QUERY_ADPARAM_RETURN;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	
	ad_para=(struct dev_query_adparam_ret *)((char *)cmd->para);		//Ϊad_para�����ڴ�ռ� 
	ad_para->result=result;	//�ȼ�������ȷ�ģ���Ϊ�ڵ�������ӿ�ʱ���ݹ����Ĳ���ʱRESULT_SUCCESS
	ad_para->channel=channel;	
	
	rc=virdev_get_devid(dev_no,ad_para->dev_id);
	if(rc<0)
	{
		gtlogerr("�����ط����û�ͼ��A/D������ѯ����ʱ��ȡdevID���󣬴�����:%d\n",rc);
		ad_para->result=ERR_ENC_NOT_ALLOW;
	}

	main_para=get_mainpara();
	
	if(ad_para->result==RESULT_SUCCESS)
	{
		ad_para->bright=main_para->vadc.enc_front[channel].bright;
		if(ad_para->bright>100)
		{
			gtlogerr("ͨ��[%d]:����[%d]������Чֵ.\n",channel,ad_para->bright);
		}
		ad_para->hue=main_para->vadc.enc_front[channel].hue;
		if(ad_para->hue>100)
		{
			gtlogerr("ͨ��[%d]:ɫ��[%d]������Чֵ\n",channel,ad_para->hue);
		}
		ad_para->contrast=main_para->vadc.enc_front[channel].contrast;
		if(ad_para->contrast>100)
		{
			gtlogerr("ͨ��[%d]:�Աȶ�[%d]������Чֵ\n",channel,ad_para->contrast);
		}
		ad_para->saturation=main_para->vadc.enc_front[channel].saturation;
		if(ad_para->saturation>100)
		{
			gtlogerr("ͨ��[%d]:���Ͷ�[%d]������Чֵ\n",channel,ad_para->saturation);
		}
	}
	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_DEV_QUERY_ADPARAM_RETURN;

#ifdef SHOW_WORK_INFO
	printf("���豸�϶�:::����:%d%%	ɫ��:%d%%	�Աȶ�:%d%%	���Ͷ�:%d%%\n",ad_para->bright,ad_para->hue,ad_para->contrast,ad_para->saturation);
#endif

	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
#ifdef SHOW_WORK_INFO
	printf("��Զ�˷���������ͼ��A/D������ѯ�����%d,rc=%d\n",result,rc);
#endif
	gtloginfo("��Զ�˷���������ͼ��A/D������ѯ�����%d,rc=%d\n",result,rc);
	
	return 0;
}

/**********************************************************************************
 *      ������: usr_query_video_ad_param()
 *      ����:   �û�ͼ��A/D������ѯ������Ӧ
 *      ����:	fd			�û����ӵ�tcp���
 *				cmd			����ṹ	
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int usr_query_video_ad_param(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
	struct usr_query_adparam *usr_adparam;
	struct sockaddr_in peeraddr;
	int i;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_VIDEO_AD_PARAM)
	{
		return -1;
	}
#ifdef DISPLAY_REC_NETCMD
		printf("vsmain recv USR_QUERY_VIDEO_AD_PARAM cmd!\n");
#endif

	//�����û�����
	usr_adparam=(struct usr_query_adparam *)cmd->para;

	gtloginfo("%s(fd=%d)������ѯ�豸��Ƶǰ��ת����������0x%04x,ͨ����%d\n",inet_ntoa(peeraddr.sin_addr),fd,USR_QUERY_VIDEO_AD_PARAM,usr_adparam->channel);

	if((usr_adparam->channel+1)>virdev_get_video_num(dev_no))
	{
		//������Χ
		gtloginfo("��Ҫ��ѯ����Ƶͨ����:%d������Χ\n",usr_adparam->channel);
		send_gate_query_adparam_return(fd,usr_adparam->channel,ERR_ENC_NOT_ALLOW,env,enc,dev_no);
		return 0;
	}
	else
	{
		if(virdev_get_virdev_number()==2)//����
			usr_adparam->channel = dev_no;
		send_gate_query_adparam_return(fd,usr_adparam->channel,RESULT_SUCCESS,env,enc,dev_no);
	}
	
	return 0;
}




/**********************************************************************************
 *      ������: fill_trigin_return_xml()
 *      ����:  ��ָ����xml��ʽ���xmlbuf�ڵ������Ϣ
 *      ����:	xmlbuf:�����Ļ�����ָ��
 *				newtrig:��ǰ�Ķ�������״̬
 *				oldtrig:��ǰ�Ķ�������״̬
 *				time:�����仯��ʱ�䣬time_t����
 *      ���:   ��
 *      ����ֵ: xmlbuf�е���Ч�ֽ�������ֵ��ʾʧ��
 *********************************************************************************/
int fill_trigin_return_xml(OUT char *xmlbuf, IN DWORD newtrig, IN DWORD oldtrig, IN time_t time)
{
	int i;
	int oldbit=0, newbit=0;
	BYTE changed_trig[200];
	IXML_Document* doc;  
	IXML_Node  *root, *value;
 	IXML_Element *Ele, *info;
 	char buf[50];
 	char timestr[200];
 	struct tm *p;
 	
	if((xmlbuf == NULL))
		return -EINVAL;
	if(newtrig == oldtrig)
		return 0;
	
	sprintf(changed_trig, "<changed_trigs> </changed_trigs>");
    if(ixmlParseBufferEx(  changed_trig, &doc ) != IXML_SUCCESS )
    {
     	printf("parse %s failed\n", changed_trig);   
        return 0;
    }
   
   	
	p=localtime(&time);
   	sprintf(timestr,"%4d-%02d-%02d %02d:%02d:%02d",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
   	for(i=0; i<32; i++)
	{
		oldbit = (oldtrig>>i) & 1;
		newbit = (newtrig>>i) & 1;
		if(oldbit!=newbit)
		{
			root = ixmlNode_getFirstChild( ( IXML_Node * ) doc );
    		if(root == NULL)
    			printf("null!\n");
     		Ele = ixmlDocument_createElement( doc, "trig" );
      		ixmlNode_appendChild( root,(IXML_Node * ) Ele);
     		
     		info = ixmlDocument_createElement( doc, "id" );
     		ixmlNode_appendChild( (IXML_Node * )Ele,(IXML_Node * ) info);
      		sprintf(buf,"%d",i);
      		value = ixmlDocument_createTextNode(doc,buf );
      		ixmlNode_appendChild( (IXML_Node * )info, value);
      		
      		info = ixmlDocument_createElement( doc, "state" );
     		ixmlNode_appendChild( (IXML_Node * )Ele,(IXML_Node * ) info);
     		sprintf(buf,"%d",newbit);
      		value = ixmlDocument_createTextNode(doc,buf );
      		ixmlNode_appendChild( (IXML_Node * )info, value);
      		
      		info = ixmlDocument_createElement( doc, "time" );
     		ixmlNode_appendChild( (IXML_Node * )Ele,(IXML_Node * ) info);
      		value = ixmlDocument_createTextNode(doc,timestr);
      		ixmlNode_appendChild( (IXML_Node * )info, value);
   
   		}
	}
	
	sprintf(xmlbuf,ixmlDocumenttoString(doc));
	printf("\n\n\n");
	printf("length is %d, result is \n%s\n",strlen(xmlbuf),xmlbuf);
	printf("\n\n\n");

	
	//ixmlNode_free(value);
	//ixmlDocument_free(doc);
	
	
	//printf("here\n");
	return strlen(xmlbuf);

}


/**********************************************************************************
 *      ������: send_alarmin_state_change()
 *      ����:  �������Ͷ�������ı仯״̬
 *      ����:	old_alarmin���仯ǰ������״̬
 *				new_alarmin,�仯�������״̬
 *				time�������仯��ʱ��
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/

int send_alarmin_state_change( DWORD old_alarmin, DWORD new_alarmin,time_t time, int dev_no)

{
	DWORD send_buf[200];							//��Ӧ��������ᳬ��800�ֽ�
	struct gt_usr_cmd_struct *cmd=NULL;
	struct dev_query_trig_return *trig_return=NULL;	
	int rc=0;
	struct mod_com_type *modcom;
	
		
	memset(send_buf,0,sizeof(send_buf));			//��ʼ�����ͻ�����
	modcom = (struct mod_com_type *)send_buf;
	modcom->env = 0;
	modcom->enc = 0;
	
	cmd=(struct gt_usr_cmd_struct *)modcom->para;	//Ϊcmd�����ڴ�ռ�
	cmd->cmd=DEV_QUERY_TRIG_RETURN;
	cmd->en_ack=1;
	
	
	
	trig_return=(struct dev_query_trig_return *)((char *)cmd->para);		//Ϊtrig_return�����ڴ�ռ�
	rc=get_devid(trig_return->dev_id);	
	
	trig_return->result=0;
	trig_return->reserve1=0;
	trig_return->alarmin = new_alarmin;
	
	trig_return->changed_info_len = fill_trigin_return_xml(trig_return->changed_info,new_alarmin,old_alarmin, time);
	if(trig_return->changed_info_len <0) //failed
	trig_return->changed_info_len = 0;
	
	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_DEV_QUERY_TRIG_RETURN+trig_return->changed_info_len-4;
	
	set_trigin_flag(dev_no,0);
	rc=	send_gate_pkt(-1,modcom,cmd->len+2,dev_no);

	printf("%s��Զ�˷��������������������״̬�仯,0x%08x->0x%08x,rc=%d\n",devlog(dev_no),old_alarmin,new_alarmin,rc);
	gtloginfo("%s��Զ�˷��������������������״̬�仯,0x%08x->0x%08x,rc=%d\n",devlog(dev_no),old_alarmin,new_alarmin,rc);

	return 0;



}

/**********************************************************************************
 *      ������: send_query_trigstate_return()
 *      ����:  ���Ͳ�ѯ�豸�������뼰�仯״̬��Ӧ
 *      ����:	fd		�û����ӵ�tcp���
 *				result	�������
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
int send_query_trigstate_return(int fd,DWORD result,int env,int enc,int dev_no)
{
	DWORD send_buf[200];							//��Ӧ��������ᳬ��800�ֽ�
	struct gt_usr_cmd_struct *cmd=NULL;
	struct dev_query_trig_return *trig_return=NULL;	
	int rc;
	struct gt_pkt_struct *send;
	
		
	memset(send_buf,0,sizeof(send_buf));			//��ʼ�����ͻ�����
	send = (struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;	//Ϊcmd��ɢ�ڴ�ռ�
	cmd->cmd=DEV_QUERY_TRIG_RETURN;
	cmd->en_ack=0;
		
	trig_return=(struct dev_query_trig_return *)((char *)cmd->para);		//Ϊtrig_return�����ڴ�ռ�
	rc=virdev_get_devid(dev_no,trig_return->dev_id);	

	
	trig_return->result=result;
	trig_return->reserve1=0;

	trig_return->alarmin = get_ip1004_state(dev_no)->alarmin_state;

	trig_return->changed_info_len = 0;
		
	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_DEV_QUERY_TRIG_RETURN+trig_return->changed_info_len-4;
	
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
	
	printf("��Զ�˷��������� ��ѯ%s��������״̬�����%d,״̬0x%08x,rc=%d\n",devlog(dev_no),result,trig_return->alarmin,rc);
	gtloginfo("��Զ�˷��������� ��ѯ%s��������״̬�����%d,״̬0x%08x,rc=%d\n",devlog(dev_no),result,trig_return->alarmin,rc);

	return 0;
}

/**********************************************************************************
 *      ������: usr_query_trigstate()
 *      ����:   ��ѯ�豸����״̬
 *      ����:	fd		�û����ӵ�tcp���
 *				cmd		����ṹ
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/

static int usr_query_trigstate(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)

{
	struct sockaddr_in peeraddr;
	int i;
	
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_TRIGSTATE )
	{
		return -1;
	}


#ifdef SHOW_WORK_INFO
		printf("ipmain recv USR_QUERY_TRIGSTATE cmd for DEV %d!\n",dev_no);
#endif	
		gtloginfo("%s(fd=%d)���� ��ѯ%s��������״̬����0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,devlog(dev_no),USR_QUERY_TRIGSTATE );




	if(virdev_get_trigin_num(dev_no)==0)
		send_query_trigstate_return(fd,ERR_DVC_NO_TRIG,env,enc,dev_no);
	else
		send_query_trigstate_return(fd,RESULT_SUCCESS,env,enc,dev_no);

	return 0;

}


/**********************************************************************************
 *      ������: send_query_hd_return()
 *      ����:   ���Ͳ�ѯ�豸Ӳ�̲���������Ӧ
 *      ����:	fd		�û����ӵ�tcp���
 *				diskno	Ӳ�̺�
 *				result	�������
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int send_query_hd_return(int fd,WORD diskno,DWORD result,int env,int enc,int dev_no)
{
//lc

	DWORD send_buf[30];							//��Ӧ��������ᳬ��30*4�ֽ�
	struct dev_query_hdstatus_return *dev_hdstatus=NULL;
	struct gt_usr_cmd_struct *cmd=NULL;
	struct gt_pkt_struct *send=NULL;
	int rc;
	int ret;
	int short_percent_done = 0, long_percent_done = 0;
	unsigned int error_flag=0;
	
	memset(send_buf,0,sizeof(send_buf));			//��ʼ�����ͻ�����
	send=(struct gt_pkt_struct *)send_buf;		//Ϊsend�����ڴ�ռ�
	cmd=(struct gt_usr_cmd_struct *)send->msg;	//Ϊcmd��ɢ�ڴ�ռ�

	cmd->cmd=DEV_QUERY_HDSTATUS_RETURN;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
		
	dev_hdstatus=(struct dev_query_hdstatus_return *)((char *)cmd->para);		//Ϊad_para�����ڴ�ռ� 
	rc=virdev_get_devid(dev_no,dev_hdstatus->dev_id);	
	if(rc<0)
	{
		gtlogerr(" ���Ͳ�ѯ�豸Ӳ�̲���������Ӧʱ,��ȡdev_id ʧ�ܣ�������:%d\n",rc);
		error_flag |=0x01;
	}
	
	dev_hdstatus->result=result;
	dev_hdstatus->diskno=diskno;	

	ret=get_hd_info(diskno,dev_hdstatus->model,dev_hdstatus->serialno,dev_hdstatus->firmware);
	if(ret<0)
	{
		gtlogerr("��ȡӲ�̲���:�ͺţ����кţ��̼��汾ʧ�ܣ�������:%d\n",ret);
		error_flag |=0x02;
	}
	
	ret=get_hd_volume_inGiga(diskno);
	dev_hdstatus->volume=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ���������󣬴�����:%d\n",ret);
		error_flag |=0x04;
	}	
	ret=get_hd_temperature(diskno);
	dev_hdstatus->temprature=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ���¶ȴ��󣬴�����:%d\n",ret);
		error_flag |= 0x08;
	}

	ret=get_hd_max_temperature(diskno);
	dev_hdstatus->maxtemprature=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ����ʷ����¶ȴ��󣬴�����:%d\n",ret);
		error_flag |= 0x10;
	}

	ret=get_hd_runninghour(diskno);
	dev_hdstatus->age=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ�̹���Сʱ�����󣬴�����:%d\n",ret);
		error_flag |= 0x20;
	}

	ret=get_hd_relocate_sector(diskno);
	dev_hdstatus->relocate=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ���ط������������󣬴�����:%d\n",ret);
		error_flag |= 0x40;
	}

	ret=get_hd_pending_sector(diskno);
	dev_hdstatus->pending=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ�̵�ǰ�������������󣬴�����:%d\n",ret);
		error_flag |= 0x80;
	}

	ret=get_hd_errorlog_num(diskno);
	dev_hdstatus->error_no=ret;
	if(ret<0)
	{
		gtlogerr("��ȡӲ�̴�����־�����󣬴�����:%d\n",ret);
		error_flag |=0x100;
	}

	ret=get_hd_shorttest_result(diskno, &short_percent_done);
	dev_hdstatus->shorttest=ret;
	gtloginfo("��ȡӲ��[��]���Խ��:%s",get_testresult_str(ret));
	if(ret==TEST_RUNNING)
	{
		gtloginfo("Ӳ��[��]���Խ���...%d%%\n",short_percent_done);
	}

	dev_hdstatus->shortstatus = short_percent_done;
	
	ret=get_hd_longtest_result(diskno,&long_percent_done);
	dev_hdstatus->longtest=ret;
	gtloginfo("��ȡӲ��[��]���Խ��:%s",get_testresult_str(ret));
	if(ret==TEST_RUNNING)
	{
		gtloginfo("Ӳ��[��]���Խ���...%d%%\n",short_percent_done);
	}

	dev_hdstatus->longstatus = long_percent_done;

	//����error_flag�����жϲ��Խ��
	if(error_flag!=0)
	{
		//������Ӳ��״̬��������ʱ���ش���
		dev_hdstatus->result=ERR_ENC_NOT_ALLOW;
	}

	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+SIZEOF_DEV_QUERY_HDSTATUS_RETURN;
	printf("gt_cmd_pkt,,,env=%d,enc=%d\n",env,enc);

	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);

	gtloginfo("��Զ�˷�������ӦӲ�̲�����ѯ�����%d,rc=%d\n",result,rc);

	return 0;
}

/**********************************************************************************
 *      ������: usr_query_hd()
 *      ����:   ��ѯ�豸����״̬
 *      ����:	fd		�û����ӵ�tcp���
 *				cmd		����ṹ
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int usr_query_hd(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{	
	struct	usr_query_hd_status *usr_query_hd=NULL;
	struct sockaddr_in peeraddr;
	int i;
	int disk_num;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_HD_STATUS)
	{
		return -1;
	}

	
#ifdef DISPLAY_REC_NETCMD
		printf("ipmain recv USR_QUERY_HD_STATUS cmd!\n");
#endif

	//�����û�����
	usr_query_hd=(struct usr_query_hd_status *)cmd->para;
	disk_num=get_disk_no();

	gtloginfo("%s(fd=%d)������ѯ�豸Ӳ�̲�������0x%04x,Ӳ�̺�[%d],�豸Ӳ����Ϊ[%d]\n",inet_ntoa(peeraddr.sin_addr),fd,USR_QUERY_HD_STATUS,usr_query_hd->diskno,disk_num);

	if(get_hd_type()!=1) //����Ӳ��
	{
		//������Χ
		gtlogerr("���豸����Ӳ�̣��޷���ѯ\n",usr_query_hd->diskno,disk_num);
		send_gate_ack(fd,USR_QUERY_HD_STATUS,ERR_ENC_NOT_ALLOW,env,enc,dev_no);
		return 0;
	}

	// check disk_no
	if((usr_query_hd->diskno+1)>disk_num)
	{
		//������Χ
		gtlogerr("Ӳ�̺�:[%d]������Χ,Ӳ����Ϊ[%d]\n",usr_query_hd->diskno,disk_num);
		send_gate_ack(fd,USR_QUERY_HD_STATUS,ERR_ENC_NOT_ALLOW,env,enc,dev_no); 
		//send_query_hd_return(fd,usr_query_hd->diskno,ERR_ENC_NOT_ALLOW,env,enc); //wsy,����Ҫȥ����Щ����
		return 0;
	}

	send_query_hd_return(fd,usr_query_hd->diskno,RESULT_SUCCESS,env,enc,dev_no);
	return 0;
}


static int usr_query_device_return(int fd,struct gt_usr_cmd_struct *netcmd, int env, int enc,int dev_no)
{
//lc 
//	process_usr_query_position(fd,netcmd,env,enc,dev_no);
//	send_gate_ack(fd,USR_QUERY_DEVICE_POSITION,RESULT_SUCCESS, env,enc,dev_no);
	return 0;
}
/**********************************************************************************
 *      ������: usr_run_hd_test()
 *      ����:   	Ӳ��״̬����,ͨ����������豸�ϵ�Ӳ�̵�
 *				smart״̬����
 *      ����:	fd		�û����ӵ�tcp���
 *				cmd		����ṹ
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int usr_run_hd_test(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
//lc

	struct usr_run_hd_test *usr_hd_test=NULL;	
	struct sockaddr_in peeraddr;
	int i;
	int ret;
	int disk_num=0;
	int err_flag=0;
	WORD result;
		
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_RUN_HD_TEST)
	{
		return -1;
	}
	
#ifdef DISPLAY_REC_NETCMD
	printf("ipmain recv USR_RUN_HD_TEST cmd!\n");
#endif
	
	
	
	//�����û�����
	usr_hd_test=(struct usr_run_hd_test *)cmd->para;
	gtloginfo("%s(fd=%d)��������Ӳ��״̬��������0x%04x,Ӳ�̺�%d,�豸Ӳ����Ϊ[%d]��������:%d\n",inet_ntoa(peeraddr.sin_addr),fd,USR_RUN_HD_TEST,usr_hd_test->diskno,disk_num,usr_hd_test->testtype);

	if(get_hd_type()!=1) //����Ӳ��
	{
		gtloginfo("���豸����Ӳ�̣��޷�����\n");
		send_gate_ack(fd,USR_RUN_HD_TEST,ERR_DVC_NO_DISK,env,enc,dev_no);	
		//return 0;
		return -1;	//zw-081222
	}	
	
	
	// check disk_no
	disk_num=get_disk_no();
	if((usr_hd_test->diskno+1)>disk_num)
	{
		//������Χ
		gtlogerr("Ӳ�̺�:[%d]������Χ,Ӳ����Ϊ[%d]\n",usr_hd_test->diskno,disk_num);
		//send_gate_ack(fd,USR_RUN_HD_TEST,ERR_DVC_INTERNAL,env,enc);
		send_gate_ack(fd,USR_RUN_HD_TEST,ERR_ENC_NOT_ALLOW,env,enc,dev_no);
		//return 0;
		return -1;	//zw-081222
	}
	
	//lc
	ret= run_hd_smarttest(usr_hd_test->diskno,usr_hd_test->testtype);
	if(ret==0)
	{
		result=RESULT_SUCCESS;
	}
	else
	{
		result=ERR_DVC_INTERNAL;
		send_gate_ack(fd,USR_RUN_HD_TEST,result,env,enc,dev_no);
		err_flag=1;
		gtlogerr("����Ӳ��״̬����ʧ�ܣ�������:%d\n",ret);
	}
	gtloginfo("����Ӳ��״̬��������ִ�н������:0x%04x\n",result);

	if(cmd->en_ack!=0)
	{
		if(err_flag==0)
		{
			return send_gate_ack(fd,USR_RUN_HD_TEST,result,env,enc,dev_no);
		}
	}

	return 0;
}


/**********************************************************************************
 *      ������: usr_query_regist()
 *      ����:  	ͨ���������ѯע����Ϣ���豸�յ���Ӧ����ע��
 *				�������ע��
 *      ����:	fd		�û����ӵ�tcp���
 *				cmd		����ṹ
 *				env		���յ�������ʹ�õ������ŷ��ʽ
 *				enc		�ս��յ�������ʹ�õļ�������
 *      ���:   ��
 *      ����ֵ:0�ɹ�����ֵʧ��
 *********************************************************************************/
static int usr_query_regist(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
	struct usr_query_regist *usr_regist=NULL;
	struct sockaddr_in peeraddr;
	int i;
	int ret;
		
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_REGIST )
	{
		return -1;
	}
#ifdef SHOW_WORK_INFO
	printf("ipmain recv USR_QUERY_REGIST  cmd!\n");
#endif
		
	//�����û�����
	usr_regist=(struct usr_query_regist *)cmd->para;

#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d)����ע���ѯ����0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,USR_QUERY_REGIST);
#endif
	gtloginfo("%s(fd=%d)����ע���ѯ����0x%04x\n",inet_ntoa(peeraddr.sin_addr),fd,USR_QUERY_REGIST);

	ret=system_regist(fd,0,env,enc,dev_no);

	if(ret<0)
	{
#ifdef SHOW_WORK_INFO
		printf("�豸ע��ʧ�ܣ�������:%d\n",ret);
#endif
		gtlogerr("�豸ע��ʧ�ܣ�������:%d\n",ret);
	}

	return 0;
}

static int usr_subscribe_record(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=VIEWER_SUBSCRIBE_RECORD))
		return -1;	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	viewer_subscribe_record_struct* query = (viewer_subscribe_record_struct*)cmd->para;

	if(	strcmp(query->peer_ip,"0.0.0.0") == 0)
	{
		sprintf(query->peer_ip,"%s",inet_ntoa(peeraddr.sin_addr));
	}
	
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d) �����豸¼��\n",inet_ntoa(peeraddr.sin_addr),fd);
#endif
	gtloginfo("%s(fd=%d) �����豸¼��\n",inet_ntoa(peeraddr.sin_addr),fd);

	return bypass_hdpb_cmd(fd, cmd, env, enc, dev_no);

}

static int usr_subscribe_rtimage(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	if((fd<0)||(cmd->cmd!=USR_REQUIRE_RT_IMAGE))
		return -1;	
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	struct usr_req_rt_img_struct* query = (struct usr_req_rt_img_struct*)cmd->para;

	//if(	strcmp(query->remoteip,"0.0.0.0") == 0)
	//{
		//sprintf(query->remoteip,"%s",inet_ntoa(peeraddr.sin_addr));
	//}
	memcpy((void*)&query->remoteip,(void*)&peeraddr.sin_addr.s_addr,sizeof(DWORD));
	
#ifdef SHOW_WORK_INFO
	printf("%s(fd=%d) �����豸ʵʱ\n",inet_ntoa(peeraddr.sin_addr),fd);
#endif
	gtloginfo("%s(fd=%d) �����豸ʵʱ\n",inet_ntoa(peeraddr.sin_addr),fd);

	return bypass_rtimg_cmd(fd, cmd, env, enc, dev_no);

}

/**********************************************************************************************
 * ������	:process_netcmd()
 * ����	:����Զ�̼��������������
 * ����	:fd:�����ӵ�socket������
 *			 cmd:�ս��յ�������ṹָ��
 *			 env:���յ�������ʹ�õ������ŷ��ʽ
 *			 enc:�ս��յ�������ʹ�õļ�������
 *����ֵ	:��
 **********************************************************************************************/

void process_netcmd(int fd,struct gt_usr_cmd_struct* cmd,int env,int enc,int dev_no)
{
	
	int i;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
//#ifdef SHOW_WORK_INFO
	printf("recv cmd[fd=%d]:0x%04x\n",fd,cmd->cmd);
//#endif

	switch(cmd->cmd)
	{		
		case USR_REQUIRE_RT_IMAGE:
			//bypass_rtimg_cmd(fd,cmd,env,enc,dev_no);//��Ҫʵʱͼ��ģ�鴦��
			usr_subscribe_rtimage(fd,cmd,env,enc,dev_no);
		break;
		case USR_STOP_RT_IMAGE:
			bypass_rtimg_cmd(fd,cmd,env,enc,dev_no);//��Ҫʵʱͼ��ģ�鴦��
		break;
		case USR_REQUIRE_SPEAK:
			//gtloginfo("%s(fd=%d)����������Ƶ�´�����0x0107\n",inet_ntoa(peeraddr.sin_addr),fd);
			usr_require_speak_cmd(fd,cmd,env,enc,dev_no);
		break;
		case USR_SET_AUTH_IP:
			usr_set_gate_ip(fd,cmd,env,enc,dev_no);
		break;//o
		case USR_CLOCK_SETING:
			usr_set_clock(fd,cmd,env,enc,dev_no);
		break;
#ifdef QUAD_CTRL
		case USR_SCREEN_SETING:
			usr_net_scr_ctl(fd,cmd,env,enc,dev_no);
		break;
		case USR_SET_VIDEO_AD_PARAM:
			usr_set_video_para(fd,cmd,env,enc,dev_no);//
		break;
		case USR_SET_SENSER_ARRAY:	//��Ҫʵʱͼ��洢ģ�鴦��
			usr_set_senser_array(fd,cmd,env,enc,dev_no);
		break;
#endif
		case USR_NET_STREAM_SETING://��Ҫʵʱͼ��ģ�鴦��
			usr_set_net_avstream(fd,cmd,env,enc,dev_no);
		break;
		//lc do
		case USR_SET_SWITCH_IN:
			usr_set_trigger_in(fd,cmd,env,enc,dev_no);//o notest
		break;
		//lc to do ���ñ����������
		/*
		case USR_SET_SWITCH_OUT:
			usr_set_alarm_out(fd,cmd,env,enc,dev_no);//o notest	
		break;
		*/
		case USR_SWITCH_OUT:
			usr_alarm_ctl(fd,cmd,env,enc,dev_no);
		break;		
#ifdef HQMODULE_USE
		case USR_LOCAL_STREAM_SETING:
			usr_local_avstream_set(fd,cmd,env,enc,dev_no);
		break;

		case USR_LOCAL_RECORDER_SETING:
			usr_local_record_set(fd,cmd,env,enc,dev_no);//o notest
		break;
		case USR_LOCAL_RECORDER_CONTROL:
			usr_local_recorder_ctl_cmd(fd,cmd,env,enc,dev_no);
		//��Ҫ������¼��ģ�鴦��
		break;
#endif
		case USR_CANCEL_ALARM:
			usr_cancel_alarm_cmd(fd,cmd,env,enc,dev_no);	
		break;
		case START_ALARM_ACTIONS_YES:
			usr_start_alarm_actions_yes_cmd(fd,cmd,env,enc,dev_no);
		break;	
		case START_ALARM_ACTIONS:
				usr_start_alarm_actions_cmd(fd,cmd,env,enc,dev_no);
		break;
	
		case USR_SEIRAL_PORT_CONTROL:
				process_serial_ctrl(fd,cmd,env,enc,dev_no);
		break;
		//��ѯͼ��A/D ����add-20071114
		case USR_QUERY_VIDEO_AD_PARAM:
				usr_query_video_ad_param(fd,cmd,env,enc,dev_no);
		break;

		//��ѯ����״̬	add-20071115
		case USR_QUERY_TRIGSTATE:
				usr_query_trigstate(fd,cmd,env,enc,dev_no);
		break;

		//Ӳ��״̬������ѯadd-20071115
#ifdef HQMODULE_USE
		case USR_QUERY_HD_STATUS:
				usr_query_hd(fd,cmd,env,enc,dev_no);
		break;

		//����Ӳ��״̬����add-2007115
		case USR_RUN_HD_TEST :
				usr_run_hd_test(fd,cmd,env,enc,dev_no);
		break;
#endif
		//ע���ѯadd-20071115
		case USR_QUERY_REGIST:
				usr_query_regist(fd,cmd,env,enc,dev_no);
		break;
			
		case USR_QUERY_STATE:
				usr_query_state_cmd(fd,cmd,env,enc,dev_no);
		break;
		
		case USR_QUERY_INDEX:

				usr_query_index_cmd(fd,cmd,env,enc,dev_no);
		break;
		
		case USR_QUERY_FTP:
				usr_query_ftp_info_cmd(fd,cmd,env,enc,dev_no);
		break;
		case USR_REBOOT_DEVICE:
				usr_reboot_device_cmd(fd,cmd,env,enc,dev_no);
		break;
		case USR_CMD_ACK:
				usr_ack_cmd(fd,cmd,env,enc,dev_no);	
		break;
		//lc do �豸��������
		
		case USER_UPDATE:              //�������
				usr_update_software_cmd(fd,cmd,env,enc,dev_no);
		break;
		case UPDATE_SOFTWARE_DIRECT: //ֱ�Ӹ��豸����
				update_sw_direct(fd,cmd,env,enc,dev_no);
		break;
		

		//case USR_TAKE_HQ_PIC:
		//		usr_take_hq_pic_cmd(fd,cmd,env,enc,dev_no);
		//break;
		case USR_RW_DEV_PARA://���������ļ�
				usr_rw_para_file(fd,cmd,env,enc,dev_no);
		break;
#ifdef	HQMODULE_USE	
		case USR_LOCK_FILE_TIME: //��ָ��ʱ��εĸ������ļ����������
				usr_lock_file_time_cmd(fd,cmd,env,enc,dev_no);
		break;
#endif		
		case USR_SET_ALARM_SCHEDULE: //���������趨������Чʱ���
				usr_set_alarm_schedule(fd,cmd,env,enc,dev_no);
		break;
		case USR_LOGIN_DEVICE://�û���¼�豸
				usr_login_device_cmd(fd,cmd,env,enc,dev_no);
		break;
		case USR_REQUIRE_SELF_IP://�û���ѯ�Լ���ip��ַ
				usr_require_self_ip_cmd(fd,cmd,env,enc,dev_no);
		break;	
#ifdef	HQMODULE_USE		
		case USR_FORMAT_HARDDISK://�û�Զ�̸�ʽ������
				usr_format_hd(fd,cmd,env,enc,dev_no);
		break;
#endif		
		//case USR_QUERY_DEVICE_POSITION://�û���ѯ�豸����λ��
		//		usr_query_device_return(fd,cmd,env,enc,dev_no);
		//break;

		// lc 2013-12-24 ¼��㲥��ع��ܣ�͸��
#ifdef HDPLAYBACK_USE
		case USR_QUERY_TIMESECTION:           //�û���ѯ¼��ʱ�������
		case VIEWER_SUBSCRIBE_RECORD_CONTROL: //�û������в��ſ���
		case VIEWER_UNSUBSCRIBE_RECORD:       //�û�ֹͣ����
				bypass_hdpb_cmd(fd,cmd,env,enc,dev_no);
		break;
		case VIEWER_SUBSCRIBE_RECORD:         //�û�����¼��㲥
				usr_subscribe_record(fd,cmd,env,enc,dev_no);
		break;

#endif
		default:
			printf("ipmain recv unknown gatecmd:0x%04x\n",cmd->cmd);	
			gtlogwarn("%s������֧�ֵ���������0x%04x\n",inet_ntoa(peeraddr.sin_addr),cmd->cmd);
			send_gate_ack(fd,cmd->cmd,ERR_EVC_NOT_SUPPORT,env,enc,dev_no);	
		break;
	}

}





