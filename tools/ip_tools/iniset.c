/*���ڽ�һ��ini�ļ������б������õ���һ��ini�ļ��Ĺ��ߣ�wsy october 2006*/
#include <stdio.h>
#include <stdlib.h>
#include "file_def.h"
#include "iniparser.h"
#include "commonlib.h"

#define CANT_FIND_KEY "�޴�key" //getstring�Ҳ�����Ӧ�ؼ���ʱ��default����

#define VERSION "1.2"           //�汾��

//1.2 ��ֲ��GTIP-E�豸��
//1.1 �Ƚϲ�����ֵʱ,����������ĳ��ȱȽ�,������ֲ������̲���ֵ������
//	  ���dic�ṹ���õ�target�ļ���������/conf/ip1004.ini
	  
//1.0 initial


//���������Ϣ���˳�
int print_helpmsg(void)
{
	printf("\n��ӭʹ��iniset�ļ����ù���!(version %s)\n\n",VERSION);
	printf("ʹ��˵��: iniset [OPTION] ��Դ�ļ� Ŀ���ļ�\n\n");
	printf("    ����: ����Դ�ļ��е�ÿ���������õ�Ŀ���ļ�\n\n");
	printf("\t-h\t���������Ϣ\n");
	printf("\t-v\t����汾��Ϣ\n\n");
	return(0);
}

//����汾��Ϣ���˳�
int print_version(void)
{
	printf("\nThis is iniset version %s\n\n",VERSION);
	return (0);
}

#if 0
//�ᵽiniparser������(shixin)
/*��source�ļ���ÿ���ڶ�������target�ļ���Ӧ�ڶ������ݱȽϣ�
  ����ͬ���дtarget,��target�����ڸý��򴴽�,������־*/
int ini_set_file(char *source,char *target)
{
	dictionary *srcini,*tgtini;
	FILE *filetgt=NULL;
	int i;
	char *key=NULL;
	int changeflag=0;
	int compare_len=0; //��Ҫ�Ƚϵ��ֽڳ���
	
	if((source==NULL)||(target==NULL))
		{
			printf("�����ļ���ΪNULL,�˳�ini_set_file\n");
			gtloginfo("�����ļ���ΪNULL,�˳�ini_set_file\n");
			return -1;
		}
	
	srcini=iniparser_load(source);
	if (srcini==NULL) 
        {
             printf("cannot parse ini file [%s]",source);
			 gtloginfo("����%sʧ���˳�\n",source);	
             return(-1);
		}

	tgtini=iniparser_load_lockfile(target,1,&filetgt);
	for (i=0 ; i<srcini->size ; i++) //����Դ�ļ���ÿ������
	{
        if (srcini->key[i]==NULL)
            continue ;
        if (strchr(srcini->key[i], ':')==NULL)//û��:,Ϊ���� 
        	continue;
    
        //��Ŀ���ļ��Ƿ��У����û���򴴽�
        key=iniparser_getstring(tgtini,srcini->key[i],NULL);
		if(key==NULL)//û�У�����
        	{
        		changeflag++;
        		iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
			   printf("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);
			gtloginfo("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);				
        	}
        else //��
        	{
        		
				//if(strncmp(key,srcini->val[i],strlen(srcini->val[i]))!=0)//�е�����ͬ
				compare_len=(strlen(key)>strlen(srcini->val[i])) ? strlen(key) : strlen(srcini->val[i]); 

				if(strncmp(key,srcini->val[i],compare_len)!=0)//�е�����ͬ
				{
					changeflag++;
					gtloginfo("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
					printf   ("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
					iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
				}
        	}
       
	}

	if(changeflag!=0)
		gtloginfo("%s������(ini file set!)\n",target);
	iniparser_freedict(srcini);
	//save_inidict_file(IPMAIN_PARA_FILE,tgtini,&filetgt);
	save_inidict_file(target,tgtini,&filetgt);
	iniparser_freedict(tgtini);
	return 0;
}

#endif



int main(int argc,char **argv)
{

/*   ���������в���:
 *		����ʱû�д���������ʾ������Ϣ��
 *		-h:���������Ϣ��
 *		-v:���ת�����ߵİ汾��Ϣ��
 *		-s:���Դ�ļ�
 *		-t:���Ŀ���ļ�*/

	gtopenlog("iniset");
	if((argc==3)&&(argv[1]!=NULL)&&(argv[2]!=NULL))
	{
		gtloginfo("start iniset %s->%s\n",argv[1],argv[2]);
		printf("start iniset %s->%s\n",argv[1],argv[2]);
		ini_set_file(argv[1],argv[2]);
		return 0;
	}
	else
	{
		if((argc==2)&&(strncmp(argv[1],"-v",2)==0))
		{
			print_version();
			return 0;
		}

		else
		{
			print_helpmsg();
			return 0;
		}

	}
}

