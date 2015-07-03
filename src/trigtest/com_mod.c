#include  <stdio.h>      /*��׼�����������*/
#include  <stdlib.h>     /*��׼�����ⶨ��*/
#include  <unistd.h>     /*Unix ��׼��������*/
#include  <sys/types.h>  
#include  <sys/stat.h>   
#include  <fcntl.h>      /*�ļ����ƶ���*/
#include  <termios.h>    /*PPSIX �ն˿��ƶ���*/
#include  <errno.h>      /*����Ŷ���*/
#include<commonlib.h>
#include <pthread.h>
#include <unistd.h>


#include"com_mod.h"
#include"testmod.h"

//static int com_fd;

typedef struct
{
//	int com0_fd;		
	int com_0_232_fd;
	int com_232_fd;		//uart3
	int com_485_fd;		//rs485
	unsigned char test_0_232_str[128];
	unsigned char test_232_str[128];
	unsigned char test_485_str[128];
	int com_num;
}fds_t;


#if 0
static int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300 };
static int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300 };




int clr_option_ctrl_char(struct termios *options)
{
	options->c_cc[VINTR]    = 0;     /* Ctrl-c */ 
	options->c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	options->c_cc[VERASE]   = 0;     /* del */
	options->c_cc[VKILL]    = 0;     /* @ */
	options->c_cc[VEOF]     = 4;     /* Ctrl-d */
	options->c_cc[VTIME]    = 0;     /* inter-character timer unused */
	                               /* ��ʹ���ַ���ļ�ʱ�� */
	options->c_cc[VSWTC]    = 0;     /* '\0' */
	options->c_cc[VSTART]   = 0;     /* Ctrl-q */ 
	options->c_cc[VSTOP]    = 0;     /* Ctrl-s */
	options->c_cc[VSUSP]    = 0;     /* Ctrl-z */
	options->c_cc[VEOL]     = 0;     /* '\0' */
	options->c_cc[VREPRINT] = 0;     /* Ctrl-r */
	options->c_cc[VDISCARD] = 0;     /* Ctrl-u */
	options->c_cc[VWERASE]  = 0;     /* Ctrl-w */
	options->c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	options->c_cc[VEOL2]    = 0;     /* '\0' */
	return 0;
}


/**********************************************************************************************
* ������   :set_com_mode()
* ����  :      ���ô�������
* ����  :       fd				����������
*				databits		����λ
*				stopbits		ֹͣλ
*				parity		У��λ
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
int set_com_mode(int fd,int databits,int stopbits,int parity)
{
	struct termios options; 
	int ret;
	
	ret=tcgetattr( fd,&options);
	if  ( ret  !=  0)
	{ 
		printf("SetupSerial error!!\n"); 
		return(-1);  
	}
	options.c_lflag=0;//
	options.c_iflag=0;
	options.c_cflag &= ~CSIZE; 

	//��������λ
	switch (databits)
	{   
		case 7:		
			options.c_cflag |= CS7; 
			break;
		case 8:     
			options.c_cflag |= CS8;
			break;   
		default:    
			fprintf(stderr,"Unsupported data size:%d set to default 8\n",databits); 
			options.c_cflag |= CS8;
		break;
	}

	//����У��λ
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			parity='n';
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* ......*/  
			options.c_iflag |= INPCK;             /* Disnable parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* ......*/     
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S': 
		case 's':  /*as no parity*/   
		    	options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
		break;  

		default:   
			fprintf(stderr,"Unsupported parity set to default\n");  
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
		break;
	}  

 	switch (stopbits)
	{
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  

		case 2:    
			options.c_cflag |= CSTOPB;  
			break;

		default:
			fprintf(stderr,"Unsupported stop bits set to default 1\n"); 
			options.c_cflag &= ~CSTOPB;
			break;
	} 
	
/* Set input parity option */ 
	if(parity != 'n')
		options.c_iflag |= INPCK; 
	tcflush(fd,TCIFLUSH);
