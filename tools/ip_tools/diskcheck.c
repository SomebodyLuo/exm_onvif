/*���Բ�����Ӳ�̻�cf���ĳ���wsy,july@2006*/
#include <iniparser.h>
#include <commonlib.h>
#include <file_def.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <diskinfo.h>
#include "mpdisk.h"
#include "errno.h"
#include "devinfo.h"

#define  MOUNT_FILE ("/proc/mounts")

#if 0
#define DEVINFO_PARA_FILE   "/conf/devinfo.ini" //����豸�̶���Ϣ���ļ�����Щ��Ϣһ�㲻���޸�
#define HD_DEVICE_NODE      "/dev/hda"          //Ӳ���豸�ڵ�
#define HD_PART_NODE        "/dev/hda1"         //Ӳ�̷����ڵ�

int disk_format(char* disk_name)
{//Ӧ�÷ŵ�����
        int ret;
        long int cap;
        cap = get_sys_disk_capacity(disk_name);
        if (cap<=0)
        {
                printf("�Ҳ���%s IDE �豸\n", disk_name);
                return 1;
        }
        printf("�ҵ�  %s IDE�豸\n", disk_name);
                
        if(cap<200)
        {
                printf("%s capacity = %ld less than 200M \n", disk_name, cap);
                printf("û�м�⵽���õ�CF��!!\n");
                return 1;
        }
        else if((cap>=200)&&(cap<=2000))
        {
                printf("��⵽���õ�CF��\n");
                printf("%s disk capacity = %ldM\n", disk_name, cap);
                ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 65536\n");  //�ٸ�ʽ��
                if(ret!=0)
                {
                        return 2;
                }
        }
        else            // if(cap[i]>2000)
        {
                printf("��⵽���õ�Ӳ��\n");
                printf("%s disk capacity = %ldG\n", disk_name, cap/1000);
                ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 524288\n"); //�ٸ�ʽ��
                if(ret!=0)
                {
                        return 2;
                }
        }
        return 0;
}

int format_disk(char *dev_name)//��ʽ������
{
    int ret;
    if(dev_name == NULL)
        return -EINVAL;
    printf("��ʼ��ʽ��%s�������ʱ��δ��ɣ�˵�������𻵲����޸�\n",dev_name);
    //system("fdisk /dev/hda -f -a");
    //system("mke2fs -j /dev/hda1");
    ret=disk_format();

    return 0;
}

#endif

#define VERSION     "v1.03"
//v1.03 2010-06-25  zw      ��ӶԵ�ǰ�����Ĺ���������жϣ������ظ����ش�ӡ�����ľ�����Ϣ
//v1.02 2009-08-10  wsy     diskinfo��֧��sd��
//v1.01 2009-06-10  wsy     ���Ӱ汾�ţ�����e2fsck·������¼system����ֵ����ʽ��Ӳ�̺�ǿ��Ӳ������

static int formated_disk_flag = 0 ;//��¼�Ƿ��ʽ��������������ʽ����������Ӳ����

void set_cferr_flag(int flag)
{   
    return;
}


int kill_programs_and_umount(char *devname)
{
    char cmd[100];
    printf("��ֹӦ�ó���ж�ش���%s..",devname);
    system("killall -9 tcpsvd 2> /dev/null ");
    system("killall -15 watch_proc 2>/dev/null");
    system("killall -15 hdmodule 2>/dev/null");
    system("killall -15 diskman 2>/dev/null");
    system("killall -9 sqlite3 2>/dev/null");
    system("killall -9 e2fsck 2>/dev/null");
    sleep(1);
    sprintf(cmd,"umount %s*",devname);  
    system(cmd);
    //printf("���.\n\n");
    gtloginfo("ж�ش���,��ֹӦ�ó���\n");
    return 0;
}   

////zw-add--->
/********************************************************
 *������:check_mounts()
 *��  ��:mountpath  ·����
 *��  ��:��
 *����ֵ:�Ѿ����طŻ�1�����򷵻�0
 *��  ע:��
 * ******************************************************/
int check_mounts(IN char *mountpath)
{
    char buff[200];
    char mount_dir[100];
    FILE *fp=NULL;
    char *str=NULL;
    char *p=NULL;
    int i;
    int mount_f=0;

    memset(buff,0,sizeof(buff));
    memset(mount_dir,0,sizeof(mount_dir));
    fp=fopen(MOUNT_FILE,"r");
    if(fp==NULL)
    {
        printf("open [%s] error,exit\n",MOUNT_FILE);
        return -1;
    }

    while(1)
    {
        str=fgets(buff,sizeof(buff),fp);
        if(str==NULL)
        {
                break;
        }
        else
        {
                //printf("������������:%s\n",str);
        }

        p=strstr(str,mountpath);
        if(p!=NULL)
        {
            //printf("�Ѿ�����Ŀ¼:%s\n",p);
            mount_f=1;
        }
    }
    fclose(fp);

    if(mount_f==1)
        return 1;   

    return 0;
}   
////<--zw-add

