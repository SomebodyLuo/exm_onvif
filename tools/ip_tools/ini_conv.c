#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtlog.h>
#include <iniparser.h>
#include "ini_conv.h"
#include "ini_conv_init.h"

static char *str_gt1;		///<����gt1000�ĵ�һ���ڵ���keyָ����ַ�����ַ
static char *str_gt2;		///<����gt1000�ĵڶ����ڵ���keyָ����ַ�����ַ
static char *str1;			///<����yy��һ�������ļ�keyָ���ַ�����ַ
static char *str2;			///<����yy�ڶ��������ļ�keyָ���ַ�����ַ
static int val_temp;		///<�޸�cmd_port����ʱ��ű���
static char str_temp[32];	///<�޸�guid����ʱ�������

static int is_ini_changed = NO_CHANGE;							///<�ж������ļ��Ƿ��޸Ĺ������򱣴�
static int conv_total = sizeof(conv_ini_all) / sizeof(conv_ini_t);		///<����ṹ��������
static int spcial_total = sizeof(conv_spcial) / sizeof(conv_spcial_t);	///<��������ڵ��������


static int help(void)
{
	printf("\n��ӭʹ��ini_conv�����ļ�ת������!(version %s)\n\n", INI_CONV_VER);
	printf("ʹ��˵��: ini_conv [OPTION]\n");
	printf("\t-h\t���������Ϣ\n");
	printf("\t-v\t����汾��Ϣ\n");
	printf("\t-m\t��תһ����%s ��%s  -> %s\n", VIRDEV_0_INI, VIRDEV_1_INI, MAIN_INI);
	printf("\t-s\tһת������ %s -> %s ��%s\n", MAIN_INI, VIRDEV_0_INI, VIRDEV_1_INI);
	return -1;
}

/**
	@brief: 	compare two strings , to see if the same
	@return:	0 is same; -1 is different
*/
static int str_compare(char *pstr1, char *pstr2)
{
	if(pstr2 != NULL)
	{
		if(strncmp(pstr1, pstr2, strlen(pstr2)) != 0)
		{
			return 0;
		}
	}
	
	return -1;	
}

