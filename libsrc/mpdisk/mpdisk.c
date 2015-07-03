/*
		Ϊ֧�ֶ�Ӳ�̶�����Ŀ�,����diskinfo��devinfo��
							--wsy Nov-Dec 2007
*/


#include "mpdisk.h"
#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "file_def.h"
#include "hdutil.h"
#include "dirent.h"


//���ڴ���get_disk_free_fn����Ҫ�Ĳ���
struct diskfree_struct 
{
	char * freest_partition;//��ǰ��յķ�����������"/hqdata/hda2"
	int  freespace_max; //��ǰ��յķ����Ŀռ���,MΪ��λ
};


//��ȡָ�������ڵ�Ĺ��ص�����,
//����:devname,����"/dev/sda2"; ���, mountpath,����"/hqdata/sda2"
//����ֵ:	���ص����Ƶ��ַ���ָ��("/hqdata/sda2"),ʧ�ܷ���null
char * partitionname2mountpath(IN char * devname, OUT char* mountpath)
{
    char *lp;
    
    if((devname == NULL) ||(mountpath == NULL))
        return NULL;

    lp=strstr(devname,"/sd");
    if(lp!=NULL)
    {
        sprintf(mountpath,"%s%s",HDMOUNT_PATH,lp);
        return mountpath;
    }
    else
        return NULL;
}



/**************************************************************************
*	��������: mpdisk_process_all_partitions()
*	��������: ���������ѹ��صĴ��̷�������fn���д���
*	����:		fn,���ڴ������з����ĺ���ָ��
				(
					fn�Ĳ�������:	
					����:	devname,����"/dev/hda3"
						 	mountpath,����"/hqdata/hda3"	 
					�������: void����ָ��fn_arg,���ڴ����Զ������Ϣ�����õĻ�Ҳ����Ϊ��
				)
*	��������� 	void����ָ��arg,�ᱻֱ�Ӵ���fn_arg�����õĻ�����Ϊ��	
*	����ֵ: 	���е�fn����ֵ֮��
*	�޸���־��
*	��ע:		��������ò��������ôʹ�ã����Բο�mpdisk.c�����غ���,
				����mpdisk_creat_partitions()�ȵ�ʵ��
*************************************************************************/
int  mpdisk_process_all_partitions(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg),IO void *arg )
{
    int i;
    int j;
    char partitionname[100];
    char mount_path[100];
    int result = 0;

    for(i=0;i<get_sys_disk_num();i++)
    {   
        for(j=1;j<=get_sys_partition_num(get_sys_disk_name(i));j++)
        {
            memset(partitionname,0,sizeof(partitionname));
            memset(mount_path,0,sizeof(mount_path));
            get_sys_disk_partition_name(i,j, partitionname);
            partitionname2mountpath(partitionname,mount_path);
            result += (*fn)(partitionname, mount_path,arg);
        }
    }
    return result;
}

//��������������mpdisk_get_emptiest_partition����
//���������Ŀ��д��̱�disk_free����(��λ:MB)
//�ͰѴ��̵������Ϣͨ��disk_free��freest_partition���
//����ֵΪ0��ʾ�ɹ���Ϊ����ʾʧ��
int get_disk_free_fn(IN const char *devname, IN const char* mountpath, IO void * arg)
{
	long partition_free = 0;
	struct diskfree_struct *diskfree;
	
	if((arg== NULL)||(mountpath == NULL))
		return -EINVAL;
	diskfree = (struct diskfree_struct *)arg;
	partition_free = get_disk_free(mountpath);
	if((partition_free > diskfree->freespace_max)&&(get_disk_total(mountpath)>200)) //��ǰ����ķ���������
	{
		diskfree->freespace_max = partition_free;
		sprintf(diskfree->freest_partition, mountpath);
	}
	return 0;
}

/*************************************************************************************
*	��������: mpdisk_get_emptiest_partition
*	��������: ��ȡ����еĴ��̷������ص㼰������
*	����:
*	����� partition name: ����еķ������ص�
*	����ֵ: ����Ŀǰ����еķ�����ʣ��ռ���(MB),��ֵ��ʾ����
*	�޸���־��
************************************************************************************/
long mpdisk_get_emptiest_partition(OUT char* partition_name)
{
	int max_diskfree = 0; //����еķ���ʣ��ռ���
	int i,j;
	struct diskfree_struct diskfree;
	
	if(partition_name == NULL)
		return -EINVAL;
	
		
	diskfree.freest_partition 	= partition_name;
	diskfree.freespace_max 		= max_diskfree;
	mpdisk_process_all_partitions(&get_disk_free_fn, &diskfree);
	return diskfree.freespace_max;
}



/****************************************************************************************
*��������: mpdisk_check_disknode
*��������: ����devinfo�⣬���ϵͳӦ�е�Ӳ�̽ڵ��Ƿ��У����û�У����¼��־
*����:��
*������� 
*����ֵ:  ��
*�޸���־��
***************************************************************************************/
void mpdisk_check_disknode(void)
{
	int i;
	for(i=0;i<get_disk_no();i++)
	{
		if(check_file(get_sys_disk_devname(i))==0)//û��Ӳ�̽ڵ�
		{
			gtlogerr("��%sӲ���豸�ڵ�!!!!!!!!!!\n",get_sys_disk_devname(i));
		}	
	}
}

//����������. ��mpdisk_get_sys_disktotal����
//����:��Ŀǰ�����������ӵ�disktotal�ϣ���λΪM
//����ֵ��0�ɹ�����ֵ������
int get_disk_total_fn(IN const char * devname, IN const char * mountpath, IO void * arg)
{
	int *disktotal;
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	disktotal = (int *)arg;	
	*disktotal += get_disk_total(mountpath);
	return 0;
}



/*************************************************************************
 * 	������:	mpdisk_get_sys_disktotal()
 *	����:	���㵱ǰϵͳ�����˵����з���������������λM
 *	����:	��
 *	���:	��
 * 	����ֵ:	����������λM
 *************************************************************************/
int mpdisk_get_sys_disktotal(void)
{
	int i,j;
	int disktotal=0;
	
	mpdisk_process_all_partitions(&get_disk_total_fn,&disktotal);
	return disktotal;
}


/*************************************************************************
 * 	������:	mpdisk_check_disk_status()
 *	����:	���ÿ����ǰ�е�Ӳ�̷����ڵ��Ƿ���ls�����ݵ���ռ�ÿռ�����
 *	����:	fn,��Ҫִ�еĻص�����
 *	���:	��
 * 	����ֵ:	0��ʾ�ɹ�����ֵ��ʾʧ��
 *************************************************************************/
int mpdisk_check_disk_status(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg))
{
	if(fn == NULL)
		return -EINVAL;	
	return mpdisk_process_all_partitions(fn,NULL);
}



//��mpdisk_creat_partitions���ã�����Ϊÿ����ǰ�е�Ӳ�̷����ڵ㴴�����ص�
//����0��ʾ�ɹ�����ֵ��ʾʧ��
int creat_partitions_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	if((devname ==NULL)||(mountpath == NULL))
		return -EINVAL;
	mkdir(mountpath,0755);
	return 0;
}

/*************************************************************************
 * 	������:	mpdisk_creat_partitionse()
 *	����:	Ϊÿ����ǰ�е�Ӳ�̷����ڵ㴴�����ص�
 *	����:	��
 *	���:	��
 * 	����ֵ:	0��ʾ�ɹ�����ֵ��ʾʧ��
 *************************************************************************/
int mpdisk_creat_partitions(void)
{
	return mpdisk_process_all_partitions(&creat_partitions_fn,NULL);
}


