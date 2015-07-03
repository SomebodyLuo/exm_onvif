/**
 	�ļ�����: extract_prog.c
 	�ļ�����: �ͷų������ָ�����ļ���
 	�޸�ʱ��: 08.08.26~08.08.28 
 	�޸���־: �޸�main������������һ���ɽ�ѹ����·��
 */
#include <stdio.h>
#include <stdlib.h>                            
#include <unistd.h>            
#include <gtlog.h>                                                                                                                                       
#include <errno.h>    

/**
	��¼ÿ���޸ĺ�İ汾��

	0.03		�޸ĺ궨�壬�޸�ע��
			ʹ�ó����Ź淶���׶�
			
	0.02		�ڵ�һ����ȥ������ı������汾ʹ�ú궨��
			����˹����˷��ڴ棬�汾�޸�������
			
	0.01		��һ��
*/
#define VERION ("0.03")

static const char	prog_pkt[]      = "/conf/ip1004.tar.lzma";			///< ����õ��豸������ļ���
static const char	backup_pkt[]  = "/hqdata/hda1/firmware/ip1004.tar.lzma";	///< ���ݵĳ����ļ���
static const char	target_dir[]    = "/";	///< ѹ�����Լ���·��,��ѹ����Ŀ��Ŀ¼


int main(int argc,char *argv[])
{
	int ret;					 ///< system ��������ֵ��0Ϊ��
	int err;					 ///< errno ����ֵ   
	char cmdbuf[256];  

	printf( "����extract_proc ( ver : %s )\n", VERION);	 ///<��ʾ����ʱ�İ汾��

	gtopenlog( "extract_prog" );	 ///< ��¼��־
	gtloginfo( "����extract_prog ( ver : %s )", VERION);	///< ��¼��־�汾
	
	sprintf( cmdbuf,"tar axvf %s >/tmp/extract.txt 2>>/tmp/extract.txt ", prog_pkt );
	printf( "prog_cmd = %s \n", cmdbuf );
	
	errno = 0;
	ret = system( cmdbuf);
	err = errno;
	printf( "prog_ret = %d, prog_errno = %d !! \n", ret, err );
	
	if( ret!= 0 )
	{
		gtlogerr(" Դ·���������ѹ��ʧ��, prog_ret = %d, prog_errno = %d", ret, err );	///< ��¼������־

		printf( "Դ���ݻٻ����ӱ������ݶ�ȡ\n" );		
		sprintf( cmdbuf,"tar axvf %s >/tmp/extract.txt 2>>/tmp/extract.txt ", backup_pkt);
		printf( "backup_cmd = %s \n", cmdbuf );
		
		errno = 0;
		ret = system( cmdbuf );		///< �ӱ���·����ѹ������
		err = errno;
		printf( "bakckup_ret = %d , backup_errno = %d !! \n", ret, err );		
		if( ret != 0 )
		{
			gtlogerr( "����·���������ѹ��ʧ��,backup_ret = %d, backup_errno = %d", ret, err );
			printf( "���ݾ�����٣��޷���ѹ��\n" );                     
		}
		
	}



	exit(0);
}



