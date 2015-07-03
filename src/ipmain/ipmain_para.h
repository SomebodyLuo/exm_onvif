#ifndef IPMAIN_PARA_H
#define IPMAIN_PARA_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iniparser.h>
#include "devinfo.h"
#include "display_osd.h"
#include "video_para_api.h"
#include "mod_socket.h"

extern int GATE_PERIOD_TIME;
#define MAX_DEV_NO 1

/*#	����������Ӧ��
#	����	����

#	0 	��Ч
#	1	����0���1
#	2	����0���0
#	3	����1���1	
#	4	����1���0
#	5	����2���1
#	6	����2���0
#	7	����3���1
#	8	����3���0
#	9	����ͷ��4�ָ�
#	10	����ͷ��0ͨ��ȫ��
#	11	����ͷ��1ͨ��ȫ��
#	12	����ͷ��2ͨ��ȫ��
#	13	����ͷ��3ͨ��ȫ��
*/
struct alarm_trigin_struct
{								//����һ�����������¼��Ľṹ
	int trigin; 					//����ͨ����
	int enable;					//�Ƿ���Ч
	int setalarm; 				// 1Ϊ������0Ϊ����
	int alarm;					//�Ƿ񱨾�,1Ϊ�ǣ�0Ϊ��
	int imact[MAX_TRIG_EVENT]; 	//����ִ��
	int ackact[MAX_TRIG_EVENT];    //ȷ��ִ�У�     
	int rstact[MAX_TRIG_EVENT];	//��λִ��
	int	starthour;				//wsy add,������Ч��ʼʱ��
	int startmin;
	int endhour;				//������Ч����ʱ��
	int endmin;
};

struct alarm_motion_struct
{	//ϵͳ���Դ���ı����¼������ṹ
	//���������Ķ˿ڣ���6���������0~5��4���ƶ����6~9	
	struct alarm_trigin_struct trigin[MAX_TRIG_IN];
	struct alarm_trigin_struct motion[MAX_VIDEO_IN];
	int audioalarm;     //�Ƿ���Ҫ������ʾ,1Ϊ��Ҫ,0Ϊ����Ҫ
    int playback_enable;    //�Ƿ���Ҫ�˾���¼��طţ�1Ϊ��Ҫ��0Ϊ����Ҫ  //zw-add 2012-04-24
    //lc do ���Ӷ��Ӵ���������Ӧ��Ƶӳ���ϵ
    int alarm_trigin_video_ch[MAX_TRIG_IN];
};


typedef struct{

	struct sockaddr_in *rmt_gate0;		//Զ�̶�̬���ط�������ַ
	struct sockaddr_in *rmt_gate1;		//��̬Զ�����ط�������ַ
	struct sockaddr_in *rmt_gate2;			
	struct sockaddr_in *rmt_gate3;
	struct sockaddr_in *rmt_gate4;
	struct sockaddr_in *alarm_gate;		//�����������ط�������ַ
	BYTE	inst_place[100];		//��װ�ص�
	struct osd_init_struct osd;		//osd���
	WORD    cmd_port;           	//�������˿�
	int		ip_chg_flag;			//ip��ַ�仯��־
	int 	sendgateini_flag;		//��Ҫ���͸����������ļ�
}dev_para_struct;

//�����̵Ĳ����ṹ
struct ipmain_para_struct
{
	 dev_para_struct	devpara[MAX_DEV_NO];
	 int		ini_version;				//ini�����ļ��İ汾��
	 int		total_com;				//����ʹ�õ�͸��������
	 in_addr_t lan_addr;				//������ip��ַ
	 in_addr_t lan_mask;				//��������
	 in_addr_t wan_addr;				//����ip��ַ
	WORD	rmt_env_mode;			//��Զ�̷�����ͨѶʹ�õ�����ǩ������
	WORD	rmt_enc_mode;			//��Զ�̷�����ͨѶ����ʹ�õļ�������
	WORD	trig_in;					//������������ (��λ��ʾͨ��)0���� 1����

	WORD	tin_mask;				//������������(��λ��ʾͨ��)
	
	WORD	alarm_out;				//�����������0���� 1����(��λ��ʾͨ��)
	WORD	alarm_mask;				//�����������
	
	struct alarm_motion_struct alarm_motion; 	//wsy add ���������ṹ
	struct video_para_struct vadc;		//��ƵadcоƬ��2835��9910
	//int	   hq_follow_net;						//��ֻ�е�·������ѹ��ʱ,������ͨ����������ͨ��ͼ��仯
	//int	   hq_save_ch;						//������¼���ͨ��(�зָ���ʱ����Ч)
	int	   net_ch;								//�������Ƶͨ��
	int    alarm_playback_ch;                 //��ǰ�˾��ط�ͨ������ʱΪ����ͨ���ţ���ʱ��Ϊ-1
	
	WORD	video_in_ch;					//ϵͳ֧�ֵ������Ƶ������
	WORD	video_mask;					//��Ƶ������Чλ����(��Ӱ�쵽��Ƶ��ʧ����)
	WORD    image_port;						//ͼ�����˿�
	WORD    audio_port;        					//��Ƶ����˿�
	WORD	telnet_port;						//telnet����˿�
	WORD	ftp_port;						//ftp����˿�	
	WORD	web_port;						//web����ҳ�����˿�
	WORD	com0_port;						//͸������0�˿ں�
	WORD	com1_port;						//͸������1�˿ں�
	WORD    pb_port;                        //¼��㲥�˿�
	WORD    dev_type;      					//�豸����,0Ϊ��2824��1Ϊû��

	int		inst_ack;						//��װ��ȷ�ϱ�־ 0��ʾδ����ʽ��װ 1��ʾ�Ѿ�����ʽ��װ,δ����ʽ��װʱ����Ҫ֤��Ҳ���Է���
	int		valid_cert;						//�Ѿ��кϷ�֤���־
	int     	reset_modem; 					//�Ƿ�����è,1��ʾ��
	int 		reset_modem_time; 				//����è�ļ��������
	int     	internet_mode; 					//������ʽ,0-adsl, 1-ר��, 2-������
	//int		log_watch_lost;					//��¼���ܽ��յ�watchģ����Ϣ
	//int 	snap_file_interval;				//����ץͼ�ļ��
	//int  	snap_file_num;					//����ץͼ������
	//char   snap_file_path[100];			//����ץͼ��·��
	int 	net_ch_osd_picsize;			//����������ߴ�
	//int		local_ch_osd_picsize;		//���ر������ߴ�
	int     multi_channel_enable;         //�Ƿ����ö�ͨ��
	int     current_audio_down_channel;  //��ǰ����ͨ����
	int     power_monitor_enable;
	int     buzzer_alarm_enable;

	//lc add 2014-10-20
	int     gatedown_relay_count;
	int     gatedown_relay_chn;
};

char * devlog(int dev_no);

typedef struct {//
	pthread_mutex_t mutex;
	int 	pb[20];	//
}Alarm_Playback_Tag;

Alarm_Playback_Tag pb_Tag;

///�����ļ�����ȡ���ļ��Ĵ��·����������
const char *get_audio_file(char *name);

/**********************************************************************************************
 * ������	:init_para()
 * ����	:ϵͳ������ʼ��������������ΪĬ��ֵ
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void  init_para(void);

/**********************************************************************************************
 * ������	:read_config_ini_file()
 * ����	:��config�ļ��еĲ�����Ϣ��ȡ�������ṹ��
 * ����	:filename:�����ļ���
 * ���	:para:����ʱ�������Ľṹָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int read_config_ini_file(char *filename, struct ipmain_para_struct *para);

/**********************************************************************************************
 * ������	:readmain_para_file()
 * ����	:��ip1004.ini�ļ��еĲ�����Ϣ��ȡ�������ṹ��
 * ����	:filename:�����ļ���
 * ���	:para:����ʱ�������Ľṹָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int  readmain_para_file(char *filename,struct ipmain_para_struct *para);

/**********************************************************************************************
 * ������	:refresh_netinfo()
 * ����	:ˢ������״̬����
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void refresh_netinfo(void);

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
int save_setalarm_para(dictionary *ini,int type, int ch,int setalarm, int starthour, int startmin, int endhour, int endmin);

/**********************************************************************************************
 * ������	:save_video_para()
 * ����	:��������ʽ��ֵ����ini�ṹ�е���Ƶ����������
 * ����	:
 *			 ch:��Ƶͨ����
 *			 type:���������� "bright"...
 *			 val:����ֵ
 * ���	: ini:����ini�ļ��Ľṹָ��,����ʱ�������ֵ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int save_video_para(dictionary      *ini,int ch,char *type,int val);

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
int save_video_para_hex(dictionary *ini,int ch,char *type,int val);

/**********************************************************************************************
 * ������	:dump_para_to_file()
 * ����	:���ڴ��д洢�Ĳ������ı��ķ�ʽ�����һ���Ѵ򿪵��ļ���
 * ����	:��
 * ���	:�Ѵ򿪵��ļ�ָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int dump_para_to_file(FILE *fp);


/**********************************************************************************************
 * ������	:CopyPara2Bak()
 * ����	:�������ļ����µ���Ӧ�ı���
 * ����	:type:�����ļ�����2:ip1004.ini 3:alarm.ini
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int CopyPara2Bak(int dev_no,int type);
	
/**********************************************************************************************
 * ������	:CheckParaFileChange()
 * ����	:��������ļ��Ƿ��б仯
 * ����	:type��ʾ�����ļ�����  2:ip1004.ini 3:alarm.ini
 * ����ֵ	:����0��ʾ�ޱ仯  1��ʾ�б仯 -1��ʾ���ʹ���
 **********************************************************************************************/
int CheckParaFileChange(int dev_no,int type);
/**********************************************************************************************
 * ������	:AddParaFileVersion()
 * ����	:�������ļ��İ汾�ż�1
 * ����	:type��ʾ�����ļ�����  2:ip1004.ini 3:alarm.ini
 * ����ֵ	:�µİ汾��,��ֵ��ʾʧ��
 **********************************************************************************************/
int AddParaFileVersion(int dev_no,int type);
/**********************************************************************************************
 * ������	:get_internet_mode_str()
 * ����	:��ȡinternet���ӷ�ʽ�ַ���
 * ����	:��
 * ����ֵ	:����internet���ӷ�ʽ���ַ���
 **********************************************************************************************/
char *get_internet_mode_str(void);

/**********************************************************************************************
 * ������	:get_mainpara()
 * ����	:��ȡ�豸�����ṹָ��
 * ����	:��
 * ����ֵ	:�豸�����ṹָ��
 **********************************************************************************************/
struct ipmain_para_struct * get_mainpara(void);

/**********************************************************************************************
 * ������	:get_serial_mode()
 * ����	:��ȡ����0����ģʽ
 * ����	:��
 * ����ֵ	:1:�������Ϊ����  0:�������Ϊ����
 **********************************************************************************************/
int get_serial_mode();

int get_serial_interval();

#endif