int test_partition(IN char * devname, IN char * mountpath, IO void * fn_arg)
{
    char diskname[100];
    char cmd[200];
    char testfile[100];
    int total;
    char c;
    int i;
    int ret;
    int fix_time=0;//��¼�������
    struct dirent **namelist;
    
    if((mountpath == NULL)||(devname == NULL))
        return -EINVAL;
        
test:
    //���ж��Ƿ��Ѿ�����
    if(check_mounts(mountpath)==1)
    {
        //printf("����[%s]�ѹ���[%s],ж��\n",devname,mountpath);
        kill_programs_and_umount(mountpath);    
    }
    
    //printf("����[%s]-->[%s]\n",devname,mountpath);    
    //printf("����%s�����Ķ�дscandir����\n",partition_name);
    sprintf(cmd,"mount %s %s",devname,mountpath);
    system(cmd);
    
    
    //���Թ�������
    if(get_disk_total(mountpath)<=1000)//<������С����1G
    {
        printf("%s�������ز�����,��Ҫ����\n",devname);
        gtloginfo("%s�������ز�����,��Ҫ����\n",devname);
        goto fix;
    }
    
    //����Ŀ¼�Ƿ�����
    sprintf(testfile,"%s/indextest",mountpath);
    mkdir(testfile,0755);
    if (access(testfile,F_OK)!=0)
    {
        printf("\n%s�����޷������ļ�����Ҫ����\n",devname);
        gtloginfo("%s�����޷������ļ�����Ҫ����\n",devname);
        goto fix;
    }
    //scandir
    total=scandir(mountpath,&namelist,0,alphasort);
    i=total;
    while(total--)
        free(namelist[total]);
    free(namelist);
    if(i<3)//�����-1˵�����ɹ������<3˵��û���ҵ��ս�����indextest,������
    {
        printf("\n%s����scandirʧ�ܣ���Ҫ����\n",devname);
        gtloginfo("%s����scandirʧ�ܣ���Ҫ����\n",devname);
        remove(testfile);
        goto fix;
    }
    //ɾ��Ŀ¼
    remove(testfile);
    if (access(testfile,F_OK)==0)
    {
        printf("\n%s�����޷�ɾ���ļ�����Ҫ����\n",devname);
        gtloginfo("%s�����޷�ɾ���ļ�����Ҫ����\n",devname);
        goto fix;
    }
    
    printf("%s����ͨ����������!\n",devname);
    gtloginfo("%s����ͨ����������\n",devname);
    return 0;
fix:
    switch(fix_time)
    {
        case(0):printf("��e2fsck�޸�����%s\n",devname);
                gtloginfo("��e2fsck�޸�����%s\n",devname);
                kill_programs_and_umount(devname);
                sprintf(cmd,"e2fsck -y -f %s",devname);
                ret = system(cmd);
                printf("�޸�%s���,���Ϊ%d,���²���\n",devname,ret);
                gtloginfo("�޸�%s���,���Ϊ%d,���²���\n",devname,ret);
                fix_time++;
                goto test;
                
        case(1):printf("%s�����޸���Ч�����Գ��Ը�ʽ���÷�����ʧ����������,ȷ����?(y/N):\n",devname);
                gtloginfo("%s�����޸���Ч�����Գ��Ը�ʽ���÷�����ʧ����������,ȷ����?(y/N):\n",devname);
                scanf("%c",&c); 
                if((c=='y')||(c=='Y'))
                {
                    gtloginfo("�û�ѡ���ʽ��%s,����ʱ������Ӧ˵��ʧ���޷��޸�\n",devname);
                    kill_programs_and_umount(devname);
                    sprintf(cmd,"mke2fs %s -b 4096 -j -L hqdata -m 1 -i 1048576 ",devname);
                    ret = system(cmd);
                    printf("��ʽ��%s��ϣ����Ϊ%d,���²���\n",devname,ret);
                    gtloginfo("��ʽ��%s��ϣ����Ϊ%d,���²���\n",devname,ret);
                    formated_disk_flag = 1;
                    fix_time++;
                    goto test;
                }
                else
                {
                    gtloginfo("�û���������ʽ��%s\n",devname);
                    printf("!!!!!!!!!!%s�����Ĺ������޷��޸�!!!!!!!!\n",devname);
                    gtloginfo("%s�����Ĺ������޷��޸�\n",devname);
                    return 0;
                }
                
        case(2):printf("%s������ʽ��������Ч���Ƿ��ط�������Ӳ����ʧ��������?(y/N):\n",devname);
                gtloginfo("%s������ʽ��������Ч���Ƿ��ط�������Ӳ����ʧ��������?(y/N):\n",devname);
                scanf("%c",&c); 
                if((c=='y')||(c=='Y'))
                {
                    strncpy(diskname,devname,8);
                    diskname[8]='\0';
                    gtloginfo("�û�ѡ���ط���%s,����ʱ������Ӧ˵������ʧ���޷��޸�\n",diskname);
                    kill_programs_and_umount(diskname);
                    sprintf(cmd,"/ip1004/initdisk -d %s -B 1",diskname);
                    ret = system(cmd);
                    printf("�ط���%s���,���Ϊ%d,���²���\n",diskname,ret);
                    gtloginfo("�ط���%s���,���Ϊ%d,���²���\n",diskname,ret);
                    fix_time++;
                    goto test;
                }
                else
                {
                    gtloginfo("�û�����������%s\n",diskname);
                    printf("!!!!!!!!%s�����Ĺ������޷��޸�!!!!!!!!\n",devname);
                    gtloginfo("%s�����Ĺ������޷��޸�\n",devname);
                    return 0;
                }   
            
        case(3): 
                printf("!!!!!!!!%s�����Ĺ������޷��޸�!!!!!!!!!!\n",devname);
                gtloginfo("%s�����Ĺ������޷��޸�\n",devname);
                return 0;
        default: return 0;
    
    
    
    
    
    }
    
    
}

