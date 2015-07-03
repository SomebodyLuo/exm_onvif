#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tw9910api.h>
#include <unistd.h>

#define	MAX_REG_NUM	112 ///<�Ĵ��������Ŀ
#define	IIC_DEV_NAME	"/dev/i2c-0"	///<iic�豸�ڵ�
static int iic_fd=-1;			///<iic�豸���ļ�������



static void help(void)
{
	printf("��ȡ����:iaccess 1 -b оƬ��ַ-d ��ʼ�Ĵ�����ַ-n Ҫ��ȡ����Ŀ\n");
	printf("д������:iaccess 0 оƬ��ַ �Ĵ�����ַ �Ĵ���ֵ\n");
	return;
}
/**
	@file:	Icaccess.c
	@brief:	test 9910
			1. test every register content to judge which register make error
			2. to confirm if this register the one makes a error
	@param:	iaccess <1/0> 1 : read; 0 : write; -b<chip addr>; -d<start register addr>; -n<number prepare to print>; 
	@modify:	08.09.08 the read module have finished,start the write module
*/
int main(int argc,char *argv[]) {

	int 		ret;
	int 		type;		///<�������� 0:д 1:��
	int 		addr;		///<оƬ��ַ
	int 		reg;		///<�Ĵ�����ַ
	unsigned char 	reg_val = 0;	///<�Ĵ���ֵ
	unsigned char	*	reg_val_pf; ///< ����Ҫ��ӡ�ļĴ��ֵ
	unsigned char		reg_adder = (unsigned char)(1);
	int ch;		///< for commend line
	int pf_num;  ///< number wait for print
	int i;

	if (argc < 5){
		help();
		return -1;
	}


	type = atoi( argv[1] );
	if(argc>5)
	{
		reg_val= strtol(argv[5],NULL,16);
	}

	/**
		��������в���
		�����ʽΪ:ִ���ļ�0/1  -b <addr> -d <register addr> -n <the number how many you want to print>
	*/

	opterr =0 ;
	while( ( ch = getopt( argc, argv, "b:d:n:" ) ) != -1 )
	{
		switch( ch )
		{
			case 'b' :
				printf( "\t opt_%s   ch_%c\n", optarg, ch);
				addr = strtol(optarg, NULL, 16);
				break;
			case 'd' :
				printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg = strtol(optarg, NULL, 16);
				printf("%d\n",reg);
				break;
			case 'n' :
				printf( "\t opt_%s   ch_%c\n", optarg, ch);
				pf_num = strtol(optarg, NULL, 10);
				printf("%d\n",pf_num);
				break;
			default:
				break;
		}
	}

	///�жϴ�ӡ��Ŀ���Ĵ�����112
	if( pf_num > MAX_REG_NUM )
	{
		printf( "n value cannot beyond 112\n" );
		return 0 ;
	}
	
	iic_fd=open(IIC_DEV_NAME,O_RDWR);
	if(iic_fd<0)
	{
         	printf("��iic�豸%s ʧ��!\n",IIC_DEV_NAME);
              exit(1);
	}

	if(type==1)
	{
		for( i=0; i<pf_num; i++ )	
		{
			ret=read_iic_reg(iic_fd,addr,reg,&reg_val);
	
			if(ret<0)
			{
				printf("��ȡоƬ%02x,addr:%02xʧ��!\n",addr,reg_val);
				exit(1);
			}
			
			else
			{
				printf("chip:%02x %02x=%02x\n",addr,reg,reg_val);
				reg++;
			}
		
		
		}
	}
	else
	{
		ret=write_iic_reg(iic_fd,addr,reg,reg_val);
		if(ret<0)
		{
			printf("����оƬ%02x,addr:%02x=%02xʧ��!\n",addr,reg,reg_val);
			exit(1);
		}
		printf("���� chip:%02x %02x=%02x\n",addr,reg,reg_val);
	}
	
	close(iic_fd);
	return 0;	
}

