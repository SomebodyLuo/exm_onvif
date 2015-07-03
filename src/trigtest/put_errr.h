#ifndef __PUT_ERRR_H
#define __PUT_ERRR_H

#define	ERR_NO						(0)	//û�д���
#define	ERR_NO_DISK				(1)	//�Ҳ���Ӳ��
#define	ERR_NO_FORMAT_DISK		(2)	//û�и�ʽ������
#define	ERR_OPEN_FILE				(3)	//���ļ�����
#define	ERR_WRITE_FILE				(4)	//д�ļ�����
#define	ERR_READ_FILE				(5)	//���ļ�ʧ��
#define	ERR_AUDIO_OPEN			(6)	//����Ƶ�豸����
#define	ERR_AUDIO_SET_PARA		(7)	//��Ƶ�豸�������ô���
#define	ERR_AUDIO_SET_GAIN		(8)	//��Ƶ������ڴ���
#define	ERR_AUDIO_REC				(9)	//¼������
#define	ERR_AUDIO_CLOSE			(10)	//�ر���Ƶ�豸ʧ��
#define	ERR_TEST					(99)	//����ʧ��

#define	STR_ERR_NO					("��ȷ")
#define	STR_ERR_NO_DISK			("�Ҳ���Ӳ�̣�����Ӳ�������Ƿ���ȷ")
#define	STR_ERR_NO_FORMAT_DISK	("����û�и�ʽ������")
#define	STR_ERR_OPEN_FILE			("��д����ʱ���ļ�����")
#define	STR_ERR_WRITE_FILE		("д�ļ�����")
#define	STR_ERR_READ_FILE			("���ļ�����")
#define	STR_ERR_AUDIO_OPEN		("����Ƶ�豸����")
#define	STR_ERR_AUDIO_SET_PARA	("��Ƶ�豸�������ô���")
#define	STR_ERR_AUDIO_SET_GAIN	("��Ƶ������ڴ���")
#define	STR_ERR_AUDIO_REC			("¼������")
#define	STR_ERR_AUDIO_CLOSE		("�ر���Ƶ�豸ʧ��")
#define	STR_ERR_TEST				("����ʧ��")
#define	STR_ERR_UNKNOW			("δ֪�����")

/**********************************************************************************************
* ������   :get_err_str()
* ����  :       ���ش������ַ���
* ����  :      errr	������					
*						
* ���  :       void        
* ����ֵ:   ����������ַ���
**********************************************************************************************/
char *get_err_str(int errr);

#endif
