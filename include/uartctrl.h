#ifndef UART_CTRL_H
#define UART_CTRL_H
/////////////////////////////////////////////////////////
/*���ڲ�����1��2 �� ��������1��2��3��4 ���б��*/
#define	INT_COM1		0
#define	INT_COM2		1
#define	EXT_COM0		2
#define	EXT_COM1		3
#define	EXT_COM2		4
#define	EXT_COM3		5
#define   COM_SET		6
/////////////////////////////////////////////////////
/*����Socket ���ӵ�ͨѶ������*/
#define	SETTING_ERR		"setting error\n"	//���ڲ������ô���
#define	SOCKETBUSSY		"bussy\n"			//�豸�˿�æ
#define	SOCKETCONNECT		"connected\n"		//�Ѿ��ɹ�����

////////////////////////////////////////////////////
/*Socket ͨѶ��·��*/
#define  IntUartSockPath1		 "/dev/intcom1"		//�ڲ�ͨ�ô���1
#define  IntUartSockPath2		 "/dev/intcom2"		//�ڲ�ͨ�ô���1
#define  ExtUartSockPath1	 "/dev/extcom0"		//�ⲿ��չ����1
#define  ExtUartSockPath2	 "/dev/extcom1"		//�ⲿ��չ����2
#define  ExtUartSockPath3	 "/dev/extcom2"		//�ⲿ��չ����3
#define  ExtUartSockPath4	 "/dev/extcom3"		//�ⲿ��չ����4
#define  ExtUartSetPath 		 "/dev/setbaud"		//���ڲ�������ͨ��

//void PrintBuf(char *buf, int len);  //test
//int CreateSockMonThread(void);
//int  InitComPortPara(void);	  //��ʼ��������
//Errno :  -ENXIO  -EBUSY
int  InitComPortCtrlDev(void);	  //��ʼ��������
int  FreeComPortCtrlDev(void);	  //�ͷŴ��ڿ����豸

//Errno :  -ENXIO  -EBUSY
int OpenComPort(int ComIndex); // ��һ������, ���ؿ��ƾ�� com_fd��

// ���ô��ڲ���
//Errno :  -ENXIO  -EBUSY  -EINVAL
int SetComPort(int ComIndex , unsigned long int baudrate, char parity , int data , int stop); 
//�رմ���
//Errno :  -ENXIO  -EBUSY 
int CloseComPort(int ComIndex);	// �ر�һ�����ڣ�
//������
//Errno :  -ENXIO  -EBUSY  -EINVAL  -EPIPE 
int ReadComPort(int ComFd, char *Buf , int Len); // ��һ�����ڶ�ȡ����
//д����
//Errno :  -ENXIO  -EBUSY  -EINVAL  -EPIPE - EFBIG
int WriteComPort(int ComFd, char *Msg , int Len); // д���ݵ�һ������

#endif

