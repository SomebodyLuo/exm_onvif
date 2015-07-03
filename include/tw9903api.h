#ifndef TW9903API_H
#define TW9903API_H
#define SCR_MODE_D1	0
#define	SCR_MODE_HD1	1
#define SCR_MODE_CIF	2
#define SCR_MODE_QCIF	3
///////////////////////
#define TW9903_ADDR	0x44	//9903 �豸��ַ
#define EEPROM_ADDR	0x50	//test eeprom 
//iic_fd:�Ѿ��򿪵�i2c�����豸��������
//chip_addr:iic������оƬ�ĵ�ַ
//mode:��Ҫ9903�������Ƶ��ʽ SCR_MODE_D1,SCR_MODE_HD1,SCR_MODE_CIF,SCR_MODE_QCIF,����SCR_MODE_D1��SCR_MODE_HD1�����ñ�һ��

/*
************************************************************************
*������	:init_iic_dev
*����	:��ʼ��IIC�豸
*����	:  ��
*����ֵ	:�ɹ������豸������, ��ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int init_iic_dev(void);
/*
************************************************************************
*������	:release_iic_dev
*����	:�ر�/�ͷ�IIC�豸
*����	:  ��
*����ֵ	:�ɹ�����0, ��ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int release_iic_dev(int iic_fd);
/*
************************************************************************
*������	:write_tw9903_reg
*����	:дtw9903�豸�ļĴ���
*����	:  
	  int iic_fd;			//���豸���صĿ��ƾ��	
	  int chip_addr;		//�豸��ַ
	  unsigned char reg;	//�Ĵ�����ַ
* 	  int val;				//�Ĵ�������ֵ
*����ֵ	:0��ʾ�ɹ�, ��ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int write_tw9903_reg(int iic_fd, int chip_addr, unsigned char reg , int value);
/*
************************************************************************
*������	:read_tw9903_reg
*����	:��ȡtw9903 �Ĵ���ֵ
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
	  int reg;			//�Ĵ����ĵ�ַ
*����ֵ	:�ɹ����ؼĴ�����ֵ��ʧ�ܷ���0xff
*�޸���־:
*************************************************************************
*/
unsigned char read_tw9903_reg(int iic_fd, int chip_addr,unsigned char reg);
/*
************************************************************************
*������	:init_tw9903
*����	:tw9903 ��ʼ��
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int mode;		//����ģʽ
//mode:��Ҫ9903�������Ƶ��ʽ SCR_MODE_D1,SCR_MODE_HD1,SCR_MODE_CIF,SCR_MODE_QCIF,����SCR_MODE_D1��SCR_MODE_HD1�����ñ�һ��

*����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int init_tw9903(int iic_fd,int chip_addr,int mode);
/*
************************************************************************
*������	:read_tw9903_vloss
*����	:��ȡ9903��Ƶ��ʧ״̬
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  
*����ֵ	:0��ʾû���ڵ���1 ��ʾ���ڵ�����ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int read_tw9903_vloss(int iic_fd,int chip_addr);
/*
************************************************************************
*������	:set_tw9903_brightness
*����	:����tw9903 ����
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//���Ȳ���ֵ,��ΧΪ0-100
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_brightness(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_hue
*����	:����tw9903 ɫ��
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//ɫ�Ȳ���ֵ,��ΧΪ0-100
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_hue(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_contrast
*����	:����tw9903 �Աȶ�
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//�ԱȶȲ���ֵ��,��ΧΪ0-100
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_contrast(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_saturation
*����	:����tw9903 ���Ͷ�
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//���ͶȲ���ֵ,��ΧΪ0-100
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_saturation(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_sharp2
*����	:�����񻯼Ĵ���1
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//�񻯼Ĵ���1�Ĳ���ֵ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_sharp1(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_sharp2
*����	:�����񻯼Ĵ���2
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//�񻯼Ĵ���2�Ĳ���ֵ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_sharp2(int iic_fd,int chip_addr,int val);
/*
************************************************************************
*������	:set_tw9903_H_range
*����	:����H_DELAY�Ĵ���
*����	:  
	  int iic_fd;		//���豸���صĿ��ƾ��	
	  int chip_addr;	//�豸��ַ
* 	  int val;			//H_DELAY�Ĵ����Ĳ���ֵ 
					//������Χ: 0x80 - 0x83  default: 0x81
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int set_tw9903_H_range(int iic_fd,int chip_addr,int val);

#endif