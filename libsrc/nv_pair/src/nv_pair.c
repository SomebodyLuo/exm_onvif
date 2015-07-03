/*
  nv_pair.c
  
*/
#define  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nv_pair.h"

#define MAGIC	0xeb90eb90

static const char	*nvp_default_seperator	= "^^";	//Ĭ�ϵķָ���
static const char	*nvp_default_equal_mark	= "=="; //Ĭ�ϵ���ȷ�
#if 1

typedef	struct{
//������ֵ�Ե����ݽṹ���⺯��ʹ�ô˽ṹ Ӧ�ó���ʹ��NVP_T�ṹ
	unsigned long magic;
	 int str_len;			//�����ַ�������
	 int num_nvp;		//��ֵ�Ե�����
	 char seperator[MAX_SEP_LEN];		// ��ŷָ���
	 char equal_mark[MAX_MARK_LEN];	//��ŵ��ں�
	 char nvp[MAX_CMD_NUM][MAX_DATA_LEN];	//�����ֵ��
	 char msg_buf[MAX_CMD_NUM*MAX_DATA_LEN];// �����Ҫ�������ַ������Ѿ���֯�õ��ַ���
	//FILE *nv_fp;							//�����м�ֵ�Ե��ļ����
}NVP_T;

/*
*************************************************************************
*������	:nvp_set_seperator
*����	: ���÷ָ��� * 
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
			const char * seperator < �ָ����ִ� 
*���	: ��ȷ0  ������
*�޸���־:
*************************************************************************
*/
int nvp_set_seperator(NVP_TP *nv,  const char * seperator)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL||(strlen(seperator)>MAX_SEP_LEN))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	memset(nt->seperator, 0 , MAX_SEP_LEN);
	memcpy(nt->seperator , seperator , strlen(seperator));
	return NVP_SUCCESS;
}
/*
*************************************************************************
*������	:nvp_get_pair_index  �ڲ�����
*����	:  ���Ѿ����õ���ֵ���в��Ҳ���������
*����	:  
		NVP_TP	*nv,          	֮ǰʹ��nvp_create�õ���ָ��
		const char * name, 	< �� 
*���	: �ɹ����������� ��ֵ��ʾ����
*�޸���־:
*************************************************************************
*/
//static __inline__ int nvp_get_pair_index(NVP_T *nv, const	char * name)
static int nvp_get_pair_index(NVP_T *nv, const	char * name)
{
	int i;
	char* p= NULL;
	char* index= NULL;
	
	if((nv==NULL)||(name==NULL)||(strlen(name)>MAX_DATA_LEN))
	{
		return -NVP_PARA_ERR;
	} 

	for(i=0;i<nv->num_nvp;i++)
	{
		p=nv->nvp[i];
//// lsk 2008 -11-28 
		if(strncmp(p, nv->seperator,strlen(nv->seperator)))
			continue;
		p+=strlen(nv->seperator);
		if(strncmp(p, name,strlen(name)))
			continue;
		p+=strlen(name);
		if(strncmp(p, nv->equal_mark,strlen(nv->equal_mark)))
			continue;
		return i;
#if 0		
		index=strstr(p,name);
		if(index!=NULL)
		return i;
#endif
	}

	return -NVP_PARA_ERR;
}

/*
*************************************************************************
*������	:nvp_set_equal_mark
*����	: ��ȷ�
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
			const char * mark 	  �ָ����ִ� 
*���	: ��ȷ0  ������
*�޸���־:
*************************************************************************
*/
int nvp_set_equal_mark( NVP_TP	*nv, const char * mark)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL||(strlen(mark)>MAX_MARK_LEN))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	memset(nt->equal_mark, 0 ,MAX_MARK_LEN);
	memcpy(nt->equal_mark , mark , strlen(mark));
	return 0;
}

