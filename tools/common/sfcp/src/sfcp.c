#include<getopt.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<ftw.h>
#include<sys/types.h>
#include<dirent.h>
#include<utime.h>
#include<filelib.h>
#include<fixdisk.h>
#include<gtlog.h>
#include<devinfo.h>
#include<iniparser.h>
#include<errno.h>
#include"sfcp.h"
#include"tab_crc32.h"


SFCP_PATH_T sf_dir;
int errr;

/**********************************************************************************
 *      ������: print_help()
 *      ����:   ��ӡ������Ϣ
 *      ����:   ��
 *      ���:   ��
 *      ����ֵ: ��
 *********************************************************************************/
void print_help(void)
{
	printf("\n");
	printf("  �÷���\n");
	printf("	./sfcp [ѡ��]... Դ... Ŀ¼\n");
	printf("   ��:	./sfcp [ѡ��]... Դ    Ŀ��\n\n");
	printf("	-r:	����Ŀ¼��Ŀ¼�ڵ�������Ŀ\n");
	printf("	-f:	����ɾ������ʱ��������ʾ��Ϣ\n");
	printf("	-d:	����ʱ��������\n");
	printf("	-u:	��ʾ���ƹ���\n");
	printf("	--help:		��ʾ�˰�����Ϣ���˳�\n");
	printf("	--version:	����汾��Ϣ���˳�\n\n");
	exit(1);
}


/**********************************************************************************
 *      ������: print_ver()
 *      ����:   ��ӡ�汾
 *      ����:   ��
 *      ���:   ��
 *      ����ֵ: ��
 *********************************************************************************/
void print_ver(void)
{
	printf("\nversion : %s\n\n",SFCP_VERSION);
	exit(1);
}


/**********************************************************************************
 *      ������: get_filename_from_dir()
 *      ����:   ��·���н����ļ���
 *      ����:   dir			·��
 *				filename	�ļ���
 *      ���:   ��
 *      ����ֵ: 
 *********************************************************************************/
int get_filename_from_dir(char *dir,char *filename)
{
	char *ret=NULL;
	
	if(dir==NULL || filename==NULL)
	{
		return -1;
	}

	// Ŀ¼��·��+�ļ���
	ret=strrchr(dir,'/');
	if(ret==NULL)
	{
		//���ڱ���·��ʱ��ֱ�����ļ���
		memcpy(filename,dir,strlen(dir));
	}
	else
	{
		memcpy(filename,ret,strlen(ret));
	}


	
	return 0;
}


/**********************************************************************************
 *      ������: set_file_len()
 *      ����:   ��ȡ�ļ�����
 *      ����:   sf_p	ָ��SFCP_PATH_T��ָ��
 *			   len	�ļ�����
 *      ���:   ��
 *      ����ֵ: ��
 *********************************************************************************/
void get_file_len(SFCP_PATH_T *sf_p,int len)
{
	sf_p->src_file_len=len;
}


/**********************************************************************************
 *      ������: check_files_size()
 *      ����:   У��Դ�ļ���Ŀ���ļ���С
 *      ����:   src_file		Դ�ļ���
 *			   dst_file		Ŀ���ļ�
 *      ���:   ��
 *      ����ֵ: ��ȷ����0����ֵ������
 *********************************************************************************/
int check_files_size(const char *src_file,const char *dst_file)
{
	struct stat buf;
	unsigned src_size;
	unsigned dst_size;
	int ret;

	//��ȡԴ�ļ�״̬
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=stat(src_file,&buf);
	if(ret!=0)
	{
		errr=errno;
		printf("У��ʱ��ȡԴ�ļ�[%s]״̬����[%s]line:%d\n",src_file,strerror(errr),__LINE__);
		gtlogerr("У��ʱ��ȡԴ�ļ�[%s]״̬����[%s]line:%d\n",src_file,strerror(errr),__LINE__);
		return -1;
	}
	src_size=buf.st_size;
	//printf("Դ�ļ�[%s:%d-Bytes]\n",src_file,src_size);

	//��ȡĿ���ļ�״̬
	ret=0;
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=stat(dst_file,&buf);
	if(ret!=0)
	{
		errr=errno;
		printf("У��ʱ��ȡĿ���ļ�[%s]״̬����[%s]line:%d\n",dst_file,strerror(errr),__LINE__);
		gtlogerr("У��ʱ��ȡĿ���ļ�[%s]״̬����[%s]line:%d\n",dst_file,strerror(errr),__LINE__);
		return -1;
	}
	dst_size=buf.st_size;
	//printf("Ŀ���ļ�[%s:%d-Bytes]\n",dst_file,dst_size);
	
	if(src_size==dst_size)
	{
		//printf("Դ�ļ���Ŀ���ļ���СУ����ȷ\n");
	}
	else
	{
		printf("Դ�ļ�[%s:%d-Bytes]��Ŀ���ļ�[%s:%d-Bytes]��С��ͬ.\n",src_file,src_size,dst_file,dst_size);
		gtlogerr("Դ�ļ�[%s:%d-Bytes]��Ŀ���ļ�[%s:%d-Bytes]��С��ͬ.\n",src_file,src_size,dst_file,dst_size);
		return -1;
	}

	return 0;
}


