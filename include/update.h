
#ifndef _UPDATE_H_
#define _UPDATE_H_

#define IN
#define OUT
#define INOUT

void kill_apps4update(void);

/*  ������ check_update_space()
	����: ����ɵ���������Ŀ¼,�����̿ռ��Ƿ��㹻,�����update����Ŀ¼�ַ���
	����: updatefilesize(��λ:byte)
	���: updatedir,�����Ĺ���Ŀ¼,��"/hqdata/update"
	����ֵ: -ERR_NO_SPACE: �ռ䲻��
			RESULT_SUCCESS: �ɹ�
*/
int check_update_space(IN int updatefilesize, OUT char * updatedir);


/*������ĸ����������ָ����ʽ�����������ַ���*/
int generate_updatemsg(IN char * username, IN char * pswd, IN char *ftpip, IN int port,
      IN char *path, OUT char *updatemsg);

      
/*Զ���������
  �������� updatefilesize ��������С�����ֽ�Ϊ��λ
  �������� updatemsg  ������Ϣ,��ʽ "wget -c ftp://usr:pswd@192.168.1.160:8080/path/xxx.tar.gz"
  ��������ֵ 0Ϊ�ɹ�������Ϊ������	
*/	
int update_software(int updatefilesize,char *updatemsg,int interval);


/*
	����: �������tar.gz�ļ���ѹ������ִ������
	����ֵ: ��������

*/
int direct_update_software(IN char* gzfilename, IN char *updatedir );

#endif