/*
*************************************************************************
*������	:nvp_create
*����	: ����һ����ֵ�Խṹ
*����	:  ��
*���	: 
*�޸���־:
*************************************************************************
*/
NVP_TP	*nvp_create(void)
{
	NVP_T* nvp_st=NULL;
	int i;
	nvp_st = malloc(sizeof(NVP_T));
	if(nvp_st == NULL)
	{
		//errno=NVP_NO_MEM;
		return NULL;
	}
	memset(nvp_st->msg_buf, 0 , sizeof(nvp_st->msg_buf));
	for(i=0;i<MAX_CMD_NUM;i++)
	{
		memset(&nvp_st->nvp[i], 0 , MAX_DATA_LEN);
	}
	nvp_st->magic = MAGIC;	// set magic byte 
	nvp_st->num_nvp = 0;
	nvp_st->str_len = 0;
	nvp_set_equal_mark(nvp_st, nvp_default_equal_mark);
	nvp_set_seperator(nvp_st, nvp_default_seperator);
	

	
	return nvp_st;
}

/*
*************************************************************************
*������	:nvp_set_pair_str
*����	: ѹ����ֵ��
* ע��: �����ͬ��ֵ�����ݴ���������滻ԭ��ֵ
*����	:  
		NVP_TP	*nv,          	֮ǰʹ��nvp_create�õ���ָ��
		const char * name, 	< �� 
		const char * value 	< ֵ 
*���	: 0��ʾ�ɹ���ֵ��ʾ����
*�޸���־: 2006 -9 -26 nvp_set_pair -> nvp_set_pair_str
*************************************************************************
*/
int nvp_set_pair_str(NVP_TP *nv, const char * name, const char * value 	)
{
	int index = 0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL)||(name==NULL)||(value==NULL))
	{
		return -NVP_PARA_ERR;
	}
	if((strlen(name)+strlen(value))>MAX_DATA_LEN)
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 
	
	// ����ͬ����ֵ��
	index = nvp_get_pair_index(nt, name); 
	if(index < 0)	// û�з�����ͬ����ֵ��
	{
		sprintf(nt->nvp[nt->num_nvp],"%s%s%s%s", 
			nt->seperator , name, nt->equal_mark, value);
		nt->num_nvp++;
		if(nt->num_nvp>MAX_CMD_NUM)
		{
			return -NVP_NO_MEM;
		}
	}
	else		//ˢ��ԭ��������
	{
		memset(nt->nvp[index], 0 , MAX_DATA_LEN);
		sprintf(nt->nvp[index],"%s%s%s%s",nt->seperator ,
		name, nt->equal_mark, value);
	}
	return NVP_SUCCESS;
}
/*
*************************************************************************
*������	:nvp_set_pair_int
*����	: ѹ����ֵ��(����)
* ע��: �����ͬ��ֵ�����ݴ���������滻ԭ��ֵ
*����	:  
		NVP_TP	*nv,          	֮ǰʹ��nvp_create�õ���ָ��
		const char * name, 	< �� 
		int value 	< ֵ 
*���	: 0��ʾ�ɹ���ֵ��ʾ����
*�޸���־:
*************************************************************************
*/
int nvp_set_pair_int(NVP_TP *nv, const char * name, int value)
{
	char buf[20];
	memset(buf, 0 , 20);
	sprintf(buf, "%d",value);
	return(nvp_set_pair_str(nv, name, (const char*)buf));
}

