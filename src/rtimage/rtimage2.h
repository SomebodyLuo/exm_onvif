/** @file	rtimage2.h
 *  @brief 	tcprtimage2ģ����ܵ�ͷ�ļ�,ģ��������Դ�ļ���Ҫ������ͷ�ļ�
 *  @date 	2007.03
 */
#ifndef RTIMAGE2_H_20070301
#define RTIMAGE2_H_20070301


static const char version[]="0.01";			///<����汾��
/*
 *
 0.01 init version 
*/




/**
 * Ĭ��ֵ����
 */
static const int def_rtstream_port 	 = 8096;    	   ///<����Ƶ���з���Ĭ�϶˿ں�
static const int def_rtsnd_port 	        = 8097;		   ///<��Ƶ���з���Ĭ�϶˿ں�

static const int def_mic_gain 		 = 8;		   ///<Ĭ��mic����
static const int def_audio_gain 	        = 8;		   ///<Ħ����Ƶ�������

static const int def_svr_timeout         = 10;               ///<Ĭ�ϵĳ�ʱʱ��

static const int def_audio_pkt_size    = 1024;           ///<Ĭ����Ƶ�������ݰ���С

//zw-modified 2012-02-07
static const int def_th_drop_p            = 30;              ///< ��ʼ����p֡��Ƶ��Ĭ����ֵ
//static const int def_th_drop_p            = 2;              ///< ��ʼ����p֡��Ƶ��Ĭ����ֵ


#define DEF_AUDIO_SAMRATE             (8000)                  ///<Ĭ����Ƶ������
#define DEF_AUDIO_FRAGSIZE            (10)                     ///<Ĭ����Ƶ�������С2^DEF_FRAGSIZE
#define DEF_AUDIO_FRAGNB               (16)                    ///<Ĭ����Ƶ�ɼ�����������

/**
 * 
 */ 
///for 3022
#define	TCPRT_MAX_VIRAVUSR_NO	10		///<3022�������豸һ��֧�ֵ��������Ƶ�����û���

#define	TCPRTIMG_MAX_AVUSR_NO       16          ///<tcprtimage2֧�ֵ��������Ƶ�����û������� 

#define TCPRTIMG_MAX_APLAY_NO       4
#define MAX_MAP_BUF_FRAMES            600        ///<���Ļ���֡��(���ڷ��仺����)


///���Թ��ܺ궨��
#if EMBEDED==0
	#define FOR_PC_MUTI_TEST				//֧����ͬһ̨pc���������������
#endif
//#define     SAVE_RAW_AUDIO                  		///<������Ƶ�豸��ȡ��ԭʼ���ݴ���Ӳ�̵�/hqdata/update/pid.raw�ļ�,����pid�ǽ��̺�




#ifndef IN
    #define IN          //���������־
    #define OUT       //���������־
    #define IO          //�������������־
#endif



//ϵͳͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

//���п�����ͷ�ļ�
#include <typedefine.h>
#include <file_def.h>
#include <devinfo.h>


#include <devinfo_virdev.h>			///<devinfo.h����չ

#include "rtimg_para2.h"
#include <signal.h>



/** 
 *   @brief     �ж�һ����ַ�Ƿ�Ϊͬһ���������е�
 *   @param  ��Ҫ�жϵ�ip��ַ
 *   @return   1��ʾ��ͬһ����������,0��ʾ����
 */ 
int is_lan_user(in_addr_t addr);


/**
  *   @brief	�ж���������Ƶ��guid�Ƿ�Ϸ�
  *   @param	cmd_dev_id��������Ƶ�������guid
  *	@param	cmd_addr��������Ƶ���豸ip�ĵ�ַ
  *	@return	�ɹ����������豸�ţ�ʧ�ܷ��ظ�ֵ
*/
int is_guid_valid(unsigned char * cmd_dev_id, struct sockaddr_in * cmd_addr);

void  begin_debug(int);
void  end_debug(int);

#endif
