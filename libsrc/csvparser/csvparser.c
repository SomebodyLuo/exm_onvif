
/**
 * csv��ʽ������
 * ˵��: csv��ʽ����','�ָ��ļ�¼
 * csv��ʽ:
 * .ÿ����¼ռһ�� 
 * .�Զ���Ϊ�ָ��� 
 * .����ǰ��Ŀո�ᱻ���� 
 * ?.�ֶ��а����ж��ţ����ֶα�����˫���������� 
 * ?.�ֶ��а����л��з������ֶα�����˫���������� 
 * ?.�ֶ�ǰ������пո񣬸��ֶα�����˫���������� 
 * ?.�ֶ��е�˫����������˫���ű�ʾ 
 * ?.�ֶ��������˫���ţ����ֶα�����˫���������� 
 * .��һ�м�¼���������ֶ��� 
 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <file_def.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <csvparser.h>
//#include <gtthread.h>
#include <commonlib.h>

#define MAGIC			0xeb90eb90
#define TEMP_REC_FILE	"/tmp/csvtemp.txt"
#define INSERT_HEAD		0		//���ļ�ͷ�����¼��Ϣ
#define INSERT_TAIL		1		//���ļ�β�������¼��Ϣ
#define CHANGE_MID		2		//�޸�/����ļ��м�ļ�¼��Ϣ
#define INSERT_MID		3		//���ļ��м����/��Ӽ�¼��Ϣ

#define CUT_HEAD		0		
#define CUT_MID			1
#define CUT_TAIL			2

#define VERSION			"0.02"
/*
V0.02������һ�������кţ�ɾ��ָ��һ�м�¼�ĺ���
�޸����ļ���û�м�¼����0������
*/


typedef struct 
{
	int atom_num;			//Ԫ�صĸ���
	unsigned long int magic;	//������
//���Ԫ���ֶ�
	char csv_buf[MAX_ATOM_NUM][MAX_ATOM_LEN];		
//���������¼��Ϣ
	char csv_str[MAX_ATOM_LEN*MAX_ATOM_NUM+MAX_ATOM_NUM];	
}CSV_ST;

