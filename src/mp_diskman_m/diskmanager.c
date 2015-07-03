#include <file_def.h>
#include <dirent.h>
#include <ftw.h>
#include <errno.h>
#include <sys/time.h>
#include <pwd.h>
#include "devinfo.h"
#include <mod_cmd.h>
#include "diskmanager.h"
#include <iniparser.h>
#include "process_modcmd.h"
#include "commonlib.h"
#include <sys/time.h>
#include <devinfo.h>
#include "fileindex.h"
#include <sys/types.h>
#include <dirent.h>
#include "gt_errlist.h"
#include "filelib.h"
#include "hdutil.h"
#include "fixdisk.h"
#include "diskinfo.h"
#include "process_modcmd.h"

#define SYSINFO_DUMP_FILE "/var/diskmaninfo.txt" //������Ϣ�ļ�


#define  MAX_PARTITION_NUM   16
static int auto_unlock_time; //���������Զ�������ʱ����,�����ļ�����СʱΪ��λ,�����ṹ������Ϊ��λ
static int hderr_reboot_enable; //�����й����Ƿ������������������ļ���ȡ,Ĭ��Ϊ1

static struct diskman_state_struct diskman_state={0,0,0};
static int fulllock=0;//�Ƿ�log��"���ܼ����ļ�����"
static int diskfull_counter[MAX_PARTITION_NUM]={0}; //�������ȴ��������׼�����
static pthread_t diskman_thread_id=-1;



int get_hd_rmval(void)
{
    if(get_hd_type()==1)
        return HD_RMVAL;
    else
        return CF_RMVAL;
}


DWORD get_diskmanstatint(void)
{
    DWORD stat;
    memcpy((void*)&stat,(void*)&diskman_state,sizeof(DWORD));
    return stat;
}

