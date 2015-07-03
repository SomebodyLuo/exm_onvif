#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

#include "testmod.h"
#include "pub_err.h"
#include <devinfo.h>
//#include "multicast_ctl.h"
//#include "hi_rtc.h"

/*
 * �����Խ�����ݽṹ���������ָ�����ļ�
 */
 
/*
****************************************************************************
���Խ�����ݽṹ 0 ��ʾ����������ֵ��ʾ��Ӧ�Ĵ���
	-1 �޷���������ģ��
	-2  �޷�������ģ��
	-3  �޷�������ģ���д
	-4  �޷�������ģ�����I/O ����
****************************************************************************
*/
/*
*****************************************************
*��������: print_rpt
*��������: ��ӡ���󱨸�
*����		    : err ������������
*			    : stat ����״̬����
*	      		    : fp ��¼�ļ���ָ��
*����ֵ      : ��
*****************************************************
*/

void print_rpt(int err, int stat, FILE * fp)
{
	fprintf(fp,"%d", err+stat);
	fprintf(fp, ":");

	switch(err)
	{
		case (NETENC_ERR):
		fprintf(fp, Net_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		case (HQENC_ERR):
		fprintf(fp, Hq_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		case (QUARD_ERR):
		fprintf(fp, Tw2834_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO2834);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		
		fprintf(fp, ",");
		break;
		
		case (DSP_ERR):
		fprintf(fp, Dsp_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODSP);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");

		fprintf(fp, ",");
		break;
		
		case (IDE_ERR):
		fprintf(fp, Ide_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODISK);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOPART);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;

		case (TW9903_ERR):
		fprintf(fp, TW9903_error);
		fprintf(fp, "(");
		
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_IIC);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NO9903);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		default:
		fprintf(fp, UNKNOW);
		fprintf(fp, ",");
		break;
	}

}
/*
*****************************************************
*��������: print_code
*��������: ��������ӡ����
*����		  : err ������������
*			  : stat ����״̬����
*	      		  : fp ��¼�ļ���ָ��
*����ֵ      : ��
*****************************************************
*/

