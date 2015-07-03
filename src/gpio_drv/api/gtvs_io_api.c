///////��io�ڿ������������api��Ŀǰ�����̵���,���Ź�,������

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <gtlog.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include"gtvs_io_api.h"
#include "devinfo.h"
#include "../../../include/hi3515/gpio/hi_gpio.h"
	 	
static int vs3io_fd=-1;		//��ʼ���ļ�������


/*��ȡ����˿ڵ�״̬*/
unsigned long get_inputPort_status(void)
{
	int j;
	int gpio_value = 0;
	gpio_groupbit_info gpioinfo;

	for (j = 0; j < 8; j++)
	{
		gpioinfo.groupnumber = 3;			//ֻ��Ե�ǰʹ�õ�оƬ����
		gpioinfo.bitnumber = j;
		gpioinfo.value = 0;
		if (ioctl(vs3io_fd, GPIO_SET_DIR, &gpioinfo) < 0)
		{
    		printf("ioctl gpio failed!");
			close(vs3io_fd);
    		return (-1);
		}

		if (ioctl(vs3io_fd, GPIO_READ_BIT, &gpioinfo) < 0)
		{
			printf("ioctl read gpio failed!");
			close(vs3io_fd);
			return (-1);
		}
		gpio_value |= gpioinfo.value;
	}
	return gpio_value;
}

/*���ó�������*/

int set_inputPort_status(unsigned long gpio_value)
{
	
	if (ioctl(vs3io_fd, GPIO_SET_STATUS, &gpio_value) < 0)
	{
		printf("ioctl gpio failed!");
		close(vs3io_fd);
		return (-1);
	}
}


/**********************************************************************************************
 * ������	:init_vs3iodrv()
 * ����	:	�򿪲���ʼ����io�ܽſ��ƵĿ��Ź�,�̵���,�������豸
 * ����	:	
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/

int	init_vsiodrv()
{
	struct trigio_num_struct trigio;
	int i;
	unsigned long gpio_value;
	pthread_t pid;
	
	if(init_devinfo()<0)
	{
		printf("init devinfo error\n");
		return -1;
	}
	
	vs3io_fd=open("/dev/hi_gpio",O_RDWR);
	if(vs3io_fd<0)
	{
		printf("can't open vs3io device !\n");
		return -1;
	}
	else
	{
		//#ifdef SHOW_WORK_INFO
		printf("open vs3io device success=%d.\n",vs3io_fd);
		//#endif
	}
	//trigio.trigin_num 	= get_trigin_num();
	//trigio.alarmout_num = get_alarmout_num();
	//ioctl(vs3io_fd,SET_TRIGIO_NUM,&trigio);	//�����豸�ϵ��������������
	
	return 0;
}


/**********************************************************************************************
 * ������       :clear_watchdog_cnt()
 * ���� :       �Կ��Ź��ļ�������������
 * ���� :       
 * ����ֵ       :0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
/*
int clear_watchdog_cnt(void)
{
	if(vs3io_fd<0)
	{
		printf("δ��ʼ��vs3iodrv.\n");
		return -1;
	}
	//printf("�û�ι��\n");
        return ioctl(vs3io_fd,CLR_WD,NULL);
}
*/

/**********************************************************************************************
 * ������	:set_relay_output()
 * ����	:	��̵�������ź�
 * ����	:	ch:�̵�����ͨ����,��0��ʼ
 *			result:	1��ʾ�̵���������
 *					0��ʾ�̵����ָ�
 *
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
/*
int set_relay_output(int ch, int result)
{
	struct relay_struct rly_info;

	if(vs3io_fd<0)
	{
		printf("δ��ʼ��vs3iodrv.\n");
		return -1;
	}

	rly_info.ch=ch;
	rly_info.result=result;

	return ioctl(vs3io_fd,OP_RLY,&rly_info);
}
*/

