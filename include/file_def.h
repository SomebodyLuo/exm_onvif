#ifndef FILE_DEF_H
#define FILE_DEF_H
#include <typedefine.h>
#include <stdio.h>
#include <string.h>
#include <gtlog.h>
#include "mod_socket.h"

//��·���ͺŶ���

#define INTIME_RDK5				0
#define GT1000_1_0				1
#define BOARD					GT1000_1_0

#define		ALARMIN_MASK			0xffff	// from 0xff lc //from 0x3f,wsy		//������������
#define		TOTAL_ALARMOUT_PORT  		4			//��󱨾����ͨ����
#define		ALARMOUT_MASK			0xff 		//fixme,������Ӹı�ʱ

#define		MAX_HQCHANNEL			5			//��������¼��ͨ����.�����ֻ������ռ���
#define		MAX_VIDEO_IN			10			//�����Ƶ������,�����ֻ������ռ���,��Ҫ�豸�ľ���ֵʱ����get_video_num()
#define		TOTAL_TRIG_IN			6			//����������
#define 	MAX_TRIG_IN				20 		////��󴥷�������Ŀ,�����ֻ������ռ���,��Ҫ�豸�ľ���ֵʱ����get_trigin_num()

#define 		MAX_TRIG_EVENT		10			//��·������󴥷��¼�����

#define 		MAX_FILE_LEN		30*1024			//�����ļ���󳤶� Զ���趨����ʱ�õ�

//��Ƶ��ʧ���ڵ����ƶ���ȡ��ʱ�䶨��,wsy
#define VLOSS_CANCEL_TIME	8	//��Ƶ��ʧ��ȡ��ʱ��
#define VMOTION_CANCEL_TIME	8	//��Ƶ�ƶ���ȡ��ʱ��
#if 0
#ifndef _WIN32
#include <syslog.h>

//����־��¼����
//name��ʾ��־��Ϣ�е�����
#define gtopenlog(name) openlog(name,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );//LOG_USER);

//#define gtlog  syslog		//ϵͳ��־��Ϣ��¼
#define gtlog syslog
//һ������Ϣ
#define gtloginfo(args...) syslog(LOG_INFO,##args)	//��¼һ����Ϣ
//���صĴ�����Ϣ
#define gtlogfault(args...) syslog(LOG_CRIT,##args)	//
//������Ϣ
#define gtlogerr(args...) syslog(LOG_ERR,##args)	//
//������Ϣ
#define gtlogwarn(args...) syslog(LOG_WARNING,##args)

#define gtlogdebug(args...) syslog(LOG_DEBUG,##args)
#endif //_WIN32
#endif


/*
	vsmain define
*/
#define		DEV_CERT_FILE		"/conf/cert/dev-peer.crt"			//�豸֤���ļ�,ͨѶʱ��ʹ��
#define		CERT_FILE			"/conf/cert/dev-gateway.crt"		//֤���ļ�
#define		CERT_BAK_FILE		"/conf/cert/dev-gateway-bak.crt"	//֤���ļ�����

#define		KEY_FILE			"/conf/cert/dev-peer.key"			//˽Կ�ļ�
#define		KEY_BAK_FILE		"/conf/cert/dev-peer-bak.key"		//˽Կ�ļ�����

#define    	TMP_AVI_LENGTH_FILE     	"/tmp/avi/avi_time.txt"

#define		IPMAIN_TMP_DIR			"/tmp"							//ipmain���ڴ����ʱ�ļ��ĵط�
#ifndef FOR_PC_MUTI_TEST
#define	 	DEVINFO_PARA_FILE		"/conf/devinfo.ini"					//����豸�̶���Ϣ���ļ�����Щ��Ϣһ�㲻���޸�
#define	 	IPMAIN_PARA_FILE		"/conf/ip1004.ini"	
#define	 	IPMAIN_PARA_GATE_BAK		"/conf/ip1004_gate.ini" 			//����ָʾ�仯�������ļ�����
#define     	MOTION_ALARM_PARA_FILE 		"/conf/ip1004.ini"
#define		MOTION_ALARM_GATE_BAK		"/conf/ip1004_gate.ini"				//����ָʾ�仯�������ļ�����
#define	 	DISK_INI_FILE           "/conf/diskinfo.ini"

