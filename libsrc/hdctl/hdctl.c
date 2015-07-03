#include "hdctl.h"
#include "smart.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "gtlog.h"
#include "devinfo.h"


int open_hd_dev(char *pathname)
{
	if((pathname == NULL) ||(access(pathname, F_OK|R_OK)!=0 ))//�ڵ㲻����
		return -ENODEV;
	else
		return open(pathname,O_RDONLY | O_NONBLOCK);
}






/****************************************************************
	��������	get_hd_temperature()
	����		��ȡָ��Ӳ�̵ĵ�ǰ�¶ȣ����ϣ�
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		��ֵ��ʾ��ǰ�¶ȣ���ֵ��ʾ������
****************************************************************/
int get_hd_temperature(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,TEMPERATURE_CELSIUS, RAWVALUE);		
		close(fd);
		return (ret & 0x3F);
	}
	else
		return -ENODEV;	
}


/****************************************************************
	��������	get_hd_max_temperature()
	����		��ȡָ��Ӳ�̵�����¶ȣ����ϣ�
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		��ֵ��ʾ����¶ȣ���ֵ��ʾ������
****************************************************************/
int get_hd_max_temperature(int diskno)
{
	int fd;
	int ret;
	int manufactor;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		manufactor = get_hd_manufactor(fd);
		switch(manufactor) 
		{
			case(SEAGATE):	ret = (int)ataReadSmartValues(fd,TEMPERATURE_CELSIUS,WORSTVALUE);
							break;
			default:		ret = get_hd_temperature(diskno);
							break;
		}
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}




