#ifndef __HI_GPIO_H__ 
#define __HI_GPIO_H__

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

#define GPIO_SET_DIR 0x1
#define GPIO_GET_DIR 0x2
#define GPIO_READ_BIT 0x3
#define GPIO_WRITE_BIT 0x4


//yk add ��������
#define GPIO_SET_INPUT 0x5 //���ñ������볣����������
#define GPIO_GET_INPUT 0X6 //��ȡ�������볣����������
#define GPIO_GET_DWORD 0X7 //��ȡ��������״̬
#define GPIO_SET_DELAY 0X8 //���ñ���������ʱ
#define GPIO_GET_DELAY 0X9 //��ȡ����������ʱ
#define GPIO_SET_OUTPUT_VALUE 0x10 //���ñ������io��ֵ
#define GPIO_SET_HEARTBEAT    0x11
#define GPIO_GET_POWER        0x12

typedef struct {
	unsigned int  groupnumber;
	unsigned int  bitnumber;
	unsigned int  value;
}gpio_groupbit_info;


typedef struct{
	unsigned int on_delay;  //�ޱ������б���ʱ��
	unsigned int off_delay; // �б������ޱ���ʱ��
}TimeDelay;

typedef struct{
	int ch;
	int result;   //0  low 1 high
}relay_struct;

#endif