//��ϵͳ������ӡ��ָ���ļ�
void dump_sysinfo(void) 
{
    //wsy 
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    FILE *dfp;
    char *filename=SYSINFO_DUMP_FILE;
    dfp=fopen(filename,"w");
    if(dfp==NULL)
        return ;
    fprintf(dfp,"\tdiskman system runtime info\n\n");

    if(gettimeofday(&tv,NULL)<0)
    {
        fprintf(dfp,"\t��ȡϵͳʱ��ʱ����\n");
    }
    else
    {
        ctime=tv.tv_sec;
        ptime=localtime(&ctime);
        if(ptime!=NULL)
        {
            fprintf(dfp,"\ttime:%d-%d-%d %d:%d:%d.%03d\n\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000);    
        }
    }

    //��ӡ��ز���
    fprintf(dfp,"diskman_state_struct\n{\n\tcf_err\t\t= %d\n\tdisk_full\t= %d\n}\n",diskman_state.cf_err,diskman_state.disk_full);
    fprintf(dfp,"\nauto_unlock_time\t= %d //���������Զ�������ʱ����\n",auto_unlock_time);
    fprintf(dfp,"\nfulllock\t\t= %d //�Ƿ�log��'���ܼ����ļ�����'\n",fulllock);
    fprintf(dfp,"\ndiskfull_counter[0]\t= %d     //�������ȴ��������׼�����\n",diskfull_counter[0]);
    fprintf(dfp,"\ndiskfull_counter[1]\t= %d     //�������ȴ��������׼�����\n",diskfull_counter[1]);
    fprintf(dfp,"\ndiskfull_counter[2]\t= %d     //�������ȴ��������׼�����\n",diskfull_counter[2]);
    fprintf(dfp,"\ndiskfull_counter[3]\t= %d     //�������ȴ��������׼�����\n",diskfull_counter[3]);
    
    fclose(dfp);
    return;
}

//�����ļ�����Ӧ���ļ�¼��ʱ��(������ʾ)
int get_fname_time(char*filename)
{
    char *p,*t;
    char tmp;
    time_t filetime;
    struct tm ftime;
    if(filename==NULL)
        return -1;
    p=strstr(filename,"_D");
    if(p==NULL)
        return -1;
    p+=2;

    t=p+4;
    tmp=*t;
    *t='\0';
    ftime.tm_year=atoi(p)-1900;
    *t=tmp;

    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_mon=atoi(p)-1;
    *t=tmp;

    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_mday=atoi(p);
    *t=tmp;
    
    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_hour=atoi(p);
    *t=tmp;


    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_min=atoi(p);
    *t=tmp;


    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_sec=atoi(p);
    *t=tmp;

    filetime=mktime(&ftime);
    return filetime;
    
}

void set_cferr_flag(int flag)
{   
    if(get_ide_flag()==0) //û��CF��
        return;
    
    if(diskman_state.cf_err!=flag)
    {
        diskman_state.cf_err=flag;
        send_state2main();
    }
}



void set_disk_full_flag(int flag)
{
    if(get_ide_flag()==0)
        return;
    
    if(diskman_state.disk_full!=flag)
    {
        diskman_state.disk_full=flag;
        send_state2main();
    }
    return ;
}


//��scandir��������ȷ������query������.txt����Ŀ¼���ļ��ṹ
int notxt (const struct dirent *dir) 
{
    if((strstr(dir->d_name,"up")!=NULL)||(strstr(dir->d_name,"index")!=NULL)||(strstr(dir->d_name,"lost")!=NULL)||(strstr(dir->d_name,"iframe")!=NULL))
        return 0;
    else 
        return 1;
}

int disk_full(char *mountpath, int partitionindex) //���������Ҳ���ɾ��ʱ����
{
    set_disk_full_flag(1);

    if(++diskfull_counter[partitionindex] >= (auto_unlock_time/DISK_MAN_LOOP)) //Ӧ����
    {
        diskfull_counter[partitionindex] = 0;
        fileindex_lock_by_time(mountpath,0, -1,-1,-1,-1);
        gtloginfo("������%d��,�Զ����������ļ�\n",auto_unlock_time);
    }
    
    return 0;
}

//�����ļ��������¼�񳤶�
int get_length_file(char *path)
{
    char *lp;
    int result;
    if(path==NULL)
        return -1;
    lp=strstr(path,"_L");
    if(lp==NULL)
        return -1;
    lp=lp+2;
    result= atoi(lp);
    //gtloginfo("����%d,lp%s\n",result,lp);
    return result;  
}



int lockfile_in_partition(IN  char *devname, IN char* mountpath, IO void *fn_arg)
{
    struct lockfile_struct * lockfile;
    
    if((mountpath == NULL)||(fn_arg == NULL))
        return -EINVAL;
        
    lockfile = (struct lockfile_struct *) fn_arg;

    fileindex_lock_by_time(mountpath, lockfile->mode,lockfile->start,lockfile->stop,lockfile->trig,lockfile->ch);
    return 0;
}

int lock_file_by_time(time_t start_time, time_t stop_time,int mode ,int channel)
{
    struct lockfile_struct lockfile;
    
    lockfile.mode   =   mode;
    lockfile.start  =   start_time;
    lockfile.stop   =   stop_time;
    lockfile.trig   =   -1;
    lockfile.ch     =   channel;
    
    return mpdisk_process_all_partitions(lockfile_in_partition,&lockfile);

}

/*************************************************
�������ƣ�usr_lock_file_time
��Ҫ��������ʱ�μӽ����ļ�����ģ��������պ������ã�
          �������Ľṹ����,��ʱ��ת��Ϊ��������ȫ�ֱ���
          ��ͨ����λ��������lock_file_by_time����
���룺    ����·��path,�ӽ����ļ��ṹָ��lockfile
�����    ����ֵΪ���þ���ӽ���������ִ�н��  
�޸���־��wsy@Jan11,2006����
*************************************************/
int usr_lock_file_time(char *path, struct usr_lock_file_time_struct *lockfile)
{
    struct gt_time_struct *start; //��ʼʱ��
    struct gt_time_struct *stop;  //����ʱ��
    struct tm timestart;
    struct tm timestop;
    int i;
    WORD ch;
    int mode;
    int result = 0;
    time_t start_timet,stop_timet;
    char mountpath[32];
    
    if( (path==NULL) || (lockfile==NULL) )
    {
        return -1;
    }
    start = &(lockfile->start);
    stop = &(lockfile->stop);
    ch = (WORD)lockfile->lock_ch;
    mode = lockfile->mode;
    
    gttime2tm(start,&timestart);
    gttime2tm(stop,&timestop);
    start_timet = mktime(&timestart);
    stop_timet = mktime(&timestop);
    
    for(i = 0; i < 4/*MAX_RECORD_CHANNEL*/; i++)
    {
        if((ch & 1<<i) == 1)
        {
            if(start_timet >= stop_timet)
            {
                if(mode==1)
                {
                    gtloginfo("��ʼʱ��>=����ʱ�䣬%dͨ��ȫ��\n",ch);
                }
                else
                {
                    gtloginfo("��ʼʱ��>=����ʱ�䣬%dͨ��ȫ����\n",ch);
                    
                }
                //result = hdutil_lock_all_files(mode);
                sprintf(mountpath,"/hqdata/sda%d",i+1);
                result = fileindex_lock_by_time(mountpath,mode, -1,-1,-1,i);
                
            }   
            else
            {
                //result = lock_file_by_time( start_timet, stop_timet,mode,i);
                sprintf(mountpath,"/hqdata/sda%d",i+1);
                result = fileindex_lock_by_time(mountpath,mode,start_timet, stop_timet,-1,i);
            }
        
        }
    }
    return result;
}

//ɾ�����ϵ�mpg���ļ����ļ�,path:·��,number:һ��ɾ������Ŀ
int remove_oldest_smallfiles(char *path,int number)
{
    
    int total,i,ret,no=0;
    struct dirent **namelist;
    char filename[100];


    
    mkdir(path,0755);
    if((path==NULL)||(access(path,F_OK|R_OK|W_OK|X_OK)!=0)||(isdir(path)!=0))
    {
        gtloginfo("%s�����ڻ��޷�access,������scandir,Ҳ�޷���ɴ����������",path);
        return -1;
    }
  //Acturally,the files are NOT deleted as the sort of the Creatting-Time but based on the order of their names.Whatever,some files will be deleted.
    total=scandir(path,&namelist,0,alphasort);
    if(total==-1)
    {
            gtloginfo("�ļ�ϵͳscandir %s ����,%s",path,strerror(errno));
            return -1;
    }
    for(i=0;i<number;i++)
    {
    
        sprintf(filename,"%s/%s",path,namelist[i+2]->d_name);
        ret=remove(filename);
        if(ret==0)
            no++;
    }
    while(total--)
        free(namelist[total]);
    free(namelist);
    return no;
}



//��ָ��������ɾ�����ϵ��ļ��������������㹻Ϊֹ,��˳��ѿ�Ŀ¼Ҳ������
//���������㹻�򷵻�0�����򷵻ظ�ֵ
int remove_oldest_from_partition(IN char *partition)
{
    int occupied = 0;
    int disk_free;
    int rm_stop;
    
    if(partition == NULL)
        return -EINVAL;
    
    occupied = get_disk_free(partition);
    if(occupied<0)
    {
        gtloginfo("[%s:%d]��ȡ����ʣ��ռ�ʧ��\n",__FILE__,__LINE__,occupied);
        return -1;
    }
    printf("remove_oldest_from_partition,get_disk_free:%d\n",occupied);
   
    if(get_hd_type()==1)//HD
    {
        rm_stop =   HD_RM_STOP;
    }
    else    
    {
        rm_stop =   CF_RM_STOP;
    }
        
    while(1)
    {
        fileindex_del_oldest(partition,10);
        disk_free = get_disk_free(partition);
        printf("del_oldest 10 files, get_disk_free:%d\n",disk_free);
        
        if((occupied == disk_free)||(disk_free >= rm_stop))//ɾ�޿�ɾ��or �Ѿ�ɾ��
            break;
        else
            occupied = disk_free;
    }
    
    //ɾ�����п�Ŀ¼
    ftw_sort(partition, NULL,500,FTW_SORT_ALPHA,1,fix_disk_ftw);//ɾ�����п�Ŀ¼

    if(get_disk_free(partition)>=rm_stop)
        return 0;
    else
        return -EPERM;
}

#if 0
//ɾ��ϵͳ�е������ļ�,
//����Ϊ,���������ļ��ж����ϵķ���;Ȼ�������ϵķ����¸��������ļ�����ɾ��,�������Ŀ¼
int remove_oldest_file()
{   
    int ret;
    char oldest_partition[100];
    int disktotal=0;//��proc��ȡ��ǰӲ�̵�������
    int parttotal=0;//��get_disk_total��ȡ��ǰ���з�����������
    int i;
        
    ret = hdutil_get_oldest_file_partition(oldest_partition); //Ѱ�����ϵĿ�ɾ���ļ����ڷ���
    if(ret!= 0)
    {
        gtloginfo("û���ҵ����ϵķ���,ret = %d\n",ret);
        if(ret == -ENOENT)
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("�����������ʷ�������ʧ��,20�������ϵͳ��ͼ�޸�\n");
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("�����������ʷ�������ʧ��,����̫��Σ���ʱ��������\n");
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("�����������ʷ�������ʧ��,�����ļ�����������\n");
            }
        }
        else
        {
                set_cferr_flag(1);
        }
        return 0;
    }
    printf("[%s:%d]�ҵ������ļ����ڷ���Ϊ[%s]\n",__FILE__,__LINE__,oldest_partition);
    ret = remove_oldest_from_partition(oldest_partition);
    if(ret!= 0) //��Ȼɾ������
    {   
        disk_full(); 
        if(fulllock==0)
        {
            gtloginfo("����%d,��ɨ������Ŀ¼��ɾ�����ռ䲻��(%s��������%d M/��%d M)�����ܼ����ļ�����?\n",ret,oldest_partition,get_disk_free(oldest_partition),get_disk_total(oldest_partition));   
            fulllock=1;
        }
        //����ʱ�Ƿ���Ӳ�̶���ж�ص����
        for(i=0;i<get_sys_disk_num();i++)
        {
            disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));    
        }
        parttotal=mpdisk_get_sys_disktotal();
        
        
        if((disktotal>=1000)&&(parttotal<1000))
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,20���������ͼ�޸�\n",parttotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,����̫��β�������\n",parttotal);
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,�����ļ�����������\n",parttotal);
                
            }
        }
        
    }   
    else
    {
        fulllock=0;
        set_disk_full_flag(0);
    }
    
    return 0;
 }