#define     	CONFIG_FILE			"/conf/config"
#define  	IPMAIN_LOCK_FILE		"/lock/ipserver/ipmain"
#define         VMMAIN_LOCK_FILE		"/lock/ipserver/vmmain"
#define		QUAD_DEV			"/dev/quadev"
#define     	SIMCOM_DEV             		"/dev/simcom" 
#define     	LEDS_DEV			"/dev/leddrv"

#define		WATCH_BD_LOCK_FILE		"/lock/ipserver/watch_board"	//����51�İ汾��
#else
extern char	DEVINFO_PARA_FILE[];
extern char	VSMAIN_PARA_FILE[];
extern char VSMAIN_PARA_GATE_BAK[];
extern char MOTION_ALARM_PARA_FILE[];
extern char	MOTION_ALARM_GATE_BAK[];
extern char 	CONFIG_FILE[];
extern char	IPMAIN_LOCK_FILE[];
//#define		QUAD_DEV				"/dev/quadev"
//#define     	SIMCOM_DEV              	"/dev/simcom" 
//#define     	LEDS_DEV				"/dev/leds"
//#define		LOCAL6410_DRV			"/gt1000/drivers/ime6410_d1.o"
//#define		LOCAL6410_NAME		"ime6410_d1"
extern char WATCH_BD_LOCK_FILE[];

#endif

#define		MAIN_PARA_FILE_DEV0		"/conf/virdev/0/gt1000.ini"
#define		MAIN_PARA_FILE_DEV1		"/conf/virdev/1/gt1000.ini"
#define		MAIN_PARA_DEV0_GATE_BAK	"/conf/virdev/0/gt1000_bak.ini"
#define		MAIN_PARA_DEV1_GATE_BAK	"/conf/virdev/1/gt1000_bak.ini"
#define		ALARM_PARA_FILE		MOTION_ALARM_PARA_FILE
#define		LOCAL6410_DRV		"/gt1000/drivers/ime6410_d1.o"
#define		LOCAL6410_NAME		"ime6410_d1"
/*
	rtimage define
*/


#if EMBEDED
	#define  RT_6410_DEV			"/dev/IME6410"
#else
	#define	RT_6410_DEV			"/vserver/raw6410_cif.dat"
#endif

#define		NET6410_DRV			"/gt1000/drivers/ime6410_pcm.o"
#define		NET6410_NAME			"ime6410_pcm"
#define  		RTIMAGE_EXEC			"/gt1000/tcprtimg"

#ifndef FOR_PC_MUTI_TEST
#define	 	RTIMAGE_PARA_FILE		"/conf/ip1004.ini"//"/conf/rtimage.ini"
#define  		RT_LOCK_FILE			"/lock/ipserver/rtimage"
#else
extern char	RT_LOCK_FILE[];
extern char 	RTIMAGE_PARA_FILE[];
#endif

#ifndef FOR_PC_MUTI_TEST
#define	 	ENCBOX_PARA_FILE		"/conf/ip1004.ini"//"/conf/rtimage.ini"
#define  		ENC_LOCK_FILE			"/lock/ipserver/encbox"
#else
extern char	RT_LOCK_FILE[];
extern char 	RTIMAGE_PARA_FILE[];
#endif
/*
	hdsave define
*/

#define 	PARTITION_NODE1		"/hqdata/hda1"

#define     ALARM_SNAPSHOT_PATH    "/picindex/alarmpic.txt"  //����ץͼ���·��

#define		IMG_FILE_EXT			".AVI"	  	 //�ļ���չ��
#define		LOCK_FILE_FLAG			'@'      		 //�������
#define 		REMOTE_TRIG_FLAG		'#'		 	 //Զ�̴������
#define     	RECORDING_FILE_EXT      ".ING"    		//����¼��
#define     	OLD_FILE_EXT            	"_OLD.AVI"	//�ϴα���ϵ�
#define 		FIX_FILE_EXT			"_FIX.AVI" //���������avi�ļ�
#if EMBEDED
	#define		HD_MINVAL				250//wsy change from 50 //shixintest 50//50*1024		//ֹͣ����¼��Ŀռ�
	#define		HD_RMVAL				280//wsy change from 80  100*1024	//��ʼɾ���ļ��Ŀռ�,�����ռ�
       #define  HD_RM_STOP             450   //wsy change from  		150  //ɾ���ļ���ֹͣ��
	#define		CF_MINVAL				50
	#define		CF_RMVAL				80
	#define		CF_RM_STOP				150		//values for CF card
