#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<iniparser.h>


#define	INI_FILE_NAME		("/conf/gt1000.ini")

/**********************************************************************************
* 	������:	show_adsl()
*	����:	��ʾadsl��Ϣ
*	����:	none
*	���:	none
*	����ֵ:	0��ʾ�ɹ�����ֵ��ʾ������
*********************************************************************************/
int show_adsl(void)
{
	dictionary *ini;
	char *data_tmp=NULL;
	
	//�ж�ini�ļ���
        if(INI_FILE_NAME==NULL)
        {
                printf("error ini file.\n");
                return -1;
        }
	
	//����ini�ļ�
        ini=iniparser_load(INI_FILE_NAME);
        if (ini==NULL)
        {
		printf("cannot parse file [%s]",INI_FILE_NAME );
                return -1 ;
        }


	data_tmp=iniparser_getstr(ini,"net_config:internet_mode");
	if(strcmp(data_tmp,"0")==0)
	{
		printf("������ʽ:	adsl����\n");
	}
	if(strcmp(data_tmp,"1")==0)
	{
		printf("������ʽ:	����������\n");
	}
	if(strcmp(data_tmp,"2")==0)
	{
		printf("������ʽ:	ר�߽���\n");	
	}

	data_tmp=iniparser_getstr(ini,"net_config:adsl_user");
	printf("    ADSL�û���:	%s\n",data_tmp);

	data_tmp=iniparser_getstr(ini,"net_config:adsl_passwd");
	printf("    ADSL����:	%s\n",data_tmp);

	//free ini
	iniparser_freedict(ini);	



	return 0;
}

/**********************************************************************************
* 	������:	XVS_set_state_callback()
*	����:	ע��xvs����״̬�Ļص�����
*	����:	XVS_state_callback����ָ��,�ú�������ΪXVS_loginʱ��õĲ��������XVS_status_t���͵�ָ��
*	���:	��
*	����ֵ:	0��ʾ�ɹ�����ֵ��ʾ������
*********************************************************************************/
int main(void)
{
	int ret;
	//printf("start...\n");
	ret=show_adsl();
	if(ret<0)
	{
		printf("ini�ļ�����\n");
	}
	//printf("return.\n");
	return 0;
}

