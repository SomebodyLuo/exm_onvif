#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>


//�豸״̬�ļ��еĽ���
#define 		DEV_VENC0		"venc0"		//��Ƶ������0
#define 		DEV_VENC1		"venc1"		//��Ƶ������1
#define 		DEV_VENC2		"venc2"		//��Ƶ������2
#define 		DEV_VENC3		"venc3"		//��Ƶ������3
#define 		DEV_VENC4		"venc4"		//��Ƶ������4
#define		DEV_QUAD		"quad"		//����ָ���
#define		DEV_SIMCOM	"simcom"		//���⴮��
#define		DEV_IDEDISK	"idedisk"		//Ӳ�̻�cf��
#define		DEV_LED		"leds"		//ָʾ������
#define		DEV_AENC0		"aenc0"		//��Ƶ�����豸
#define		DEV_ADEC0		"adec0"		//��Ƶ�����豸

//�豸״̬�ļ��еı�����
#define		DEV_INSTALL	"install"		//�豸������װ��־
#define		DEV_STATE		"state"		//��ǰ����״̬ 0��ʾ����
#define		DEV_ERRNUM	"errnum"		//�豸����Ĵ���




 /**********************************************************************************************
 * ������	:SetDeviceStateValInt()
 * ����	: ����ָ���豸��ָ��״̬������ֵ(������ʽ)
 * ����	:DevName:�豸���ͣ���ͷ�ļ��ж���DEV_VENC0,DEV_QUAD��
 *			 ValName:Ҫ���õı����� ��ͷ�ļ��ж��� DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 Val:Ҫ���õ�ֵ
 * ���	:��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int SetDeviceStateValInt(char *DevName,char *ValName,int Val);

 /**********************************************************************************************
 * ������	:GetDeviceStateValInt()
 * ����	: ��ȡָ���豸��ָ��״̬������ֵ(������ʽ)
 * ����	:DevName:�豸���ͣ���ͷ�ļ��ж���DEV_VENC0,DEV_QUAD��
 *			 ValName:Ҫ���õı����� ��ͷ�ļ��ж��� DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 DefVal:Ĭ��ֵ
 * ���	:��
 * ����ֵ	:������ֵ
 **********************************************************************************************/
int GetDeviceStateValInt(char *DevName,char *ValName,int DefVal);

 /**********************************************************************************************
 * ������	:IncDeviceStateValInt()
 * ����	: ��ָ���豸��ָ��״̬������ֵ��1(������ʽ)
 * ����	:DevName:�豸���ͣ���ͷ�ļ��ж���DEV_VENC0,DEV_QUAD��
 *			 ValName:Ҫ���õı����� ��ͷ�ļ��ж��� DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 * ���	:��
 * ����ֵ	:���Ӻ��ֵ
 **********************************************************************************************/
int IncDeviceStateValInt(char *DevName,char *ValName);

/**********************************************************************************************
 * ������	:SetDeviceStateValStr()
 * ����	:����ָ���豸��ָ��״̬������ֵ(�ַ�����ʽ)
 * ����	:DevName:�豸���ͣ���ͷ�ļ��ж���DEV_VENC0,DEV_QUAD��
 *			 ValName:Ҫ���õı����� ��ͷ�ļ��ж��� DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 SVal:Ҫ���õ��ַ���ָ��
 * ���	:��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 **********************************************************************************************/
int SetDeviceStateValStr(char *DevName,char *ValName,char *SVal);

/**********************************************************************************************
 * ������	:SetDeviceState()
 * ����	:�����豸�Ĵ��ڱ�־���������
 * ����	:DevName:�豸���ͣ���ͷ�ļ��ж���DEV_VENC0,DEV_QUAD��
 *			 InstFlag:�Ƿ���ڱ�־1��ʾ���ڣ�0��ʾ������
 *			 ErrNum:�������ֵ
 * ���	:��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 *			������һ�����ڳ��������ʱ
 **********************************************************************************************/
int SetDeviceState(char *DevName,int InstFlag,int ErrNum);

/**********************************************************************************************
 * ������	:GetVEncStateSec()
 * ����	:������Ƶ�������Ż�ȡ�豸��
 * ����	:EncNo:��Ƶ��������
 * ���	:��
 * ����ֵ	:�豸���ַ���
 *			������һ�����ڳ��������ʱ
 **********************************************************************************************/
char *GetVEncStateSec(int EncNo);


#endif
