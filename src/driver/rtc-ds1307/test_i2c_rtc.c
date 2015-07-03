#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "time.h"
#include "gpio_i2c.h"



/*��bcd��ת��ʮ����*/
unsigned char inline bcdtobin(unsigned char hex)
{
	return (  (hex>>4)*10 + (hex&0xf) );
}

unsigned char inline bintobcd(unsigned char bin)
{
	return ( bin/10*16+bin%10  );
}
/*���rtcû�й�����������������Ҫ�ǼĴ���0���λ��0*/
void init()
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	int value=0;
	int ret;

	/*��ȡֵ*/
	value = 0;
	value |=0xd0<<24;	//device addr
	value |=0x0<<16;	//reg addr
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	//printf("value:%#X \n",value);

	if(value&0x80)//������λΪ1����0д��
	{
		/*д��ֵ*/
		value &=~0x80;  //��rtc�Ĵ��������λ��0����rtc��ʼ����
		//value |=0x80; //��rtc�Ĵ��������λ��1����rtcֹͣ����
		ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
		if(ret<0)
				printf("ioctl write2\n");
		//printf("value:%#X \n",value);
	}

	//�ͷ���Դ
	close(fd);
}
void readtime(struct tm *time)
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	int value=0;
	int ret;


	value = 0;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("year:%#X \n",value);
	time->tm_year=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x1f;//��ֻ�ǵ�5λ��Ч
	printf("month:%#X \n",value);
	time->tm_mon=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x3f;//��ֻ�ǵ�6λ��Ч
	printf("year:%#X	\n",value);
	time->tm_mday=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x3<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("week:%#X	\n",value);


	value = 0;
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//ʱ
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x3f;//ʱֻ�ǵ�7λ��Ч
	printf("hour:%#X \n",value);
	time->tm_hour=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x7f;//��ֻ�ǵ�7λ��Ч
	printf("min:%#X \n",value);
	time->tm_min=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x7f;//��ֻ�ǵ�7λ��Ч
	printf("sec:%#X \n",value);
	time->tm_sec=bcdtobin((unsigned char)value);


	//�ͷ���Դ
	close(fd);

}


void settime(struct tm *time)
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	unsigned int value=0;
	int ret;
	unsigned char tm;


	tm=bintobcd(time->tm_year);
	value = tm;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("year:%#X \n",value);
	time->tm_year=bcdtobin((unsigned char)value);

	tm=bintobcd(time->tm_mon);
	value = tm;
	value &=0x1f;//��ֻ�ǵ�5λ��Ч
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	printf("month:%#X \n",value);

	tm=bintobcd(time->tm_mday);
	value = tm;
	value &=0x3f;//��ֻ�ǵ�6λ��Ч
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	printf("year:%#X	\n",value);
/*
	tm=bintobcd(time->tm_year);
	value = tm;
	value |=0xd0<<24;
	value |=0x3<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
*/
	tm=bintobcd(time->tm_hour);
	value = tm;
	value &=0x3f;//ʱֻ�ǵ�7λ��Ч
	value &= ~(1L<<6);//��6λ��0��24Сʱ��
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//ʱ
	if(ret<0)
			printf("ioctl write2\n");
	printf("hour:%#X \n",value);

	tm=bintobcd(time->tm_min);
	value = tm;
	value &=0x7f;//��ֻ�ǵ�7λ��Ч
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	printf("min:%#X \n",value);

	tm=bintobcd(time->tm_sec);
	value = tm;
	value &=0x7f;//��ֻ�ǵ�7λ��Ч
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//��
	if(ret<0)
			printf("ioctl write2\n");
	printf("sec:%#X \n",value);



	//�ͷ���Դ
	close(fd);

}


int main()
{
	struct tm time;

	init();
	 //readtime(&time );

	 //printf("year:%d,mon:%d,day:%d,  hour:%d,min:%d,sec:%d\n",time.tm_year,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

	 //time.tm_hour +=1;

	if(0)
	{
		 time.tm_year=0; time.tm_mon=2;time.tm_mday=28; time.tm_hour=23; time.tm_min=58; time.tm_sec=59;
		 settime(&time );
	}

	 readtime(&time );
	 printf("year:%d,mon:%d,day:%d,  hour:%d,min:%d,sec:%d\n",time.tm_year,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

}