/*
*************************************************************************
*������	:nvp_get_pair_str
*����	: �������Ƶõ�ֵ,���δ�ҵ��򷵻�dev_val
*����	:  
		NVP_TP	*nv,          	֮ǰʹ��nvp_create�õ���ָ��
		const char * name, 	< �� 
		const char * def_val 	< Ĭ��ֵ 
*���	: 0��ʾ�ɹ���ֵ��ʾ����
*�޸���־:2006 -9 -26  nvp_get_pair -> nvp_get_pair_str
*************************************************************************
*/
const char * nvp_get_pair_str(NVP_TP *nv, const	char * name , const	char * def_val)
{
	int i;
	int len =strlen(name); //// lsk 2008 -11-28
	char* p= NULL;
	char* s = NULL;
	char* index= NULL;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	
	if((nv==NULL)||(name==NULL)||(len>MAX_DATA_LEN))
	{
		return NULL;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return NULL;	// not an initilized struct pointer 
	}
	i=nvp_get_pair_index(nt, name);
	if(i<0)
		return def_val;
	p=nt->nvp[i];
	s=strstr(p,nt->equal_mark);
	if(s)
		s+=strlen(nt->equal_mark);
	else
		return def_val;
	if(*s=='\0')
	{
		return def_val;
	}
	return s;	
#if 0
	for(i=0;i<nt->num_nvp;i++)
	{
		p=nt->nvp[i];

		index=strstr(p,name);
		if(index==NULL)
			continue;
		else
		{
			s = index + len;
	  		if(strncmp(s,nt->equal_mark ,strlen(nt->equal_mark))==0)
	  		{
	  			s+=strlen(nt->equal_mark);
	  			if(*s=='\0')
				{
					return def_val;
				}
				return s;
	  		}
			continue;	//���Ƿָ����ͼ�������
		}
	}
	return def_val;
#endif
}
/*
*************************************************************************
*������	:nvp_get_pair_int
*����	: �������Ƶõ�ֵ,���δ�ҵ��򷵻�dev_val
*����	:  
		NVP_TP	*nv,          	֮ǰʹ��nvp_create�õ���ָ��
		const char * name, 	< �� 
		const char * def_val 	< Ĭ��ֵ 
*���	: ���ҵ�������ֵ
*�޸���־:2006 -9 -26  nvp_get_pair -> nvp_get_pair_str
*************************************************************************
*/
int nvp_get_pair_int(NVP_TP *nv, const	char * name , const int def_val)
{
	const char* ret=NULL; 
	ret = nvp_get_pair_str(nv, name,NULL);
	if(ret == NULL)
	{
		return def_val;
	}
	else
	{
		return atoi(ret);
	}
}


/*
************************************************************************
*������	:nvp_get_string
*����	: �õ�������ֵ�ԵĴ��Ӹ�ʽ
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
*���	: �ַ���ָ��
*�޸���־:
*************************************************************************
*/
const char *nvp_get_string(NVP_TP	*nv)
{
	int i;
	int len=0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL)
	{
		return NULL;
	} 

	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return NULL;	// not an initilized struct pointer 
	} 

	nt->str_len = 0;
	len = 0;
	//���������
	memset(nt->msg_buf , 0 , MAX_CMD_NUM*MAX_DATA_LEN);
	// �����е���ֵ����䵽������
	for(i=0; i<nt->num_nvp; i++)
	{
		len = strlen(nt->nvp[i]);
		memcpy(&nt->msg_buf[nt->str_len] , nt->nvp[i] , len);
		nt->str_len+=len;
	}
	return nt->msg_buf+strlen(nt->seperator);
//	return nt->msg_buf;
}

 /*
************************************************************************
*������	:nvp_parse_string
*����	:  ������ֵ�ԵĴ��Ӹ�ʽ
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
			const char *  str      ��ֵ�ԵĴ��Ӹ�ʽ 
*���	:  �����õ���ֵ�Ե�����
*�޸���־:
*************************************************************************
*/
int nvp_parse_string(NVP_TP *nv, const char *  str )
{
	char* p = NULL;
	char* s = NULL;
	char* index = NULL;
	int offset =0;
	int i = 0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL)||(str==NULL))
	{
		return -NVP_PARA_ERR;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	if((strlen(str))>(MAX_CMD_NUM*MAX_DATA_LEN))
	{
		return -NVP_NO_MEM;
	} 
	//���������
	memset(nt->msg_buf , 0 , MAX_CMD_NUM*MAX_DATA_LEN);
	for(i=0; i<nt->num_nvp; i++)
	{
		memset(nt->nvp[i], 0 , MAX_DATA_LEN);
	}
	////ǰ��Ĳ��Ƿָ���
	if(strncmp(str, nt->seperator,strlen(nt->seperator))!=0)
	{
	////���Ϸָ���
		memcpy(nt->msg_buf, nt->seperator, strlen(nt->seperator));
		memcpy(&nt->msg_buf[strlen(nt->seperator)], str, strlen(str));
	}
	else
	{
		memcpy(nt->msg_buf, str, strlen(str));
	}
	nt->str_len = strlen(nt->msg_buf);
	nt->num_nvp = 0;
#if 1
	index = nt->msg_buf;
	for(i=0;i<MAX_CMD_NUM;i++)
	{
		p = strstr(index, nt->seperator);
		if(p!=NULL)
		{
			index = p+strlen(nt->seperator);
			s = strstr(index, nt->seperator);
			if(s!=NULL)		// û�е���β
			{
				offset = (int )(s - p);
				if(offset>(int)strlen(nt->seperator))	//�������������ķָ���
				{
					memcpy(nt->nvp[nt->num_nvp], p, offset);
					nt->num_nvp++;
				}
			}
			else				// �������ݰ��Ľ�β
			{
				offset = (int )(nt->msg_buf+(nt->str_len) - p);
				memcpy(nt->nvp[nt->num_nvp], p, offset);
				nt->num_nvp++;
			}
		}
		else 
		{
			if(nt->num_nvp==0)
			{
				return -NVP_PARA_ERR;
			}
			break;
		}
	}
#endif	
	return nt->num_nvp;
}