/**********************************************************************************
 *      ������: check_file_crc32()
 *      ����:   ��CRCУ���ļ�����
 *      ����:   src_file		Դ�ļ���
 *			   dst_file		Ŀ���ļ�
 *      ���:   ��
 *      ����ֵ: ��ȷ����0����ֵ������
 *********************************************************************************/
 int check_file_crc32(char *src,char *dst)
{
	FILE *src_stream=NULL;
	FILE *dst_stream=NULL;
	char read_buf[RD_BUF_MAX_LEN];//RD_BUF_MAX_LEN
	int rd_len_cnt;
	int rd_len_src;
	int rd_len_dst;
	int ret;
	struct stat stbuf;
	unsigned long crc_src;
	unsigned long crc_dst;

	if(src==NULL || dst==NULL)
	{
		printf("src==null,dst==null\n");
		return -1;
	}

	//��Դ�ļ�
	errno=0;
	src_stream=fopen(src,"r");
	if(src_stream==NULL)
	{
		errr=errno;
		printf("���ļ�[%s]ʧ��[%s]line[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("���ļ�[%s]ʧ��[%s]line[%d]\n",src,strerror(errr),__LINE__);
		return -1;
	}
	
	//��Ŀ���ļ�
	errno=0;
	dst_stream=fopen(dst,"r");
	if(dst_stream==NULL)
	{
		errr=errno;
		printf("�����ļ�%sʧ��[%s]line[%d]\n",dst,strerror(errr),__LINE__);
		gtlogerr("�����ļ�%sʧ��[%s]line[%d]\n",dst,strerror(errr),__LINE__);
		fclose(src_stream);
		return -1;
	}

	//��ȡĿ���ļ���С
	memset(&stbuf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
		errr=errno;
		printf("��ȡ�ļ�[%s]״̬ʧ��[%s]line[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("��ȡ�ļ�[%s]״̬ʧ��[%s]line[%d]\n",src,strerror(errr),__LINE__);
		fclose(src_stream);
		fclose(dst_stream);
		return -1;
	}
	
	//��ʼ�����ļ�	
	rd_len_cnt=0;
	while(1)
	{
		//��Դ�ļ�
		memset(read_buf,0,sizeof(read_buf));
		errno=0;
		rd_len_src=fread((void *)read_buf,1,sizeof(read_buf),src_stream);
		//printf("У��������ļ�����rd_len=%d\n",rd_len);
		if(rd_len_src>=0)
		{
			rd_len_cnt+=rd_len_src;
		}
		else
		{
			errr=errno;
			printf("У���ȡԴ�ļ�[%s]����[%s]line[%d[\n",src,strerror(errr),__LINE__);
			gtlogerr("У���ȡԴ�ļ�[%s]����[%s]line[%d[\n",src,strerror(errr),__LINE__);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;
		}			
		//fflush(src_stream);
		crc_src=0;
		if(rd_len_src!=0)
		{
			crc_src=tab_crc32((const unsigned char *)read_buf,rd_len_src);
			//printf("Դ�ļ�CRCУ��ֵ[%ld]\n",crc_src);
		}
		
		//��Ŀ���ļ�
		memset(read_buf,0,sizeof(read_buf));
		errno=0;
		rd_len_dst=fread((void *)read_buf,1,sizeof(read_buf),dst_stream);
		//printf("д�ļ��ĳ���wr_len=%d\n",wr_len);
		if(rd_len_dst<0)
		{
				errr=errno;
			printf("У���ȡĿ���ļ�[%s]����[%s]line:[%d]\n",src,strerror(errr),__LINE__);
			gtlogerr("У���ȡĿ���ļ�[%s]����[%s]line:[%d]\n",src,strerror(errr),__LINE__);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;
		}	
		//fflush(dst_stream);
		crc_dst=0;
		if(rd_len_src==0)
		{
			if(rd_len_dst!=0)
			{
				printf("CRCУ��Դ�ļ�[%s]Ϊ0��Ŀ���ļ�[%s]�����Ȳ�Ϊ0������\n",src,dst);
				gtlogerr("CRCУ��Դ�ļ�[%s]Ϊ0��Ŀ���ļ�[%s]�����Ȳ�Ϊ0������\n",src,dst);
				return -1;
			}
		}
		
		if(rd_len_dst!=0)
		{
			crc_dst=tab_crc32((const unsigned char *)read_buf,rd_len_dst);
			//printf("Ŀ���ļ�CRCУ��ֵ[%ld]\n",crc_dst);
		}
		
		if(crc_src!=crc_dst)
		{
			printf("Դ�ļ�[%s][%ld]��Ŀ���ļ�[%s]CRCУ��[%ld]����\n",src,crc_src,dst,crc_dst);
			gtlogerr("Դ�ļ�[%s][%ld]��Ŀ���ļ�[%s]CRCУ��[%ld]����\n",src,crc_src,dst,crc_dst);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;	
		}
	
		if(rd_len_cnt==stbuf.st_size)
		{
			//printf("CRC232У���ļ�������.\n");
			break;
		}
		
		if(sf_dir.opt_d_flag==0)
		{
			if(rd_len_src==0)
			{
				//printf("�������ӵ��ļ�������\n");
				break;
			}
		}
	}
	
	fclose(src_stream);
	fclose(dst_stream);

	

	return 0;
}

/**********************************************************************************
 *      ������: tip_cover()
 *      ����:   
 *      ����:
 *      ���:   ��
 *      ����ֵ: 
 *********************************************************************************/
int tip_cover(void)
{
	int u_chose;
		
	u_chose=getchar();
	getchar();
	if(u_chose=='y')
	{
		return 1;
	}
	
	return 0;
}


/**********************************************************************************
 *    ������: tip_cover()
 *    ����: ������ȫ��д�뻺���� 
 *	����:	ptr		ָ����д������ݵ�ַ
 *				size		��λ�ṹ���ַ���
 *				nmemb	�ж��ٸ���λ�ṹ���ܵ�д������=size*nmemb
 *				stream	�Ѵ򿪵��ļ�ָ��
 *     
 *      ���:   ��
 *      ����ֵ: ����ʵ��д����ַ���
 *********************************************************************************/
int sf_write_buf(const char *ptr,int size,int nmemb,FILE *stream)
{
	unsigned long left_len=0;
	unsigned long wr_cnt=0;
	int ret;
		
	if(ptr==NULL || stream==NULL)
	{
		printf("sf_write_buf����ָ��ptrΪ��line:%d.\n",__LINE__);
		gtlogerr("sf_write_buf����ָ��ptrΪ��line:%d.\n",__LINE__);
		return -1;
	}

	left_len=size*nmemb;
	//printf("%d\n",__LINE__);
	while(left_len>0)
	{
		ret=fwrite(ptr,1,left_len,stream);
		if(ret<=0)
		{
			errr=errno;
			if(errr==EINTR)
			{
				ret=0;
			}
			else
			{
				if(errr>0)       // �������� û�а취
				{
					return -errr;
				}
				else
				{
					return -1;
				}
			}	
		}
		left_len-=ret;      
		wr_cnt +=ret;
 		ptr+=ret;			// ��ʣ�µĵط�����д 
        }
	fflush(stream);
	
        return wr_cnt;;
}
		

/**********************************************************************************
 *      ������: copy_files()
 *      ����:   ����Դ�ļ����ݵ�Ŀ���ļ�
 *      ����:	src		Դ�ļ�
 *				dst		Ŀ��Ŀ¼
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int trans_data_src_to_dst(const char *src,const char *dst)
{
	FILE *src_stream=NULL;
	FILE *dst_stream=NULL;
	char read_buf[RD_BUF_MAX_LEN];//RD_BUF_MAX_LEN
	int rd_len;
	int rd_len_cnt;
	int wr_len;
	int ret;
	struct stat stbuf;


	if(src==NULL || dst==NULL)
	{
		return -1;
	}

	
	//��Դ�ļ�
	src_stream=fopen(src,"r");
	if(src_stream==NULL)
	{
			errr=errno;
		printf("��Դ�ļ�[%s]ʧ��[%s],line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("��Դ�ļ�[%s]ʧ��[%s],line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}

	if(access(dst,F_OK)==0)
	{
		ret=remove(dst);
		if(ret<0)
		{
			printf("���¸����ļ�ʧ��line:%d\n",__LINE__);
			gtlogerr("���¸����ļ�ʧ��line:%d\n",__LINE__);
			return -1;
		}
			
	}
	
	//�򿪻���Ŀ���ļ�
	dst_stream=fopen(dst,"wb");
	if(dst_stream==NULL)
	{
			errr=errno;
		printf("����Ŀ���ļ�[%s]ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("����Ŀ���ļ�[%s]ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
		fclose(src_stream);
		return -1;
	}
	else
	{
		//printf("�����ļ�[%s]�ɹ�.\n",dst);
	}

	memset(&stbuf,0,sizeof(struct stat));
	//���������ӿ���
	errno=0;
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡ�ļ�[%s]״̬ʧ��[%s]line:[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("��ȡ�ļ�[%s]״̬ʧ��[%s]line:[%d]\n",src,strerror(errr),__LINE__);
		fclose(src_stream);
		fclose(dst_stream);
		return -1;
	}

	//printf("lstat�ļ�����Ϊ[%ld]\n",stbuf.st_size);

	if(sf_dir.opt_u_flag==1)
	{
		printf("[%s]----->[%s]\n",src,dst);
	}
	
	//��ʼ�����ļ�	
	rd_len_cnt=0;
	while(1)
	{
		//���ļ�
		rd_len=0;
		memset(read_buf,0,sizeof(read_buf));
		rd_len=fread((void *)read_buf,1,sizeof(read_buf),src_stream);
		//printf("���ƶ������ļ�[%s]����rd_len=%d\n",src,rd_len);
		if(rd_len>0)
		{
			rd_len_cnt+=rd_len;
		}
		else //if(rd_len<0) 
			{
				if(rd_len==0)
				{
					rd_len_cnt+=rd_len;
				}
				else
				{
						errr=errno;
					printf("���ƶ�ȡԴ�ļ�[%s]����[%s]line:%d\n",src,strerror(errr),__LINE__);
					gtlogerr("���ƶ�ȡԴ�ļ�[%s]����[%s]line:%d\n",src,strerror(errr),__LINE__);
					fclose(src_stream);
					fclose(dst_stream);
					return -1;
				}
			}			
		//fflush(src_stream);

		//д�ļ�
#if 0
		errno=0;
		wr_len=fwrite(read_buf,1,rd_len,dst_stream);
		//printf("д�ļ�[%s]�ĳ���wr_len=%d\n",dst,wr_len);
		if(wr_len!=rd_len)
		{
				errr=errno;
				printf("д�ļ�[%s]����[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				gtlogerr("д�ļ�[%s]����[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				fclose(src_stream);
				fclose(dst_stream);
				return -1;	
		}	
#endif

		wr_len=sf_write_buf(read_buf,1,rd_len,dst_stream);
		if(wr_len!=rd_len)
		{
				errr=errno;
				printf("д�ļ�[%s]����[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				gtlogerr("д�ļ�[%s]����[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				fclose(src_stream);
				fclose(dst_stream);
				return -1;	
		}	
		if(rd_len_cnt==stbuf.st_size)
		{
			//printf("�ļ�������.\n");
			break;
		}
		if(sf_dir.opt_d_flag==0)
		{
			if(rd_len==0)
			{
				//printf("�������ӵ��ļ�������\n");
				break;
			}
		}
	}
	fflush(dst_stream);
	
	fclose(src_stream);
	fclose(dst_stream);
	
	return 0;
}


/**********************************************************************************
 *      ������: set_file_time()
 *      ����:   �޸��ļ�ʱ������
 *      ����:   src		Դ�ļ�
 *				dst		Ŀ��Ŀ¼
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int change_file_attri(const char *src,const char *dst)
{
	int ret;
	struct utimbuf tmp_time;
	mode_t tmp_mode;
	mode_t d_mode;
	struct stat stbuf;

	memset(&tmp_time,0,sizeof(struct utimbuf));
	memset(&tmp_mode,0,sizeof(mode_t));
	memset(&d_mode,0,sizeof(mode_t));
	memset(&stbuf,0,sizeof(struct stat));
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡ�ļ�[%s]״̬ʧ��[%s]line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("��ȡ�ļ�[%s]״̬ʧ��[%s]line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	tmp_time.actime = stbuf.st_atime;
	tmp_time.modtime = stbuf.st_mtime;
	tmp_mode=stbuf.st_mode;
	//printf("src-actime=[%ld]\n",tmp_time.actime);
	//printf("src-modtiime=[%ld]\n",tmp_time.modtime);
	//printf("lstat---src-mode=[0x%x]\n",tmp_mode);

	memset(&stbuf,0,sizeof(struct stat));
	ret=stat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡ�ļ�[%s]״̬ʧ��[%s]line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("��ȡ�ļ�[%s]״̬ʧ��[%s]line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	//printf("stat---src-mode=[0x%x]\n",stbuf.st_mode);
	d_mode=stbuf.st_mode;

	
	errno=0;
	ret=utime(dst,&tmp_time);
	if(ret<0)
	{
			errr=errno;
		printf("����Ŀ���ļ�[%s]ʱ�����Դ���[%s]line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("����Ŀ���ļ�[%s]ʱ�����Դ���[%s]line:%d\n",dst,strerror(errr),__LINE__);
		return -1;		
	}

	if(sf_dir.opt_d_flag==1)
	{
		errno=0;
		ret=chmod(dst,tmp_mode);
		if(ret<0)
		{
				errr=errno;
			printf("����Ŀ���ļ�[%s]����ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
			gtlogerr("����Ŀ���ļ�[%s]����ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
			return -1;
		}
	}
	else
	{
		ret=chmod(dst,d_mode);
		if(ret<0)
		{
				errr=errno;
			printf("����Ŀ���ļ�[%s]����ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
			gtlogerr("����Ŀ���ļ�[%s]����ʧ��[%s]line:%d\n",dst,strerror(errr),__LINE__);
			return -1;
		}
	}




	memset(&tmp_time,0,sizeof(struct utimbuf));
	memset(&tmp_mode,0,sizeof(mode_t));
	memset(&stbuf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(dst,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡ�ļ�[%s]״̬ʧ��[%s],line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("��ȡ�ļ�[%s]״̬ʧ��[%s],line:%d\n",dst,strerror(errr),__LINE__);
		return -1;
	}
	tmp_time.actime = stbuf.st_atime;
	tmp_time.modtime = stbuf.st_mtime;
	tmp_mode=stbuf.st_mode;
	//printf("dst-actime=[%ld]\n",tmp_time.actime);
	//printf("dst-modtiime=[%ld]\n",tmp_time.modtime);
	//printf("dst-mode=[0x%x]\n",tmp_mode);





	return 0;
}

/**********************************************************************************
 *      ������: copy_files()
 *      ����:   
 *      ����:   src		Դ�ļ�
 *				dst		Ŀ��Ŀ¼
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int copy_files(const char *src,const char *dst)
{
	char dst_tmp[DST_FILENAME_MAX_LEN];
	char dst_filename[128];
	int crc_err_cnt;
	int cp_err_cnt;
	int ret;
	struct stat stbuf;


	if(src==NULL || dst==NULL)
	{
		return -1;
	}


	//���Դ�ļ����ڲ�
	if(access(src,F_OK)!=0)
	{
		printf("Դ�ļ�[%s]������\n",src);
		return -1;
	}



        memset(dst_tmp,0,sizeof(dst_tmp));
        memset(&stbuf,0,sizeof(struct stat));
        memset(dst_filename,0,sizeof(dst_filename));

	
	ret=lstat(dst,&stbuf);
	//if(ret<0)
	//{
	//	printf("��ȡĿ���ļ�[%s]״̬ʧ��[%s]\n",dst,strerror(errno));
	//	gtlogerr("��ȡĿ���ļ�[%s]״̬ʧ��[%s]\n",dst,strerror(errno));
	//	//return -1;
	//}
		
	if(S_ISDIR(stbuf.st_mode))
	{
		//��Դ�ļ�·���з�����Դ�ļ���
		get_filename_from_dir((char *)src,dst_filename);
		//printf("��·���ֽ�������ļ���Ϊ[%s]\n",&dst_filename[1]);
		sprintf(dst_tmp,"%s%s",dst,&dst_filename[1]);
		//printf("ƴ�ճ�����Ŀ���ļ�·��Ϊ[%s]\n",dst_tmp);
	}
	else
	{
		sprintf(dst_tmp,"%s",dst);
		//printf("[%s] is a file\n",dst_tmp);
	}
	
       // printf("dst=[%s]\n",dst);


	//���Ŀ��Ŀ¼����û��ͬ���ļ����У�ɾ����û�У�����
	errno=0;
	if(access(dst_tmp,F_OK)==0)
	{
		if(sf_dir.opt_f_flag==1)
		{	
			ret=remove(dst_tmp);
			if(ret==0)
			{
				//printf("ɾ��ͬ���ļ�[%s]�ɹ�.\n",dst_tmp);
			}
			else
			{
					errr=errno;
				printf("ɾ��ͬ���ļ�[%s]ʧ��[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
				gtlogerr("ɾ��ͬ���ļ�[%s]ʧ��[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
				return -1;
			}
		}
		else
		{
			printf("ɾ���Ѵ����ļ� [%s] ? ",dst_tmp);
			if(tip_cover())
			{
				errno=0;
				ret=remove(dst_tmp);
				if(ret==0)
				{
					//printf("ɾ��ͬ���ļ�[%s]�ɹ�.\n",dst_tmp);
				}
				else
				{
						errr=errno;
					printf("ɾ��ͬ���ļ�[%s]ʧ��[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
					gtlogerr("ɾ��ͬ���ļ�[%s]ʧ��[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
					return -1;
				}	
			}
			else
			{
				//printf("��ɾ���ļ�.\n");
				return 1;	
			}
		
		}
	}
	//else
	//{
	//	printf("[%s]line:%d\n",strerror(errno),__LINE__);
	//}

	//printf("dst_tmp=%s,line:%d\n",dst_tmp,__LINE__);

	crc_err_cnt=1;
	cp_err_cnt=1;
	while(1)
	{
		
		if(crc_err_cnt>3)
		{
			printf("����У���������3�Σ��˳�.\n");
			gtlogerr("����У���������3�Σ��˳�.\n");
			exit(1);
		}

		if(cp_err_cnt>3)
		{
			printf("���¿�����������3�Σ��˳�.\n");
			gtlogerr("���¿�����������3�Σ��˳�.\n");
			exit(1);
		}

		
		ret=trans_data_src_to_dst(src,dst_tmp);
		//printf("%s--->%s,ret=%d\n",src,dst_tmp,ret);
		if(ret<0)
		{
			printf("����Դ�ļ�[%s]��[%s]����,��%d��\n",src,dst_tmp,cp_err_cnt);
			gtlogerr("����Դ�ļ�[%s]��[%s]����,��%d��\n",src,dst_tmp,cp_err_cnt);	
			cp_err_cnt++;
			continue;
		}
		
		ret=check_files_size(src,dst_tmp);
		//printf("check_files_size...ret=%d,,line:%d\n",ret,__LINE__);
		if(ret<0)
		{
			printf("У���ļ�[%s][%s]��С���󣬵�%d��\n",src,dst_tmp,crc_err_cnt);
			gtlogerr("У���ļ�[%s][%s]��С���󣬵�%d��\n",src,dst_tmp,crc_err_cnt);
			crc_err_cnt++;
			continue;
		}

		ret=	check_file_crc32((char *)src,dst_tmp);	
		//printf("check_file_crc...ret=%d,line:%d\n",ret,__LINE__);
		if(ret<0)
		{	
			printf("�ļ�[%s][%s]CRCУ�����,��%d��\n",src,dst_tmp,crc_err_cnt);
			gtlogerr("�ļ�[%s][%s]CRCУ����󣬵�%d��\n",src,dst_tmp,crc_err_cnt);
			crc_err_cnt++;
			continue;
		}
		else 
		{
			if(crc_err_cnt!=1||cp_err_cnt!=1)
			{
				//printf("У��ȫ����ȷ\n");
				printf("�ļ����¸��Ƴɹ�(����%d�Σ�У��%d��).line:%d\n",cp_err_cnt,crc_err_cnt,__LINE__);
				gtloginfo("�ļ����¸��Ƴɹ�(����%d�Σ�У��%d��).line:%d\n",cp_err_cnt,crc_err_cnt,__LINE__);
			}
			break;
		}
	}

	ret=change_file_attri(src,dst_tmp);
	if(ret<0)
	{
		printf("�޸��ļ�[%s]���Է���ֵ����\n",dst_tmp);
		gtlogerr("�޸��ļ�[%s]���Է���ֵ����\n",dst_tmp);
		return ret;
	}

	
	return 0;
}



/**********************************************************************************
 *      ������: copy_dirs()
 *      ����:   �ص���������ftw�ص�,����Ŀ¼
 *      ����:   src		Դ�ļ�
 *				dst		Ŀ��Ŀ¼
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int copy_dirs(const char *file_dir,const char *dst_dir)
{
	//int len;
	int ret;
	//char remv_tmp[DST_FILENAME_MAX_LEN];
	
	if(file_dir==NULL || dst_dir==NULL)
	{
		return -1;
	}

	//���Ŀ��Ŀ¼�Ƿ���ڣ�������ɾ��������ļ�
	if(access(dst_dir,F_OK)!=0)
	{
		errno=0;
		ret=mkdir(dst_dir,0777);
		if(ret==0)
		{
			//printf("����Ŀ¼[%s]�ɹ�\n",dst_dir);
		}
		else
		{
				errr=errno;
			printf("����Ŀ¼[%s]ʧ��[%s]line:%d\n",dst_dir,strerror(errr),__LINE__);
			gtlogerr("����Ŀ¼[%s]ʧ��[%s]line:%d\n",dst_dir,strerror(errr),__LINE__);
			return -1;
		}
	}
	

	return 0;
}


/**********************************************************************************
 *      ������: creat_lnk()
 *      ����:   �ص���������ftw�ص�,����Ŀ¼
 *      ����:   src		Դ�ļ�
 *				dst		Ŀ��Ŀ¼
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
 int creat_lnk(char *lnk_buf,char *tmp_buf)
{
	int ret;

	errno=0;
	ret=symlink(lnk_buf,tmp_buf);
	if(ret<0)
	{
			errr=errno;
		printf("����[%s] ����[%s].line:%d\n",tmp_buf,strerror(errr),__LINE__);
		gtlogerr("����[%s] ����[%s].line:%d\n",tmp_buf,strerror(errr),__LINE__);
		return -1;
	}


	return 0;
}

/**********************************************************************************
 *      ������: copy_lnk()
 *      ����:   ���������в���
 *      ����:	lnk_buf		���������ַ���ָ��
 *				src			Դ��������·��ָ��
 *				tmp_buf		Ŀ���������ָ��tmp_buf=dst_path+filename
 *				dst_path		Ŀ��·��ָ��
 *				filename		�ļ���ָ��
 *				name_len	�ļ�������
 *				get_flag		����get_filename_from_dir��־
 *      ���:   ��
 *	 ��ע: ����ǰ��Ҫ��lnk_buf,tmp_buf,���г�ʼ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *********************************************************************************/
int copy_lnk(char *lnk_buf,char *src,char *tmp_buf,char *dst_path,char *filename,int name_len,int get_flag)
{
	int ret;
	int exist_flag;
	struct stat buf;

	//memset(lnk_buf,0,sizeof(lnk_buf));
	//printf("link----[%s]--\n",src);
	errno=0;
	ret=readlink(src,lnk_buf,LNK_PATH_MAX_LEN);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡ��������[%s] ����[%s].line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("��ȡ��������[%s] ����[%s].line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	//printf("lnk_buf[%s]\n",lnk_buf);
	//memset(tmp_buf,0,sizeof(tmp_buf));
	
	if(get_flag==1)
	{
		memset(filename,0,name_len);
		get_filename_from_dir((char *)src,filename);
		sprintf(tmp_buf,"%s%s",dst_path,&filename[1]);
	}
	else
	{
		sprintf(tmp_buf,"%s",dst_path);
	}

	//�жϷ�������ʹ��lstat
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	exist_flag=1;
	ret=lstat(tmp_buf,&buf);
	if(ret<0)
	{	
		errr=errno;
		//�п�����Ŀ��·���������Ǹ��ļ�������������Ĵ���ͱ�����־
		if(errr==2)
		{
			exist_flag=0;
			ret=creat_lnk(lnk_buf,tmp_buf);
			if(ret<0)
				return ret;
			
		}
		else
		{
			printf("��ȡ[%s]״̬ʧ��errno=%d,[%s]line:%d\n",tmp_buf,errr,strerror(errr),__LINE__);
			gtlogerr("��ȡ[%s]״̬ʧ��errno=%d[%s].line:%d\n",tmp_buf,errr,strerror(errr),__LINE__);
			return -1;
		}
	}


	if(exist_flag==1)
	{
		//����ļ����ڣ���ȥ��ͼɾ��
		if(S_ISLNK(buf.st_mode))
		{	
			if(sf_dir.opt_f_flag==1)
			{
				errno=0;
		 	       ret=unlink(tmp_buf);
      				if(ret<0)
        			{
        				errr=errno;
       				printf("ɾ�� [%s] ����[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
					gtlogerr("ɾ�� [%s] ����[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
      		          		return -1;
      		  		}
      		  		else
        			{
         				//printf("del [%s] ok\n",tmp_buf);
       	 		}
	
			}
			else
			{
				printf("ɾ���Ѵ��ڵķ�������[%s] ?",tmp_buf);
				if(tip_cover())
				{
					errno=0;
					ret=unlink(tmp_buf);
      					if(ret<0)
       				{
       					errr=errno;
       					printf("ɾ�� [%s] ����[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
						gtlogerr("ɾ�� [%s] ����[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
       					return -1;
       				}
       				else
       				{
       					//printf("del [%s] ok\n",tmp_buf);
       				}
				}	
				else
				{
					return 1;
				}

			}
										
		}
		else
		{
			printf("���Ƿ�������Ϊʲô����������line:%d\n",__LINE__);
			gtlogerr("���Ƿ�������Ϊʲô����������line:%d\n",__LINE__);
			return -1;
		}
	
		ret=creat_lnk(lnk_buf,tmp_buf);
		if(ret<0)
			return ret;
	}

	
	if(sf_dir.opt_u_flag==1)
	{
		printf("[%s]----->[%s]\n",src,tmp_buf);
	}

	return 0;
}


/**********************************************************************************
 *      ������: show_directions()
 *      ����:   �ص���������ftw�ص�
 *      ����:   ��
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int show_directions(const char *file,const struct stat *sb,int flag)
{
	struct stat buf;
	int len;
	int len_sub;
	int ret;
	char *tmp_sub=NULL;
	char tmp_sub_nh[SRC_FILENAME_MAX_LEN];
	char tmp_buf[SRC_FILENAME_MAX_LEN];
	char lnk_buf[LNK_PATH_MAX_LEN];


	//���ָ����Ч
	if(file==NULL || sb==NULL)
	{
		return -1;
	}

	//sf_dir->src_path��·��Ӧ�ñ�������file_dir·���У�����ʹ�õ�ǰ��Ŀ¼����sf_dir->src_path��
	//���ȵ�ƫ�������ǵ�ǰ��Եĵ�ǰĿ¼�ˣ��Ϳ��԰����Ŀ¼����Ŀ��Ŀ¼���ˡ�
	//���Ŀ¼ȥͷ,Ŀ����ƴ��Ŀ��Ŀ¼
	len=strlen(sf_dir.src_path);
	memset(tmp_sub_nh,0,sizeof(DST_FILENAME_MAX_LEN));
	if((*(file+len))=='/')
	{
		sprintf(tmp_sub_nh,"%s",(file+len+1));
	}
	else
	{
		sprintf(tmp_sub_nh,"%s",(file+len));
	}
		
	//�����Ŀ¼ȥβ,Ŀ���ǱȽϵ�ǰ��Ŀ¼��û�иı�
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(file,&buf);
	if(ret<0)
	{
			errr=errno;
		printf("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",file,strerror(errr));
		gtlogerr("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",file,strerror(errr));
		return -1;
	}
	
	if(S_ISREG(buf.st_mode))
	{
		memset(sf_dir.sub_path_nf,0,sizeof(sf_dir.sub_path_nf));
		sprintf(sf_dir.sub_path_nf,"%s",file);
		tmp_sub=strrchr(file,'/');		
		if(tmp_sub!=NULL)
		{
			//printf("ȡ�õ�βΪ[%s]\n",tmp_sub);
			len_sub=strlen(tmp_sub);
		}
		else
		{
			//printf("tmp_subΪ��\n");
			len_sub=strlen(sf_dir.sub_path_nf);
		}
		len=strlen(sf_dir.sub_path_nf);
		sf_dir.sub_path_nf[len-len_sub]='\0';
		//printf("���ļ�Ŀ¼��ȥβ�����Ŀ¼Ϊ[%s]\n",sf_dir.sub_path_nf);
	}

	//printf("file=[%s]------sf_dir.sub_path_nf=[%s]\n",file,sf_dir.sub_path_nf);


	//��鿴�ǲ��ǽ�����Ŀ¼
	ret=strcmp(sf_dir.sub_path_nf,sf_dir.sub_path_nf_old);
	if(ret!=0)
	{
		//printf("������Ŀ¼��\n");
		memset(sf_dir.sub_path_nf_old,0,sizeof(sf_dir.sub_path_nf_old));
		memcpy(sf_dir.sub_path_nf_old,sf_dir.sub_path_nf,sizeof(sf_dir.sub_path_nf));
	}

	memset(sf_dir.cur_dst_path,0,sizeof(sf_dir.cur_dst_path));
	sprintf(sf_dir.cur_dst_path,"%s",sf_dir.dst_path);
	
	if(strlen(tmp_sub_nh)!=0)
	{
		strcat(sf_dir.cur_dst_path,"/");
		strcat(sf_dir.cur_dst_path,tmp_sub_nh);
	}
	//printf("��ʱ��Ŀ��·��Ϊ[%s]\n",sf_dir.cur_dst_path);

	//��ʼ���ø���ģ��	
	if(S_ISLNK(buf.st_mode))		
		{
			//printf("[%s]-----��������\n",file);
			if(sf_dir.opt_d_flag==1)
			{
				memset(lnk_buf,0,sizeof(lnk_buf));
				memset(tmp_buf,0,sizeof(tmp_buf));
				ret=copy_lnk(lnk_buf,(char *)file,tmp_buf,sf_dir.cur_dst_path,NULL,0,0);
				if(ret<0)
				{
					printf("������������[%s]ʧ��line:%d\n",file,__LINE__);
					gtlogerr("������������[%s]ʧ��line:%d\n",file,__LINE__);
					return ret;
				}
			}
			else
			{
				//lnk-->file
				//copy_files(tmp_buf,sf_dir.dst_path);
				ret=copy_files(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("��������[%s]���ļ�����ʧ��line:%d \n",file,__LINE__);
					gtlogerr("��������[%s]���ļ�����ʧ��line:%d \n",file,__LINE__);
					return ret;
				}
			}
		}
	else if(S_ISREG(buf.st_mode))
		{
			//printf("[%s]----�ļ�line:%d\n",file,__LINE__);
			ret=copy_files(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("�����ļ�[%s]ʧ��line:%d\n",file,__LINE__);
					gtlogerr("�����ļ�[%s]ʧ��line:%d\n",file,__LINE__);
					return ret;
				}

		}
	else if(S_ISDIR(buf.st_mode))
		{
			//printf("[%s]---Ŀ¼\n",file);
			ret=copy_dirs(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("����Ŀ¼[%s]ʧ��line:%d\n",file,__LINE__);
					gtlogerr("����Ŀ¼[%s]ʧ��line:%d\n",file,__LINE__);
					return ret;
				}
		}

	//printf("\n\n");

	return 0;
}



/**********************************************************************************
 *      ������: err_fn()
 *      ����:   ����ص�����
 *      ����:   *df_dir			ָ��SFCP_PATH_T ��ָ��
 *      ���:   ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *	 ע:
 *	��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
	�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
	�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
 *********************************************************************************/
int err_fn(const char *dir_file,int errnum)
{
	printf("����Ŀ¼[%s]����,errno:[0x%x]\n",dir_file,errnum);
	return -1;
}


/**********************************************************************************
 *      ������: fix_src_dst_dir()
 *      ����:   ����Դ·���޸�Ŀ��·��
 *      ����:	src		Դ·��
 *				dst		Ŀ��·��
 *				filename	�ݴ�Դ·���е�ҪҪ������·����
 *      ���:   ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *********************************************************************************/
int fix_src_dst_dir(char *src,char *dst,char *filename)
{
	int ret;
	int len;

	//printf("Դ·����Ŀ¼��׼����ȡԴ·���е�Ŀ��·��\n");
	//printf("��ȡǰsrc=[%s]\n",src);
	//printf("dst=[%s]\n",dst);
	get_filename_from_dir(src,filename);
	//printf("��ȡ����·��Ϊ=[%s]\n",filename);

	if(strlen(dst)+strlen(filename)>DST_FILENAME_MAX_LEN)
	{
		printf("Ŀ��·���ļ���%s����\n",dst);
		gtlogerr("Ŀ��·���ļ���%s����\n",dst);
		return -1;
	}

	len=strlen(dst);
	if(*(dst+len-1)!='/')
	{
		//printf("����/\n");
		strcat(dst,filename);
	}
	else if(*(dst+len-1)=='/')
		{
			//printf("����/\n");
			strcat(dst,(char *)&filename[1]);
		}
	
	//printf("�޸ĺ��dst=[%s]\n",dst);

	errno=0;
	if(access(dst,F_OK)!=0)
	{
		//ΪԴ·���к���*׼��
		if(sf_dir.opt_r_flag==1)
		{
			//printf("Ŀ¼[%s]�����ڣ�׼������\n",dst);
			errno=0;
			ret=mkdir(dst,0777);
			if(ret==0)
			{
				//printf("����Ŀ¼[%s]�ɹ�\n",dst);
			}
			else
			{
					errr=errno;
				printf("����Ŀ¼[%s]ʧ��:%s\n",dst,strerror(errr));
				gtlogerr("����Ŀ¼[%s]ʧ��:%s\n",dst,strerror(errr));
				return -1;
			}
		}
	}
	
	return 0;
}



/**********************************************************************************
 *      ������: creat_dir()
 *      ����:   ����Ŀ¼
 *      ����:   dir			·��
 *      ���:   ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *********************************************************************************/
int creat_dir(char *dir)
{
	int ret;
	
	errno=0;
	if(access(dir,F_OK)!=0)
	{
		ret=mkdir(dir,0777);
		if(ret==0)
		{
			//printf("����Ŀ¼[%s]�ɹ�\n",dst_dir);
		}
		else
		{
			errr=errno;
			printf("����Ŀ¼[%s]ʧ��[%s]line:%d\n",dir,strerror(errr),__LINE__);
			gtlogerr("����Ŀ¼[%s]ʧ��[%s],line:%d\n",dir,strerror(errr),__LINE__);
			return -1;
		}

		sf_dir.creat_ndir_flag=1;
	}
	return 0;
}


/**********************************************************************************
 *      ������: check_same_dir()
 *      ����:   ���Դ·����Ŀ��·���Ƿ�����ͬ��
 *      ����:   dir			·��
 *      ���:   ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *********************************************************************************/
int check_same_dir(char *src,char *dst)
{
	int len;
	char tmp_buf[DST_FILENAME_MAX_LEN];
	
	if(src==NULL || dst==NULL)
	{
		return 0;
	}

	//printf("���src=[%s]��Ŀ��[%s],line:%d\n",src,dst,__LINE__);

		if(strcmp(src,dst)==0)
		{
			printf("Դ·��[%s]��Ŀ��·��[%s]��ͬ,����ݹ鿽��line:%d\n",src,dst,__LINE__);
			gtlogerr("Դ·��[%s]��Ŀ��·��[%s]��ͬ,����ݹ鿽��line:%d\n",src,dst,__LINE__);
			exit(1);
		}

	memset(tmp_buf,0,sizeof(tmp_buf));
	sprintf(tmp_buf,"%s",dst);
	len=strlen(dst);
	if(tmp_buf[len-1]=='/')
	{
		//printf("Ŀ��·��[%s]����/\n",tmp_buf);
		tmp_buf[len-1]='\0';
		//printf("ȥ����dst=[%s]\n",tmp_buf);
		if(strcmp(src,tmp_buf)==0)
		{
			printf("Դ·��[%s]��Ŀ��·��[%s]��ͬ,����ݹ鿽��line:%d\n",src,tmp_buf,__LINE__);
			gtlogerr("Դ·��[%s]��Ŀ��·��[%s]��ͬ,����ݹ鿽��line:%d\n",src,tmp_buf,__LINE__);
			exit(1);
		}
	}

	return 0;
}

/**********************************************************************************
 *      ������: parser_args()
 *      ����:   ���������в���
 *      ����:   *df_dir			ָ��SFCP_PATH_T ��ָ��
 *      ���:   ��
 *      ����ֵ:  �ɹ�����0����ֵ������
 *********************************************************************************/
 int parser_args(SFCP_PATH_T *df_dir,char *argv[])
{
	struct stat buf;
	struct stat sub_buf;
	int len;
	int i;
	int dst_flag=0;
	int multi_dirs_flag=0;
	int ret;
	char tmp_buf[SRC_FILENAME_MAX_LEN];
	char lnk_buf[LNK_PATH_MAX_LEN];
	char dst_tmp[DST_FILENAME_MAX_LEN];

	//���ָ����Ч
	if(df_dir==NULL)
	{
		return -1;
	}
					
	memset(&buf,0,sizeof(struct stat));
	memset(&sub_buf,0,sizeof(struct stat));

        //printf("argc=%d\n",df_dir->arg_total);
        for(i=0;i<df_dir->arg_total-1;i++)
        {
               if(argv[df_dir->arg_total-1-i][0]!='-')
                {
                        if(dst_flag==0)
                        {
				sprintf(df_dir->dst_path,"%s",argv[df_dir->arg_total-1-i]);
                               
				//check dst_path exist
				errno=0;
				if(access(df_dir->dst_path,F_OK)==0)
				{
					errno=0;
					ret=lstat(df_dir->dst_path,&buf);
					if(ret<0)
					{
							errr=errno;
						printf("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",df_dir->src_path,strerror(errr));
						gtlogerr("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",df_dir->src_path,strerror(errr));
						return -1;
					}
					if(S_ISDIR(buf.st_mode))
					{
						//dst_path include '/'
						len=strlen(df_dir->dst_path);
						//printf("len=[%d]---df_dir->dst_path[len-1]=%c\n",len,df_dir->dst_path[len-1]);
						if(df_dir->dst_path[len-1]=='/')
						{
							//printf("df_dir->dst-path include '/' sight\n");
						}
						else
						{
							//printf("df_dir->dst-paht dosen't include,but i fix it\n");
							df_dir->dst_path[len]='/';
							//printf("df_dir->dst-path=[%s]\n",df_dir->dst_path);
						}
                                		//printf("Dst-dir=[%s]\n",df_dir->dst_path);			
						memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
						memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
					}
					
				}
				else
				{
					//printf("Ŀ��·��%s,[%s],line:%d\n",df_dir->dst_path,strerror(errno),__LINE__);
					//gtlogerr("Ŀ��·��%s,[%s],line:%d\n",df_dir->dst_path,strerror(errno),__LINE__);
	////				creat_dir(df_dir->dst_path);					
					memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
					memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
				}
				
				dst_flag=1;
                            continue;
                        }
                        else
                        {
                                sprintf(df_dir->src_path,"%s",argv[df_dir->arg_total-1-i]);
                        }
				//printf("Dst-dir=[%s]\n",df_dir->dst_path);
                       	//printf("[%d]--->Src-dir=[%s]\n",i,df_dir->src_path);			
				//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
				//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
			check_same_dir(df_dir->src_path,df_dir->dst_path);
				
			errno=0;
			ret=lstat(df_dir->src_path,&buf);
			if(ret<0)
			{
					errr=errno;
				printf("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",df_dir->src_path,strerror(errr));
				gtlogerr("��ȡԴ�ļ�[%s]״̬ʧ��[%s]\n",df_dir->src_path,strerror(errr));
				return -1;
			}
                        if(S_ISDIR(buf.st_mode))
                        {
              ////
              			memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
        				creat_dir(df_dir->dst_path);		
						
					memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
					memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
		////
 				memset(dst_tmp,0,sizeof(dst_tmp));
				////memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
				////memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
				if(df_dir->creat_ndir_flag==0)
				{
					fix_src_dst_dir(df_dir->src_path,df_dir->dst_path,dst_tmp);
				}
				
				//src dir =1
				if(multi_dirs_flag==0)
				{
			
                                	//printf("׼������Ŀ¼\n");
					//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
					//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
					if(df_dir->opt_r_flag==1)
					{		

                                		ret=ftw_sort(df_dir->src_path,show_directions,DIR_MAX_DEEP,FTW_SORT_ALPHA,0,err_fn);
						if(ret<0)
						{
							printf("ftw_sort����Ŀ¼����ֵ����line:%d\n",__LINE__);
							gtlogerr("ftw_sort����Ŀ¼����ֵ����line:%d\n",__LINE__);
							return ret;
						}
					}
					else
					{
						printf("�˹�Ŀ¼[%s]\n",df_dir->src_path);
						//printf("ftw_only-1 deep fs\n");
					      //ret=ftw_sort(df_dir->src_path,show_directions,0,FTW_SORT_ALPHA,0,err_fn);
						//if(ret<0)
						//{
						//	printf("ֻ����1��Ŀ¼ʱ����\n");
						//	return ret;
						//}
					}

					multi_dirs_flag=0;
				}
                        }
                        else if(S_ISREG(buf.st_mode))
                                {
                                       	//printf("׼�������ļ�\n");
						//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
						//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
						df_dir->src_file_flag=1;
						if(strlen(df_dir->dst_path_bak)!=0)
						{
							//printf("��path-bak--->path\n");
                                      		 memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					   		 memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
						}

						ret=copy_files(df_dir->src_path,df_dir->dst_path);
						if(ret<0)
						{
							printf("copy_file�����ļ�����ֵʧ��line:%d\n",__LINE__);
							gtlogerr("copy_file�����ļ�����ֵʧ��line:%d\n",__LINE__);
							return ret;
						}
                                }
			else if(S_ISLNK(buf.st_mode))
				{
					memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));

					if(df_dir->opt_d_flag==1)
					{
						//printf("׼��������������------\n");
						memset(lnk_buf,0,sizeof(lnk_buf));
						memset(tmp_buf,0,sizeof(tmp_buf));
						memset(dst_tmp,0,sizeof(dst_tmp));

						ret=copy_lnk(lnk_buf,df_dir->src_path,tmp_buf,df_dir->dst_path,dst_tmp,sizeof(dst_tmp),1);
						if(ret<0)
						{
							printf("copy_lnk�����������ӷ���ֵʧ��line:%d\n",__LINE__);
							gtlogerr("copy_lnk�����������ӷ���ֵʧ��line:%d\n",__LINE__);
							return ret;
						}						
					}
					else
					{
						//lnk-->file
						//printf("׼���ѷ������ӵ��ļ�����\n");
						//printf("src=[%s],dst=[%s]\n",df_dir->src_path,df_dir->dst_path);

						ret=copy_files(df_dir->src_path,df_dir->dst_path);
						if(ret<0)
						{
							printf("�ѷ������ӵ��ļ���������ֵʧ��line:%d\n",__LINE__);
							gtlogerr("�ѷ������ӵ��ļ���������ֵʧ��line:%d\n",__LINE__);
							return ret;
						}
						
					}
				}
                }

        }
	
	return 0;
}



/**********************************************************************************
 *      ������: main()
 *      ����:   main
 *      ����:   �����в���
 *      ���:   ��
 *      ����ֵ: �ɹ�����0����ֵ������
 *********************************************************************************/
int main(int argc,char *argv[])
{
	int opt;
	const char *shortopts="rfdu";
	const struct option longopts[]={
			{"help",0,NULL,'h'},
			{"version",0,NULL,'v'},
			{NULL,0,NULL,0},
			};
	int ret;

	//SFCP_PATH_T sf_dir;
	gtopenlog("sfcp");
	
	memset(&sf_dir,0,sizeof(SFCP_PATH_T));

	sf_dir.arg_total=argc;
	while((opt=getopt_long(argc,argv,shortopts,longopts,NULL))!=-1)
	{
		switch(opt)
		{
			case 'r':
				sf_dir.opt_r_flag=1;
				break;

			case 'f':
				sf_dir.opt_f_flag=1;
				break;

			case 'd':
				sf_dir.opt_d_flag=1;
				break;

			case 'u':
				sf_dir.opt_u_flag=1;
				break;
		
			case 'h':
                		print_help();
                		break;
            	
			case 'v':
                		print_ver();
                		break;

            		case '?':   // δ֪�������
                		printf("δ֪ѡ��\n");
				gtlogerr("δ֪ѡ��:[%c]\n",opt);
				exit(1);
                		break;

            		default:
	               	printf("����ѡ��\n");
				gtlogerr("����ѡ��:[%c]\n",opt);
				exit(1);
                		break;	
		}
	}

	ret=parser_args(&sf_dir,argv);
	if(ret<0)
	{
		exit(1);
	}
	else
	{
		exit(0);
	}

	return 0;
}


