#include "audiofmt.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/soundcard.h>
#include <typedefine.h>
#include "ulaw16.h"
#include "string.h"
#include <errno.h>
#ifndef DWORD
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#endif

/**********************************************************************************************
 * ������	:conv_raw2ulaw()
 * ����	:��raw-pcm��ʽ������ת��Ϊu-law������
 * ����	:source:���raw-pcm���ݵ�Դ������ָ��
 *			 len:Դ�������е���Ч�ֽ���
 * ���	:target:���ת����u-law���ݵĻ����� 
 * ����ֵ	:��ֵ��ʾ�ɹ�ת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
  **********************************************************************************************/
int conv_raw2ulaw(char* target,char *source,int len)
{
	unsigned char *t=target;
	unsigned short *s=(unsigned short*)source;
	int			l=len/2;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=linear2ulaw(*s);
		t++;
		s++;
	}
	return l;
}

#if 0	
/**********************************************************************************************
 * ������	:conv_data()
 * ����	:��������,u-law���ݿ�ת����ָ���ֽ��������ݿ�
 * ����	:source:���u-law���ݵ�Դ������ָ��
 *		tgtlen:Ŀ���ֽ���
 *		srclen:Դ�������е���Ч�ֽ���		
 * ���	:target:���ת�������ݵĻ����� 
 * ����ֵ	:��ֵ��ʾ�ɹ�ת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
  **********************************************************************************************/
int conv_data(char* target,char *source,int tgtlen, int srclen)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int i;
	int interval; //���⴦�������
	int convlen=0;
	
	if(tgtlen==srclen)//���
	{
		for(i=0;i<srclen;i++)
		{
			*t=*s;
			t++;
			s++;
		}
		return srclen;
	}
	if(tgtlen>srclen)//��Ҫ����
	{
		interval=srclen/(tgtlen-srclen);
		//printf("%s-%d\n",__FILE__,__LINE__);
		for(i=0;i<srclen;i++)
		{
			if(i%interval!=0)
			{
				*t=*s;
				t++;
				convlen++;
				s++;
			}
			else
			{
				*t=*s;
				t++;
				s++;
				*t=*(s-1);
				t++;
				convlen++;
				convlen++;
			}
			
		}
		//printf("%s-%d\n",__FILE__,__LINE__);
		return convlen;
	}
	if(tgtlen<srclen)//��Ҫѹ��
	{
		interval=srclen/(srclen-tgtlen);
		for(i=0;i<srclen;i++)
		{
			if(i%interval!=0)
			{
				*t=*s;
				t++;
				s++;
				convlen++;
			}
			else
			{
				s++;
			}
		}
		return convlen;

	}
	return 0;
}


int conv_data_new(char* target,char *source,int len)
{
	char *t=(char *)target;
	char *s=source;
	int i;
	int interval=4096;
	//char ch1,ch2,ch3,ch4;
	for(i=0;i<len;i=i+interval)
	{
		
		memcpy(t,s+len-interval,interval);
		t=t+interval;
	}

	ch1=*(s+len-4);
	ch2=*(s+len-3);
	ch3=*(s+len-2);
	ch4=*(s+len-1);
		
	
		for(i=0;i<len;i=i+4)
		{
			*t=ch1;
			t++;
			*t=ch2;
			t++;
			*t=ch3;
			t++;
			*t=ch4;
			t++;
		}

		printf("\n\nnow s:\n");
		s=(char*)source;
		for(i=0;i<len;i++)
		{
			printf("%02X ",*s);
			s++;
		}
		printf("\n\nnow t:\n");
		t=(char *)target;
		for(i=0;i<len;i++)
		{
			printf("%02X ",*t);
			t++;
		}

		return len;
}
#endif



/**********************************************************************************************
 * ������	:conv_ulaw2raw()
 * ����	:��u-law��ʽ������ת��Ϊraw-pcm������
 * ����	:source:���u-law���ݵ�Դ������ָ��
 *			 len:Դ�������е���Ч�ֽ���
 * ���	:target:���ת����raw-pcm���ݵĻ����� 
 * ����ֵ	:��ֵ��ʾ�ɹ�ת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
  **********************************************************************************************/
int conv_ulaw2raw(char* target,char *source,int len)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int			l=len;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=ulaw2linear(*s);
		t++;
		s++;
	}
	return l*2;
}