//static pthread_mutex_t	CSV_file_mutex=PTHREAD_MUTEX_INITIALIZER;		
//����ֵ����
/*
*************************************************************************
*������	:rm_unused_space
*����	: ɾ���ַ���ǰ��Ŀո񣬵���������ԭ���Ļ�����
*����	:  char* para
*���	: char* para
*�޸���־:
*************************************************************************
*/
static __inline__ int csv_rm_unused_space(char * para)
{	
	char *p_str;
	int i;
	int len;
	char buf[MAX_ATOM_LEN];
	char swap_buf[MAX_ATOM_LEN];
	if(para==NULL)
	{
		return -CSV_PARA_ERR;
	}
	if(strlen(para)>sizeof(buf))
	{
		return -CSV_NO_MEM;
	}
	memset(buf, 0, sizeof(buf));
	memset(swap_buf, 0, sizeof(swap_buf));
	if(strlen(para)>0)
	{
		memcpy(buf, para, strlen(para));
	}
	else
	{
		return CSV_SUCCESS;
	}
	len = strlen(para);
	///remove space at end of string
	for(i=0;i<len;i++)
	{
		p_str=strrchr(buf, ' ');
		if(p_str!=NULL)
		{
			if(p_str==(buf+strlen(buf)-1))
			{
				*p_str = 0;
				continue;
			}
			else
			{ 
				break;
			}
		}
		else
		{
			break;
		}
	}

	///remove space at beginning of string
	for(i=0;i<len;i++)
	{
		p_str=strchr(buf, ' ');
		if(p_str!=NULL)
		{
			if(p_str==buf)
			{
				memcpy(swap_buf, &buf[1], (strlen(buf)-1));
				memset(buf, 0, strlen(buf));
				memcpy(buf,swap_buf, strlen(swap_buf));
				memset(swap_buf, 0, sizeof(swap_buf));
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	memset(para, 0 ,len);
	memcpy(para, &buf, strlen(buf));
//	printf("%s\n", para);
	return CSV_SUCCESS;
}
/*
*************************************************************************
*������	:csv_rm_enter
*����	: ɾ���ַ�����Ļ���
*����	:  char* para
*���	: char* para
*�޸���־:
*************************************************************************
*/
static __inline__ int csv_rm_enter(char * para)
{
	int len;
	if(para==NULL)
		return -CSV_PARA_ERR;
	len = strlen(para);
	if((para[len-1]==0x0a)&&(para[len-2]==0x0d))
		para[len-2]=0;
	return CSV_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
*������	:csv_get_line_num
*����	: ���û�ָ�����к�ת����������к�
*����	:  
			int total  �ܹ�����������
			int no 	�û���Ҫ�������к�
*���	: 	����Ĳ����к�
*�޸���־:
*************************************************************************
*/
static __inline__ int csv_get_line_num(int total, int no)
{
	if(no>0)
	{
		if(no>total)
		{
			return total;
		}
		else
		{
			return no;
		}
	}
	if(no<0)
	{
		if(-no>total)
		{
			return 1;
		}
		else
		{
			return total+no+1;
		}
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
*������	:csv_create
*����	: ����һ������csv��¼�����ݽṹ
*����	:  ��
*���	: 
*�޸���־:
*************************************************************************
*/
 CSV_T	*csv_create(void)
{
	CSV_ST* cs = NULL;
	cs = malloc(sizeof(CSV_ST));
	if(cs==NULL)
	{
		return NULL;
	}
	cs->magic = MAGIC;
	cs->atom_num = 0;
	memset(cs->csv_buf, 0 , sizeof(cs->csv_buf));
	memset(cs->csv_str, 0 , sizeof(cs->csv_str));
	return cs;
}
/*
*************************************************************************
*������	:csv_set_str
*����	:����csv�ṹ�б�����ֵ(�ַ�����)
* ע��  :�����ͬλ�õ����ݴ���������滻ԭ��ֵ
*����	:  
		CSV_T	*csv,          	֮ǰʹ��csv_create�õ���ָ��
		const 	int no,		�������(����)
		const char * value 	����ֵ 
*���	: 0��ʾ�ɹ���ֵ��ʾ����
*�޸���־:
*************************************************************************
*/
 int csv_set_str(CSV_T	*csv, const int no, const char  *value)
{
	CSV_ST *cs=NULL;
	int index;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
		
	if((no<=0)||(value==NULL)||(no>MAX_ATOM_NUM)||(strlen(value)>MAX_ATOM_LEN))
	{
		return -CSV_PARA_ERR;
	}
	index = no-1;
	if(cs->csv_buf[index][0]==0)
	{
		cs->atom_num++;
	}
	else
	{
		memset(cs->csv_buf[index], 0 , sizeof(cs->csv_buf[index]));
	}
	memcpy(cs->csv_buf[index], value, strlen(value));
	csv_rm_unused_space(cs->csv_buf[index]);
	return CSV_SUCCESS;
}
/*
*************************************************************************
*������	:csv_set_int
*����	:����csv�ṹ�б�����ֵ(����ֵ)
* ע��  :�����ͬλ�õ����ݴ���������滻ԭ��ֵ
*����	:  
		CSV_T	*csv,          	֮ǰʹ��csv_create�õ���ָ��
		const char * name, 	< �� 
		const char * value 	< ֵ 
*���	: 0��ʾ�ɹ���ֵ��ʾ����
*�޸���־:
*************************************************************************
*/
 int csv_set_int(CSV_T	*csv,    const int	no, const int	value )
{
	char buf[MAX_ATOM_LEN];
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", value);
	return csv_set_str(csv , no , (const char *)buf);
}

/* 
 *************************************************************************
 *������:csv_get_int
 *����	:��ȡcsv�ṹ��ָ����ŵı���(������)
 *����	:  
		IN CSV_T 	*csv, 		//֮ǰʹ��csv_create�õ���ָ��
		IN const int	no, 		//�������
		IN const int 	def_val		//û���ҵ�����ʱ�ķ���ֵ
 *����ֵ: 	ָ����ű�����ֵ,û���ҵ��򷵻�def_val
 *�޸���־:
 *************************************************************************
*/
int csv_get_int(CSV_T 	*csv, const int no, const int def_val	)
{
	const char* p_str=NULL;
	char buf[MAX_ATOM_LEN];
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", def_val);
	p_str = csv_get_str(csv,no,(const char *)buf);
	if(p_str == buf)
	{
		return def_val;		
	}
	return atoi(p_str);
}
/* 
 *************************************************************************
 *������:csv_get_str
 *����	:��ȡcsv�ṹ��ָ����ŵı���(�ַ�����)
 *����	:  
		IN CSV_T 	*csv, 		//֮ǰʹ��csv_create�õ���ָ��
		IN const int	no, 		//�������
		IN const char 	*def_val	//û���ҵ�����ʱ�ķ���ֵ
 *����ֵ: 	ָ����ű�����ֵ,û���ҵ��򷵻�def_val
 *�޸���־:
 *************************************************************************
 */
const char *csv_get_str(CSV_T *csv, const int no, const char *def_val)
{
	CSV_ST* cs=NULL;
	int index;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return def_val;
	}
////���û�ж�Ӧ��ŵĲ����ͷ���Ĭ��ֵ
	if((no>MAX_ATOM_NUM)||(no<=0))
	{
		return def_val;
	}
	index = no-1;
	if((cs->csv_buf[index][0])==0)
	{
		return def_val;
	}
	return (const char *)cs->csv_buf[index];
}
/* 
 *
 *************************************************************************
 *������:csv_get_var_num
 *����	:��ȡcsv�ṹ�б���������
 *����	:  
		IN CSV_T 	*csv, 		//֮ǰʹ��csv_create�õ���ָ��
 *����ֵ: 	��ֵ��ʾ����,�Ǹ�ֵ��ʾcsv�ṹ�б���������
 *�޸���־:
 *************************************************************************
 */
int csv_get_var_num(CSV_T 	*csv)
{
	CSV_ST* cs=NULL;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	return cs->atom_num;
}
/*
************************************************************************
*������	:csv_get_string
*����	:�õ�csv�ṹ���ַ�������
*����	:  
		CSV_T	*csv,          ֮ǰʹ��csv_create�õ���ָ��
*���	: ����csv�ṹ���ַ���ָ��
*�޸���־:
*************************************************************************
*/
const char * csv_get_string( CSV_T *csv)
{
	CSV_ST* cs=NULL;
	int i;
	int len;

	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return NULL;
	}
	memset(cs->csv_str, 0 ,sizeof(cs->csv_str));
	len = 0;
	// ���������ֶ�����һ���ַ���
	for(i=0;i<cs->atom_num;i++)
	{
		sprintf(&cs->csv_str[len], "%s,", cs->csv_buf[i]);
		len = strlen(cs->csv_str);
	}
//	printf("%s!!!!!!!!!!!!",cs->csv_str);
	////���ַ���ĩβ���ַ�(',') ����'\r' ��'\n'
//	cs->csv_str[len]=0x0d;
//	cs->csv_str[len+1]=0x0a;
#if 1
	if((cs->csv_str[len-1]!=0x0a)&&(cs->csv_str[len-1]!=0x0d))
	{
		cs->csv_str[len-1]=0x0d;
		cs->csv_str[len]=0x0a;
	}
#endif
//	printf("%s!!!!!!!!!!!!",cs->csv_str);
	return cs->csv_str;
}

/*
************************************************************************
*������	:csv_parse_string
*����	:  ����csv��ʽ���ַ������ṹ
*����	:  
		IN const char *  str 		//����csv��ʽ���ַ���
*���	: 
		IN CSV_T	*csv,        	//֮ǰʹ��csv_create�õ���ָ��
*����ֵ :	0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
 int csv_parse_string( CSV_T	*csv, const char *  str )
{
	CSV_ST* cs=NULL;
	char* p = NULL;
	char* s = NULL;
	char* index = NULL;
	int offset =0;
	int i = 0;
	int len=0;
	cs = (CSV_ST*)csv;
	
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if(strlen(str)>(MAX_ATOM_LEN*MAX_ATOM_NUM+1))
	{
		return -CSV_NO_MEM;
	}
////��ս��ջ�����
	memset(cs->csv_str, 0 , sizeof(cs->csv_str));
	memset(cs->csv_buf, 0 , sizeof(cs->csv_buf));
	cs->atom_num = 0;
	index = cs->csv_str;
////��ǰ�����"," ��Ϊ���
	if(*str!=',')
	{
		cs->csv_str[0] = ',';
		memcpy(&cs->csv_str[1], str, strlen(str));
	}
	else
	{
		memcpy(cs->csv_str, str, strlen(str));
	}

//	printf("%s\n",cs->csv_str);
	len = strlen(cs->csv_str);
	for(i=0;i<MAX_ATOM_NUM;i++)
	{
		p = strchr(index, ',');
		if(p!=NULL)
		{
			index = p + 1;
			////�����������ŵļ������ֶγ���
			s = strchr(index, ',');
			if(s!=NULL)
			{
				offset =(int)(s-index);
			}
			else				// �������ݰ��Ľ�β
			{
				offset = (int)(cs->csv_str + len - index);
			}
////�ֶ� ��Ϊ��
			if(offset>=1)
			{
				memcpy(cs->csv_buf[cs->atom_num], index, offset);
//				printf("%s\n",cs->csv_buf[cs->atom_num]);
			}
////�ֶ�Ϊ��
			else
			{
				; 	//��ʱ��������
			}
			cs->atom_num++;
		}
		else
		{
			if(cs->atom_num==0)	//û���ҵ��κη��ϸ�ʽ���ֶ�
			{
				return -CSV_PARSER_ERR;
			}
			break;
		}
	}
	for(i=0;i<cs->atom_num;i++)
	{
//		printf("%d\n",strlen(cs->csv_buf[i]));
		csv_rm_unused_space(cs->csv_buf[i]);	//ɾ���ֶ�ǰ��û�õĿո�
//		printf("%s",cs->csv_buf[i]);
	}
	return CSV_SUCCESS;
}
 /*
************************************************************************
*������	:csv_destroy
*����	:  ����һ���Ѿ�ʹ�ù���csv�ṹ
*����	:  
			CSV_T	*csv,          ֮ǰʹ��csv_create�õ���ָ��
*���	:  ��
*�޸���־:
*************************************************************************
*/
void csv_destroy(CSV_T *csv)
{
	CSV_ST* cs=NULL;
	cs = (CSV_ST*)csv;

	if((cs!=NULL)&&((cs->magic)==MAGIC))
	{
		cs->magic=0;	
		free(cs);
	}
}
/*
************************************************************************
*������	:csv_get_error_str
*����	:  ��ȡ��������ַ�������
*����	:  
		int errno, �ӿڷ��صĴ������ľ���ֵ
*����ֵ	:     ���������ַ���ָ��
*�޸���־:
*************************************************************************
*/
const char *csv_get_error_str(int errno)
{
	switch(errno)
	{
		case CSV_NO_MEM:
		return "not enough memory";
		break;
		
		case CSV_PARA_ERR:
		return "parameters error";
		break;

		case CSV_PARSER_ERR:
		return "can not parser string";
		break;
		
		case CSV_OPEN_FILE_ERR:
		return "can not open file";
		break;

		case CSV_LOCK_FILE_ERR:
		return "can not lock file";
		break;
		
		case CSV_NO_RECORD:
		return "can not find records in file";
		break;
		
		default:
		return "unknow errno";
		break;	
	};
}
#if 1
////added by lsk 2006 -11-7
//ǿ�Ƽ����ļ�  
//���� fd �ļ������� ,cmd ����, wait �ȴ���־
//����0 �ɹ� -1 ʧ��
// cmd = F_RDLCK ����ֹ �� F_WRLCK д��ֹ; F_UNLCK �������
// wait = 0 �޷��������������أ� =1 �ȴ�����
int force_lockfile(int fd, short int cmd, int wait)
{
//	int ret;
	struct flock tp;
	if(fd<0)
		return -1;
	memset(&tp , 0, sizeof(tp));		/// ��������
//	printf("cmd = %d , wait = %d, fd = %d \n", cmd, wait, fd);
	tp.l_type = cmd;
	tp.l_whence = SEEK_SET;
	tp.l_len = 0; 
//	tp.l_start = 0;
	tp.l_pid = getpid();
//	printf("wait = %d pid = %d \n", wait, tp.l_pid);
	if(wait ==0)
	{
		return fcntl(fd , F_SETLK, &tp);
	}
	if(wait ==1)
	{
		return fcntl(fd , F_SETLKW, &tp);
	}
	
	return fcntl(fd , F_SETLKW, &tp);
}
/*
************************************************************************
*������	:csvfile_lock
*����	:��csv�ļ���������
*����	: 	 IN const char *filename;	//csv�ṹ�ļ�
*����ֵ	:�ɹ������ļ������֣���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
static __inline__ int csvfile_lock( const char *filename)
{
	int fd = -1;
	fd=open(filename, O_RDONLY);
	if(fd<0)
	{
		return -1;
	}
	if(lock_file(fd, 1)<0)
	{
		perror("lock file error");
		close(fd);
		return -1;
	}
	return fd;
}
/*
************************************************************************
*������	:csvfile_open
*����	:��csv�ļ�
*����	: 	 IN const char *filename;	//csv�ṹ�ļ�
			 IN const char* mode // ���ļ��ķ�ʽ
*����ֵ	:�ɹ������ļ������֣���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
static __inline__ FILE* csvfile_open( const char *filename, const char* mode )
{
	FILE *fp=NULL;
	if(filename==NULL)
	{
		return NULL;
	}
	fp = fopen(filename,mode);
	return fp; 
}

/*
************************************************************************
*������	:csvfile_close
*����	:�ر�csv�ļ���
*����	: IN const char *filename;	//csv�ṹ�ļ�
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
static __inline__ int csvfile_close( FILE * csv_fp)
{
//	int fd = -1;
	if(csv_fp==NULL)
	{
		return -CSV_PARA_ERR;
	}
//	fd = fileno(csv_fp);
	fclose(csv_fp);
	return 0;
}

/*
************************************************************************
*������	:csvfile_get_total_records
*����	:��ȡһ���ļ��е�csv��¼����
*����	:  	 IN const char *filename;	//csv�ṹ�ļ�
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int csvfile_get_total_records(const char *filename)
{
	FILE* fp=NULL;
	char s[1024];
	int index;
	int fd;
	index=0;
	if(filename==NULL)
	return -CSV_PARA_ERR;
	fd = csvfile_lock(filename);
	if(fd<0)
	return -CSV_LOCK_FILE_ERR;

	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	return -CSV_OPEN_FILE_ERR;
	memset(s, 0 ,sizeof(s));
	while(fgets(s, sizeof(s),fp)!=NULL)
	{
		index++;
	}
	fclose(fp);
	unlock_file(fd);
	close(fd);
	return index;
}
/*
************************************************************************
*������	:csvfile_get_record
*����	:��csv�ļ��л�ȡָ����¼�ŵĽṹ��Ϣ
*����	:
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
*��� 	:OUT CSV_T *csv;		//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*ע:Ŀǰֻ����˴�ĩβ׷�Ӽ�¼�Ĺ���
*************************************************************************
*/
int csvfile_get_record(const char *filename , int record_no, CSV_T *csv)
{
	FILE* fp=NULL;
	CSV_ST* cs=NULL;
	int ret, index;
	int i, fd;
	char buf[1024];

	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if((record_no==0)||(filename==NULL))
	{
		return -CSV_PARA_ERR;
	}
	
	ret = csvfile_get_total_records(filename);
	if(ret <=0)
		return ret;

	index = csv_get_line_num(ret , record_no);
	
	
	
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}
	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		return -CSV_OPEN_FILE_ERR;
	}
	for(i=0;i<index;i++)
	{
		memset(buf,0,sizeof(buf));
		fgets(buf, sizeof(buf),fp);
	}
	fclose(fp);
	unlock_file(fd);
	close(fd);
	csv_rm_enter(buf);
	ret = csv_parse_string(cs, (const char *) buf);
	return ret;
}
/*
************************************************************************
*������	:csvfile_set_sub
*����	:��csv�ṹд��csv�ļ���
*����	:
	 IN int cmd;		//������
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;		//csv�ṹ�еļ�¼��(����)
* 	 IN CSV_T *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
ע��: ��Ҫ����һ����ʱ�ļ�����ȡ�����屣��
*************************************************************************
*/
static __inline__ int csvfile_set_sub(int cmd, const char *filename, int record_no, CSV_ST *csv)
{
	FILE* fp = NULL;
	FILE *fp_temp = NULL;
	CSV_ST* cs = NULL;
	const char *p_str = NULL;
	int i;
	int fd = -1;
	int fd_temp = -1;
	char temp_file[]="/tmp/csvtemp-XXXXXX";
	char buf[1024];
//	record_no=1;//test !!
	if(filename==NULL)
	return -CSV_PARA_ERR;
	cs = csv;
	p_str = csv_get_string(cs);
	if(p_str==NULL)
	{
		return -CSV_PARA_ERR;
	}
//	printf("%s!!!!!!!!!!!!!!!!!!",p_str); // test
	memset(buf, 0, sizeof(buf));
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}
/// ֻ��Ҫ׷�ӵ��������Ƚϼ�
	if(cmd==INSERT_TAIL)
	{
		fp = csvfile_open(filename, "a+");
		if(fp==NULL)
		{
			unlock_file(fd);
			close(fd);
			return -CSV_OPEN_FILE_ERR;
		}
		fprintf(fp,"%s",p_str);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return CSV_SUCCESS;
	}
	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}
	////����������ʱ�ļ�
	fd_temp = mkstemp(temp_file);
	if(fd_temp<0)
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_LOCK_FILE_ERR;
	}
	fp_temp = fdopen(fd_temp, "w+");	//ת��Ϊ�ļ�ָ��
	if(fp_temp==NULL)
	{
		close(fd_temp);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}

	switch(cmd)
	{
		case INSERT_HEAD:		// ���ļ�ͷ�����¼��Ϣ
		fprintf(fp_temp,"%s",p_str);
		while((i=fgetc(fp))!=EOF)
		{
			fprintf(fp_temp,"%c",i);
		}
		break;
			
		case INSERT_TAIL:		//���ļ�β�������¼��Ϣ
		while((i=fgetc(fp))!=EOF)
		{
			fprintf(fp_temp,"%c",i);
		}
		fprintf(fp_temp,"%s",p_str);
		break;
//�м�����µļ�¼��Ϣ��ԭ���ļ�¼��ŵ�����Ϣ����
		case INSERT_MID:			
		i=0;
		while(fgets(buf, sizeof(buf),fp)!=NULL)
		{
			i++;
			if(i==record_no)		//Ҫ�������к�
			{
				fprintf(fp_temp,"%s", p_str);	
				fprintf(fp_temp, "%s",buf);		// ԭ���ļ�¼������
			}
			else
			{
				fprintf(fp_temp, "%s",buf);
			}
			memset(buf, 0, sizeof(buf));
		}
		break;

		case CHANGE_MID:			//�����м�ļ�¼��Ϣ
		i=0;
		while(fgets(buf, sizeof(buf),fp)!=NULL)
		{
			i++;
			if(i==record_no)			//Ҫ����ĵ��к�
			{
				fprintf(fp_temp,"%s", p_str);	
			}
			else
			{
				fprintf(fp_temp, "%s",buf);
			}
			memset(buf, 0, sizeof(buf));
		}
		break;

		default:
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//ɾ����ʱ�ļ�
		system(buf);
		return -CSV_PARA_ERR;
		break;
	}
	fflush(fp_temp);
	fclose(fp);
	fclose(fp_temp);
	close(fd_temp);
	