/*
************************************************************************
*������	:nvp_dump
*����	:  ��ӡ������ֵ�Ե�����(������)
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
*���	:  �����õ���ֵ�Ե�����
*�޸���־:
*************************************************************************
*/
int nvp_dump(NVP_TP *nv )
{
	int i;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;

	if((nv==NULL))
	{
		return -NVP_PARA_ERR;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	printf("there are %d pairs in buffer\n",  nt->num_nvp);
	for(i=0;i<nt->num_nvp;i++)
	{
		printf("%s\n",nt->nvp[i]);
	}
	return NVP_SUCCESS;
}
/*
************************************************************************
*������	:nvp_get_count
*����	:  �õ���ֵ�Ե�����
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
*���	:  �����õ���ֵ�Ե�����
*�޸���־:
*************************************************************************
*/
int nvp_get_count(NVP_TP *nv)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	return nt->num_nvp;
}

 /*
************************************************************************
*������	:nvp_destroy
*����	:  ����һ���Ѿ�ʹ�ù���nvp�ṹ
*����	:  
			NVP_TP	*nv,          ֮ǰʹ��nvp_create�õ���ָ��
*���	:  ��
*�޸���־:
*************************************************************************
*/
void nvp_destroy(NVP_TP	*nv)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;

	if((nv!=NULL)&&((nt->magic)==MAGIC))
	{
		nt->magic=0;	
		free(nv);
	}
}

 /*
************************************************************************
*������	:nvp_get_error_str
*����	:  ��ȡ��������ַ�������
*����	:  
			int errno, �ӿڷ��صĴ������ľ���ֵ
*���	:     ���������ַ���ָ��
*�޸���־:
*************************************************************************
*/
const char *nvp_get_error_str(int errno)
{
	switch(errno)
	{
		case NVP_NO_MEM: 
			return "not enough memory";
			break;
		case NVP_PARA_ERR: 
			return"parameters error";
			break;
		default:
			return "unknow nvp errno";
			break;
	};
}

#if 0
/************************************************************************
*������	:nvp_open_file
*����	:  ��һ����ֵ���ļ�
*����	:  �ļ���
			int errno, �ӿڷ��صĴ������ľ���ֵ
*���	:     ���������ַ���ָ��
*�޸���־:
*************************************************************************/
int nvp_open_file(NVP_TP *nv,char *filename)
{
	FILE *fp;

	if(filename==NULL)
	{
		gtlogerr("�ļ�[%s]Ϊ��\n",filename);
		printf("�ļ� [%s]Ϊ��\n",filename);
		return -1;
	}
	fp=fopen(filename,"r");
	if(fp==NULL)
	{
		gtlogerr("���ļ�[%s]��ȡnvp��Ϣʧ��\n",filename);
		printf("���ļ�[%s]��ȡnvp��Ϣʧ��\n",filename);
		return -1;
	}

	return 0;
}
#endif

 
#endif
