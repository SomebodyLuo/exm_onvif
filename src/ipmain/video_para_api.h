#ifndef VIDEO_PARA_API_H
#define VIDEO_PARA_API_H
#include "ipmain.h"
#include "mod_socket.h"
//hi3520A include
//#include "sample_comm.h"

//��ʾģʽ���� 
#define SCR_FULL			0	//ȫ����ʾ
#define SCR_QUAD		1	//4������ʾ 
//#define SCR_DUAL		2	//˫������ʾ

#define DEF_BLIND_ALARMTIME		0
#define DEF_BLIND_UNALARMTIME  	3
#define DEF_BLIND_SEN			    0

#define MAX_CAMERANAME_LENGTH	    20 //to be fixed

#define SET_VIDEO_SATURATION	1
#define SET_VIDEO_SHARPNESS		2
#define SET_VIDEO_BRIGHTNESS	3
#define SET_VIDEO_HUE			4
#define SET_VIDEO_CONTRAST		5

#define USER_PIC_PATH   "/conf/pic_704_576_p420_novideo01.yuv"

struct enc_front_struct{	//��Ƶǰ��ת������
	int bright;
	int hue;
	int contrast;
	int saturation;
	int enable;		//�Ƿ���Ч  1��ʾ��Ч 0��ʾ��Ч
	char name[MAX_CAMERANAME_LENGTH*2];	//��Ƶ���ƣ���"��������ͷ",ȱʡΪ"��Ƶx",�ֽ���������10��
};

struct motion_struct{		//�ƶ�������
	unsigned long ch;			//��Ƶ����ͨ��
	unsigned long sen;		//������ 0��ʾ����Ҫ�ƶ����
	unsigned long alarm;    //�����ƶ��Ƿ񱨾�,��������ʹ��
	WORD starthour;  //��ʼhour,��ʼmin ,����hour,����min����������ʹ��
	WORD startmin;
	WORD endhour;
	WORD endmin;
	WORD area[12];//����
};
struct blind_struct{		//�ڵ���������
	int ch;
	int sen;		//���������� 0��ʾ����Ҫ�ڵ�����
	int alarm_time; //�ڵ�����ʱ��
	int cancelalarm_time; //���ڵ�ȡ������ʱ��
};

struct quad_dev_struct
{
	int current_net_ch;			//��ǰ������ʾͨ��,0-3��ʾ��Ӧȫ����4��ʾ4�ָ�
	int last_net_ch;
	//int current_local_ch;		//��ǰ������ʾͨ��,0-3��ʾ��Ӧȫ����4��ʾ4�ָ�
	struct motion_struct 	motion[4];
	struct blind_struct 	blind[4];
};

struct video_para_struct
{
	struct enc_front_struct enc_front[4];
	struct quad_dev_struct quad;
};

//������Ƶ����(���ȣ�ɫ�ȵ��õ��Ľṹ)
struct set_video_para_struct{
	int		ch;		//��Ƶ����ͨ��
	int		val;		// ֵ
};

struct video_info_struct{
	int  sec_flag;				//���־
	unsigned long loss;		//��Ƶ��ʧ��־,1��ʾ��ʧ
	unsigned long motion;		//�ƶ�������־,1��ʾ���ƶ�����
	unsigned long blind;		//��ͷ�ڵ���־,1��ʾ�о�ͷ�ڵ�
};



/*
 * ��ʼ����Ƶ�����豸
 * ����ֵ 0:�ɹ���-1:ʧ��
*/
int init_video_vadc(struct video_para_struct *init_para);

/*
 * ����ʼ��
 */
int uninit_video_vadc();

/********************************************************
 * �����紫��ͨ������ʾ�л�Ϊȫ������chͨ��
 * ����ֵ   0���ɹ�,��ֵ��ʾ����
 * ����:		ch��	��Ҫ�л���Ŀ������ͷͨ��(��Чֵ0,1,2,3)	
 ***************************************************/
int set_net_scr_full(BYTE ch,struct sockaddr *addr,char *username);

/********************************************************
 * ������¼��ͨ������ʾ�л�Ϊȫ������chͨ��
 * ����ֵ   0���ɹ�,��ֵ��ʾ����
 * ����:		ch��	��Ҫ�л���Ŀ������ͷͨ��(��Чֵ0,1,2,3)	
 ***************************************************/
 //int set_local_scr_full(BYTE ch);
 
/********************************************************
 * �����紫��ͨ������ʾ�л�Ϊ�ķָ�
 * ����ֵ   0���ɹ�,��ֵ��ʾ����
 * ����:		ch��	��Ҫ�л���Ŀ������ͷͨ��(��Чֵ0,1,2,3)	
 ***************************************************/
 int set_net_scr_quad(struct sockaddr *addr,char *username);
 
/********************************************************
 * ������¼��ͨ������ʾ�л�Ϊ�ķָ�
 * ����ֵ   0���ɹ�,��ֵ��ʾ����
 * ����:		ch��	��Ҫ�л���Ŀ������ͷͨ��(��Чֵ0,1,2,3)	
 ***************************************************/ 
//int set_local_scr_quad(void);



/********************************************************
 * ����vda��ͨ����������
 * ����ֵ   0���ɹ�,��ֵ��ʾ����
 * ����:		ch��	��Ҫ�л���Ŀ������ͷͨ��(��Чֵ0,1,2,3)	
 ***************************************************/ 
int init_video_vda_param(struct video_para_struct *vadc);

/*********************************************************
 * ����vda��ͨ���ƶ���������ȼ�����
 * ����ֵ  0 �ɹ�����ֵ��ʾ����
 * ����:  ch Ŀ������ͷͨ��(0,1,2,3)  sen ������  area �ƶ��������
 ********************************************************/
int set_motion_vda_sen(int ch,int sen,WORD *area);

/*********************************************************
 * ����vda��ͨ���ڵ�����������
 * ����ֵ  0 �ɹ�����ֵ��ʾ����
 * ����:  ch Ŀ������ͷͨ��(0,1,2,3)  sen ������ 
 ********************************************************/
int set_blind_vda_sen(int ch,int sen);





//������Ƶ��������,valΪ0~100
int set_video_bright(BYTE ch,int val);
//������Ƶ����ɫ��,valΪ0~100
int set_video_hue(BYTE ch,int val);

//������Ƶ����Աȶȣ�valΪ0~100
int set_video_contrast(BYTE ch,int val);

//������Ƶ���뱥�Ͷȣ�valΪ0~100
int set_video_saturation(BYTE ch,int val);


int creat_vda_proc_thread(pthread_attr_t *attr,void *arg);

int video_blind_detected(unsigned long blind);

void vadc_second_proc(void);

#endif
