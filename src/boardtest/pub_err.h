#ifndef PUB_ERR_H
#define PUB_ERR_H

#define SUCCESS                 		0	//�ɹ�
#define ERR_DEVICE_BAD			1	//�豸�й���,���������ı���ʽ�������ļ�
#define ERR_CANNOT_OPEN_FILE    4	//���ܴ�ָ���Ľ���ļ�
#define ERR_INTERNAL			5	//���Գ����ڲ���

/*
****************************************************************************
���Խ�����ݽṹ 0 ��ʾ����������ֵ��ʾ��Ӧ�Ĵ���
	-1 �޷���������ģ��
	-2  �޷�������ģ��
	-3  �޷�������ģ���д
	-4  �޷�������ģ�����I/O ����
****************************************************************************
*/
#define	Net_error 			"������0����"
#define	Hq_error 			"������1����"
#define	Tw2834_error 		"��Ƶ�и�������"
#define	Ide_error 			"IDE ����������"
#define	Dsp_error			"��Ƶ����������"
#define    TW9903_error		"TW9903����"
#define    UNKNOW 			"δ֪���豸����"

#define ERR_NO6410			"������оƬû�к���,"		// error code 1
#define ERR_NODATA			"���������������/6410�麸,"			// error code 3
#define ERR_NOINPUT			"����������������/6410�麸/C95144 �麸/TW2834�麸,"			// error code 2
#define ERR_UNSTABLE		"�������������ȶ�������,"  // error code 4

#define ERR_NO2834			"TW2834 оƬû�к���,"		// error code 1
//#define ERR_NO2834			"TW2834 оƬû�к���,"		// error code 1

#define ERR_NOIDE			"IDE ���������������û�к���, "  //error code 1
#define ERR_NODISK			"û���ҵ�IDE�豸,"
#define ERR_NOPART			"����û�з���,"
#define ERR_OPEN			"�򿪲����ļ�ʧ��,"
#define ERR_WRITE			"д�����ļ�ʧ��,"
#define ERR_READ				"�������ļ�ʧ��,"
//#define ERR_NOPART			"����û�и�ʽ������,"


#define ERR_NODSP			"����оƬû�к���,"			//error code 1

#define ERR_IIC				"ģ��IIC����û����"
#define ERR_NO9903			"��д9903ʧ��"
//������붨��
#define	NETENC_ERR		10
#define	HQENC_ERR			20
#define	QUARD_ERR			30
#define	IDE_ERR			40
#define	DSP_ERR			50
#define 	TW9903_ERR		60
#define 	USB_ERR		70

#endif