void print_code(int err, int stat, FILE * fp)
{
	fprintf(fp,"%d,", err+stat);
}
/*
*****************************************************
*��������: print_stat
*��������: ������Ϣ��ӡ����
*����		  : err ������������
*	      		  : stat ����״̬����
*	      		  : fp ��¼�ļ���ָ��
*����ֵ	  : ��
*****************************************************
*/
void  print_stat(int err, int stat, FILE * fp)
{
  switch(err){
	case (NETENC_ERR):
		fprintf(fp, Net_error);
		fprintf(fp,":");
		
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
	case (HQENC_ERR):
		
		fprintf(fp, Hq_error);
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
			  
		  case (4):
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");

		break;
		
	case (QUARD_ERR):
		fprintf(fp, Tw2834_error );
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO2834);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");

		break;
		
	case (IDE_ERR):
		fprintf(fp, Ide_error );
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODISK);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOPART);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_OPEN);		///lsk 2007-11-8
			  break;
			  
		  case (4):
			  fprintf(fp, ERR_WRITE);		///lsk 2007-11-8
			  break;
			  
		  case (5):
			  fprintf(fp, ERR_READ);		///lsk 2007-11-8
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
		
	case (DSP_ERR):
		fprintf(fp, Dsp_error);
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODSP);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
		
	case (TW9903_ERR):
	fprintf(fp, TW9903_error);
	fprintf(fp,":");
	switch (stat){
	  case (1):
		  fprintf(fp, ERR_IIC);
		  break;
		  
	  case (2):
		  fprintf(fp, ERR_NO9903);
		  break;
		  
	  case (3):
		  fprintf(fp, UNKNOW);
		  break;
	  case (4):
		  fprintf(fp, UNKNOW);
		  break;
		  
	  case (5):
		  fprintf(fp, UNKNOW);
		  break;
		  
	  default:
		  fprintf(fp, UNKNOW);
		  break;
	  }
	fprintf(fp, "\n            ");
	break;
	
	default:
		fprintf(fp, UNKNOW);
		fprintf(fp, "\n            ");
		break;
  	}
}
/*
*****************************************************
*��������: save_result_to_file
*��������: ����洢����
*����		  : filename �ļ���
*	      		  : devstat ����״̬���ݽṹָ��
*����ֵ	  : �������
*****************************************************
*/
int save_result_to_file(char *filename,struct dev_test_struct *devstat, multicast_sock* ns)
{
	FILE *fp;
	struct dev_test_struct *stat;
	int rc;
	if(filename==NULL)
		return ERR_CANNOT_OPEN_FILE;
	if(devstat==NULL)
		return ERR_INTERNAL; 
	fp=fopen(filename,"w");
	if(fp==NULL)
		return ERR_CANNOT_OPEN_FILE;
	stat=devstat;
	fprintf(fp,"[boardtest]\n");
        if((stat->ide_stat==0)&&
	   (stat->netenc_stat==0)&&
	   (stat->hqenc0_stat==0)&&
	   (stat->audio_stat==0)&&
	   (stat->quad_stat==0)&&
	   (stat->tw9903_stat==0)&&
	   (stat->rtc_stat==0)&&
	   (stat->usb_stat==0))	//lsk 2006 -12-27
	{
		fprintf(fp,"result = 0, \n");
		fprintf(fp,"description = ���Գɹ�,\n");
		fprintf(fp,"report = 0: ���Գɹ�,\n");
		
		rc=SUCCESS;
		printf("all OK!\n");
		result_report(SUCCESS, ns);// lsk 2007-10-26
	}
	else
	{
	   fprintf(fp,"result = ");
	   rc=ERR_DEVICE_BAD;

	   if(stat->netenc_stat!=0)
	   print_code(NETENC_ERR, stat->netenc_stat, fp);
	   
	   if(stat->hqenc0_stat!=0)
	   print_code(HQENC_ERR, stat->hqenc0_stat, fp);
	   
	   if(stat->quad_stat!=0)
	   print_code(QUARD_ERR, stat->quad_stat, fp);
	   
	   if(stat->ide_stat!=0)
	   print_code(IDE_ERR, stat->ide_stat, fp);

          if(stat->usb_stat!=0)
	   print_code(IDE_ERR, stat->usb_stat, fp);
	   
	   if(stat->audio_stat!=0)
	   print_code(DSP_ERR, stat->audio_stat, fp);
	   
	   if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	   print_code(TW9903_ERR, stat->tw9903_stat, fp);
	   
#if 0
	   
	   printf("%d,%d,%d,%d,%d ",
	   	stat->netenc_stat,
	   	stat->hqenc0_stat,
	   	stat->quad_stat,
	   	stat->ide_stat,
	   	stat->audio_stat
	   	);
#endif
	   
	fprintf(fp,"\ndescription=");

	if(stat->netenc_stat!=0)
	{
		print_stat(NETENC_ERR, stat->netenc_stat, fp);
		result_report(NETENC_ERR+stat->netenc_stat, ns);//lsk 2007 -6-1
	} 

	if(stat->hqenc0_stat!=0)
	{	  
		print_stat(HQENC_ERR, stat->hqenc0_stat, fp);
		result_report(HQENC_ERR+stat->hqenc0_stat, ns);//lsk 2007 -6-1
	}	   

	if(stat->quad_stat!=0)
	{
		print_stat(QUARD_ERR, stat->quad_stat, fp);
		result_report(QUARD_ERR+stat->quad_stat, ns);//lsk 2007 -6-1
	}

	if(stat->usb_stat!=0)
	{
		print_stat(USB_ERR, stat->usb_stat, fp);
		result_report(USB_ERR+stat->usb_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->ide_stat!=0)
	{
		print_stat(IDE_ERR, stat->ide_stat, fp);
		result_report(IDE_ERR+stat->ide_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->audio_stat!=0)
	{
		print_stat(DSP_ERR, stat->audio_stat, fp);
		result_report(DSP_ERR+stat->audio_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	{
		print_stat(TW9903_ERR, stat->tw9903_stat, fp);
		result_report(TW9903_ERR+stat->tw9903_stat, ns);//lsk 2007 -6-1
	}

//////////  new format for net report
	   fprintf(fp,"\nreport=");
	   if(stat->netenc_stat!=0)
   	   print_rpt(NETENC_ERR, stat->netenc_stat, fp);
	   
	   if(stat->hqenc0_stat!=0)
	   print_rpt(HQENC_ERR, stat->hqenc0_stat, fp);
	   
	   if(stat->quad_stat!=0)
	   print_rpt(QUARD_ERR, stat->quad_stat, fp);
	   
	   if(stat->ide_stat!=0)
	   print_rpt(IDE_ERR, stat->ide_stat, fp);
	   
	   if(stat->audio_stat!=0)
	   print_rpt(DSP_ERR, stat->audio_stat, fp);

	   if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	   print_rpt(TW9903_ERR, stat->tw9903_stat, fp);
	}
	fclose(fp);
	return rc;
}
/*
 * ģ��������̿���
 */
 
/*
*****************************************************
*��������: testsim
*��������: Ӳ�����Ժ���
*����		  : stat ����״̬���ݽṹָ��
*����ֵ	  : 0 ���� -1 stat ��Ч
*****************************************************
*/
int testsim(struct dev_test_struct *stat, multicast_sock* net_st, int prog)
{  
	int progress=prog;
       int result;
       
	if(stat==NULL)
		return -1;
	init_devinfo();
	printf("�豸�ͺ�: %s\n", get_devtype_str()); //lsk 2006-12-27  print device type

       result = testRTC();
       if(result )
	{
		printf("RTC test error\n" );
	}
	else 
	{
		printf("RTC test  OK\n");
		send_test_report(net_st, "RTC OK", progress);
	}

       result = testUSB();
       if(result )
	{
		printf("USB test error\n" );
              stat->usb_stat = result;
	}
	else 
	{
		printf("USB test  OK\n");
		send_test_report(net_st, "USB OK", progress);
	}       
	if(get_ide_flag()>=0)	//lsk 2006 -10 -26 get_ide_flag()==1 -> get_ide_flag()>=0
	{
		printf("testing IDE...");
		stat->ide_stat=test_IDE(net_st, &progress);		//����IDE������
		if(stat->ide_stat)
		{
			printf("IDE state: %d\n", stat->ide_stat);
		}
		else 
		{
			printf("IDE OK\n");
			send_test_report(net_st, "IDE OK", progress);
		}
		sleep(1);
	}
	
	if(get_videoenc_num()>0)
	{
		if(get_audio_num()>0)
		{
		}
		if(get_quad_flag()==1)
		{
		}
		////lsk 2006 -12 -27
		if(get_quad_flag()==0)
		{
		}
	}

	send_test_report(net_st, "board test finished", 100);//��ɺ��Ͳ��Խ���=100
	return 0;
}

