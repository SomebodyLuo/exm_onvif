#ifndef DEVICE_ACCESS_H
#define DEVICE_ACCESS_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>



//�豸�����ļ��еķ�������
#define 		ACCESS_RTSTREAM	"rt_stream"		//ʵʱ����Ƶ�ϴ�
#define 		ACCESS_AUDIOPLY	"audio_play"		//��Ƶ�´�
#define		ACCESS_COM		"com"			//͸������
#define		ACCESS_CMD		"cmd"			//�������
#define		ACCESS_SET	1						//���÷����б�
#define		ACCESS_CLR	0						//��������б�

 /**********************************************************************************************
 * ������	:SetDeviceAccess()
 * ����	: ����/���ָ����ַ�ķ���Ȩ��
 * ����	:AccType:��������(ACCESS_RTSTREAM,ACCESS_AUDIOPLY,ACCESS_COM,ACCESS_CMD)
 *			 Addr:������ʵĵ�ַ�ַ���
 *			 Type:����/���(ACCESS_SET,ACCESS_CLR)
 * ���	:��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
int SetDeviceAccess(char *AccType,char *Addr,int Type);

 /**********************************************************************************************
 * ������	:CheckDeviceAccess()
 * ����	: ���ָ����ַ�Ƿ��з���Ȩ��
 * ����	:AccType:��������(ACCESS_RTSTREAM,ACCESS_AUDIOPLY,ACCESS_COM,ACCESS_CMD)
 *			 Addr:Ҫ���ĵ�ַ�ַ���
 * ���	:��
 * ����ֵ	:1��ʾ��Ȩ�� 0��ʾû��Ȩ�� ��ֵ��ʾ����
 **********************************************************************************************/
int CheckDeviceAccess(char *AccType,char *Addr);

#endif
