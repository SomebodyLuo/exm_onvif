#include<stdio.h>
#include<unistd.h>

#include "gtvs_io_api.h"		//����io�������
#include "testmod.h"
#include "io_test.h"
#include <unistd.h>
#include <commonlib.h>
#include <iniparser.h>
#include <devinfo.h>

#define CHECK_VAL_A		(0x00)	//��1�β��Զ�������ֵ
#define CHECK_VAL_B		(0xff)	//��2�β��Զ�������ֵ

/**********************************************************************************************
* ������   :check_trigin_bit()
* ����  :       ��λ��������˿�λ
* ����  :       stat		�����Ķ���״̬
*			   val		������ֵ
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
int check_trigin_bit(unsigned int stat,unsigned int val)
{
	unsigned int i;
	unsigned int tmp_stat;
	unsigned int tmp_val;
	char tmp_info[128];
	
	for(i=0;i<8;i++)
	{
		tmp_stat=(stat>>i) & 0x01;
		tmp_val =(val>>i) & 0x01;

		memset(tmp_info,0,sizeof(tmp_info));
		if(tmp_stat != tmp_val)
		{
			sprintf(tmp_info,"%s%d%s\n","����ͨ��IN",i,"���ϻ���Թ�װδ����");
			//printf("����ͨ��IN[%d]��������.\n",i);
			printf(tmp_info);
			s_test_rp(tmp_info,30);
		}


	}
	return 0;	

}




	
/**********************************************************************************************
* ������   :test_io_mod()
* ����  :       ����ioģ��
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
int test_io_mod(void)
{
	//int vs3_fd;
	int ret;
	int ch=0;
	DWORD status;
	int err_flag;
	int i;
	
	printf("��ʼ����...\n");
	
	ret=init_gpiodrv();
	if(ret<0)
	{
		printf("��ʼ��io����ʧ��:%d\n",ret);
		return -1;
	}
	//printf("��vs3iodrv�ɹ�\n");

	for(i=0;i<4;i++)
	{
		//����������û����ʱ
		//printf("��ʼ���̵���%d����ʱ��\n",i);
		set_trigin_delay(0,0);
		
		//printf("�̵���%d��ʼ��.\n",i);
		set_relay_output(i,0);
	}

	//���ȫΪ1����
	printf("��ʼ���Զ��� \n");

	printf("--------------------------\n");
	for(ch=0;ch<4;ch++)
	{
		//printf("��[%d]�����Ӷ���\n",ch);
		ret=set_relay_output(ch,1);
		if(ret>0)
		{
			printf("���ö���ʧ��\n");
			return -1;
		}
	}
	sleep(1);
	read_trigin_status(&status);
	printf("�̵�������������״̬Ϊ[0x%x]\n",status);
	err_flag=0;
	check_trigin_bit(status,CHECK_VAL_A);	
	if(status!=CHECK_VAL_A)
	{
		printf("���ӵ�1�β��Դ���\n");
		//s_test_rp("���ӵ�1�β��Դ���",10);
		err_flag|=0x01;
	}
	else
	{
		printf("���ӵ�1�β�����ȷ\n");
	}

	printf("--------------------------\n");
	sleep(2);

	//���ȫΪ0���� 
	for(ch=0;ch<4;ch++)
	{
		//printf("��[%d]�����ӻָ�\n",ch);
		ret=set_relay_output(ch,0);
		if(ret>0)
		{
			printf("���ö���ʧ��\n");
			return -1;
		}
	}
	sleep(1);
	read_trigin_status(&status);
	printf("�̵����ָ��������״̬Ϊ[0x%x]\n",status);
	check_trigin_bit(status,CHECK_VAL_B);
	if(status!=CHECK_VAL_B)
	{
		printf("���ӵ�2�β��Դ���\n");
		//s_test_rp("���ӵ�2�β��Դ���",10);
		err_flag|=0x20;
	}
	else
	{
		printf("���ӵ�2�β�����ȷ\n");
	}
	printf("--------------------------\n");

	if(err_flag==0)
	{
		s_test_rp("��������",35);
		return 0;
	}
	else
	{
		s_test_rp("���Ӵ���",35);
		return -1;
	}

	exit_gpiodrv();

       return 0;

}