/****************************************************************
	��������	get_hd_shorttest_result()
	����		��ȡָ��Ӳ�̵���һ�ζ̲��Խ��
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	���		percent_done:ָ����ɰٷ�����ָ�룬��ֵȡֵ0-100���ڲ��Խ����е�����²�������
	����ֵ		0��ʾ�ɹ�ͨ����1��ʾʧ��,2��ʾû�ж������,3��ʾ������
				��ֵ��ʾʧ�ܵĴ�����
****************************************************************/
int get_hd_shorttest_result(int diskno, int *percent_done)
{
	int fd;
	int ret;
	
	if(percent_done == NULL)
		return -EINVAL;
	fd = open_hd_dev(get_hd_nodename(diskno));
	
	
	if(fd > 0)
	{
		
		ret = ataGetSelfTestLog(fd,GT_SMART_SHORTTEST,percent_done);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}


/****************************************************************
	��������	get_hd_shorttest_result()
	����		��ȡָ��Ӳ�̵���һ�γ����Խ��
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	���		percent_done:ָ����ɰٷ�����ָ�룬��ֵȡֵ0-100���ڲ��Խ����е�����²�������
	����ֵ		0��ʾ�ɹ�ͨ����1��ʾʧ��,2��ʾû�ж������,3��ʾ������
				��ֵ��ʾʧ�ܵĴ�����
****************************************************************/
int get_hd_longtest_result(int diskno, int *percent_done)
{
	int fd;
	int ret;
	
	
	if(percent_done == NULL)
		return -EINVAL;
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = ataGetSelfTestLog(fd,GT_SMART_LONGTEST,percent_done);
	
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}




/****************************************************************
	��������	run_hd_smarttest()
	����		��ָ��Ӳ�̽���ָ�����ʵĲ���
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
				testtype,Ϊ0��ʾ�̲��ԣ�Ϊ1��ʾ������,Ϊ2��ʾ�̲���ͨ�����ٳ�����
	����ֵ		0��ʾ�ɹ�������ֵ��ʾʧ��
****************************************************************/
int run_hd_smarttest(int diskno, int testtype)
{
	int fd,ret;
	char cmd[100];
	int percent;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if (fd > 0)
	{
		switch(testtype)
		{
			case(0):
			case(1):	ret = ataSmartTest(fd,testtype);
						break;
			case(2):	
			{
				//lc do ��ͨ��ϵͳ���ֱ�ӵݹ����
				
				sprintf(cmd,"/ip1004/trigsmarttest %d &",diskno);
						ret = system(cmd);
						break;
				/*
				printf("����%d�Ŵ��̵Ķ̲���!�����ĵȴ�2��������\n",diskno);
				ret = run_hd_smarttest(diskno,GT_SMART_SHORTTEST); //shorttest
				if(ret != 0)
				{
					printf("%d�Ŵ��̶̲���ʧ��,%d:%s\n",diskno,ret,strerror(-ret));
					break;
				}
				sleep(100);
	
				ret =get_hd_shorttest_result(diskno,&percent) ;
				if(ret == 0)
				{
					printf("%d�Ŵ��̶̲���ͨ�������д��̳�����!\n",diskno);
					run_hd_smarttest(diskno,GT_SMART_LONGTEST);
					printf("%d�Ŵ����ѿ�ʼ���ԡ�����5Сʱ����ú��ѯ���Խ����\n",diskno);
				}
				else
					printf("%d�Ŵ��̶̲��Խ�� %d:%s�������г�����\n",diskno,ret,get_testresult_str(ret));

				break;
				*/
			}
			default:	ret = -EINVAL;
						break;
		}
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
}

/****************************************************************
	��������	get_hd_info()
	����		��ȡָ��Ӳ�̵��ͺ�,���к�,�̼��汾�ŵȻ�����Ϣ
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	���		model,Ӳ���ͺ��ַ���
				serialno,Ӳ�����к��ַ���
				firmware,�̼��汾�ַ���
	����ֵ		0��ʾ�ɹ�����ֵ��ʾ������
	˵��		model�ַ����Ļ�����������Ҫ40�ֽ�
				serialno�Ļ�����������Ҫ20�ֽ�
				firmware�Ļ�����������Ҫ8�ֽ�
****************************************************************/
int get_hd_info(IN int diskno, OUT char *model, OUT char* serialno, OUT char *firmware)
{
	int fd,ret=0;
	struct ata_identify_device  drive;
	
	if((model==NULL)||(serialno==NULL)||(firmware==NULL))
		return -EINVAL;
	
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		if(ataReadHDIdentity(fd, &drive)== 0)
		{
			formatdriveidstring(model, (char *)drive.model,40);
  			formatdriveidstring(serialno, (char *)drive.serial_no,20);
  			formatdriveidstring(firmware, (char *)drive.fw_rev,8);
		}
		else
		 	ret = -EPERM;
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
	
}









/***************************************************************
	��������	get_hd_volume_inGiga()
	����		��ȡָ��Ӳ�̵���������λΪG,�����۱�׼,��250G,320G��
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		��ֵ��ʾ����,0��ֵ��ʾʧ��
****************************************************************/
int get_hd_volume_inGiga(int diskno)
{
	struct ata_identify_device  drive;
	long long capacity;
	
	int fd;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		if(ataReadHDIdentity(fd,&drive) == 0)
		{
			capacity = determine_capacity(&drive);
			close(fd);
			return capacity/1000000;
		}
		close (fd);
		return -EPERM;
	}
	else
		return -ENODEV;
}



/****************************************************************	
	��������	get_hd_runninghour()
	����		��ȡָ��Ӳ�̵��ϵ���Сʱ��
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		�Ǹ�ֵ��ʾСʱ��,��ֵ��ʾ������
****************************************************************/
int get_hd_runninghour(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,POWER_ON_HOURS,RAWVALUE);
		if((get_hd_manufactor(fd)==MAXTOR)) //���ص����Է��Ӽ���
			ret/= 60;
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	

}



/****************************************************************	
	��������	get_hd_relocate_sector()
	����		��ȡָ��Ӳ�̵��ط���������
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		�Ǹ�ֵ��ʾ������,��ֵ��ʾ������
****************************************************************/
int get_hd_relocate_sector(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,REALLOCATED_SECTOR_CT,RAWVALUE);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	

}




/****************************************************************	
	��������	get_hd_pending_sector()
	����		��ȡָ��Ӳ�̵Ĺ���������
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		�Ǹ�ֵ��ʾ������,��ֵ��ʾ������
****************************************************************/
int get_hd_pending_sector(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,CURRENT_PENDING_SECTOR,RAWVALUE);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}


/****************************************************************	
	��������	get_hd_errorlog_num()
	����		��ȡָ��Ӳ�̵Ĵ�����־����
	����		diskno,Ϊ0,1,2..��Ӳ�̱��
	����ֵ		�Ǹ�ֵ��ʾ������־����,��ֵ��ʾ����
****************************************************************/
int get_hd_errorlog_num(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = ataReadErrorLog(fd);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
}