/**
	@brief: 	��ѭ��ת��ǰ��Ҫ��һЩ����ڵ��������ת��
	@param:	directionΪ0����-sǰ����ת����Ϊ1��֮��
	@param:	d, d1, d2Ϊini�ṹ����ָ�룻
	@return:
*/
static int spcial_proc(dictionary * d, dictionary *d1, dictionary *d2, int direction)
{
	int i;
	int port_num;

	/**
		-s ǰ����ڵ㴦��
		����devinfo��port
	*/
	if(direction == CONV_S)
	{
		///ת��ǰ��Ҫ�Ȱ����⻷�ڽ���ת���������Ժ�ת��
		///<��ֵ��gt1000.ini  devinfo_dev2 ��guid
		str_gt1 = iniparser_getstring(d, conv_spcial[0].sec_key_gt1, NO_FOUND_STR);
		str_gt2 = iniparser_getstring(d, conv_spcial[0].sec_key_gt2, NO_FOUND_STR);
		if(strcmp(str_gt1, NO_FOUND_STR) != 0)
		{
			if(strcmp(str_gt2, NO_FOUND_STR) == 0)
			{
				is_ini_changed = CHANGE_SAVE;
			}
			snprintf(str_temp, sizeof(str_temp), "%s", str_gt1);
			str_temp[1] = '1';
			str_gt2 = str_temp;
			iniparser_setstr(d, conv_spcial[0].sec_key_gt2, str_gt2);
		}
		///<��ֵ��gt1000.ini  port_dev2��cmd_port/com0_port/com1_port��key
		for(i = 1; i < 4; i++)
		{	
			if(i==1)
			{
				port_num = 100;
			}
			else
			{
				port_num = 1;
			}
			str_gt1 = iniparser_getstring(d, conv_spcial[i].sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_spcial[i].sec_key_gt2, NO_FOUND_STR);
			if(strcmp(str_gt1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				val_temp = atoi(str_gt1)+port_num;
				iniparser_setint(d, conv_spcial[i].sec_key_gt2, val_temp); 
			}
		}
		///<��ֵ��gt1000.ini  ����port_dev2��key
		for(i = 4; i < spcial_total; i++)
		{
			str_gt1 = iniparser_getstring(d, conv_spcial[i].sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_spcial[i].sec_key_gt2, NO_FOUND_STR);
			if(strcmp(str_gt1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				iniparser_setstr(d, conv_spcial[i].sec_key_gt2, str_gt1); 
			}
		}
			
	}
	/**
		-m ת��ǰ����ڵ㴦��
		��ӵڶ�����ʼ����һ������-s only
		����ֻ����port
	*/
	else
	{
		///<a.ini��ֵ��b.ini  ��cmd_port/com0_port/com1_port
		for(i = 1; i < 4; i++)
		{	
			if(i==1)
			{
				port_num = 100;
			}
			else
			{
				port_num = 1;
			}
			str1 = iniparser_getstring(d1, conv_spcial[i].sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_spcial[i].sec_key_b, NO_FOUND_STR);
			if(strcmp(str1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				val_temp = atoi(str1)+port_num;
				iniparser_setint(d2, conv_spcial[i].sec_key_b, val_temp); 
			}
		}
		///<a.ini��ֵ��b.ini  ����port
		for(i = 4; i < spcial_total; i++)
		{
			str1 = iniparser_getstring(d1, conv_spcial[i].sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_spcial[i].sec_key_b, NO_FOUND_STR);
			if(strcmp(str1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				iniparser_setstr(d2, conv_spcial[i].sec_key_b, str1); 
			}
		}
	}

	return 0;
}

/**
	@brief: 	�Ա�����key�����Ƿ�ı䣬���ı䣬�����ini�ļ���keyֵ��ת��
	@param: d,d1,d2�ֱ�������gt1000����������ini�ļ��Ľṹ
	@param: conv_ini��ָ��Ҫת���������ļ������Ľṹָ��
	@param: conv_direction��ת������-s/-m,0Ϊ-s��1Ϊ-m
	@return: �ɹ�����0	ʧ�ܷ��ظ�ֵ
*/
static int comp_conv(dictionary * d, dictionary *d1, dictionary *d2, conv_ini_t *conv_ini, int conv_direction)
{
	/*
		��ʼgt1000�����������ļ���ת��
	*/
	if(conv_direction == CONV_S)
	{
		///�ж�gt1000.ini���Ƿ���ڴ˽ڵ㣬��ֹ�����ӽڵ�
		str_gt1= iniparser_getstring(d, conv_ini->sec_key_gt1, NO_FOUND_STR);
		///��������ڣ�����xxx,yyy�ҷ�-s only�����⣬����ֱ�ӷ���
		if(strcmp(str_gt1, NO_FOUND_STR) == 0)
		{
			printf("%s û��%s��\n", MAIN_INI, conv_ini->sec_key_gt1);
			//gtloginfo("%s û��%s��\n", MAIN_INI, conv_ini->sec_key_gt1);
			
			if((conv_ini->conv_att == NO_ACCORD2_A)&&(conv_ini->is_s_only == NOT_S_ONLY)&&(conv_ini->sec_key_gt2 != NULL))
			{
				str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					printf("%s û��%s��\n", MAIN_INI, conv_ini->sec_key_gt2);
					//gtloginfo("%s û��%s��\n", MAIN_INI, conv_ini->sec_key_gt2);
					return -1;
				}
				else
				{
					str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
					if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
					{
						gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
						printf("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
					
						iniparser_setstr(d2, conv_ini->sec_key_b, str_gt2);///<��ֵ��b.ini
					
						///˵�����޸Ĺ�
						is_ini_changed = CHANGE_SAVE;
					}
				}
			}
			else
			{
				return -1;
			}
		}
		///���ڵĻ�����Ҫ����ת��
		else
		{
			///��ȡ�����ڵ�key���Ա�Ա�
			str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
			str1 = iniparser_getstring(d1, conv_ini->sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);

			///dev2��ؽڵ�ת��
			if(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) == 0)
			{	
				if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
				{	
					gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
					printf("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
					iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<��ֵ��a.ini
					is_ini_changed = CHANGE_SAVE;
				}
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					printf("%s û��%s��\n", MAIN_INI, conv_ini->sec_key_gt2);
					return -1;
				}
				else if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
				{
					gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
					printf("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
	
					iniparser_setstr(d2, conv_ini->sec_key_b, str_gt2);///<��ֵ��b.ini
					is_ini_changed = CHANGE_SAVE;
				}
			}
			///ͬ���ֽڵ�ת��
			else
			{
				if(conv_ini->is_s_only == IS_S_ONLY)
				{
					if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
					{
						gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
						printf("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
						iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<��ֵ��a.ini
						is_ini_changed = CHANGE_SAVE;
					}
					if(strncmp(str2, str_gt1, MAX(str2, str_gt1)) != 0)
					{
						gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
						printf("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
	
						iniparser_setstr(d2, conv_ini->sec_key_b, str_gt1);///<��ֵ��b.ini
						is_ini_changed = CHANGE_SAVE;
					}
				}
				else 
				{
					if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
					{
						gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
						printf("%s��%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
						iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<��ֵ��a.ini
						is_ini_changed = CHANGE_SAVE;
						if(conv_ini->conv_att == IS_ACCORD2_A)
						{
							gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
							printf("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
	
							iniparser_setstr(d2, conv_ini->sec_key_b, str_gt1);///<��ֵ��b.ini
							is_ini_changed = CHANGE_SAVE;
						}
					}

				}
			}
			
		}///<end �ڵ���ڵ����
		
	}///<end -s mode
	///start -m mode!!!
	/*
		��ʼ���������ļ���gt1000��ת��
	*/
	else
	{
		///����ת���ڵ�ֱ�ӷ���
		if(conv_ini->is_s_only == IS_S_ONLY)
		{
			return -1;
		}
		///�ж�a.ini���Ƿ���ڴ˽ڵ㣬��ֹ�����ӽڵ�
		str1 = iniparser_getstring(d1, conv_ini->sec_key_a, NO_FOUND_STR);
		///��������ڣ�����xxx,yyy�ҷ�-s only�����⣬����ֱ�ӷ���
		if(strcmp(str1, NO_FOUND_STR) == 0)
		{
			printf("%s û��%s��\n", VIRDEV_0_INI, conv_ini->sec_key_a);
			//gtloginfo("%s û��%s��\n", VIRDEV_0_INI, conv_ini->sec_key_a);
			
			if((conv_ini->conv_att == NO_ACCORD2_A)&&(conv_ini->is_s_only == NOT_S_ONLY)&&(conv_ini->sec_key_gt2 != NULL))
			{ 
				str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					printf("%s û��%s��\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					//gtloginfo("%s û��%s��\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					return -1;
				}
				else
				{
					str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
					if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
					{
						gtloginfo("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
						printf("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
					
						iniparser_setstr(d, conv_ini->sec_key_gt2, str2);///<��ֵ��gt1000.ini
					
						///˵�����޸Ĺ�
						is_ini_changed = CHANGE_SAVE;
					}
				}
			}
			else
			{
				return -1;
			}
		}
		///a.ini�нڵ���ڵĻ�
		else
		{
			str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
			str_gt1 = iniparser_getstring(d, conv_ini->sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
				
			if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
			{
				gtloginfo("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt1, str_gt1, str1);
				printf("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt1, str_gt1, str1);
	
				iniparser_setstr(d, conv_ini->sec_key_gt1, str1);///<��ֵ��gt1000.ini
				is_ini_changed = CHANGE_SAVE;
				if((conv_ini->conv_att == IS_ACCORD2_A)&&(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) != 0))
				{
					gtloginfo("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str1);
					printf("%s��%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str1);
	
					iniparser_setstr(d2, conv_ini->sec_key_b, str1);///<��ֵ��gt1000.ini
				}
			}
			///����yyy�ڵ�Ļ���Ҫ��b.ini�Ա���
			if(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) == 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					printf("%s û��%s��\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					return -1;
				}
				else if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
				{
					gtloginfo("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
					printf("%s��%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
	
					iniparser_setstr(d, conv_ini->sec_key_gt2, str2);///<��ֵ��gt1000.ini _dev2
					is_ini_changed = CHANGE_SAVE;
				}
			}
		}///<end a.ini�ڵ����
	}///<end -m mode
	
	return 0;
}	


/**
	@brief:   ��gt1000.ini ת�����������ļ���
	@param: conv_direcΪ0��ִ��-s��Ϊ1ִ��-m
	@return: �ɹ�����0��ʧ�ܷ��ظ�ֵ
*/
static int s2m_or_m2s(int conv_direc)
{
	dictionary *d;		///<�����ļ��ṹ����ָ��
	dictionary *d1;
	dictionary *d2;
	FILE *lock;		///<�������ļ�ָ��
	FILE *lock1;
	FILE *lock2;
	int i;	

	
	///����Ҫת����ini�����ļ�
	d = iniparser_load_lockfile(MAIN_INI, 0, &lock);
	if(d == NULL)
	{
		printf("��%sʧ��\n", MAIN_INI);
		gtloginfo("��%sʧ��\n", MAIN_INI);
		return -1;
	}
	d1 = iniparser_load_lockfile(VIRDEV_0_INI, 0, &lock1);
	if(d1 == NULL)
	{
		printf("��%sʧ��\n", VIRDEV_0_INI);
		gtloginfo("��%sʧ��\n", VIRDEV_0_INI);
		return -1;
	}
	d2 = iniparser_load_lockfile(VIRDEV_1_INI, 0, &lock2);
	if(d2 == NULL)
	{
		printf("��%sʧ��\n", VIRDEV_1_INI);
		gtloginfo("��%sʧ��\n", VIRDEV_1_INI);
		return -1;
	}

	if(conv_direc == CONV_S)
	{
		///��devinfo��cmd_port�Ƚ���ת����gt1000�����dev2�ڵ���
		spcial_proc(d, d1, d2, CONV_S);
		
		///��ʼ����-s ת��
		gtloginfo("����ini_conv, version:%s, -s mode\n", INI_CONV_VER);
		printf("����ini_conv, version:%s, -s mode\n", INI_CONV_VER);
		
		for(i = 0; i < conv_total; i++)
		{
			comp_conv( d,  d1,  d2,  (conv_ini_t *)(conv_ini_all+i), CONV_S);
		}
	}
	
	else if(conv_direc == CONV_M)
	{
		///��a.ini��cmd_port + 1000��ֵ��b.ini��,����port���丳ֵ��b.ini��
		spcial_proc(d, d1, d2, CONV_M);
		
		///��ʼ����-m ת��
		gtloginfo("����ini_conv, version:%s, -m mode\n", INI_CONV_VER);
		printf("����ini_conv, version:%s, -m mode\n", INI_CONV_VER);

		for(i = 0; i < conv_total; i++)
		{
			comp_conv( d,  d1,  d2,  (conv_ini_t *)(conv_ini_all+i), CONV_M);
		}

	}
	else
	{
		///<��ִ���κκ���
	}
	
	///û�иĶ��Ļ��������д洢
	if(is_ini_changed == NO_CHANGE)
	{
		fclose(lock);
		fclose(lock1);
		fclose(lock2);
		if(conv_direc == CONV_S)
		{
			printf("ini_conv, version:%s, -s mode, δ�޸������ļ����˳�\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -s mode, δ�޸������ļ����˳�\n", INI_CONV_VER);
		}
		else
		{
			printf("ini_conv, version:%s, -m mode, δ�޸������ļ����˳�\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -m mode, δ�޸������ļ����˳�\n", INI_CONV_VER);
		}
		
	}
	
	///�иĶ����洢�����������ļ���
	else
	{
		save_inidict_file(MAIN_INI, d, &lock);
		save_inidict_file(VIRDEV_0_INI, d1, &lock1);
		save_inidict_file(VIRDEV_1_INI, d2, &lock2);
		if(conv_direc == CONV_S)
		{
			printf("ini_conv, version:%s, -s mode, �޸��������ļ����˳�\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -s mode, �޸��������ļ����˳�\n", INI_CONV_VER);
		}
		else
		{
			printf("ini_conv, version:%s, -m mode, �޸��������ļ����˳�\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -m mode, �޸��������ļ����˳�\n", INI_CONV_VER);
		}
	}
		
	iniparser_freedict(d);
	iniparser_freedict(d1);
	iniparser_freedict(d2);
		
	return 0;
}


int main(int argc, char **argv)
{

	int cmd_val;	///< �����д���󷵻ز���
	//int ret;		///<ת����������ֵ


	///����־
	gtopenlog("ini_conv");
	
	
	///���������в���
	if(argc != 2)
	{
		help();
		return -1;
	}
	
	opterr = 0 ;
	while( ( cmd_val = getopt( argc, argv, "smhv" ) ) != -1 )
	{
		switch( cmd_val )
		{
			case 's' :
				///���������ļ�ת����ip1004.ini��
				s2m_or_m2s(CONV_S);
				break;
				
			case 'm' :
				///ip1004.iniת�������������ļ���
				s2m_or_m2s(CONV_M);
				break;

			case 'v' :
				///��ӡ�汾��
				printf("Ini_conv verion is %s\n", INI_CONV_VER);
				break;

			case 'h' :
				///���������Ϣ
				
			default:
				help();
				break;
		}
	}
	
	
	return 0;
}