/**********************************************************************************************
 * ������	:read_trigin_status()
 * ����	:	��ȡ����������ӵĵ�ǰ����״̬,
 *			��׼read�ӿڣ������ݲŷ��أ�
 *			(���˴��豸��ĵ�һ���⣬ÿ�������б仯�ŴӴ˽ӿڷ���)
 * ���	:	status,DWORD����ָ��,��ָ���DWORD��ֵ�ӵ͵���λ��ʾ����
 *				��0��32λ��״̬����������ʱ����
 *			1��ʾ��λ�д�����0��ʾû��
 * ����ֵ:  �Ǹ�ֵ��ʾ�������ֽ�������ֵ��ʾʧ��			
 * **********************************************************************************************/
 int  read_trigin_status(OUT DWORD *status)
{
	unsigned long allPortStatus;
	//�ж�ָ��
	if(status==NULL)
	{
		return -EINVAL;
	}
	
	if ((allPortStatus = get_inputPort_status()) == -1)
	{
		return (-1);
	}
	else
	{
		*status = allPortStatus;
	}
	return 4;                      //������������
}


/**************************************************************************
 * ������	:set_trigin_attrib_perbit()
 * ����	:�������ö�����������
 * ����	:attrib:��λ��ʾ�Ķ�����������ֵ 1��ʾ���� 0��ʾ����,ȱʡΪȫ0
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************************/
int set_trigin_attrib_perbit(DWORD attrib)
{
	if (set_inputPort_status(attrib) != 0)
	{
		printf("set_inputPort_status failed!\n");
		return (-1);
	}
	return 0;
}


/**********************************************************************************************
 * ������	:set_trigin_delay()
 * ����	:	�趨������ӵ���ʱ����ʱ��
 		(û���趨�Ļ��������ж�����ȱʡֵDEF_VALID_DELAY, DEF_INVALID_DELAY)
 * ����	:	no:������ӱ�ţ���0��ʼ
 			valid_delay: �ö��Ӵӻָ�����������ʱ����
 			invalid_delay:�ö��ӴӴ������ָ�����ʱ����	
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/

int set_trigin_delay(int no, int valid_delay, int invalid_delay)
{	
	TimeDelay timeDelay;
	timeDelay.bitnumber = no;
	timeDelay.beginToEnd = invalid_delay;
	timeDelay.endToBegin = valid_delay;
	if (ioctl(vs3io_fd, GPIO_SET_DELAY, &timeDelay) < 0)
	{
		printf("ioctl gpio failed!");
		close(vs3io_fd);
		return (-1);
	}
}


/**********************************************************************************************
 * ������	:send_require_reset()
 * ����	:	���������������Ӧֹͣι������һ��ʱ����豸������
 * ����	:	
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
/*
int send_require_reset(void)
{
	if(vs3io_fd<0)
	{
		printf("δ��ʼ��.\n");
		return -EINVAL;
	}

	gtloginfo("�û���������������,send_require_reset\n");
	return ioctl(vs3io_fd,RESET_CMD,NULL);
}
*/

/**************************************************************************
 * ������	:set_beep()
 * ����	�������
 * ����	:��Ĵ��� 
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************************/
/*
int set_beep(int cnt)
{
	if(vs3io_fd<0)
	{
		printf("δ��ʼ��.\n");
		return -EINVAL;
	}

	return ioctl(vs3io_fd,BEEP,&cnt);

}
*/

/**************************************************************************
 * ������	:get_trigin_stat()
 * ����		:���������˿�	
 * ����		:�˿�״ָ̬�� 
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************************/
/*
int get_trigin_stat(DWORD *stat)
{
	if(vs3io_fd<0)
        {
                printf("δ��ʼ��.\n");
                return -EINVAL;
        }

	return ioctl(vs3io_fd,READ_CMD,stat);
}
*/
/**************************************************************************
 * ������	:reset_tw9910()
 * ����		:Ӳ��λ9910
 * ����		:chip_no,оƬ��,0Ϊ��0Ƭ��1Ϊ��1Ƭ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************************/
/*
int reset_tw9910(unsigned int chip_no)
{
	if(vs3io_fd<0)
        {
                printf("δ��ʼ��.\n");
                return -EINVAL;
        }

	printf("api.....chip=%d\n",chip_no);
	return ioctl(vs3io_fd,RESET_TW9910,&chip_no);
}
*/


/**********************************************************************************************
 * ������	:exit_vsiodrv()
 * ����	:	�򿪲���ʼ����io�ܽſ��ƵĿ��Ź�,�̵���,�������豸
 * ����	:	
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
int	exit_vsiodrv(void)
{
	if(vs3io_fd<0)
        {
                printf("δ��ʼ��.\n");
                return -EINVAL;
        }

	return close(vs3io_fd);
}
