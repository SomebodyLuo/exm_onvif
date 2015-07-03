#ifndef __INI_CONV_H_
#define __INI_CONV_H_

/**
	@file:	ini_conv.c
	@brief:	��gt3022������gt1000.ini��yy�������������ļ�֮�������Ӧ�ı���ֵת����
			���б䶯�Ľ����޸Ĳ����棬�޸Ķ��Ĳ����κα���
	@date:	aah 2009.02.09��ʼ��д��2009.02.19�ݽ���һ��
	@modify:2009.09.09	ver:0.03����remote_gate:alarm_gate��ת�� (wsy,BUG 743)
			2009.02.20  ver:0.02ȥ������������д�������
							   ���Ӻ궨�壬�������������֣�
			2009.02.27  ver:0.03 �޸ı�������Ϊstatic,���Ӳ����궨�壬����ά����
							   �ı�ά���������������ԣ�
							   �ı�ģ��ܹ������������Թ��ࣻ
							   single2multi��multi2single�����ϲ���һ����������s2m_or_m2s��
							   �����ַ�����ͬ�ԱȺ���str_compare��
							   ��������ڵ㴦����spcial_proc������ڵ��;
							   �����.c��.h����ά����
			2009.03.02  ver:0.04 �ڵ�hqenc0��netencode����λ�ã�
							   �Ҳ���Դ�ڵ㲻��¼��־��
							   ���洢Ҳ��¼����־���棻
			2009.03.23  ver:0.05 ���������б��ʼ��ͷ�ļ����ӱ�ͷ�ļ��������
			2009.05.21	ver:0.06 ���Ӷ���net_config:route_default��dns_server��ת��
			2011.03.31     ver:0.07 zsk ���Ӷ���gps��Ϣgps:band gps:enable gps:port gps:stop gps:parity gps:date��ת��
*/

#include <string.h>


#define VIRDEV_0_INI	 "/conf/virdev/0/ip1004.ini"	///<yy��һ�������ļ�·��
#define VIRDEV_1_INI 	"/conf/virdev/1/ip1004.ini"	///<yy�ڶ��������ļ�·��
#define MAIN_INI		     	"/conf/ip1004.ini"	///<ip1004.ini·��
#define INI_CONV_VER  	("0.07")					///<����汾��

#define NO_FOUND_STR 	"no_str"					///<ini�ڵ㲻����ʱ�ķ���ֵ

///<�Ա�A.B��ָ����ַ����ĳ��ȣ����ظ�strncmp���������ڱȽ��Ƿ��в����޸�
#define MAX(A, B)			((strlen(A)) >= (strlen(B)) ? (strlen(A)) : (strlen(B)))	

///�����ļ�ת������
#define CONV_S				0					///<gt1000�����������ļ�ת��
#define CONV_M				1					///<���������ļ���gt1000ת��

///�ڵ�key����
#define NO_ACCORD2_A		0					///<gt1000�����������ļ�ת��ʱ�Ա��Ƿ��޸���A.INIΪ��
#define IS_ACCORD2_A		1					///<����AΪ��
	
///�ڵ�key����
#define NOT_S_ONLY			0					///<������-s only�������໥ת��
#define IS_S_ONLY			1					///<��-s only

///�ж������ļ��Ƿ��޸�
#define NO_CHANGE			0					///<�ж��Ƿ��иĶ���0û�иĶ�
#define CHANGE_SAVE		1					///<�ж��Ƿ��иĶ�,    1�����Ķ�����Ҫ����

//static  int main_ini_changed =	NO_CHANGE;			///<�ж�gt1000.ini�����ļ��Ƿ��޸Ĺ���0��ʾû�иĶ���������Ҫ���棻1��ʾ�иĶ�����Ҫ����
//static  int vir0_ini_changed =	NO_CHANGE;			///<�ж�virdev 0�����ļ��Ƿ��޸Ĺ���0��ʾû�иĶ���������Ҫ���棻1��ʾ�иĶ�����Ҫ����
//static  int vir1_ini_changed =	NO_CHANGE;			///<�ж�virdev 1�����ļ��Ƿ��޸Ĺ���0��ʾû�иĶ���������Ҫ���棻1��ʾ�иĶ�����Ҫ����



///ת�������ļ���Ϣ���ýṹ��
typedef struct{
	int conv_att;			///<�����ļ�ת������:conv_attΪ0��ʾ����AΪ����Ϊ1��ʾ��AΪ��
	int is_s_only;		///<�ж��Ƿ���-s only��0˵����-s only��1Ϊ-s/-m����
	char *sec_key_gt1;	///<gt1000��Ҫ���һ��yy��ini�����ļ�ת����key
	char *sec_key_gt2;	///<gt1000��Ҫ��ڶ���yy��ini�����ļ�ת����key
	char *sec_key_a;		///<yy��һ�������ļ���Ӧgt1000��key
	char *sec_key_b;		///<yy�ڶ��������ļ���Ӧgt1000��key
}conv_ini_t;


///��ת��ǰ��Ҫ�������⴦��Ľڵ�key�ṹ
typedef struct{
	char *sec_key_gt1;	///<gt1000��Ҫ���һ��yy��ini�����ļ�ת����key
	char *sec_key_gt2;	///<gt1000��Ҫ��ڶ���yy��ini�����ļ�ת����key
	char *sec_key_a;		///<yy��һ�������ļ���Ӧgt1000��key
	char *sec_key_b;		///<yy�ڶ��������ļ���Ӧgt1000��key
}conv_spcial_t;


#endif