////����ʱ�ļ��е�����д���ļ���
	fp_temp = fopen(temp_file, "r");
	fp = csvfile_open(filename, "w+");
	if((fp==NULL)||(fp_temp==NULL))
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//ɾ����ʱ�ļ�
		system(buf);
		return -CSV_OPEN_FILE_ERR;
	}
	while((i=fgetc(fp_temp))!=EOF)
	{
		fprintf(fp,"%c",i);
	}
	fflush(fp);
	fclose(fp);
	unlock_file(fd);
	close(fd);
	fclose(fp_temp);
	close(fd_temp);
	sprintf(buf, "rm %s \n", temp_file);	//ɾ����ʱ�ļ�
	system(buf);
	return CSV_SUCCESS;
}
/*
************************************************************************
*������	:csvfile_set_record
*����	:��csv�ṹд��csv�ļ���
*����	:  
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 				//����ʱ����������������ʾ���ļ�β������
					//����ʱ���������Χ���ʾ���ײ�����
* 	 IN CSV_T *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int csvfile_set_record(const char *filename, int record_no, CSV_T *csv)
{
	CSV_ST* cs=NULL;
//	char *p_str=NULL;
	int line_num;
//	int ret;
	if(filename==NULL)
	return -CSV_PARA_ERR;	
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	return -CSV_PARA_ERR;

//	if(record_no==0)
//	return -CSV_PARA_ERR;
//	printf("%s\n", filename);
////��ȡ�ļ��м�¼���ݵ���Ŀ
	line_num = csvfile_get_total_records(filename);
	if(line_num <0)
	{
		return line_num;
	}
	if((line_num==0)||(record_no==0))
		return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
		
////���ݼ�¼���кŽ���Ϣ��¼���ļ���	
	if(record_no>0)
	{
		if(record_no<=line_num)
		{
			return(csvfile_set_sub(CHANGE_MID, filename, record_no, cs));
		}
		else
		{
			return(csvfile_set_sub(INSERT_TAIL, filename, line_num+1, cs));
		}
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		{
			return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
		}
		else 
		{
			return(csvfile_set_sub(CHANGE_MID, filename, line_num+record_no+1, cs));
		}
	}
	return -CSV_PARA_ERR;
}

/*
************************************************************************
*������	:csvfile_insert_record
*����	:��csv�ṹ����csv�ļ���
*����	:  
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;			//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 						//����ʱ����������������ʾ���ļ�β������
							//����ʱ���������Χ���ʾ���ײ�����
* 	 IN CSV_T *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int csvfile_insert_record(const char *filename , int record_no, CSV_T *csv)
{
	CSV_ST* cs=NULL;
//	char *p_str=NULL;
	int line_num;
	int ret;

	if(filename==NULL)
	return -CSV_PARA_ERR;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if((record_no==1)||(record_no==0))
	{
		return csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
	}
	if(record_no==-1)
	{
//		printf("set tail \n");
//		return csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
		return csvfile_set_sub(INSERT_TAIL, filename, -1, cs);//
	}
//	printf("%s\n", filename);
////��ȡ�ļ��м�¼���ݵ���Ŀ
	line_num = csvfile_get_total_records(filename);
	if(line_num <0)
	{
		return line_num;
	}
	if(line_num==0)
		return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
////���ݼ�¼���кŽ���Ϣ��¼���ļ���	
//	printf("%s\n", filename);
	if(record_no>0)
	{
		if(record_no<=line_num)
		{
			ret = csvfile_set_sub(INSERT_MID, filename, record_no, cs);
//			printf("ERROR INSERT_MID\n");
			return ret;
		}
		else
		{
			ret = csvfile_set_sub(INSERT_TAIL, filename, line_num+1, cs);
			return ret;
		}
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		{
			ret = csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
			return ret;
		}
		else 
		{
			ret = csvfile_set_sub(INSERT_MID, filename, line_num+record_no+1, cs);
			return ret;
		}
	}
	return -CSV_PARA_ERR;

}
/*
************************************************************************
*������	:csvfile_rm_record_sub
*����	:���ļ��е�һ����¼ɾ��
*����	:
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;		//csv�ṹ�еļ�¼��(����)
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
ע��: ��Ҫ����һ����ʱ�ļ�����ȡ�����屣��
*************************************************************************
*/
static __inline__ int csvfile_rm_record_sub(const char *filename, int record_no)
{
	FILE* fp = NULL;
	FILE *fp_temp = NULL;
	int i, line=record_no;	
	int fd = -1;
	int fd_temp = -1;
	char temp_file[]="/tmp/csvtemp-XXXXXX";
	char buf[1024];

	if((filename==NULL))
	return -CSV_PARA_ERR;
	if(line==0)
		line=1;
	memset(buf, 0, sizeof(buf));
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}

	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}
	////����������ʱ�ļ�
	fd_temp = mkstemp(temp_file);
	if(fd_temp<0)
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_LOCK_FILE_ERR;
	}
	fp_temp = fdopen(fd_temp, "w+");	//ת��Ϊ�ļ�ָ��
	if(fp_temp==NULL)
	{
		close(fd_temp);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}

	i=0;
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		i++;
		if(i==line)		//Ҫ��ɾ�����к�
		{
			continue;
//			fprintf(fp_temp,"%s", p_str);	
//			fprintf(fp_temp, "%s",buf);		// ԭ���ļ�¼������
		}
		else
		{
			fprintf(fp_temp, "%s",buf);
		}
		memset(buf, 0, sizeof(buf));
	}
	fflush(fp_temp);
	fclose(fp);
	fclose(fp_temp);
	close(fd_temp);
	
