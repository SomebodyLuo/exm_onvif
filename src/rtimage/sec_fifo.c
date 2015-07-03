#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<pthread.h>
#include"sec_fifo.h"


//sec_buffer_t sec_buff;

static int sec_max_len=SEC_MAX_LEN;

/*************************************************************
 * ��������:	sec_buff_write()
 * ����:	�򻺳���д������
 * �������:	*sec_buff,*buff,buff_len
 * ����ֵ:	��ȷ������ֵ����ʾд��ĳ��ȣ���ֵ����
 * ��ע:	д�����������ظ��ĸ���д����д��һ�ֺ����û��д
 * 		����ô��ת���������Ŀ�ͷ���¿�ʼд���������û��
 * 		����ô���ظ�ת���������Ŀ�ͷ�����¸���ԭ��������
 * 		����д��ֱ����ȫ������д��λ��
 * *************************************************************/
int sec_buff_write(sec_buffer_t *sec_buff,char *buff,int buff_len)
{
	int valid_len;		//���ó���
	int *in=&sec_buff->in;	//׼��д���λ��
	int buff_cnt=0;		//Դ���ݻ�������ָ��
	int left;		//ʣ����ֽ���

	left=buff_len;

	//check the sec_buff 
	if(sec_buff->buffer==NULL)
	{
		printf("error,NOINIT\n");
		return -1;
	}

	//TODO Lock the process--------->
	//pthread_mutex_lock(&sec_buff->lock);
	while(left>0)
	{
		valid_len=sec_buff->size-*in;	
		printf("IN:	valid_len=%d\n",valid_len);
		if(left<valid_len)
		{
			//copy all the data to the sec_buff
			DBG("IN:	left < valid_len\n");
			DBG("IN-b:	buff_cnt=[%d],left=[%d],valid_len=[%d],in=[%d]\n",buff_cnt,left,valid_len,*in);
			memcpy(&sec_buff->buffer[*in],(buff+buff_cnt),left);	
			*in = (*in+left) % sec_buff->size; //update the wp
			buff_cnt=left;
			left=0;
			DBG("IN-a:	buff_cnt=[%d],left[%d],valid_len=[%d],in=[%d]\n",buff_cnt,left,valid_len,*in);
		}
		else
		{
			//left >=valid_len	
			DBG("IN:	left > valid_len\n");
			DBG("IN-b:	buff_cnt=[%d],left=[%d],valid_len[%d],in=[%d]\n",buff_cnt,left,valid_len,*in);
			memcpy(&sec_buff->buffer[*in],(buff+buff_cnt),valid_len);
			left -=valid_len;
			*in = (*in+valid_len) % sec_buff->size;
			buff_cnt+=valid_len;
			DBG("IN-a:	buff_cnt=[%d],left=[%d],valid_len[%d],in=[%d]\n",buff_cnt,left,valid_len,*in);
		}
	}
	//<--------------------------
	//pthread_cond_signal(&sec_buff->isvalid);
	//pthread_mutex_unlock(&sec_buff->lock);
        //NOTE::MUST be sure the array is filled with a string
        //DBG("IN:	SUCCESSED.sec_buff->buffer=[%s]\n",sec_buff->buffer);
	return buff_cnt;
}

/***************************************************************
 *��������:	sec_buff_read()
 *����:		�ӻ������ж�ָ����������
 *���������	*sec_buff	ָ�򻺳����ṹ��ָ��
 *		*buff		ָ������Ļ�����ָ��
 *		len		��ȡ�ĳ���
 *����ֵ:	��ֵ�������ֽ�������ֵ��ʾʧ��
 *��ע��	��ȡ������д�����ݲ�ͬ����ȡִֻ��һ�Σ�������while
 *		ѭ��������Ϊд��������Ǹ���д��ģ����Լ�ʹʹ��
 *		ѭ�����������������Ҳ�����¸��ǵ����ݡ�
 *		�˽ӿڴӻ��λ������ж�ȡָ�����ȵ�����:
 *		1.�����ǰ׼����ȡ��λ����д���λ��֮ǰʱ��
 *		(1.1)��ǰ��ȡλ��+��ȡ����<=׼��д���λ�ã����ȡ
 *		ָ�������ֽ�����
 *		(1.2)��ǰ��ȡλ��+��ȡ����>׼��д���λ�ã����ȡ
 *		(׼��д��λ��-׼����ȡλ��)�ĳ����ֽ�����
 *		2.�����ǰ׼����ȡ��λ����д���λ��֮��ʱ��
 *		(2.1)��ǰ��ȡλ��+��ȡ����<=size����ʱ�����ȡָ��
 *		���ȵ��ֽ�����
 *		(2.2)��ǰ��ȡλ��+��ȡ����>size����ʱ�����ȶ�ȡ
 *		(size-��ǰ��ȡλ��)�����ֽ������ٶ�ȡʣ�³���
 *		(��ȡ����-size+��ǰ��ȡλ��)�����ֽ���;
 * ************************************************************/