#else
	#define		HD_MINVAL				10240//50*1024		//�����ռ�
	#define		HD_RMVAL				24000//100*1024	//��ʼɾ���ļ��Ŀռ�
	#define     	RM_STOP                		23000
#endif
#define 		MAX_HQFILE_LEN		180	//����¼���зֳ���	

struct dir_info_struct   //Ŀ¼�ṹ
{
	int year;                  
	int month;
	int date;
	int hour;
	int min;
	int sec;
};


#ifndef FOR_PC_MUTI_TEST
#define     HDMOD_PARA_FILE         "/conf/ip1004.ini"//"/conf/rtimage.ini"
#define     HDMOD_LOCK_FILE         "/lock/ipserver/hdmodule"
#define     PLAYBACK_LOCK_FILE    "/lock/ipserver/playback"
#define     HDSAVE_PATH                "/hqdata"   //�������洢�ļ����·��
#else
extern char HDMOD_PARA_FILE[];
extern char HDMOD_LOCK_FILE[];
extern char HDSAVE_PATH[];			//�������洢�ļ����·��

#endif
#define  	HDMOD_EXEC			"/ip1004/hdmodule"
#if EMBEDED==1
#define		HDMOUNT_PATH			HDSAVE_PATH			//�ļ�ϵͳ�ҽ�·��
#define		HQDEV0		"/dev/sda1"
#define     HQDEV1		"/dev/sda2"   
#define     HQDEV2		"/dev/sda3"
#define     HQDEV3		"/dev/sda4"

#else
#define		HDMOUNT_PATH			HDSAVE_PATH			//�ļ�ϵͳ�ҽ�·��
#define	HQDEV0		"/vserver/noaudio.dat"
#define	HQDEV1		"/vserver/noaudio.dat"
#define	HQDEV2		"/vserver/noaudio.dat"
#define	HQDEV3		"/vserver/noaudio.dat"

#endif


/*
	diskman define
*/

#define  		DISKMAN_EXEC			"/ip1004/diskman"
#ifndef FOR_PC_MUTI_TEST
#define  		DISKMAN_LOCK_FILE		"/lock/ipserver/diskman"
#else
extern char DISKMAN_LOCK_FILE[];
#endif
#define RESULT_TXT        "/var/tmp/result.txt"  //lost+foundĿ¼����ʱ�Ľ��

#define HD_DEVICE_NODE		"/dev/hda"		//��һ��Ӳ���豸�ڵ�
#define HD_PART_NODE		"/dev/hda1"		//Ӳ��1�����ڵ�1

#define CFERR_PERCENT		80		//������������Ӳ�������ٷֱ�������ڴ�ֵ����Ӳ�̹���		

#define FIXDISK_LOG_FILE   "/var/tmp/fixdisk.txt"	 //�޴��̵���־
#define FIXDISK_LOG_FILE_0 "/var/tmp/fixdisk.txt.0"

#define REBOOT_FILE    "/var/tmp/rbt" //���޸��ļ���˵����Ӳ����
/*
	rtaudio define
*/
#define		RTAUDIO_EXEC			"/vserver/rtaudio"			
#define		RTAUDIO_LOCK_FILE		"/lock/ipserver/rtaudio"


//
/*
	alarm_mod define
*/
#define 		ALARM_LOCK_FILE		"/lock/ipserver/alarm_mod"



#define 		TCPSNDPLAY_LOCK_FILE	"/lock/ipserver/tcpsndplay"

#define 		AUDIO_CDC_LOCK_FILE	"/lock/ipserver/audiocdc"



//
/*
     hdplayback  define
*/
#define          HDPLAYBACK_LOCK_FILE  "/lock/ipserver/playback"


#define  showbug()		printf("maybe a bug at %s:%d!!!!!!!!!!!!!\n",__FILE__,__LINE__)
#define	logbug()			gtlogerr("maybe a bug at %s:%d\n",__FILE__,__LINE__)
#endif