#endif







//��ʼ�����̹����߳��õ��ı���
int init_diskman(void)
{
    char path[100];
    
    memset((void*)&diskman_state,0,sizeof(diskman_state));

    //���û�������ļ�Ŀ¼���򴴽�
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //���û��ץͼ����Ŀ¼���򴴽�
    sprintf(path,"%s/picindex",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //���û��ץͼ�洢Ŀ¼���򴴽�
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //���û������Ŀ¼���򴴽�
    sprintf(path,"%s/update",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);

    send_state2main();
    return 0;
}



/*written by wsy,2006 June,����/hqdata�µ�Ŀ¼������ÿ��ɾ�����ļ���*/
int get_remove_number(char *dirname)
{
    if(strcmp(dirname,MPGDIR)==0)
        { 
#ifdef SNAP_TO_AVIFILE   
            return RM_MPG_ONCE*2;
#else
            return RM_MPG_ONCE;
#endif
        }
    if(strcmp(dirname,PICINDEXDIR)==0)
            return RM_PICINDEX_ONCE;
    if(strcmp(dirname,INDEXDIR)==0)
            return RM_INDEX_ONCE;
    if(strcmp(dirname,UPDATEDIR)==0)
            return RM_UPDATE_ONCE;
    return 0;
}

/*written by wsy,2006 June,����/hqdata�µ�Ŀ¼���������Ŀ¼�µ�����ļ���*/
int get_filenumber_limit(char *dirname)
{
    if(strcmp(dirname,MPGDIR)==0)
        { 
#ifdef SNAP_TO_AVIFILE   
            return MPGNUMBER*2;
#else
            //return MPGNUMBER; //zw-modified-2011-06-16
            return MPGNUMBER*2;
#endif
        }
    if(strcmp(dirname,PICINDEXDIR)==0)
            return PICINDEXNUMBER;
    if(strcmp(dirname,INDEXDIR)==0)
            return INDEXNUMBER;
    if(strcmp(dirname,UPDATEDIR)==0)
            return UPDATENUMBER;

    return 0;
}

/*written by wsy,2006 June,�������/hqdata�µ�ָ��Ŀ¼*/
void manage_dir(char *dirname)
{
    int number=0;
    int ret=0;
    struct dirent **namelist;


    //�������û�и�Ŀ¼�ʹ�����Ŀ¼
    if(access(dirname,F_OK)!=0)
        mkdir(dirname,0755);
    number=scandir(dirname,&namelist,0,alphasort);
    if(number==-1)
    {
        gtloginfo("�ļ�ϵͳscandir %s ����,%s",dirname,strerror(errno));
        return;
    }
    if((number-2)>get_filenumber_limit(dirname))
    {   
        ret=remove_oldest_smallfiles(dirname,get_remove_number(dirname));
#ifdef SHOW_WORK_INFO
        printf("remove oldest %s %d",dirname,ret);
#endif
    }
    while(number--) 
        free(namelist[number]);
    free(namelist);
    return;
}

//��check_disk_status_fn���ã��������Ӳ��df��du������������
//����0��ʾ�ɹ�����ֵ��ʾʧ��
int check_disk_status_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
    int disktotal,diskfree;
    int dirtotal;
    struct dirent **namelist;

    disktotal   = get_disk_total(mountpath);
    diskfree    = get_disk_free(mountpath);
    
    if(disktotal-diskfree > 1024)   //ռ�ó���1G
    {
        dirtotal=scandir(mountpath,&namelist,0,alphasort);
        if(dirtotal == 0)
        {
                      //��scandir()����ʱ�Ĵ���,��ǰ�Ĵ���������TODO...
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("%s����ռ��%dM/%dM,����Ŀ¼�ڵ�,20���������ͼ�޸�\n",mountpath,disktotal-diskfree,disktotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {   
                    while(dirtotal--)
                        free(namelist[dirtotal]);
                    free(namelist);
                    set_cferr_flag(1);
                    printf("%s����ռ��%dM/%dM,����Ŀ¼�ڵ�,������̫��β�������\n",mountpath,disktotal-diskfree,disktotal);
                }
            }
            else
            {
                while(dirtotal--)
                            free(namelist[dirtotal]);
                free(namelist);
                set_cferr_flag(1);
                gtloginfo("%s����ռ��%dM/%dM,����Ŀ¼�ڵ�,�����ļ�����������\n",mountpath,disktotal-diskfree,disktotal);
            }
        }
        else
        {
            while(dirtotal--)
                free(namelist[dirtotal]);
            free(namelist);
        }
    }
    return 0;

}



