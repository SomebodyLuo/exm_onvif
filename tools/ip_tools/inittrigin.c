#include<stdio.h>
#include<string.h>
#include <commonlib.h>
#include <iniparser.h>
#include <devinfo.h>
#include <gtvs_io_api.h>

#define IP1004_SYS_FILE		("/conf/ip1004.ini")

#define Ver		("0.01")
//ver:0.01	2013-07-05 ����

void print_help(void)
{
	printf("Ver:[%s]\n",Ver);
	printf("inittrigin [trigin_state] \n");
	printf("[trigin_state] 0 ȫΪ����,1 ȫΪ����\n");

}

int main(int argc,char *argv[])
{

	int trigin_s; 	
	int attrib;
	int trig_num;
	int i;
	int ret;
	char buf[32];
	FILE *lock_fp=NULL;
	dictionary      *ini;	


	if(argc!=2)
	{
		print_help();
		return -1;	
	}

	if(argv[1]!=NULL)
	{
		trigin_s=strtol(argv[1],NULL,0);
	}
	else
	{
		print_help();
		return -1;
	}


	printf("trigin_s=[%d]\n",trigin_s);

	ini=iniparser_load_lockfile(IP1004_SYS_FILE,1,&lock_fp);
	if (ini==NULL) 
	{
                printf("inittrigin  cannot parse ini file file [%s]", IP1004_SYS_FILE);
                return -1 ;
        }
	printf("��ʼ����ini�ļ�\n");

	if(init_devinfo()<0)
	{
		printf("init devinfo error\n");
		return -1;
	}

	memset(buf,0,sizeof(buf));	
	trig_num=get_trigin_num();
	printf("trig_num=%d\n",trig_num);
	for(i=0;i<trig_num;i++)
	{
#if 0	
		if(trigin_s==1)
		{
			//Ĭ��Ϊ����ʱ���������������
			sprintf(buf,"alarm%d:enable",i);
			ret=iniparser_setint(ini,buf,0);	
			if(ret<0)
			{
				printf("����[%s]Ϊ[%d]����\n",buf,0);	
			}
		}
#endif

		sprintf(buf,"alarm%d:enable",i);
		ret=iniparser_setint(ini,buf,1);	
		if(ret<0)
		{
			printf("����[%s]Ϊ[%d]����\n",buf,0);	
		}

		sprintf(buf,"alarm%d:attrib",i);
		ret=iniparser_setint(ini,buf,trigin_s);
		if(ret<0)
		{
			printf("����[%s]Ϊ[%d]����\n",buf,trigin_s);
		}
		printf("����[%s]Ϊ[%d]\n",buf,trigin_s);
	
	}
	if(trigin_s==0)
	{
		attrib=0;
	}
	else
	{
		attrib=0xff;
	}
	printf("�ڵ���gtvsioapi��?\n");

	printf("��ʼ������Ƶͨ��\n");	
	//����4�ָ�ΪĬ����Ƶͨ����ʾ
	sprintf(buf,"%s","netencoder:net_ch");
	ret=iniparser_setint(ini,buf,4);
	if(ret<0)
	{
		printf("����[%s]Ϊ[%d]����\n",buf,4);
	}

	save_inidict_file(IP1004_SYS_FILE,ini,&lock_fp);
  	iniparser_freedict(ini);

#if 0
	init_vsiodrv();
#else
	init_gpiodrv();
#endif
	set_trigin_attrib_perbit(attrib);
#if 0
	exit_vsiodrv();
#else
	exit_gpiodrv();
#endif
	

	return 0;
}