////����ʱ�ļ��е�����д���ļ���
	fp_temp = fopen(temp_file, "r");
	fp = csvfile_open(filename, "w+");
	if((fp==NULL)||(fp_temp==NULL))
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//ɾ����ʱ�ļ�
		system(buf);
		return -CSV_OPEN_FILE_ERR;
	}
	while((i=fgetc(fp_temp))!=EOF)
	{
		fprintf(fp,"%c",i);
	}
	
	fflush(fp);
	fclose(fp);
	unlock_file(fd);
	close(fd);
	fclose(fp_temp);
	close(fd_temp);
	sprintf(buf, "rm %s \n", temp_file);	//ɾ����ʱ�ļ�
	system(buf);
	return CSV_SUCCESS;
}
/*
************************************************************************
*������	:csvfile_rm_record
*����	:��filename�ļ��еĵ�record_no����Ϣɾ����
*����	:  
	 IN const char *filename;	//csv�ṹ�ļ�
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 				//����ʱ����������������ʾ���ļ�β��ɾ��
					//����ʱ���������Χ���ʾ���ײ�ɾ��
*����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
int csvfile_rm_record(const char *filename, int record_no)
{
	int line_num;
	int line=1;
	if(filename==NULL)
	return -CSV_PARA_ERR;

//	if(record_no==0)
//	return -CSV_PARA_ERR;
//	printf("%s\n", filename);

////��ȡ�ļ��м�¼���ݵ���Ŀ
	line_num = csvfile_get_total_records(filename);
	if(line_num <=0)
	{
		return line_num;
	}
		
////ɾ����¼	
	if(record_no>0)
	{
		if(record_no<=line_num)
		line = record_no;
		else
		line = line_num;
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		line = 1;
		else 
		line =  line_num+record_no+1;
	}
	return(csvfile_rm_record_sub(filename,line));
}
#if 0
/*
************************************************************************
*������	:csvfile_insert_record_head
*����	:��csv�ṹ����csv�ļ�ͷ��ԭ���ļ�¼��Ϣ����
*����	:  
	 IN const char *filename;	//csv�ṹ�ļ�
* 	 IN CSV_ST *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
static __inline__ int csvfile_insert_record_head(const char *filename, CSV_ST *csv)
{
////���ݼ�¼���кŽ���Ϣ��¼���ļ���	
	return csvfile_set_sub(INSERT_HEAD, filename, 1, csv);
}
/*
************************************************************************
*������	:csvfile_insert_record_tail
*����	:��csv�ṹ����csv�ļ�β��ԭ���ļ�¼��Ϣ����
*����	:  
	 IN const char *filename;	//csv�ṹ�ļ�
* 	 IN CSV_ST *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************
*/
static __inline__ int csvfile_insert_record_tail(const char *filename, CSV_ST *csv)
{
////���ݼ�¼���кŽ���Ϣ��¼���ļ���	
	return csvfile_set_sub(INSERT_TAIL, filename, -1, csv);
}
#endif
#endif