//���������̵��߳�
static void *diskman_thread(void *para)
{
    long freespace;
    struct passwd *user;
    int loguid=-1;
    int ret;
    long disktotal = 0;

    gtloginfo("start diskman_thread...\n");
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);     //�����߳�Ϊ��ȡ����
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    user=getpwnam("gtlog");
    if(user!=NULL)
        loguid=user->pw_uid;

    if(access("/hqdata/update",R_OK|W_OK|F_OK)!=0)
        mkdir("/hqdata/update",0755);

    while(1)
    {
              //����4�н��Ը��Ե�Ŀ¼����ά������������ļ��Ƿ���ڣ��ļ������Ƿ񳬳�����ֵ��������ɾ��
        //�������hqdata/iframe�µ�mpg��avi�ļ�
        manage_dir(MPGDIR);
        //�������/hqdata/picindex�µ�txt�ļ�
        manage_dir(PICINDEXDIR);
        //�������/hqdata/update�µ�������¼
        manage_dir(UPDATEDIR);
        //�������/hqdata/index�µ�txt�ļ�
        manage_dir(INDEXDIR);   
    
              //��ʵ���Ƕ�ÿ������ʹ��scandir()�鿴һ�£���scandir()�Ƿ�������������˾͸������ò��������豸    
        mpdisk_check_disk_status(&check_disk_status_fn);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int i;
        int j;
        char partitionname[100];
        char mount_path[100];
        int parttotal;

        //����ʱ�Ƿ���Ӳ�̶���ж�ص����
        for(i=0;i<get_sys_disk_num();i++)
        {
            disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));    
        }
        parttotal=mpdisk_get_sys_disktotal();

        if((disktotal>=1000)&&(parttotal<1000))
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,20���������ͼ�޸�\n",parttotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,����̫��β�������\n",parttotal);
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("����Ӳ�̷����ѱ�ж��,���й��ص�ռ�%dM,�����ļ�����������\n",parttotal);
                
            }
        }
        
 
        for(j=1;j<=get_sys_partition_num(get_sys_disk_name(0));j++)
        {
            memset(partitionname,0,sizeof(partitionname));
            memset(mount_path,0,sizeof(mount_path));
            get_sys_disk_partition_name(0,j, partitionname);
            partitionname2mountpath(partitionname,mount_path);

           long partition_free= 0;
	    partition_free = get_disk_free(mount_path);


            if(partition_free < get_hd_rmval()) //���̿ռ䲻��
            {   
                ret = remove_oldest_from_partition(mount_path);
                if(ret!= 0) //��Ȼɾ������
                {   
                    disk_full(mount_path, j); 
                    if(fulllock==0)
                    {
                        gtloginfo("����%d,��ɨ������Ŀ¼��ɾ�����ռ䲻��(%s��������%d M/��%d M)�����ܼ����ļ�����?\n",ret,mount_path,get_disk_free(mount_path),get_disk_total(mount_path));   
                        fulllock=1;
                    }
                }   
                else
                {
                    fulllock=0;
                    set_disk_full_flag(0);
                }
                     
            }
            else 
            {
                set_disk_full_flag(0);
                diskfull_counter[j]=0;
            }
        }


        sleep(DISK_MAN_LOOP); //cancelation

    }

    return NULL;
}

int creat_diskman_thread(pthread_attr_t *attr,void *arg)
{
    return pthread_create(&diskman_thread_id,attr, diskman_thread, arg);//�������̹����߳�
}

int read_diskman_para_file(char *filename)
{

    int val;
    dictionary      *ini;
    
    if(filename==NULL)
        return -1;
    
    ini=iniparser_load(filename);
    if(ini==NULL)
    {
        printf("diskman  cannot parse ini file file [%s]", filename);
        gtloginfo("�������ļ�%s��load iniʧ��,����-1\n",filename);
        return -1 ;
    }

    val=iniparser_getint(ini,"product:auto_unlock_time",24);
    val*=3600;
    auto_unlock_time=val;


    hderr_reboot_enable = iniparser_getint(ini,"product:hderr_reboot_enable",1);

    
    iniparser_freedict(ini);
    return 0;
}
//#endif

