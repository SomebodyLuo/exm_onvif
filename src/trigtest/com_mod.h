#ifndef __COM_MOD_H
#define __COM_MOD_H

#define	TEST_COM0_232				("/dev/ttyAMA0")	//����ʱ��ӡ��Ϣ���Ǹ��ն�
//lc for ip1004xm 485 is ama1 232 is ama2
#define	TEST_SERIAL_485			("/dev/ttyAMA1")	//rs485
#define 	TEST_SERIAL_232			("/dev/ttyAMA2")	//com3
#define	TEST_SERIAL_BAUD		(115200)

#define	TEST_485					(0)
#define	TEST_232					(1)

#define TEST_485_STR			("this is 485 test")
#define TEST_232_STR			("this is 232 test")
#define TEST_0_232_STR			("this is 0-232 test")

//���ڵȴ���������ʱ��
#define TEST_TIMEOUT_SEC			(5)
#define TEST_TIMEOUT_USEC			(0)

/**********************************************************************************************
* ������   :init_com()
* ����  :      ��ʼ������
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
int init_com(void);



#endif