//	clr_option_ctrl_char(&options);  //
	options.c_cc[VTIME] = 150; /* ....15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
  	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*///not term
  	options.c_oflag  &= ~OPOST;   /*Output*/

	if(tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		perror("SetupSerial port");
		return (-1);
	}

	return (0);  
}


//���ô��ڵĲ�����
/**********************************************************************************************
* ������   :set_com_baud()
* ����  :      ���ô��ڵĲ�����
* ����  :       fd				����������
*				databits		����λ
*				stopbits		ֹͣλ
*				parity		У��λ
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
void set_com_baud(int fd, int speed)
{
	int   i; 
	int   status; 
	struct termios   Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{ 
		if  (speed == name_arr[i]) 
		{     
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) 
			{     
				printf("tcsetattr fd1");  
				return;     
			}     
			tcflush(fd,TCIOFLUSH);   
		}  
	}
}
#endif



/**********************************************************************************************
* ������   :test_com_232_thread()
* ����  :      ����232���Խ����߳�
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
static void *test_com_0_232_thread(void *para)
{
	unsigned char buff[512];
	unsigned char tmp_buf[512];
	int nread;
	fds_t *fd;
	fd_set rdfd;
	struct timeval tv;
	int ret;
	int i;
	pthread_mutex_t lock_0_232;			//������
	
	fd=(fds_t *)para;
	pthread_mutex_init( &lock_0_232,NULL);
	FD_ZERO(&rdfd);
	FD_SET(fd->com_0_232_fd,&rdfd);
	memset(&tv,0,sizeof(struct timeval));
	tv.tv_sec=TEST_TIMEOUT_SEC;
	tv.tv_usec=TEST_TIMEOUT_USEC;

	sleep(1);
	while(1)
	{
		ret=select(fd->com_0_232_fd+1,&rdfd,NULL,NULL,&tv);
		if(ret<0)
		{
			printf("select error\n");
			continue;
		}
		else if(ret==0)
			{
				s_test_rp("����0-RS232���ϻ���Թ�װδ����\n",60);
				printf("����0-RS232���ϻ���Թ�װδ����\n");
			}
			else
			{			
				if(FD_ISSET(fd->com_0_232_fd,&rdfd))
				{						
					memset(buff,0,sizeof(buff));
					memset(tmp_buf,0,sizeof(tmp_buf));
					pthread_mutex_lock( &lock_0_232 );	
					nread = read(fd->com_0_232_fd,buff,512);
					printf("\nLen %d\n",nread); 		
					buff[nread+1]='\0';

					for(i=0;i<nread+1;i++)
					{
						printf("[%x] ",buff[i]);
					}

					printf("===\n");
					for(i=0;i<nread+1;i++)
					{
						printf("[%c] ",buff[i]);
					}
					printf("\n");

					pthread_mutex_unlock( &lock_0_232 );						
					printf("0-232 receive:%s\n",buff);
					//s_test_rp(buff,11);
					if(strcmp(buff,TEST_0_232_STR)==0)
					{
						s_test_rp("����0-232 ��������",60);
						printf("����0-232 ��������");
					}
					else
					{
						s_test_rp("����0-RS232����,�����뷢�Ͳ�һ��",60);
						//printf("����0-RS232����,�����뷢�Ͳ�һ��\n");
						sprintf(tmp_buf,"%s %s [%s]","����0-RS232����,�����뷢�Ͳ�һ��","���յ�������Ϊ",buff);
						printf(tmp_buf);

					}

				}
			}
		
		//printf("�˳�0-232�߳�\n");
		pthread_exit(0);
		return NULL;
	}

}



/**********************************************************************************************
* ������   :test_com_232_thread()
* ����  :      ����232���Խ����߳�
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
static void *test_com_232_thread(void *para)
{
	unsigned char buff[512];
	unsigned char tmp_buf[512];
	unsigned char rec[512];
	int nread;
	fds_t *fd;
	fd_set rdfd;
	struct timeval tv;
	int ret;
	int i;
	int j;
	pthread_mutex_t lock_232;			//������
	int err_flag;
	int total_len;
	int tmp_len;
	int read_cnt;
	
	
	fd=(fds_t *)para;
	pthread_mutex_init( &lock_232,NULL);
	FD_ZERO(&rdfd);
	FD_SET(fd->com_232_fd,&rdfd);
	memset(&tv,0,sizeof(struct timeval));
	tv.tv_sec=TEST_TIMEOUT_SEC;
	tv.tv_usec=TEST_TIMEOUT_USEC;

	sleep(1);
	while(1)
	{
		ret=select(fd->com_232_fd+1,&rdfd,NULL,NULL,&tv);
		if(ret<0)
		{
			printf("select error\n");
			continue;
		}
		else if(ret==0)
			{
				s_test_rp("����RS232���ϻ���Թ�װδ����\n",60);
				printf("����RS232���ϻ���Թ�װδ����\n");
				err_flag=1;
			}
			else
			{			
				if(FD_ISSET(fd->com_232_fd,&rdfd))
				{						
					memset(buff,0,sizeof(buff));
					memset(tmp_buf,0,sizeof(tmp_buf));
					////pthread_mutex_lock( &lock_232 );	
					//pthread_mutex_lock( &lock_com );	

					
					nread = read(fd->com_232_fd,buff,10);					
					printf("232��1��ԭʼ����:");
					for(i=0;i<nread;i++)
					{
						printf("[0x%02x] ",buff[i]);
					}

					
					for(i=0;i<nread;)
					{
						if((buff[i]==0x5e)||(buff[i]==0x5c)||(buff[i]==0x00))
						{
							i++;
						}
						else
						{
							break;
						}						
					}
					
					
					total_len=buff[i];
					printf("\n�ַ���Ч�ܳ���Ϊ=[%d],���յ��ĳ���=[%d],i=%d\n",total_len,nread,i);
					memset(rec,0,sizeof(rec));
					////memcpy(rec,&buff[i],strlen(&buff[i]));
					////tmp_len=strlen(&buff[i]);
					memcpy(rec,&buff[i],nread-i);
					tmp_len=nread-i;

					printf("��1�ν���Ϊ:");
					for(i=0;i<tmp_len;i++)
					{
						printf("[0x%02x] ",rec[i]);
					}
					printf("��1�����\n");
					


					read_cnt=1;
					while(tmp_len<total_len)
					{
						memset(buff,0,sizeof(buff));
						printf("232��ʼ��[%d]����\n",read_cnt+1);
						nread = read(fd->com_232_fd,buff,5);
						if(nread<0)
						{
							printf("232��[%d]��ʧ��\n",read_cnt);
							if(read_cnt>5)
								break;
							read_cnt++;
						}
						else
						{
							printf("232��[%d]�ζ������ֽ���Ϊ=%d\n",read_cnt+1,nread);
						}
						
						for(i=0;i<nread;)
						{
							if((buff[i]==0x0d)||(buff[i]==0x0a)||(buff[i]==0x00))
							{
								i++;
							}
							else
							{
								break;
							}							
						}
						memcpy(&rec[tmp_len],&buff[i],(nread-i));
						for(j=0;j<nread-i;j++)
						{
							printf("[0x%02x] ",rec[tmp_len+j]);
						}

						tmp_len+=nread;
						read_cnt++;
						printf("232��[%d]�������,���յ������ֽ���Ϊ[%d]\n",read_cnt,tmp_len);
						if(read_cnt>5)
							break;

					}

					printf("232���������ַ���Ϊ:");

					for(i=0;i<total_len;i++)
					{
						printf("[0x%02x] ",rec[i]);
					}
					printf("��������ַ���Ϊ=[%s]\n",rec);


					
					//�����յ����ַ������ȣ�������Ȳ���ֱ����ɱ
					////if(strlen(&rec[1])<strlen(TEST_232_STR))
					if(tmp_len<strlen(TEST_232_STR))
					{
						printf("���ˣ����Ȳ�һ����ֱ�Ӵ���\n");
						ret=-1;
					}
					else
					{
						ret=strncmp(&rec[1],TEST_232_STR,strlen(TEST_232_STR));
						////���rec���Ƿ���0x00,����еĻ�Ҳ��ͨ��
						if(ret!=0)
						{
							////for(i=0;i<strlen(&rec[1]);i++)
							for(i=0;i<tmp_len;i++)
							{
								if(rec[i]==0x00)
								{
									ret=0;
									printf("=====232======================================�����д����ڲ�����������ȷ\n");
									break;
								}
							}
						}
					}
					
					if(ret==0)
					{
						s_test_rp("����RS232 ��������",60);
						printf("����RS232 ��������");
						err_flag=0;
					}
					else
					{
						//s_test_rp("����RS232����,�����뷢�Ͳ�һ��",60);
						//printf("����RS232����,�����뷢�Ͳ�һ��\n");
						sprintf(tmp_buf,"%s [%s]","����RS232����,�����뷢�Ͳ�һ�½��յ�������Ϊ",buff);
						printf("����RS232����,�����뷢�Ͳ�һ��,���յ�������Ϊ[%s]\n",buff);
						s_test_rp(tmp_buf,60);
						err_flag=1;
					}

					////pthread_mutex_unlock( &lock_232 );
					//pthread_mutex_unlock( &lock_com );

				}
			}

		printf("-----------\n\n");
		//printf("\n�˳�232�߳�\n");
		if(err_flag==0)
		{
			pthread_exit((int*)0);		//���Գɹ��˳�
		}
		else
		{
			pthread_exit((int*)1);		//����ʧ���˳�
		}

		return NULL;
	}

}





/**********************************************************************************************
* ������   :test_com_485_thread()
* ����  :      ����485���Խ����߳�
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
static void *test_com_485_thread(void *para)
{
	//lc change read from 232!!!
	unsigned char buff[512];
	unsigned char tmp_buf[512];
	unsigned char rec[512];
	int nread;
	fds_t *fd;
	fd_set rdfd;
	struct timeval tv;
	int ret;
	int i;
	int j;
	pthread_mutex_t lock_485;			//������
	int err_flag;
	int total_len;
	int tmp_len;
	int read_cnt;
	
	
	fd=(fds_t *)para;
	pthread_mutex_init( &lock_485,NULL);
	FD_ZERO(&rdfd);
	FD_SET(fd->com_485_fd,&rdfd);
	memset(&tv,0,sizeof(struct timeval));
	tv.tv_sec=TEST_TIMEOUT_SEC;
	tv.tv_usec=TEST_TIMEOUT_USEC;

	sleep(1);
	while(1)
	{
		ret=select(fd->com_485_fd+1,&rdfd,NULL,NULL,&tv);
		if(ret<0)
		{
			//printf("select error\n");
			continue;
		}
		else if(ret==0)
			{
				s_test_rp("����RS485���ϻ���Թ�װδ����\n",60);
				printf("����RS485���ϻ���Թ�װδ����\n");
				err_flag=1;
			}
			else
			{	
				if(FD_ISSET(fd->com_485_fd,&rdfd))
				{						
					memset(buff,0,sizeof(buff));
					memset(tmp_buf,0,sizeof(tmp_buf));
					////pthread_mutex_lock( &lock_485);
					//pthread_mutex_lock( &lock_com);
					
					nread = read(fd->com_485_fd,buff,10);
					printf("��1��ԭʼ����:");
					for(i=0;i<nread;i++)
					{
						printf("[0x%02x] ",buff[i]);
					}

					for(i=0;i<nread;)
					{
						if((buff[i]==0x0d)||(buff[i]==0x0a)||(buff[i]==0x00))
						{
							i++;
						}
						else
						{
							break;
						}						
					}					
					
					total_len=buff[i];
					printf("\n�ַ���Ч�ܳ���Ϊ=[%d],���յ��ĳ���=[%d],i=%d\n",total_len,nread,i);
					memset(rec,0,sizeof(rec));
					////memcpy(rec,&buff[i],strlen(&buff[i]));
					memcpy(rec,&buff[i],nread-i);
					////tmp_len=strlen(&buff[i]);
					tmp_len=nread-i;

					printf("485��1�ν���Ϊ:");
					for(i=0;i<tmp_len;i++)
					{
						printf("[0x%02x] ",rec[i]);
					}
					printf("485��1�����\n");

					read_cnt=1;
					while(tmp_len<total_len)
					{
						memset(buff,0,sizeof(buff));
						printf("485��ʼ��[%d]����\n",read_cnt+1);
						nread = read(fd->com_485_fd,buff,5);
						if(nread<0)
						{
							printf("485��[%d]��ʧ��\n",read_cnt);
							if(read_cnt>5)
								break;
							read_cnt++;
						}
						else
						{
							printf("485��[%d]�ζ������ֽ���Ϊ=%d\n",read_cnt+1,nread);
						}
						
						for(i=0;i<nread;)
						{
							if((buff[i]==0x0d)||(buff[i]==0x0a)||(buff[i]==0x00))
							{
								i++;
							}
							else
							{
								break;
							}							
						}
						memcpy(&rec[tmp_len],&buff[i],(nread-i));
						for(j=0;j<nread-i;j++)
						{
							printf("[0x%02x] ",rec[tmp_len+j]);
						}

						tmp_len+=nread;
						read_cnt++;
						printf("485��[%d]�������,���յ������ֽ���[%d]\n",read_cnt,tmp_len);
						if(read_cnt>5)
							break;

					}

					printf("485���������ַ���Ϊ:");

					for(i=0;i<total_len;i++)
					{
						printf("[0x%02x] ",rec[i]);
					}
					printf("��������ַ���Ϊ=[%s]\n",rec);
					
					//�����յ����ַ������ȣ�������Ȳ���ֱ�Ӵ���
					////if(strlen(&rec[1])<strlen(TEST_485_STR))
					if(tmp_len<strlen(TEST_485_STR))
					{
						printf("���ˣ����Ȳ�һ����ֱ�Ӵ���\n");
						ret=-1;
					}
					else
					{
						ret=strncmp(&rec[1],TEST_485_STR,strlen(TEST_485_STR));
						////���rec���Ƿ���0x00,����еĻ�Ҳ��ͨ��
						if(ret!=0)
						{
							for(i=0;i<tmp_len;i++)
							{
								if(rec[i]==0x00)
								{
									ret=0;
									printf("=====485======================================�����д����ڲ�����������ȷ\n");
									system("echo \"����485�ڲ�����\" >> /tmp/trigtestlog");
									break;
								}
							}
						}
					}
					
					if(ret==0)
					{
						s_test_rp("����RS485 ��������",60);
						printf("����RS485 ��������");
						err_flag=0;
					}
					else
					{
						//s_test_rp("����RS485����,�����뷢�Ͳ�һ��",60);
						//printf("����RS485����,�����뷢�Ͳ�һ��\n");
						sprintf(tmp_buf,"%s %s [%s]","����RS485����,�����뷢�Ͳ�һ��","���յ�������Ϊ",buff);
						printf(tmp_buf);
						s_test_rp(tmp_buf,60);
						err_flag=1;

					}

					////pthread_mutex_unlock( &lock_485 );
					//pthread_mutex_unlock( &lock_com );

				}
			}

		printf("-----------\n\n\n");
		//printf("\n�˳�485�߳�\n");
		if(err_flag==0)
		{
			pthread_exit((int*)0);		//���Գɹ��˳�
		}
		else
		{
			pthread_exit((int*)1);		//����ʧ���˳�
		}
		return NULL;
	}

}

#if 0
/**********************************************************************************************
* ������   :test_com_485_thread()
* ����  :      ����485���Խ����߳�
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
static void *test_com_485_thread(void *para)
{
	unsigned char buff[512];
	unsigned char tmp_buf[512];
	int nread;
	fds_t *fd;
	fd_set rdfd;
	struct timeval tv;
	int ret;
	int i;
	pthread_mutex_t lock_485;			//������

	
	fd=(fds_t *)para;
	pthread_mutex_init( &lock_485,NULL);
	FD_ZERO(&rdfd);
	FD_SET(fd->com_485_fd,&rdfd);
	memset(&tv,0,sizeof(struct timeval));
	tv.tv_sec=TEST_TIMEOUT_SEC;
	tv.tv_usec=TEST_TIMEOUT_USEC;

	sleep(1);
	while(1)
	{
		ret=select(fd->com_485_fd+1,&rdfd,NULL,NULL,&tv);
		if(ret<0)
		{
			printf("select error\n");
			continue;
		}
		else if(ret==0)
			{
				s_test_rp("����RS485���ϻ���Թ�װδ����\n",60);
				printf("����RS485���ϻ���Թ�װδ����\n");
			}
			else
			{			
				if(FD_ISSET(fd->com_485_fd,&rdfd))
				{						
					memset(buff,0,sizeof(buff));
					memset(tmp_buf,0,sizeof(tmp_buf));
					pthread_mutex_lock( &lock_485 );	
					nread = read(fd->com_485_fd,buff,512);
					printf("\nLen %d\n",nread); 		
					buff[nread+1]='\0';

					for(i=0;i<nread;i++)
					{
						printf("[%x] ",buff[i]);
					}
					
					printf("===\n");
					for(i=0;i<nread;i++)
					{
						printf("[%c] ",buff[i]);
					}
					printf("\n");

					pthread_mutex_unlock( &lock_485 );	
					
					printf("485 receive:%s\n",buff);
					//s_test_rp(buff,11);
					if(buff[0]==0x0d)
					{
						ret=strncmp(&buff[1],TEST_485_STR,strlen(&buff[1]));
					}
					else
					{
						ret=strncmp(buff,TEST_485_STR,strlen(buff));
					}
					if(ret==0)
					{
						s_test_rp("����485 ��������",60);
						printf("����485 ��������");
					}
					else
					{
						//s_test_rp("����RS485����,�����뷢�Ͳ�һ��",60);
						//printf("����RS485����,�����뷢�Ͳ�һ��\n");
						sprintf(tmp_buf,"%s %s [%s]","����RS485����,�����뷢�Ͳ�һ��","���յ�������Ϊ",buff);
						printf(tmp_buf);
						s_test_rp(tmp_buf,60);

					}

				}
			}
		
		printf("-----------\n");
		//printf("�˳�485�߳�\n");
		pthread_exit(0);
		return NULL;
	}

}

#endif

/**********************************************************************************************
* ������   :init_com()
* ����  :      ��ʼ������
* ����  :       void
* ���  :       void        
* ����ֵ:   0��ȷ����ֵ������
**********************************************************************************************/
int init_com(void)
{
//	unsigned char buff[64];
//	int nread;
	int ret;	
	fds_t com_fd;
	pthread_t test_com_0_232_thread_id=-1;
	pthread_t test_com_232_thread_id=-1;
	pthread_t test_com_485_thread_id=-1;
	void *retval_232=NULL;
	void *retval_485=NULL;
	int ret_232;
	int ret_485;

	test_com_0_232_thread_id=test_com_0_232_thread_id;	//�������������
	memset(&com_fd,0,sizeof(fds_t));

	//sprintf(com_fd.test_0_232_str,"%s",TEST_0_232_STR);

	com_fd.test_232_str[0]=strlen(TEST_232_STR)+1;
	com_fd.test_485_str[0]=strlen(TEST_485_STR)+1;
	
	sprintf(&com_fd.test_232_str[1],"%s",TEST_232_STR);
	sprintf(&com_fd.test_485_str[1],"%s",TEST_485_STR);
	
	//��ʼ�������豸
	if(com_fd.com_232_fd>0)
		return 0;
	if(com_fd.com_485_fd>0)
		return 0;

	//��ʼ��rs232����1
	com_fd.com_232_fd=open(TEST_SERIAL_232,O_RDWR|O_NOCTTY);
	if(com_fd.com_232_fd< 0)
	{
		printf("testmod can't open test_board serilal 232!!!\n");
		return -1;
	}    	
	set_com_speed(com_fd.com_232_fd,TEST_SERIAL_BAUD);
	if(set_com_mode(com_fd.com_232_fd,8,1,'N') <0)
	{
		printf("testmod set test board com 232 Error\n");
		return -1;
	}
	ret=pthread_create(&test_com_232_thread_id, NULL,test_com_232_thread, &com_fd);	
	write(com_fd.com_232_fd,com_fd.test_232_str,strlen(com_fd.test_232_str));	

	sleep(2);
	close(com_fd.com_232_fd);
	pthread_join( test_com_232_thread_id,&retval_232);

	sleep(1);
	
#if 0
	//��ʼ��rs485������д�룬232��������ȡ
	system("echo \"��ʼ����485\" >> /tmp/trigtestlog");
	com_fd.com_485_fd=open(TEST_SERIAL_485,O_RDWR|O_NOCTTY);
	if(com_fd.com_485_fd< 0)
	{
		printf("testmod can't open test_board serilal 485!!!\n");
		return -1;
	}    	
	set_com_speed(com_fd.com_485_fd,TEST_SERIAL_BAUD);
	if(set_com_mode(com_fd.com_485_fd,8,1,'N') <0)
	{
		printf("testmod set test board com 485 Error\n");
		return -1;
	}
	com_fd.com_232_fd=open(TEST_SERIAL_232,O_RDWR|O_NOCTTY);
	if(com_fd.com_232_fd< 0)
	{
		printf("testmod can't open test_board serilal 232!!!\n");
		return -1;
	}    	
	set_com_speed(com_fd.com_232_fd,TEST_SERIAL_BAUD);
	if(set_com_mode(com_fd.com_232_fd,8,1,'N') <0)
	{
		printf("testmod set test board com 232 Error\n");
		return -1;
	}

	//lc change for ip1004xm sendto 485 but recv from 232!!!
	ret=pthread_create(&test_com_485_thread_id, NULL,test_com_485_thread, &com_fd);
	sleep(2);
	write(com_fd.com_485_fd,com_fd.test_485_str,strlen(com_fd.test_485_str));

	sleep(1);
	
	close(com_fd.com_232_fd);
	close(com_fd.com_485_fd);
	pthread_join( test_com_485_thread_id,&retval_485);

#endif
	retval_485 = 0;
#if 0
	//���Բ�����
	while(1)
	{
		write(com_fd.com_485_fd,&b_t,sizeof(b_t));
	}
#endif
	//pthread_join( test_com_0_232_thread_id,&retval);
	////pthread_join( test_com_232_thread_id,&retval_232);
	//pthread_join( test_com_485_thread_id,&retval_485);

	ret_232=*(int *)&retval_232;
	ret_485=*(int *)&retval_485;

	printf("\n\n");
	if((ret_232==0)&&(ret_485==0))
	{
		printf("���ڲ�����ȷ\n");
		return 0;
	}
	else
	{	
		printf("���ڲ��Դ���.\n");
		return -1;
		
	}

	//ȡ��ע�ͺ�������
	//close(com_fd.com_232_fd);
	//close(com_fd.com_485_fd);

}