/**********************************************************************************************
 * ������	:conv_mono2stereo()
 * ����	:����������raw-pcm����ת����˫����
 * ����	:src:ָ������Դ��������ָ��
 *			 srclen:����Դ�������е���Ч�ֽ���
 * ���	:target:ת���������Ŀ�껺����ָ��
 * ����ֵ	:��ֵ��ʾת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
 **********************************************************************************************/
int conv_mono2stereo(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
#if 0
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/2;
	convlen=0;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		*pt=*ps;
		 pt++;
		 ps++;
		 convlen++;
	}
	return convlen*4;//srclen*2;
#else
	//changed by shixin ֧�ָ���(Դ��������Ŀ�껺����ʹ����ͬ�Ļ�����)
       len=srclen/2;
       ps=(WORD*)src;
	ps=&ps[len-1];
	pt=(WORD*)target;
       pt=&pt[len*2-1];
       
	convlen=0;
	for(i=0;i<len;i++)	
	{
		*pt=*ps;
		pt--;
		*pt=*ps;
		pt--;
		ps--;
		convlen++;
	}
	return convlen*4;
#endif
}

/**********************************************************************************************
 * ������	:conv_stereo2left()
 * ����	:��˫������raw-pcm����ת���ɵ�����(������)
 * ����	:src:ָ������Դ��������ָ��
 *			 srclen:����Դ�������е���Ч�ֽ���
 * ���	:target:ת���������Ŀ�껺����ָ��
 * ����ֵ	:��ֵ��ʾת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
 **********************************************************************************************/
int conv_stereo2left(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/4;
	convlen=0;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		 ps=ps+2;
		 convlen++;
	}
	return convlen*2;
}

/**********************************************************************************************
 * ������	:conv_stereo2right()
 * ����	:��˫������raw-pcm����ת���ɵ�����(������)
 * ����	:src:ָ������Դ��������ָ��
 *			 srclen:����Դ�������е���Ч�ֽ���
 * ���	:target:ת���������Ŀ�껺����ָ��
 * ����ֵ	:��ֵ��ʾת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
 **********************************************************************************************/
int conv_stereo2right(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/4;
	convlen=0;
	ps++;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		 ps=ps+2;
		 convlen++;
	}
	return convlen*2;
}
#ifdef ALAW
/**********************************************************************************************
 * ������	:conv_raw2alaw()
 * ����	:��raw-pcm��ʽ������ת��Ϊa-law������
 * ����	:source:���raw-pcm���ݵ�Դ������ָ��
 *			 len:Դ�������е���Ч�ֽ���
 * ���	:target:���ת����a-law���ݵĻ����� 
 * ����ֵ	:��ֵ��ʾ�ɹ�ת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
  **********************************************************************************************/
int conv_raw2alaw(char* target,char *source,int len)
{
	unsigned char *t=target;
	unsigned short *s=(unsigned short*)source;
	int			l=len/2;
	int			i;
	int  sign   = 0;
       int  exponent  = 0;
       int  mantissa  = 0;

	for(i=0;i<l;i++)
	{
    		sign = ((~(*s)) >> 8) & 0x80;
    		if (sign == 0)
    		{
       		 *s = (unsigned short)-(*s);
    		}
    		if (*s > 0x7F7B)
    		{
        		*s = 0x7F7B;
    		}
   		if (*s >= 0x100)
    		{
        		exponent  = (int)dsp16_alaw[((*s) >> 8) & 0x7F];
        		mantissa  = ((*s) >> (exponent + 3)) & 0x0F;
        		*t = (unsigned char)((exponent << 4) | mantissa);
    		}
    		else
    		{
        		*t = (unsigned char)((*s) >> 4);
    		}
    		(*t) ^= (unsigned char)(sign ^ 0x55);

    		t++;
    		s++;
	}
	
   	 return l;
}

/**********************************************************************************************
 * ������	:conv_alaw2raw()
 * ����	:��a-law��ʽ������ת��Ϊraw-pcm������
 * ����	:source:���a-law���ݵ�Դ������ָ��
 *			 len:Դ�������е���Ч�ֽ���
 * ���	:target:���ת����raw-pcm���ݵĻ����� 
 * ����ֵ	:��ֵ��ʾ�ɹ�ת����target�������е���Ч�ֽ��� ��ֵ��ʾ����
  **********************************************************************************************/
int conv_alaw2raw(char* target,char *source,int len)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int			l=len;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=alaw2linear(*s);
		t++;
		s++;
	}
	return l*2;
}

#endif