int test_all_partitions(int partition_num)
{
    init_devinfo();
    if(get_devtype()<T_GTVS3021) //gt1k
    {
            return test_partition("/dev/sda1","/hqdata",NULL);
    }
    else    //�������
        return mpdisk_process_all_partitions(test_partition,NULL);
}

int main(void)
{
    int partition_num;
    int diskno,i;

    
    init_devinfo();
    gtopenlog("testcf");
    gtloginfo("##########��ʼִ��%s�����޸�����! version %s##########\n\n",get_devtype_str(),VERSION);
    printf("\n##########��ʼִ��%s�����޸�����! version %s##########\n\n",get_devtype_str(),VERSION);
    printf("ע��!������̹��϶�ִ�б�������ȷ�������Ӳ�����������д��̹���\n\n");
    
    printf("��1��. ���Ӳ�̽ڵ�:");
    diskno = get_sys_disk_num();
    if(diskno==0)
    {
        printf("\n\n����!! û���κ�Ӳ�̽ڵ㣬Ӳ���޷��޸�\n");
        gtloginfo("û��Ӳ�̽ڵ�,Ӳ���޷��޸�\n");
    
        return -1;
    }
    else
    {
        printf("�ҵ�%d��Ӳ�̽ڵ�\n\n",diskno);
        gtloginfo("�ҵ�%d��Ӳ�̽ڵ�\n",diskno);
        
    }
    
    printf("��2��. ����������,����,��������״̬: \n\n");
    for(i=0;i<diskno;i++)
    {
        partition_num = get_sys_disk_partition_num(get_sys_disk_devname(i));
        printf("����%s,��%d��������������%ldG\n",get_sys_disk_devname(i),partition_num,get_sys_disk_capacity(get_sys_disk_name(i))/1000);
        gtloginfo("����%s,��%d��������������%ldG\n",get_sys_disk_devname(i),partition_num,get_sys_disk_capacity(get_sys_disk_name(i))/1000);
        if(partition_num == 0)
        {
            
            printf("�޿��÷���������%s�޷��޸�,������initdisk��ʽ��������\n",get_sys_disk_devname(i));
            gtloginfo("�޿��÷���������%s�޷��޸���������initdisk��ʽ��������\n",get_sys_disk_devname(i));
        }
    }
    
    printf("\n��3��. �����п��÷������ж�д����..\n");
    
    test_all_partitions(partition_num);

    if(formated_disk_flag == 1) //��ʽ���˷����������������ʹ��    
    {
        printf("��ϡ�����豸��ʱ�����ٴα����̴���Ҳ˵���豸���޷�Զ���޸���\n\n");
        printf("\n�����������ʽ���˷�����ִ��Ӳ����..");
        gtloginfo("�����������ʽ���˷�����ִ��Ӳ����\n");
        system("/ip1004/hwrbt");
    }
    else
    {
        printf("\n������������Ӧ�ó���..");
        gtloginfo("����Ӧ�ó���\n");
        system("tcpsvd -vE 0.0.0.0 21 ftpd /hqdata/ &");
        system("/ip1004/watch_proc > /dev/null 2>/dev/null &");
        gtloginfo("������\n");    
    }
    return 0;

}
