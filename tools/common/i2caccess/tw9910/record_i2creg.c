/**
	@file:	record_i2creg.c
	@brief:	test assigned register of one chip
			1. test  registers to see which get a error
			2. can test for any register of any chip
	@param:	record_i2creg -a<chip addr>  -s<test interval (seconds)>  -n<test register number>  -r<register addr1 addr2 addr...>  ; 	
	@modify:	08.09.19  have finished
			08.09.26  modified once
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tw9910api.h>
#include <unistd.h>
#include <time.h>
#include <iiclib.h>

#define REG_MAX_NUM	20   ///<Ҫ��ȡ�ļĴ��������Ŀ

static int i2c_init = -1;			///<iic�豸���ļ�������



static void help(void)
{
	printf("record_i2creg -a оƬ��ַ-s ��ȡʱ����(   ��)  -n Ҫ��ȡ�ļĴ�����Ŀ-r �Ĵ�����ַ1 ��ַ2 ��ַ...  \n");
	printf("************�Ĵ����鸳ֵ��ȡ�ĵ�·������/log/collectreg.txt**********\n");
	return;
}

static void init_iic(void)
{
	
	i2c_init = open_iic_dev();
	if(i2c_init < 0)
	{
         	printf("��iic�豸%s ʧ��!\n", i2c_init);
              exit(1);
	}

	return;
}

int main(int argc,char *argv[]) {

	int 		ret;
	int 		addr;		///<оƬ��ַ
	unsigned char  reg[REG_MAX_NUM]={0};  		 ///<�Ĵ�����ַ
	unsigned char reg_val[REG_MAX_NUM] = {0};	///<�Ĵ���ֵ
	unsigned char	*reg_val_pf; 				///< ����Ҫ��ӡ�ļĴ��ֵ
	int ch;		///< �����з���ֵ
	int tag = 0;	///<	�����������ж�
	
	unsigned int reg_num;  ///< ��¼�Ĵ�������
	unsigned int second; ///< ��¼���
	unsigned int i = 0, j = 6;
	time_t timep; ///< ʱ�Ӳ���

	FILE *fp = NULL;     
	unsigned char reg_buf[100]; 
	unsigned char reg_temp[REG_MAX_NUM] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; 


	/**
		��������в���
	*/
	opterr =0 ;
	while( ( ch = getopt( argc, argv, "a:s:n:r:" ) ) != -1 )
	{
		switch( ch )
		{
			case 'a' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				addr = strtol(optarg, NULL, 16);
				break;
				
			case 'r' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg[0] = strtol(optarg, NULL, 16);
				break;
				
			case 's' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				second = strtol(optarg, NULL, 10);
				//printf("%d\n",pf_num);
				break;
				
			case 'n' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg_num = strtol(optarg, NULL, 10);
				break;
				
			default:
				break;
		}
	}

		
	///��������и���
	if (argc < ( 8 + reg_num))
	{
		help();
		return -1;
	}

	///�Ĵ��������Ŀ��Ҫ����REG_MAX_NUM
	if(reg_num > REG_MAX_NUM)
	{
		printf("The registers you want read have beyond %d\n", REG_MAX_NUM);
		return;
	}

	///�Ĵ�ֵ��ֵ
	for(i = 1; i < reg_num; i++)
	{
		reg[i] = strtol(argv[(i+8)], NULL, 16);
	}

	init_iic();

	fp = fopen("/log/collectreg.txt", "a+");
	if(fp == NULL)
	{
		printf("open collectreg failed \n");
		return -1;
	}

	///��ʼ�Ĵ�����ʱ�洢ֵ���Թ��ԱȲ���
	for(i = 0; i < reg_num; i++)
	{
		ret=read_iic_reg(addr, reg[i], &reg_temp[i]);
		if(ret<0)
		{
			printf("��ȡоƬ%02x,addr:%02xʧ��!\n",addr,reg_temp[i]);
			exit(1);
		}
		time(&timep);
		sprintf(reg_buf,"chip:%02x %02x=%02x  current time=%s\n", addr, reg[i], reg_temp[i], ctime(&timep));
		printf("%s", reg_buf);
		fputs(reg_buf, fp);
		fflush(fp);
	}

	///δ����-n����
		while(1)
		{
			for(i = 0; i < reg_num; i++)
			{
				reg_val[i] = reg_temp[i];
				ret=read_iic_reg(addr, reg[i], &reg_temp[i]);
				if(ret<0)
				{
					printf("��ȡоƬ%02x,addr:%02xʧ��!\n",addr,reg_temp[i]);
					exit(1);
				}	
				if(reg_val[i] != reg_temp[i])
				{
					time(&timep);
					sprintf(reg_buf,"chip:%02x %02x=%02x  current time=%s\n", addr, reg[i], reg_temp[i], ctime(&timep));
					printf("%s", reg_buf);
					fputs(reg_buf, fp);
					fflush(fp);
					sleep(second);
				}
				else
				{
					sleep(second);
				}
				//printf("argc=7 reg_temp_%02x\n", reg_temp);
			}
		}
	

	fclose(fp);
	return 0;	
}