int sec_buff_read(sec_buffer_t *sec_buff,char *buff,int len)
{
	int *out=&sec_buff->out;
	int *in=&sec_buff->in;
	int buff_cnt=0;
	int tmp_len=0;
	int size=sec_buff->size;

	
	if(sec_buff->stop!=0)
	{
		printf("stop=1,Don NOT read\n");
		return -1;
	}

	DBGR("OUT:	read_len=%d\n",len);
	//pthread_mutex_lock(&sec_buff->rlock);
  
	//pthread_cond_wait(&sec_buff->isvalid,&sec_buff->rlock);               //waitting
        
	if(*out<=*in)	
	{
		if(*out+len<=*in)
		{
			DBGR("OUT:	out+len < in\n");
			DBGR("OUT-b:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
			memcpy(buff,&sec_buff->buffer[*out],len);
                    memset(&sec_buff->buffer[*out],0,len);
			*out= (*out +len)%size;
			buff_cnt+=len;
			DBGR("OUT-a:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
		}
		else
		{
			//TODO in = out ? it will be read in the next cycle
			DBGR("OUT:	out+len > in\n");
			DBGR("OUT-b:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
			tmp_len=*in-*out;
			memcpy(buff,&sec_buff->buffer[*out],tmp_len);
                        memset(&sec_buff->buffer[*out],0,tmp_len);
			*out = (*out +tmp_len)%size;
			buff_cnt+=tmp_len;
			DBGR("OUT-a:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
		}
	}
	else	
	{
		if(*out+len<=size)
		{
			DBGR("OUT:	out+len <= size\n");
			DBGR("OUT-b:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
			memcpy(buff,&sec_buff->buffer[*out],len);
			*out = (*out +len)%size;
			buff_cnt+=len;
			DBGR("OUT-a:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
		}
		else
		{
			DBGR("OUT:	out+len > size\n");
			DBGR("OUT-b:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
			tmp_len=size-*out;
			memcpy(buff,&sec_buff->buffer[*out],tmp_len);
                        memset(&sec_buff->buffer[*out],0,tmp_len);
			*out = (*out +tmp_len)%size;
			buff_cnt+=tmp_len;
			tmp_len=len-tmp_len;
			memcpy(&buff[buff_cnt],&sec_buff->buffer[0],tmp_len);
                        memset(&sec_buff->buffer[0],0,tmp_len);
			*out=(*out +tmp_len)%size;
			buff_cnt+=tmp_len;
			DBGR("OUT-a:	buff_cnt=[%d],out=[%d],len=[%d],in=[%d]\n",buff_cnt,*out,len,*in);
		}
	}
	//pthread_mutex_unlock(&sec_buff->rlock);
        //NOTE::MUST be sure the array is fill with a string.
	//DBGR("OUT:	buff=%s\n",buff);		
	return buff_cnt;
}

/*************************************************************
 *��������:	sec_buff_init()
 *����:		��ʼ��������
 *���������	��
 *����ֵ:	0��ȷ����ֵ����
 *��ע:		���仺������ʱ�򽫷���SEC_MAX_LEN���Ļ�����
 * ***********************************************************/
int sec_buff_init(sec_buffer_t *sec_buff)
{
	memset(sec_buff,0,sizeof(sec_buffer_t));
	sec_buff->buffer=(char *)malloc(sizeof(char)*SEC_MAX_LEN);
	if(sec_buff->buffer==NULL)
	{
		printf("malloc buffer error,exit\n");
		return -1;
	}
	sec_buff->size=sec_max_len-1;

	pthread_mutex_init(&sec_buff->lock,NULL);
        pthread_mutex_init(&sec_buff->rlock,NULL);
        pthread_cond_init(&sec_buff->isvalid,NULL);
	return 0;
}

/*************************************************************
 *��������:	sec_buff_exit()
 *����:		�ͷŻ������ڴ棻
 *�������:	ָ�򻺳����ṹ��ָ��
 *����ֵ:	��
 * **********************************************************/
void sec_buff_exit(sec_buffer_t *sec_buff)
{
	//free(sec_buff->buffer);
	//sec_buff->buffer=NULL;
}


/************************************************************
 *��������:	sec_buff_remain()
 *����:		��ȡ��ǰ����������δ��ȡ�����ݵĳ���
 *�������:	*sec_buff	ָ�򻺳����ṹ��ָ��
 *����ֵ:	����������δ��ȡ�ĳ���
 * **********************************************************/
int sec_buff_remain(sec_buffer_t *sec_buff)
{
	int len;
	int in;
	int out;
	int size;

	pthread_mutex_lock(&sec_buff->lock);
	in=sec_buff->in;
	out=sec_buff->out;
	size=sec_buff->size;
	if(in>=out)
	{
		len= in-out;
	}
	else
	{
		len=in+size-out;
	}
	pthread_mutex_unlock(&sec_buff->lock);			

	return len;

}

/************************************************************
 *��������:	sec_buff_resize() 
 *����:		�ı仺������С
 *�������:	resize	��ָ���Ļ�������С�����ֵӦ��С��
 *			sec_max_len
 *����ֵ:	��
 * **********************************************************/
void sec_buff_resize(sec_buffer_t *sec_buff,int resize)
{
	if(resize>=SEC_MAX_LEN)
	{
		resize=SEC_MAX_LEN;
	}
	pthread_mutex_lock(&sec_buff->lock);
	sec_buff->size=resize;
	if(sec_buff->in >= sec_buff->size)
	{
		sec_buff->in = sec_buff->size;
	}
	if(sec_buff->out >= sec_buff->size)
	{
		sec_buff->out = sec_buff->size;
	}
	pthread_mutex_unlock(&sec_buff->lock);
}


/*************************************************************
*��������:  sec_buff_get_size()
*����:              ��ȡ��ǰ��������С
*�������:  *sec_buff   ָ�򻺳����ṹ��ָ��
*����ֵ:        ��ǰ��������С
*��ע:              ��
**************************************************************/
int sec_buff_get_size(sec_buffer_t *sec_buff)
{
    int size;

    //pthread_mutex_lock(&sec_buff->lock);
    size=sec_buff->size;
    //pthread_mutex_unlock(&sec_buff->lock);

    return size;
}

#if 0
//����Ϊ���Գ���
/************************************************************
 *��������:	sec_buff_get_data() 
 *����:		��ȡ�������е�����
 *�������:	para
 *����ֵ:	��
 * **********************************************************/
void *sec_buff_get_data(void *para)
{
	char buff[1024];

	while(1)
	{
		if(sec_buff.stop==1)
		{
			sleep(1);
			continue;
		}
		
		sleep(2);
		memset(buff,0,sizeof(buff));
		sec_buff_read(&sec_buff,buff,6);
		printf("THREAD:	read_sec_buff=%s\n",buff);
	}

	return NULL;
}

/************************************************************
 *
 *
 *
 * **********************************************************/
void exit_handle(int signo)
{
	switch(signo)
	{
		case SIGSEGV:
			printf("�����˶δ�����\n");
			sec_buff_exit(&sec_buff);
			exit(0);
			break;

		case SIGINT:
			printf("ctrl+c ��\n");
			sec_buff_exit(&sec_buff);
			exit(0);
			break;

		default:
			sec_buff_exit(&sec_buff);
			break;
	}
}

/*************************************************************
 *��������:	main()
 *���ܣ�	������
 *���������	��
 *����ֵ:	0��ȷ����ֵ����
 * **********************************************************/
int main(int argc,char *argv[])
{
	pthread_t write_id;
	pthread_t read_id;
	char buff[1024];
	char read_buff[1024];
		
	char *cmd=NULL;
	int len = 0;
	int remain=0;
	int resize_cnt=0;

	if(argc>2)
	{
		cmd=strdup(argv[2]);
		len=strlen(cmd);
	}
	
	sec_max_len=atoi(argv[1]);
	printf("sec_max_len=%d\n",sec_max_len);	

	signal(SIGSEGV,exit_handle);
	signal(SIGINT,exit_handle);
	//init the struct
	sec_buff_init(&sec_buff);	

	//sec_buff_write(&sec_buff,cmd,len);
	pthread_create(&read_id,NULL,sec_buff_get_data,NULL);
	while(1)
	{
		resize_cnt++;
		memset(buff,0,sizeof(buff));
		printf("input:\n");
		fgets(buff,sizeof(buff),stdin);
		buff[strlen(buff)-1]=0;
		printf("i got the string=%s,len=%d\n",buff,strlen(buff));
		sec_buff_write(&sec_buff,buff,strlen(buff));
		remain=sec_buff_remain(&sec_buff);
		if(resize_cnt>5)
		{
			sec_buff_resize(&sec_buff,15);
			if(resize_cnt==10)resize_cnt=0;
		}
		else
		{
			sec_buff_resize(&sec_buff,7);
		}
		printf("==========remain=[%d],======resize_cnt=[%d]======size=[%d]\n",remain,resize_cnt,sec_buff.size);
	}

	printf("sec_buff->buffer=%s\n",sec_buff.buffer);

	memset(read_buff,0,sizeof(read_buff));
	sec_buff_read(&sec_buff,read_buff,6);

	sec_buff_read(&sec_buff,read_buff,6);
	sec_buff_read(&sec_buff,read_buff,6);



	sec_buff_exit(&sec_buff);

	return 0;
}

#endif
