#include "mp_hdmodule.h"
#include "mpdisk.h"
#include <gt_errlist.h>
#include <math.h>
#include "hdutil.h"
#include "hdmod.h"
#include "process_modcmd.h"
#include <commonlib.h>
#include <devinfo.h>
#include <venc_read.h>
#include <avilib.h>
#include <aenc_read.h>
#include "audiofmt.h"
#include "filelib.h"
#include "diskinfo.h"
#include "fixdisk.h"
#include "ftw.h"
#include "fileindex.h"

////zw-test
#include <sys/time.h>
#include <unistd.h>
////zw-test


#ifdef RECORD_PS_FILE
#include "669/es2ps.h"
#endif 

#ifdef USE_FFMPEG_LIB
    //��ȡ��Ƶ��ʽ
static int audio_save_fmt=1;                        ///��Ƶ�洢��ʽ 1:u-pcm 3:raw-pcm 4:mpeg2
#endif
//static int CFflag=1;//�Ƿ���CF��
static struct hd_enc_struct g_hdenc[MAX_HQCHANNEL];   //��0·������¼��ṹ
static struct hdmod_state_struct hdmod_state={0,0,0};//,0,0,0};

static int hd_playback_flag=0;

int get_hd_minval(void)
{
    if(get_hd_type()==1)
        return HD_MINVAL;
    else
        return CF_MINVAL;
}

/**********************************************************************************************
 * ������   :disk_get_record_partition()
 * ���� :��ȡ¼�����
 * ���� :��
  * ��� :partition_name:��ǰ¼���Ӳ�̷�����
 * ����ֵ   :0��ʾ��������ֵ��ʾ����
 * ע       :Ӧ�ó����ڸ�������ʱ����Ҫ�������������/conf/diskinfo.ini�ж�ȡϵͳ��Ϣ
 *          ���/conf/diskinfo.ini�����ڣ�����豸��Ϣ���óɳ�ʼֵ��������-1
  **********************************************************************************************/
int disk_get_record_partition(struct hd_enc_struct *phdenc)
{

    dictionary    *ini=NULL;
    char            *pstr=NULL;
    FILE            *fp=NULL;
    
    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("disk_get_record_partition() cannot parse ini file file [%s]", DISK_INI_FILE);
          gtlogerr("disk_get_record_partition() cannot parse ini file file [%s]", DISK_INI_FILE);
          return -1 ;
    }


    //��ǰ¼����
    pstr = iniparser_getstring(ini,"diskinfo:record_disk",NULL);
    if(pstr != NULL)
    {
        if((strlen(pstr) == 4) && (pstr[0] == 's') && (pstr[1] == 'd'))//������sdXX,��sda1
        {
            strcpy(phdenc->partition,"/hqdata/");
            strncat(phdenc->partition,pstr,strlen("sda1"));   
        }
        else
        {
            gtlogerr("get record_disk error:%s,len:%d , set to sda1\n",pstr,strlen(pstr));
            strcpy(phdenc->partition,"/hqdata/sda1");
        }
    }
    else
    {
        gtloginfo("no record_disk, set to sda1\n");
        strcpy(phdenc->partition,"/hqdata/sda1");
    }
    printf("record disk =%s\n",phdenc->partition);
  
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   

    iniparser_freedict(ini);
    
    return 0;
    
}

/**********************************************************************************************
 * ������	:is_keyframe()
 * ����	:�ж�һ֡�����Ƿ��ǹؼ�֡,���������ļ��и�ʱ���õ�
 * ����	:frame ��Ҫ���жϵ�����֡
 * ����ֵ	:0 ��ʾ�ǹؼ���
 *			 1��ʾ�ǹؼ���
 **********************************************************************************************/
int is_keyframe(struct stream_fmt_struct *frame)
{
	if((frame->media==MEDIA_VIDEO)&&(frame->type==FRAMETYPE_I))
		return 1;
	return 0;
}


struct pool_head_struct *get_stream_pool(int channel)
{
        return &(g_hdenc[channel].streampool);
}
struct hdmod_state_struct *get_hdmod_stat(void)
{
    return &hdmod_state;
}
static int old_cferr_state=0;

void set_cferr_flag(int flag)
{   

    if(get_ide_flag()==0)//û�д洢
        return;
    
    if(old_cferr_state!=flag)
    {
        //gtloginfo("test,flag is %d,old is %d\n",flag,old_cferr_state);
        hdmod_state.cf_err=flag;
        send_state2main();
        old_cferr_state=flag;
    }
}



//���ļ���Ϣת������Ӧ���ļ���
int finfo2filename(struct file_info_struct *info,char *filename)//wsy,���б�־��length���зֳ���
{
    char *name;
    struct tm *ptime;
    if((info==NULL)||(filename==NULL))  
        return -1;
    ptime=localtime(&info->stime);
    if(ptime==NULL)
        return -1;  
    name=filename;
    
    
    if(info->remote==1)
    {
        //sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%c%s",
        //wsy changed to support multi-partition
        sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%c%s",
        info->partition,
        (1900+ptime->tm_year),
        ptime->tm_mon+1,
        ptime->tm_mday,
        ptime->tm_hour,
        info->channel,
        (1900+ptime->tm_year),
        ptime->tm_mon+1,
        ptime->tm_mday,
        ptime->tm_hour,
        ptime->tm_min,
        ptime->tm_sec,
        info->len,
        info->trig,
        REMOTE_TRIG_FLAG,
        LOCK_FILE_FLAG,
        IMG_FILE_EXT);
    }
    else
    {
        if(info->trig)
            {
                sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%s",
                info->partition,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                info->channel,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                ptime->tm_min,
                ptime->tm_sec,
                info->len,
                info->trig,
                LOCK_FILE_FLAG,
                IMG_FILE_EXT);
            }
        else
            {
                sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%s",
                info->partition,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                info->channel,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                ptime->tm_min,
                ptime->tm_sec,
                info->len,
                info->trig,
                IMG_FILE_EXT);
            }
    }
    
    return 0;   
    
}


/**********************************************************************************************
 * ������   :disk_get_next_partition(char* partition_name)
 * ���� :��ȡ��һ������
 * ���� :partition_name:��ǰ������������/hqdata/sda1
  * ��� :partition_name:��һ��������
 * ����ֵ   :0��ʾ��������ֵ��ʾ����
  **********************************************************************************************/
int disk_get_next_partition(char* partition_name)
{

    char        diskno; /*Ӳ�̺�*/
    int           partitionnamelen;
    int           disknum;
    char        partitionno;/*������*/

    partitionnamelen = strlen(partition_name);
    if(partitionnamelen < strlen("/hqdata/sda1"))
    {
          printf("disk_get_next_partition() partition_name:%s error\n",partition_name);
          gtlogerr("disk_get_next_partition() partition_name:%s error",partition_name);
          return -1;
     }
        

    disknum = get_sys_disk_num();

    /*Ӳ�̺�sda....sdd*/
    diskno = partition_name[10];  

    /*������sda1....sda3....sdb1...sdb4.....*/
    partitionno = partition_name[11];

    partitionno++;
    if(partitionno > '4')
    {
        partitionno = '1';
        diskno++;
        if(diskno -'a' >=  disknum)
        {
           diskno = 'a';
        }
    }

    /*�޸ķ���*/    
    /*Ӳ�̺�sda....sdd*/
    partition_name[10] = diskno;  

    /*������sda1....sda3....sdb1...sdb4.....*/
    partition_name[11] = partitionno;
    

    return 0;    

      
}
/**********************************************************************************************
 * ������   :disk_get_next_record_partition(char* partition_name)
 * ���� :��ȡ��һ��¼�����
 * ���� :partition_name:��ǰ¼�����
 * ��� :partition_name:��һ��¼�������
 * ����ֵ   :0��ʾ��������ֵ��ʾ����
  **********************************************************************************************/
int disk_get_next_record_partition(char* partition_name)
{

    dictionary    *ini=NULL;
    char            *pstr=NULL;
    FILE            *fp=NULL;
    int               seachpartition = 0;/*���ҵķ�����*/
    int               disknum;


    gtloginfo("%s disk free is %dM,chang to next disk\n",partition_name,get_disk_free(partition_name));
    
    disknum = get_sys_disk_num();
    while(get_disk_free(partition_name) < get_hd_minval())
    {

        //printf("current disk:%s\n",partition_name);
        disk_get_next_partition(partition_name);
        //printf("current disk:%s\n",partition_name);

        seachpartition++;
        /*����������disknum*4�������ж��Ǳ��������еķ���*/
        if(seachpartition >= disknum*4)
        {
            printf("disk is full,record disk is not find,seach partition:%d\n",seachpartition);
            gtlogerr("disk is full,record disk is not find,seach partition:%d\n",seachpartition);
            return -1;
        }
        
    }

    gtloginfo("current disk is full ,chang to next disk:%s,disk free:%dM\n",partition_name,get_disk_free(partition_name) );
    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("init_devinfo() cannot parse ini file file [%s]\n", DISK_INI_FILE);
          gtlogerr("init_devinfo() cannot parse ini file file [%s]", DISK_INI_FILE);
          return -1 ;
    }

    //��ǰ¼����
    pstr=strstr(partition_name,"sd");
    iniparser_setstr(ini, "diskinfo:record_disk", pstr);
    save_inidict_file(DISK_INI_FILE,ini,&fp);
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   

    iniparser_freedict(ini);
    return 0;
    
}



//��ȡ������ʾ��hdmod״̬
DWORD get_hdmodstatint(void)
{
    DWORD stat;
    memcpy((void*)&stat,(void*)&hdmod_state,sizeof(DWORD));
    return stat;
}
//removed by shixin static int hdsave_mode[HQCHANNEL]={1};//������¼����ģʽ 1��ʾ��Ҫ�������� 0��ʾ��Ҫֹͣ
//����ָ��ͨ��Ӧ�еĹ���ģʽ
// 1��ʾ��Ҫ�������� 0��ʾ��Ҫֹͣ
// �ı乤��״̬�Ĺ�������������ֹͣ������¼���̵߳�ʱ������
int get_hqsave_mode(int ch)
{//changed by shixin
    struct hd_enc_struct    *hd=NULL;
    hd=get_hdch(ch);
    if(hd==NULL)
        return 0;
    return hd->hdsave_mode;
}



int convert_old_ing_files_fn(IN char * devname, IN char * mountpath, IO void *arg)
{   
    if(mountpath == NULL)
        return -EINVAL;
    printf("[%s:%d],path:%s\n",__FILE__,__LINE__,mountpath);    
    return  fileindex_convert_ing(mountpath);
}



int convert_old_ing_files(void)
{
  //��ÿ����������convert_old_ing_files_fn(),��ÿ��������ת��
    return mpdisk_process_all_partitions(convert_old_ing_files_fn,NULL);
}



/*
    ��������  dump_clearinfo_to_log
    ��������: ���յ���clear_hdmod_trig_flag��Ϣ��ӡ��ָ���ļ�
    ��дʱ��: wsy@Jan.2006
*/
void dump_clearinfo_to_log(void)
{
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    struct stat buf;
    FILE *dfp;
    
    char *filename = ALARMLOG_FILE;

    //�ж��ļ��Ƿ����
    stat(ALARMLOG_FILE,&buf);
    if( (buf.st_size>>10) >= ALARMLOGFILE_MAX_SIZE )    //����һ������
    {
        remove(ALARMLOG_FILE_0);
        rename(ALARMLOG_FILE,ALARMLOG_FILE_0);
    }
    
    dfp = fopen(filename,"a+");
    if(dfp == NULL)
        return ;
    //дʱ��
    if(gettimeofday(&tv,NULL) < 0)
    {
        fprintf(dfp,"<��ȡϵͳʱ��ʱ���� >   ");
    }
    else
    {
        ctime = tv.tv_sec;
        ptime = localtime(&ctime);
        if(ptime != NULL)
            fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d>   ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);    
    }
    gtloginfo("���trig��־׼��д��Ϣ\n");
    //д��Ϣ
    fprintf(dfp,"[CLEAR]            \n");
    fclose(dfp);

    
}

/*
    ��������  dump_hqsaveinfo_to_log
    ��������: ���յ���hqsave/hqstop��Ϣ��ӡ��ָ���ļ�,mode=1Ϊsave
    ��дʱ��: wsy@Jan.2006
*/
void dump_saveinfo_to_log(int mode,int ch,int time)
{
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    struct stat buf;
    FILE *dfp;
    
    char *filename = ALARMLOG_FILE;

    //�ж��ļ��Ƿ����
    stat(ALARMLOG_FILE,&buf);
    if( (buf.st_size>>10) >= ALARMLOGFILE_MAX_SIZE )    //����һ������
    {
        remove(ALARMLOG_FILE_0);
        rename(ALARMLOG_FILE,ALARMLOG_FILE_0);
    }
    
    dfp = fopen(filename,"a+");
    if(dfp == NULL)
        return ;
    //дʱ��
    if(gettimeofday(&tv,NULL) < 0)
    {
        fprintf(dfp,"<��ȡϵͳʱ��ʱ���� >   ");
    }
    else
    {
        ctime = tv.tv_sec;
        ptime = localtime(&ctime);
        if(ptime != NULL)
            fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d>   ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);    
    }
    //д��Ϣ
    if(mode==1)
        fprintf(dfp,"[SAVE]  %05d��    \n",time);
    if(mode==0)
        fprintf(dfp,"[STOP]             \n");
    fclose(dfp);
}

/*
    ������:   check_alarmlog_for_trigflag
    ��������: ����ʱ����,���alarmlog�е�clear��alarm��Ŀ
    ����ֵ:   trigflag,��Ϊ0��ʾ�ѱ�clear,��Ϊ1��ʾ���һ���Ǳ���
              alarm_log_struct�ṹ  
    ��дʱ��: wsy@Jan.2006
*/
int check_alarmlog_for_trigflag(struct alarm_log_struct *log)
{
    FILE *stream;
    int i = 0;
    int trigflag = 0; 
    int row = 1;
    
    //���ļ��������ػ�������
    stream = fopen(ALARMLOG_FILE,"r");

    if(stream == NULL)
        return 0;
    
    while(i >= 0) //ֱ����������������Ϊֹ
    {
        i = fseek(stream,-row*sizeof(struct alarm_log_struct),SEEK_END);
        fread(log,sizeof(struct alarm_log_struct),1,stream);
        if(strncmp(log->type,"[CLEAR]",7) == 0) //��һ����CLEAR
        {
            break;
        }
        if(strncmp(log->type,"[ALARM]",7) == 0) //��һ����ALARM
        {
            trigflag = 1;
            break;
        }
        row++;
    }

    fclose(stream);
    return trigflag; 
}





int check_alarmlog_for_hqsaveflag(struct alarm_log_struct *log)
{
    FILE *stream;
    int i=0;
    int row=1;
    int saveflag=0; //����δstop��save����Ϊ1
    
    //���ļ��������ػ�������
    stream=fopen(ALARMLOG_FILE,"r");

    if(stream==NULL)
        return 0;
    while(i>=0) //ֱ����������������Ϊֹ
    {
        i=fseek(stream,-row*sizeof(struct alarm_log_struct),SEEK_END);
        fread(log,sizeof(struct alarm_log_struct),1,stream);
        if(strncmp(log->type,"[STOP] ",7)==0) //��һ����STOP
        {
            break;
        }
        if(strncmp(log->type,"[CLEAR]",7)==0) //��һ����CLEAR
        {
            break;
        }
        if(strncmp(log->type,"[SAVE] ",7)==0) //��һ����SAVE
        {
            saveflag=1;
            break;
        }
        row++;
    }

    fclose(stream);
    return saveflag; 
}

    
/*
    ��������:��alarm_log�ṹ�м������¼ָ���ʱ��
*/
int get_record_time(struct alarm_log_struct *info)
{
    char timeinfo[24];
    char *lp,*lk;

    struct tm timetrig; //����ʱ���tm�ṹ
    
    if(info==NULL)
        return -1;
        
    strncpy(timeinfo,info->time,23);
    timeinfo[23]='\0';
    //printf("time is %s\n",time);

    //�������
    lp=index(timeinfo,'<');
    lp++;
    timetrig.tm_year=atoi(lp)-1900;
    //printf("year is %04d\n",timetrig.tm_year);

    //�����·�
    lk=index(lp,'-');
    lk++;
    timetrig.tm_mon=atoi(lk)-1;
    //printf("month is %02d\n",timetrig.tm_mon);

    //��������
    lp=index(lk,'-');
    lp++;
    timetrig.tm_mday=atoi(lp);
    //printf("date is %02d\n",timetrig.tm_mday);

    //����Сʱ���
    lk=index(lp,' ');
    lk++;
    timetrig.tm_hour=atoi(lk);
    //printf("hour is %02d\n",timetrig.tm_hour);

    //������Ӳ��
    lp=index(lk,':');
    lp++;
    timetrig.tm_min=atoi(lp);
    //printf("min is %02d\n",min);

    //��������
    lk=index(lp,':');
    lk++;
    timetrig.tm_sec=atoi(lk);
    //printf("sec is %02d\n",sec);

    return mktime(&timetrig);

}
#if 0
//���ַ���ת��Ϊ16��������
DWORD atohex(char *buffer)
{
    int i,len;
    DWORD hex=0;
    char *p;
    DWORD ret;
    char ch;
    char buf[12];
    if(buffer==NULL)
        return 0;
    memcpy(buf,buffer,sizeof(buf));
    p=strrchr(buf,'x');//�ҵ����һ��x��λ��
    if(p==NULL)
    {
        p=strrchr(buf,'X');
    }   
    if(p==NULL)
        p=buf;
    else
        p++;
    len=strlen(p);

    if(len>8)
    {
        i=len-8;
        p+=i;
        len=strlen(p);  
    }
    p+=len;
    p--;
    for(i=0;i<len;i++)
    {
        ch=(char)toupper((int)*p);
        *p=ch;
        if(isdigit(*p))
        {
            ret=*p-'0';
        }
        else //��ĸ
        {
            //if(!isupper(*p))
            *p=(char)toupper((int)*p);
            ret=*p-'A'+10;
        }
        hex|=ret<<(i*4);
        p--;
    }
    return hex; 
    
}
#endif



//
int get_hqsave_time(struct alarm_log_struct *info)
{
    char data[12];
    int time;
    
    if(info==NULL)
        return -1;

    strncpy(data,info->data,11);
    data[11] = '\0';
    time=(atoi(data));
    //gtloginfo("time is %d����\n",time);
    return time;
}



/*
    ��������: get_record_ch
    ��������: ��alarm_log�ṹ�м��������Ҫ��¼�Ĵ���
    ��������: ��alarmlog�ж��������һ��alarm_log�ṹ
    ����ֵ:   ����״̬
    ��дʱ��: wsy@Jan.2006
*/
int get_record_ch(struct alarm_log_struct *info)
{
    char data[12];
    DWORD trig;

    if(info==NULL)
        return -1;

    strncpy(data,info->data,11);
    data[11] = '\0';
    trig=(DWORD)(atohex(data));
    //loginfo("ch is 0x%04x����\n",(int)trig);
    return trig;
}


/*
    ��������: get_trig_status
    ��������: ��ʼ������ã����alarmlog���Ƿ���Ҫ��¼����¼�񲢽�����Ӧ����
    ��������: ��
    ����ֵ:   ��
    ˵��:��Ϊ3022�ı���ֻ�������豸0�У���ֱ�Ӷ�Ӧ��Ƶͨ��0
    ��дʱ��: wsy@Jan.2006
*/
void get_trig_status(void)
{
    struct alarm_log_struct log;
    struct hd_enc_struct *hd_enc;
    int trig=0;
    int rec_len=0; //�����ľ���ʱ��
    int trig_ch=0;
    time_t timenow; 
    
    trig=check_alarmlog_for_trigflag(&log);
    if(trig!=0)
    {
        //ȡ��ʱ��
        rec_len=get_record_time(&log);
        hd_enc=get_hdch(0);
        if(hd_enc==NULL)
        return;

        timenow=time((time_t *)NULL);
        if(rec_len>timenow)//�������ʱ�������ʱ�仹���򷵻�
            return;//����Ϊ�˱�����ֵ�ع��������֢״����
            
        //gtloginfo("reclen+hd_enc->dly_rec-timenow=%d\n",(int)(rec_len+hd_enc->dly_rec-timenow));
        if(rec_len+hd_enc->dly_rec>timenow) //����Ҫ����¼�ͼ�����������trig
        {
            trig_ch=get_record_ch(&log);
            hd_enc=get_hdch(0);
            hd_enc->state=2;
            hd_enc->trig=trig_ch;
            hd_enc->recordlen=rec_len+hd_enc->dly_rec-timenow;
            gtloginfo("����ǰ0x%04x����¼��Ҫ��¼%d��\n",trig_ch,(int)(rec_len+hd_enc->dly_rec-timenow));
            return;
        }
    } 
    
    
    return;
}


/*
    ��������: get_save_status
    ��������: ��ʼ������ã����alarmlog���Ƿ���Ҫ��¼�ֶ�¼�񲢽�����Ӧ����
    ��������: ��
    ����ֵ:   ��
    ˵��:��Ϊ3022�ı���ֻ�������豸0�У���ֱ�Ӷ�Ӧ��Ƶͨ��0
    ��дʱ��: wsy@Jan.2006
*/
void get_save_status(void)
{
    struct alarm_log_struct log;
    struct hd_enc_struct *hd_enc;
    int save=0;
    int rec_len=0; 
    int rec_time=0;
    time_t timenow;
    
    save=check_alarmlog_for_hqsaveflag(&log);
    if(save!=0)
    {
        hd_enc=get_hdch(0);
        if(hd_enc==NULL)
        return;

        timenow=time((time_t *)NULL);

        
        //�Ƚ�ʱ��+¼��ʱ�������ʱ��,���������Ҫ¼��ʱ��
        rec_len=get_record_time(&log);
        if(rec_len>timenow)//�������ʱ�������ʱ�仹���򷵻�
            return;//����Ϊ�˱�����ֵ�ع��������֢״����
        rec_time=get_hqsave_time(&log);
        if(rec_len+rec_time>timenow)    //����Ҫ����¼�ͼ�����������CH
        {
            
            hd_enc=get_hdch(0);
            hd_enc->state=2;
            hd_enc->remote_trigged=1;
            hd_enc->remote_trig_time=rec_time+rec_len-timenow;
            gtloginfo("����ǰ�ֶ�¼��Ҫ��¼%d��\n",(int)(rec_time+rec_len-timenow));
            return;
        }
    } 
    return;
}







//���ָ��ͨ����trig��־
int clear_hdmod_trig_flag(int channel)
{

    struct hd_enc_struct *hd;
    
    gtloginfo("���%dͨ��trig��־\n",channel);
    
    hd=get_hdch(channel);
    
    hd->trig=0;
    hd->recordlen=0;
    if(hd->remote_trigged==0)
    {
        if((hd->rec_type==0)&&(hd->pre_rec!=0))
            hd->state=1;
        else
            hd->state=0;
    }
	else
		hd->cutflag=1;
    return 0;
}


//���ļ��������ֹ�¼���־
int rtflag_filename(char *filename,char* tname)
{
    char *p;
    
    if((filename==NULL)||(tname==NULL))
        return -1;
    
    strncpy(tname,filename,strlen(filename)+1);
    p=index(tname,REMOTE_TRIG_FLAG);
    if(p!=NULL)
    {
        return 0;
        //�Ѽӱ�־�����ټ�  
    }   
    p=index(tname,LOCK_FILE_FLAG);
    if(p!=NULL)
    {
        //�ļ��Ѿ�����
        sprintf(p,"%c%c%s",REMOTE_TRIG_FLAG,LOCK_FILE_FLAG,IMG_FILE_EXT);
        return 0;
    }
    p=strstr(tname,IMG_FILE_EXT);
    if(p==NULL)
    {
        //����¼����ļ����ܱ��ӱ�־
        return 0;
    }
    sprintf(p,"%c%s",REMOTE_TRIG_FLAG,IMG_FILE_EXT);
    return 0;   
}



int set_msgmnb_value(int queueid,int msgnum)
{

    struct msqid_ds buf;
    

    if (msgnum<115)
        return 0;
    
    if(msgctl(queueid,IPC_STAT,&buf)<0)
    {
        gtloginfo("�޷�ȡ����Ϣ���еĽṹ\n");
        return -1;
    }
    //gtloginfo("test,buf.msg_qbytes=%ld\n",buf.msg_qbytes);
    buf.msg_qbytes=msgnum*128;
    
    if(msgctl(queueid,IPC_SET,&buf)<0)
    {
        gtloginfo("�޷�������Ϣ���еĳ���\n");
        return -1;
    }
    gtloginfo("��Ϣ���г�������Ϊ%ld\n",buf.msg_qbytes);
    
    return 0;
    

}


//�������ļ��ж�ȡ������¼����ز���
int read_hqsave_para_file(char *filename,char *section, int channel)
{
    char parastr[100];
    dictionary      *ini;   
    char *cat;
    int section_len;
    int msgnum=0;
    char sec[100];
    struct hd_enc_struct *hdenc = get_hdch(channel);
    FILE            *fp=NULL;

    if((filename==NULL)||(section==NULL)||(hdenc == NULL))
        return -1;
    
    section_len=strlen(section);
    if(section_len>30)
        return -1;
    ini=iniparser_load_lockfile(filename,1,&fp);
    if (ini==NULL) 
    {
        printf("hdsave  cannot parse ini file file [%s]", filename);
        gtloginfo("�������ļ�%s��load iniʧ��,����-1\n",filename);
        return -1 ;
    }
    memcpy(parastr,section,section_len);
    parastr[section_len]=':';
    section_len++;
    parastr[section_len]='\0';
    cat=strncat(parastr,"pre_rec",strlen("pre_rec"));   
    hdenc->pre_rec=iniparser_getint(ini,parastr,600);
    if(hdenc->pre_rec>3600)
    {
        gtloginfo("���õ�Ԥ¼ʱ��%d���������������ȵ���Ϊ3600��\n",hdenc->pre_rec);
        hdenc->pre_rec=3600;
    }
    
    parastr[section_len]='\0';
    cat=strncat(parastr,"dly_rec",strlen("dly_rec"));   
    hdenc->dly_rec=iniparser_getint(ini,parastr,30);

    parastr[section_len]='\0';
    cat=strncat(parastr,"cut_len",strlen("cut_len"));   
    hdenc->max_len=iniparser_getint(ini,parastr,30);
    if(hdenc->max_len<90)
    {
        gtloginfo("���õ��з�ʱ��%d����,����Ϊ90��\n",hdenc->max_len);
        hdenc->max_len=90;
    }
    if(hdenc->max_len>1800)
    {
        gtloginfo("���õ��з�ʱ�����,����Ϊ1800��\n",hdenc->max_len);
        hdenc->max_len=1800;
    }
    

    parastr[section_len]='\0';
    cat=strncat(parastr,"del_typ",strlen("del_typ"));   
    hdenc->del_typ=iniparser_getint(ini,parastr,0);

    parastr[section_len]='\0';
    cat=strncat(parastr,"rec_type",strlen("rec_type")); 
    hdenc->rec_type=iniparser_getint(ini,parastr,0);

#if 0
    parastr[section_len]='\0';
    cat=strncat(parastr,"a_channel",strlen("a_channel")); 
    hdenc->audiochannel = iniparser_getint(ini,parastr,0);
    if((hdenc->audiochannel < 0)||(hdenc->audiochannel >= MAX_AUDIO_ENCODER))
    {
        gtloginfo("��Ƶ�˿���Ч%d������Ϊ0�˿�\n",hdenc->audiochannel);
        hdenc->audiochannel = 1;
    }
#endif    
#if 0
    //��ǰ¼����
    cat=iniparser_getstring(ini,"diskinfo:record_disk",NULL);
    if(cat != NULL)
    {
        if((strlen(cat) == 4) && (cat[0] == 's') && (cat[1] == 'd'))//������sdXX,��sda1
        {
            strcpy(hdenc->partition,"/hqdata/");
            strncat(hdenc->partition,cat,strlen("sda1"));   
        }
        else
        {
            gtlogerr("get record_disk error:%s,len:%d , set to sda1\n",cat,strlen(cat));
            strcpy(hdenc->partition,"/hqdata/sda1");
        }
    }
    else
    {
        gtloginfo("no record_disk, set to sda1\n");
        strcpy(hdenc->partition,"/hqdata/sda1");
    }
    printf("record disk =%s\n",hdenc->partition);
#endif




    sprintf(sec,"video%d:enable",channel);
    hdenc->enable = iniparser_getint(ini,sec,1);
        
    if(hdenc->enable == 0) //��Ч
    {
        gtloginfo("channel %d ��Ƶ��Ч��������¼���߳�\n",channel);
    }
    else
    {
        if(hdenc->rec_type==1)//����Ԥ¼ģʽ 
        {
            gtloginfo("channel %d �ƶ�����Ԥ¼ģʽ\n",channel);
        }
        else
        {
            hdenc->rec_type=0;
            gtloginfo("channel %d ����Ԥ¼ģʽ\n",channel);
        }
    }
    //Ϊ��ֹ��Ϣ�����������pre_rec/cut_len>115ʱ��������Ϣ���г���
    msgnum=2*hdenc->pre_rec/hdenc->max_len;
    set_msgmnb_value(hdenc->qid,msgnum);

#ifdef USE_FFMPEG_LIB
    //��ȡ��Ƶ��ʽ
    ///Ŀǰ����u-pcm ���� mpeg2
    if(channel == 0)
        audio_save_fmt=iniparser_getint(ini,"hqpara:audio_fmt",1);      //��Ƶ�洢��ʽ1Ϊu-pcm 
    else
        audio_save_fmt=iniparser_getint(ini,"hqpara1:audio_fmt",1);      //��Ƶ�洢��ʽ1Ϊu-pcm 

#endif

    //��������ץͼ������·��
    sprintf(sec,"alarm:snap_pic_path"); 
    
    cat=iniparser_getstring(ini,sec,ALARM_SNAPSHOT_PATH);
    if(cat==NULL)
        return -1;
    //gtloginfo("test,read out is WOW, %s\n",cat);
    sprintf(hdenc->alarmpic_path,"%s",cat);


    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    } 
    iniparser_freedict(ini);
    return 0;   

}

//�������ڴ���ץͼ��־
int set_takingpicflag(int value, int ch)
{
    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->takingpicflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//������Ҫ����ץͼ��־
int set_alarmpic_required(int value, int ch)
{

    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->alarmpic_required=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//�������ڱ���ץͼ��־
int set_alarmpicflag(int value, int ch)
{
    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->alarmpicflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//��ʼ��������¼�����ݽṹ
int init_hdenc(void)
{   
    int rc;
    int i;
    //�����Ƿ����ϴ�δ¼����ļ������У��������Ϊ_OLD.AVI
    if(access(HDSAVE_PATH,F_OK)<0)
    {
        mkdir(HDSAVE_PATH,0777);
    }

    for(i=0;i<MAX_RECORD_CHANNEL_M;i++)
    {
        rc=init_hdenc_ch(i);
        set_takingpicflag(0,i);
        set_alarmpic_required(0,i);
        set_alarmpicflag(0,i);
    }
    
    set_cferr_flag(0);   
    send_state2main();
    return 0;   
}



//�ڻ���������������ָ��ͨ����semflag
int set_semflag(int channel,int value)
{
    struct hd_enc_struct *hd;
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->semflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}

int init_hqpara_default_val(struct hd_enc_struct    *hdenc)
{
    hdenc->pre_rec=600;
    hdenc->dly_rec=300;
    hdenc->max_len=30;
    hdenc->del_typ=0;
    hdenc->bitrate=1024;    
    hdenc->queryflag=0;
    
    return 0;
}
//��ʼ����channel·������¼�����ݽṹ
int init_hdenc_ch(int channel)
{
    struct hd_enc_struct    *hdenc;
    int qid;
    //int check,ret;

    //key_t key;
    
//    if((channel>=get_total_hqenc_num())&&(channel<0))
//        return -1;

    printf("init_hdenc_ch channel=%d\n",channel);
    
    hdenc=get_hdch(channel);
    memset((void*)hdenc,0,sizeof(hdenc));
    pthread_mutex_init(&hdenc->mutex, NULL);//ʹ��ȱʡ����
    init_hqpara_default_val(hdenc);
    read_hqsave_para_file(HDMOD_PARA_FILE,"hqpara",channel);

 
    hdenc->channel = channel;
    hdenc->audiochannel = channel;
    sprintf(hdenc->partition,"/hqdata/sda%d",channel+1);
    hdenc->recordlen=0;

    hdenc->thread_id=-1;
    hdenc->keyframe_pool_thread_id=-1;
    if((hdenc->pre_rec!=0)&&(hdenc->rec_type==0))
        hdenc->state=1;

    hdenc->watchcnt=0;
    hdenc->keyframe_cnt=0;
    hdenc->readenc_flag=0;
    hdenc->takingpicflag=0;
    hdenc->picerrorcode=0;
    hdenc->pictime=0;
    hdenc->timemax=0;
    hdenc->takepic_thread_id=-1;
    hdenc->remote_trig_time=0;
    hdenc->remote_trigged=0;
    hdenc->semflag = 0;
    hdenc->threadexit = 0;

#if 0
    printf("22222  init_hdenc_ch channel=%d\n",channel);
    //�����ͳ�ʼ�������
    //ret=mkrtpool(get_stream_pool(0),D1_MAX_FRAME_SIZE,POOLSIZE+4); //���������
    ret=mkrtpool(get_stream_pool(hdenc->channel),MAX_FRAME_SIZE,1); //���������,modified by wsy,ʹ��devinfo.h���size����

#ifdef SHOW_WORK_INFO
    printf("mkrtpool rc=%d\n",ret);
#endif

    ret=initrtpool(get_stream_pool(hdenc->channel));

#ifdef SHOW_WORK_INFO
    printf("initrtpool rc=%d\n",ret);
#endif

    switch(channel)
    {
        case(0):sprintf(hdenc->devname,"%s",HQDEV0);break;
        case(1):sprintf(hdenc->devname,"%s",HQDEV1);break;
        case(2):sprintf(hdenc->devname,"%s",HQDEV2);break;
        case(3):sprintf(hdenc->devname,"%s",HQDEV3);break;
        default: break;

    }
#endif    
    
    pthread_mutex_init(&hdenc->audio_mutex, NULL);//ʹ��ȱʡ����
    pthread_cond_init(&hdenc->audio_cond,NULL);
    pthread_mutex_init(&hdenc->file_mutex,NULL);
    hdenc->audio_thread_id=-1;
    
    return 0;
}

//��ȡָ���0·ѹ���ṹ��ָ��
/*struct compress_struct *get_encoder(int channel)
{
    if((channel<get_total_hqenc_num())&&(channel>=0))
        return &(hdenc[channel].encoder);
    else 
        return NULL;
}*/
    
//��ȡָ���0·��¼�Ľṹָ��
struct hd_enc_struct    *get_hdch(int channel)
{
    if((channel<MAX_RECORD_CHANNEL_M)&&(channel>=0))
        return &g_hdenc[channel];
    else 
        return NULL;
}
//������ת������Ӧ��path��
int time2path (struct dir_info_struct *pathdir, time_t time)
{
    struct tm *p;
  
    p=localtime(&time);
    pathdir->year=1900+p->tm_year,
    pathdir->month=1+p->tm_mon,
    pathdir->date=p->tm_mday,
    pathdir->hour=p->tm_hour;
  
    return 0;
}




//����hd����Ϣ�Լ���ǰʱ������ļ������ظ�filename
int make_file_name(struct hd_enc_struct *hd,char * filename)
{
    time_t ctime;
    struct file_info_struct finfo;
    
    if((hd==NULL)||(filename==NULL))
        return -1;
        
    ctime=time(NULL);
    finfo.stime=ctime;
    finfo.channel=hd->channel;
    finfo.type=hd->state;
    finfo.len=0;
    finfo.trig=hd->trig;
    finfo.remote=hd->remote_trigged;
    sprintf(finfo.partition,hd->partition);
    //gtloginfo("test,remote is %d\n",finfo.remote);
    return finfo2filename(&finfo,filename);
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


//������д��I���ļ�
//ifile:�������ļ���ָ��,�����ߵĻ�����Ӧ���㹻�������100
int  create_hqpic_file(struct stream_fmt_struct *frame, char *indexname,char *ifile,int channel)
{
    FILE    *filep=NULL;
    FILE    *fp=NULL;
    int       ret;
    //int devtype;
    char    filename[100];
    struct timeval *tm;
    char    path[20];
    char    *p=NULL;
    int     devtype;
    media_attrib_t *media_attrib=NULL;
    media_format_t *media_format=NULL;
    video_format_t *v_fmt=NULL;
    char    *v_avitag=NULL; //��Ƶ�����ʽ��avi���
    //char mpg_head[4]={0x00,0x00,0x01,0x00};
    
    
    tm=(struct timeval *)&frame->tv;
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(filename,"%s/iframe/%d-%06d.mpg",HDSAVE_PATH,(int)tm->tv_sec,(int)tm->tv_usec);
    filep=fopen(filename,"w+");
    if(filep==NULL)
        return -1;
    //д���16�ֽڿ�ʼ��ͼƬ���ݳ���
    ret=fprintf(filep,"%d",(int)frame->len); //?
    
    
    //д����Ƶ�����ʽ�ַ���
    
    media_attrib=get_venc_attrib_keyframe(get_hqenc_video_ch(channel));
    media_format=&media_attrib->fmt;
    
    v_fmt=&media_format->v_fmt;
    
    switch(v_fmt->format)
    {
        case VIDEO_H264:
            v_avitag="H264";
        break;
        case VIDEO_MJPEG:
            v_avitag="MJPEG";
        break;
        case VIDEO_MPEG4:
            v_avitag="MPEG4";
        break;
        default:
            v_avitag="MPEG4";
        break;

   }
    
    ret=fseek(filep,8,SEEK_SET);
    ret=fprintf(filep,"%s",v_avitag); //?
#ifdef  REAL_D1_TEST_FOR_3000
    //deinterlace��־
    devtype = get_devtype();
    if((devtype>=T_GTVS3021)&&(devtype<T_GTVM3001))
    {
        if((v_fmt->v_width==720)&&(v_fmt->v_height==576))
        {
            ret=fseek(filep,15,SEEK_SET);
            ret=fprintf(filep,"%c",0x01);
        }
    }

#endif  

    
    //д��ͼƬ����
    ret=fseek(filep,16,SEEK_SET);
    fwrite(frame->data,frame->len,1,filep); //?

    
    

    fclose(filep);


    chmod(filename,0755);
    fp=fopen(indexname,"a+");
    if(fp==NULL)
        return -1;
    if (PATH_TYPE)      
        ret=fprintf(fp,"%s\n",filename);
    else
    {
        p=strstr(filename,"/iframe");
        if(p!=NULL)
            ret=fprintf(fp,"%s\n",p);
        else
            ret=fprintf(fp,"%s\n",filename); ///<û�ҵ�,ֱ������ļ���
    }
    fclose(fp);
    chmod(indexname,0755);
    if(ifile!=NULL)
    {
        memcpy(ifile,filename,strlen(filename)+1);
    }
    
        
    return 0;
    
}
//����һ��ʱ����Ͳ��ʱ��(����Ϊ��λ)�����������ʱ���
int  get_time_before(struct timeval *timenow, int diff_in_msec,struct timeval *timebefore)
{
    int diffsec,diffusec;
    
    if((timenow==NULL)||(timebefore==NULL))
        return -1;
    
    diffsec=diff_in_msec/1000;
    diffusec=1000*(diff_in_msec%1000);
    if(timenow->tv_usec>=diffusec)
    {
        timebefore->tv_usec=timenow->tv_usec-diffusec;
        timebefore->tv_sec=timenow->tv_sec-diffsec;
    }
    else
    {
        timebefore->tv_usec=timenow->tv_usec-diffusec+1000000;
        timebefore->tv_sec=timenow->tv_sec-diffsec-1;
    }
    return 0;
}

//�������ʱ����Ĳ�࣬��Ϊ��λ��double����
double diff_timeval(struct timeval *timenow, struct timeval *timerequired)
{
    long sec;
    long usec;
    double diff; 
    
    if((timenow==NULL)||(timerequired==NULL))
        return -1;
    
    sec=timenow->tv_sec-timerequired->tv_sec;
    usec=timenow->tv_usec-timerequired->tv_usec;
    diff=sec+(double)usec/1000000;
    
    return(diff);       
}


//����ץͼ�߳�
int usr_take_pic(gateinfo *gate, struct takepic_struct *takepic)
{

    double length;
    takepic_info *takepicinfo;
    int rc=-1;
    pthread_attr_t  thread_attr,*attr;
    struct hd_enc_struct *hd;

    if(takepic==NULL)
    {
        gtloginfo("ץͼ����ָ��Ϊ��\n");
        return -1;
    }
    else
    {
        hd=get_hdch(takepic->channel);
        if(hd==NULL)
        {
            gtloginfo("ץͼͨ����%d������Ƶ�������������޷�ץͼ\n",takepic->channel);
            return -EINVAL;
        }
        memcpy(&hd->current_takepic,takepic,sizeof(struct takepic_struct));
    }
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    takepicinfo = (takepic_info *)malloc(sizeof(takepic_info));
    memcpy(&takepicinfo->gate,gate,sizeof(gateinfo));
    memcpy(&takepicinfo->takepic,takepic,sizeof(struct takepic_struct));
    rc=pthread_create(&hd->takepic_thread_id,attr,takepic_thread,(void *)(takepicinfo));
    if(rc==0)
    {           
        hd->pictime=0;
        length=takepic->interval*takepic->takepic*3/1000;
        hd->timemax=30+(int)length;
#ifdef SHOW_WORK_INFO
        printf("takepic_timemax=%d\n\n",hd->timemax);
#endif      
        usleep(500000);
    }
    else
    {
        gtloginfo("�޷�Ϊ%dͨ������ץͼ�߳�,%d",takepic->channel,rc);

        if(get_takingpicflag(takepic->channel)==1)
            set_takingpicflag(0,takepic->channel);
        if(get_alarmpicflag(takepic->channel)==1)
            set_alarmpicflag(0,takepic->channel);
    }

    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    
    return rc;
    
}


//����������ֹץͼ�̵߳��峡����
void takepic_thread_cleanup(void *para)
{
    struct takepic_struct   takepic;
    char cmd[100];
    FILE *fp;
    struct stat buf;
    char indexnametype1[100];
    char indexnametype0[100]; //type=0ʱ��������
    struct timeval timeprint;
    struct hd_enc_struct *hd;
    char alarmindex[200];


    
    takepic_info takepicinfo;
    memcpy((void*)&takepicinfo,para,sizeof(takepicinfo));
    memcpy(&takepic,&takepicinfo.takepic,sizeof(takepic));
    memcpy(&timeprint,&takepic.time,8);
    hd=get_hdch(takepic.channel);
    if(hd==NULL)
        return;
    gtloginfo("��ʱ��ֹ%dͨ��ץͼ�߳�\n",takepic.channel);
   //printf("\n\n come into takepic_thread_cleanup!!\n\n");
    
    sprintf(indexnametype1,"%s/picindex/%d-%d.txt",HDSAVE_PATH,(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    sprintf(indexnametype0,"/picindex/%d-%d.txt",(int)timeprint.tv_sec,(int)timeprint.tv_usec);

    if(get_takingpicflag(takepic.channel)==1) //��ͨץҪ�ͻ�����
    {
        if(PATH_TYPE==1)
            get_hq_pic_answer(&takepicinfo.gate,ERR_DVC_INTERNAL,(BYTE*)&timeprint,(char*)indexnametype1);
        if(PATH_TYPE==0)
            get_hq_pic_answer(&takepicinfo.gate,ERR_DVC_INTERNAL,(BYTE*)&timeprint,(char*)indexnametype0);

        set_takingpicflag(0,takepic.channel);
    }
    if((get_alarmpic_required(takepic.channel)==1)||(get_alarmpicflag(takepic.channel)==1))//����ץҪ
    {   
        sprintf(alarmindex,"%s%s",HDSAVE_PATH,hd->alarmpic_path);
        sprintf(cmd,"cp %s %s",indexnametype1,alarmindex);
        system(cmd);
        fp=fopen(alarmindex,"r+");
        if(fp!=NULL)
        {
            stat(alarmindex,&buf);
            if(buf.st_size!=0)
                {
                    gtloginfo("����ץͼ����ʱȡ��,����Ϊ��ץ����ͼ\n");
                }
            else//��*��
                {
                    gtloginfo("����ץͼ����ʱȡ��,ûץ��ͼ,����д����ԭ��\n");
                    fprintf(fp,"*%s",get_gt_errname(ERR_DVC_INTERNAL));
                }
            fclose(fp);
        }
        else
            gtloginfo("�޷��򿪱���ץͼ�����ļ�%s\n",alarmindex);
        set_alarmpicflag(0,takepic.channel);
        set_alarmpic_required(0,0);
    }
    sem_destroy(&hd->sem);
    set_semflag(takepic.channel,0);
    return;
}



//��ȡ���ڴ���ץͼ��־
int get_takingpicflag(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;
    
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->takingpicflag;
        pthread_mutex_unlock(&hd->mutex);
    }
    
    return value;
}
//��ȡ��Ҫ����ץͼ��־
int get_alarmpic_required(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;

    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->alarmpic_required;
        pthread_mutex_unlock(&hd->mutex);
    }
    return value;
}
//��ȡ����ץͼ��־
int get_alarmpicflag(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;
    
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->alarmpicflag;
        pthread_mutex_unlock(&hd->mutex);
    }
    return value;
}



#ifdef SNAP_TO_AVIFILE//added by shixin
//indexname��ʱû���� 
int create_hqpic_avi_file(struct stream_fmt_struct *frame, char *indexname,char *ifile)
{
    FILE *filep;
    int ret;
    char filename[100];
    struct timeval *tm;
    char path[20];
    struct hd_enc_struct *hd;
    AVIVarHeader avihead;
    avi_t *aviinfo = NULL;
    
    if((frame==NULL))//||(indexname==NULL))||(ifile==NULL))
    {
        return -1;  
    }

    tm=(struct timeval *)&frame->tv;
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(filename,"%s/iframe/%d-%06d.avi",HDSAVE_PATH,(int)tm->tv_sec,(int)tm->tv_usec);
    filep=fopen(filename,"w+");
    if(filep==NULL)
        return -1;
    hd=get_hdch(0);
    //memcpy((void*)&avihead,(void*)&hd->avi,sizeof(avihead));
    aviinfo=AVI_open_output_file(filename);
    if(aviinfo==NULL)
    {
        return -2;
    }
    AVI_write_frame(aviinfo, frame->data, frame->len, 1);
    AVI_close(aviinfo);
    chmod(filename,0755);
    if(ifile!=NULL)
    {
        memcpy(ifile,filename,strlen(filename)+1);
    }
    fclose(filep);
    return 0;

}
#endif
//����ץͼ



void *takepic_thread(void *takepic)//ץͼ�߳�
{
    
    char indexname[100];
    char shortindexname[100];
    FILE *fp,*fpindex;
    char filename[100];
    char path[30];
    int picno=0;
    char cmd[100];
    int ret;
    int i,number=0;
    int error=0;
    char alarmindex[200];
    int waitsemflag=0;//Ϊ0���ڳ�ץ��Ϊ1���Ѿ��Ǻ���ץ
    struct pool_ele_struct *ele,*elebak;
    int interval;
    struct timeval *tm,timeprint;
    struct takepic_struct *takepicpara;
    double diff,bakdiff=1000;
    double point=0;
    struct hd_enc_struct *hd;
    struct stream_fmt_struct *frame, *bakframe; 
    int firstpictaken=0;
    int bakused=1;//firstpictakenΪ1���Ѿ�ȡ����һ����Ƭ;bakusedΪ1���ʾ��һ��Ԫ���Ѿ��ù��򲻴���
    int active=0;

    if(takepic==NULL)
    {   
        error=ERR_EVC_NOT_SUPPORT;
        gtloginfo("����ץͼָ��Ϊ��,����ץͼ!\n");
        goto send_result;
    }
    takepic_info *takepicinfo;
    takepicinfo = (takepic_info *)takepic;
    takepicpara = &takepicinfo->takepic;
    
    memcpy((void*)&timeprint,(void*)takepicpara->time,sizeof(timeprint));
    gtloginfo("ץͼ�߳���,����%d,���%d,ͨ��%d,ʱ��%ld-%ld\n",takepicpara->takepic,takepicpara->interval,takepicpara->channel,timeprint.tv_sec,timeprint.tv_usec);  
    
    hd=get_hdch(takepicpara->channel);
    if(hd==NULL)
    {
        error=ERR_DVC_INTERNAL;
        gtloginfo("ȡ����hd�ṹ������ץͼ\n");
        goto send_result;
    }
    printf("start takepic_thread... \n");

    


    //����ȡ��
    ret=pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    ret=pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
    pthread_cleanup_push(takepic_thread_cleanup,(void *)takepicinfo);
    //��ʼ���ֲ�����
    
    set_semflag(takepicpara->channel,0); 
    picno=takepicpara->takepic;
    interval=takepicpara->interval;
    //Step1.�ж�ʣ��ռ��Ƿ�С��GET_PIC_SPACE
    if(get_disk_free(HDSAVE_PATH)<GET_PIC_SPACE)
    {
        
        printf("space not enough, cannot get picture\n");
        //gtlogerr("�ռ䲻��,%d M,����ץͼ\n",get_disk_free(HDSAVE_PATH));
        gtlogerr("�ռ䲻��,%d M����ץͼ\n",get_disk_free(HDSAVE_PATH));
        
        error=ERR_NO_SPACE;
        goto send_result;
    }

    //Step2.��ʼ��ץͼ��Ŀ����Ҫ���ץͼ�������ֻ࣬����ץTAKE_PIC_MAX��
    if(picno>TAKE_PIC_MAX)
    {   
        picno=TAKE_PIC_MAX;
        gtloginfo("ץͼ��������%d,ֻ��ץ%d��\n",TAKE_PIC_MAX,TAKE_PIC_MAX); 
    }
    if(picno<=0)
    {
     gtloginfo("ץͼ����%dС�ڵ���0,������ץͼ\n",picno);
     error=ERR_ENC_NOT_ALLOW;
     goto send_result;
     }

    
    //Step3.�����ҳ�ʼ��һ�������ļ�.txt
    sprintf(path,"%s/picindex",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(indexname,"%s/picindex/%d-%08d.txt",HDSAVE_PATH,(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    sprintf(shortindexname,"/picindex/%d-%08d.txt",(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    fp=fopen(indexname,"w+");
    if(fp==NULL)
    {
        error=ERR_NO_SPACE;
        gtlogerr("�򲻿�ָ���ļ�%s,����ץͼ\n",indexname);
        goto send_result;
    }
    else
        fclose(fp); 

    if(hd->picerrorcode!=0)//����������
    {   
        gtloginfo("���������ִ���0x%04x������ץͼ!\n",hd->picerrorcode);
        error=hd->picerrorcode;
        goto send_result;
    }
    
    //Step4.�ѻ������е�Ԫ�����ȡ�����Ƚ�ʱ�������������Ҫ��ȡ����д���ļ�
    
    getpicture:
    active=get_pool_active_num(get_stream_pool(takepicpara->channel));
    for(i=number;i<number+active;i++)
    {

        if(picno==0) 
            break;  
        ele=get_active_ele(get_stream_pool(takepicpara->channel)); //ȡ��һ��������Ԫ��
        frame=(struct stream_fmt_struct*)&ele->element;
        tm=(struct timeval *)&frame->tv;
        
        diff=diff_timeval(tm,&timeprint);

        if(diff<point) //��ʱ���֮ǰ
        {   
            if((i==number+active-1)&&(firstpictaken==0))//���һ�ţ�ȡ,���0
            {
                create_hqpic_file(frame,indexname,filename,takepicpara->channel);               

#ifdef SNAP_TO_AVIFILE//added by shixin
                create_hqpic_avi_file(frame,indexname,NULL);
#endif
                point=diff+(double)interval/1000;
                picno--;
                bakused=1;
                ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                if(picno!=0) 
                    elebak=ele;
                firstpictaken=1;
#ifdef SHOW_WORK_INFO
                printf("take this one,0\n");
#endif
            }
            else
            {
                bakframe=frame; //����
                bakdiff=diff;
                ret=free_ele(get_stream_pool(takepicpara->channel),elebak);

                if(picno!=0) 
                    elebak=ele;
                bakused=0;                      
#ifdef SHOW_WORK_INFO
                printf("backup,0\n");
#endif  
            }
        }   
        else//��ʱ���֮��
        {
        
            if(firstpictaken==0) //��ûȡ��һ��,��������һ�ž�ȡ��һ�ţ�����ȡ��һ��
            {
                if(bakused==0)
                {
                    //test=(struct timeval *)&bakframe->tv;
                    //gtloginfo("���1 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(bakframe,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(bakframe,indexname,NULL);
#endif
                    point=bakdiff+(double)interval/1000;
                    picno--;
                    bakframe=frame; //����
                    bakdiff=diff;
                    ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
                    bakused=0;
                    firstpictaken=1;
#ifdef SHOW_WORK_INFO
                    printf("take last one��1\n");
#endif
                }
                else
                {
                    //test=(struct timeval *)&frame->tv;
                    //gtloginfo("���2 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(frame,indexname,NULL);
#endif                      
                    point=diff+(double)interval/1000;
                    picno--;
                    bakused=1;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
                    firstpictaken=1;
#ifdef SHOW_WORK_INFO
                    printf("take this one��2");
#endif
                }
            }
            else //��ȡ����һ�ţ����Ķ�����ν
            {
                //�����һ���ù��ˣ���ȡ��һ��
                if(bakused==1)
                {
                    //test=(struct timeval *)&frame->tv;
                    //gtloginfo("���3 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(frame,indexname,NULL);
#endif
                    point=diff+(double)interval/1000;
                    picno--;
                    bakused=1;
                    ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
#ifdef SHOW_WORK_INFO
                    printf("take this one 3,\n");
#endif
                }
                else //�����һ��û�ù�����Ƚ�˭������ȡ˭
                {
                    if(fabs(bakdiff-point)<fabs(diff-point)) //��һ�Ÿ��� 
                    {
                        //test=(struct timeval *)&bakframe->tv;
                        //gtloginfo("���4 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                        create_hqpic_file(bakframe,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                        create_hqpic_avi_file(bakframe,indexname,NULL);
#endif
                        point=bakdiff+(double)interval/1000;
                        picno--;
                        bakframe=frame; //����
                        bakdiff=diff;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                        if(picno!=0) 
                            elebak=ele;
                        bakused=0;
#ifdef SHOW_WORK_INFO
                        printf("take last one...4 it's nearer\n");
#endif
                    }
                    else //��һ�Ÿ���
                    {
                        create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                        create_hqpic_avi_file(frame,indexname,NULL);
#endif
                        point=diff+(double)interval/1000;
                        picno--;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                        if(picno!=0) 
                            elebak=ele;
                        bakused=1;
#ifdef SHOW_WORK_INFO       
                        printf("take this one.5.nearer\n");
#endif
                    }   
                }
            }
        }
    }
    
#ifdef SHOW_WORK_INFO
    printf("picno= %d left\n",picno);
#endif
    //Step5.���ʣ�µ���ץͼƬ������������Step4�����ץͼ���ͽ��
    if(picno!=0)
    {
        if(waitsemflag!=1)
        {
            ret=sem_init(&hd->sem,0,0);
            waitsemflag=1;
        }
        number=number+active;
        set_semflag(takepicpara->channel, 1);
        pthread_testcancel();
        ret=sem_wait(&hd->sem);
        goto getpicture;
    
    }
    gtloginfo("��%d·��Ƭץ��\n",takepicpara->channel);

    sem_destroy(&hd->sem);
    ret=free_ele(get_stream_pool(takepicpara->channel),ele);
    set_semflag(takepicpara->channel,0);
    chmod(indexname,0755);

    //���ͽ��
send_result:
  printf("[%s:%d]get_takingpicflag=%d\n",__FILE__,__LINE__,get_takingpicflag(takepicpara->channel));
    if(get_takingpicflag(takepicpara->channel)==1)
    {
        if(PATH_TYPE==1)
            ret=get_hq_pic_answer(&takepicinfo->gate, error,takepicpara->time,indexname);
        else
            ret=get_hq_pic_answer(&takepicinfo->gate,error,takepicpara->time,shortindexname);
        ret=set_takingpicflag(0,takepicpara->channel);
    }
    if((get_alarmpicflag(takepicpara->channel)==1)||(get_alarmpic_required(takepicpara->channel)==1))//�б���ץͼ����,Ҫ���ļ�����
    {
        sprintf(alarmindex,"%s%s",HDSAVE_PATH,hd->alarmpic_path);
        if(error==0)
        {
            sprintf(cmd,"cp %s %s",indexname,alarmindex);
            gtloginfo("����ץͼ��%s����%s\n",indexname,alarmindex);
            system(cmd);
        }
        else
        {
            gtloginfo("����ץͼʧ��,ԭ��%s,д�������ļ�\n",get_gt_errname(error));
            fpindex=fopen(alarmindex,"w+");
            if(fpindex!=NULL)
            {
                fprintf(fpindex,"*%s",get_gt_errname(error));
                fclose(fpindex);
            }
            else
                gtloginfo("�򲻿�����ץͼ�����ļ�������%s\n",strerror(errno));
        }
        set_alarmpicflag(0,takepicpara->channel);
        set_alarmpic_required(0,takepicpara->channel);
    }


    pthread_cleanup_pop(0);
    pthread_exit(0); 
}   


//�鵽һ������Ҫ��ļ�¼,�����������index�ļ���������ֵ
int process_filename_to_index(int channel,char *filename, int indexno, FILE *fp)
{
    char *ing, *lp, *lk;
    char ingname[200],temp[200],flag[50];
    struct hd_enc_struct * hd;
    
    if((filename ==NULL)||(fp ==NULL))
        return -EINVAL;     
    
    
    //�����_OLD.AVI�򲻼�������
    if(strstr(filename,OLD_FILE_EXT)!=NULL)
        return -ENOENT;
    
    ing=strstr(filename,RECORDING_FILE_EXT);
    if(ing!=NULL) //ing�ļ���������������һ���ļ���
    {
        hd = get_hdch(channel);
        if(hd == NULL)
            return -EINVAL;
        sprintf(ingname,"%s",filename);
        pthread_mutex_lock(&hd->mutex); 
        hd->queryflag=1;
        pthread_mutex_unlock(&hd->mutex);   
        ing=strstr(ingname,RECORDING_FILE_EXT); 
        *ing='\0';  //ingname='HQ......T00'
        lp=index(ingname,'L');
        lp++;       //lp='00_T00'
        sprintf(temp,"%s",lp); 
        *lp='\0'; //ingname='HQ.._L'
        lk=index(temp,'T');  //temp='00_T00'
        lk++;    //lk='00'
        if(atoi(lk)!=0)  //�������״̬��Ϊ0��Ҫ����
            sprintf(flag,"%c%s",LOCK_FILE_FLAG,IMG_FILE_EXT);
        else 
            strcat(flag,IMG_FILE_EXT); //��ʱflag="(@).AVI"
        sprintf(temp,"%02d_T%02d%s",hd->max_len,atoi(lk),flag); //temp='120_T00(@).AVI'                                     
        strcat(ingname,temp);
        return fprintf(fp,"%s\n",strstr(ingname,"/sd"));//��ȥ/hqdata
            
    }
    else    
        return fprintf(fp,"%s\n",strstr(filename,"/sd"));       
}


int query_index_in_partition(IN char *devname, IN char* mountpath, IO void *fn_arg)
{
    struct query_index_process_struct * query;
    
    if((mountpath == NULL)||(fn_arg == NULL))
        return -EINVAL;
        
    query = (struct query_index_process_struct *) fn_arg;
    if(query->index_fp == NULL)
        return -EINVAL;
    return fileindex_query_index(mountpath,query);
}



//��Ϊ�������ļ��в�,
int query_record_index(char *indexname,int ch,time_t start,time_t stop,int trig)
{
    int result;//��ѯ���
    FILE *fp;
    struct stat statbuf;
    struct query_index_process_struct queryindex;
    char path[20];
    char cmd[200];
    char tmpname[100];//�������ǰ����������
    
    
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(tmpname,"%s/index/%d-tmp.txt",HDSAVE_PATH,(int)start);
    fp=fopen(tmpname,"w+");
    if(fp==NULL)
        return -1;
    
    
    queryindex.index_fp =   fp;
    queryindex.ch       =   -1;//ch;ip1004ֻ��4���棬ȫ����
    queryindex.start    =   start;
    queryindex.stop     =   stop;
    queryindex.trig_flag=   trig;      
    result = mpdisk_process_all_partitions(query_index_in_partition, &queryindex);

    fclose(fp);
    stat(tmpname,&statbuf);
    //printf("st_size is %d\n",statbuf.st_size);
    if(statbuf.st_size !=0)
    {
        sprintf(indexname,"%s/index/%d.txt",HDSAVE_PATH,(int)start);
        sprintf(cmd,"/ip1004/record_sort %s %s",tmpname,indexname);
        system(cmd);
        sprintf(indexname,"/index/%d.txt",(int)start);
        return 0;
    }
    else
        return -ERR_DVC_NO_RECORD_INDEX;
}

//static volatile int rtstream_cnt=0;

void hd_playback_en(void)
{
    hd_playback_flag=1;
}

void hd_playback_cancel(void)
{
    hd_playback_flag=0;
}




//��������¼ģ���봦�����
void hd_second_proc(void)
{
    struct hd_enc_struct    *hd_enc;    
    media_attrib_t *attrib=NULL;//
    media_attrib_t *attrib_keyframe=NULL;
    int intid;
    int ret;
    int i;

    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    { 
    //������Ƶ������
        hd_enc=get_hdch(i);
        intid = hd_enc->audio_thread_id;
        if(intid > 0)
        {
            attrib=get_aenc_rec_attrib(hd_enc->audiochannel);  //NULL��ʾ��û�����ӵ���Ƶ������
            if((attrib!=NULL))
            {
                if(++hd_enc->audio_cnt<10)
                {       
                }
                else
                {   
                    //gtloginfo("wsytest,reactive_audio_enc\n");
                    ret=reactive_audio_rec_enc(hd_enc->audiochannel);
                    if(ret<0)                   
                      {
                          gtloginfo("���¼�����Ƶ������%dʧ��!ret =%d\n",0,ret);
                          printf("���¼�����Ƶ������%dʧ��ret= %d\n",0,ret);
                      } 
                  //else
                      //gtloginfo("wsytest,reactive_audio_enc done\n");
                    hd_enc->audio_cnt=0;
                }
            }
        }
        else
        {
            hd_enc->audio_cnt=0;
        }
    }
    
  for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
  {
      hd_enc=get_hdch(i);
      //Ϊ¼���̼߳�����Ƶ������
      intid=hd_enc->thread_id;
      if(intid>0)
      {
          attrib=get_venc_attrib_record(hd_enc->channel);  //NULL��ʾ��û�����ӵ���Ƶ������
          if(attrib!=NULL)
          {
              if(++hd_enc->watchcnt<10)
              {     
                
              }
              else
              {         
                  if(hd_enc->watchcnt%10==0)
                  {
                      //gtloginfo("¼���߳�%dsû�ж�����Ƶ����\n",hd_enc->watchcnt);            
                      if((hd_enc->watchcnt<100)&&(hd_enc->watchcnt >= 10))//����100��Ͳ�����
                      {
                          if(hd_enc->readenc_flag == 1)
                          {
                              gtloginfo("¼���߳�%dsû�ж�����Ƶ����\n",hd_enc->watchcnt);          
                              printf("record thread %dsû�ж�����Ƶ����\n",hd_enc->watchcnt);
                          }
                          else
                          {
                              gtloginfo("¼���߳�%dsû��д����Ƶ����\n",hd_enc->watchcnt);
                              printf("record thread %dsû��д����Ƶ����\n",hd_enc->watchcnt);
                          }
                      }

                      pthread_mutex_lock(&hd_enc->file_mutex);
                      if((hd_enc->watchcnt >= 10)&&(hd_enc->aviinfo != NULL))
                      { 
                          gtloginfo("��%d�������ݣ��ر�¼���ļ�\n",hd_enc->watchcnt);
                          close_record_file(hd_enc);        
                      }
                      pthread_mutex_unlock(&hd_enc->file_mutex);
                      ret = reactive_video_record_enc(hd_enc->channel);
                      if(ret<0)
                      {
                          printf("���¼�����Ƶ������%dʧ��:%d\n",hd_enc->channel,ret);
                          gtloginfo("���¼�����Ƶ������%dʧ��:%d\n",hd_enc->channel,ret);
                      }
                  }
              }
          }
      }
      else
      {
          hd_enc->watchcnt=0;
      }
#if 0    //ip1004������ץͼ
    //Ϊkeyframe�̼߳�����Ƶ������
      intid=hd_enc->keyframe_pool_thread_id;
      if(intid>0)
      {
          attrib_keyframe=get_venc_attrib_keyframe(get_hqenc_video_ch(hd_enc->channel));
          if(attrib_keyframe!=NULL)
          {
              if(++hd_enc->keyframe_cnt<10)
              {     
              }
              else
              {         
                  if(hd_enc->keyframe_cnt%10==0)
                  {
                      if(hd_enc->keyframe_cnt<100)//����100��Ͳ�����
                      {
                          printf("keyframe�߳�%dsû�ж�����Ƶ����\n",hd_enc->keyframe_cnt);
                          gtloginfo("keyframe thread %dsû�ж�����Ƶ����\n",hd_enc->keyframe_cnt);          
                      }
                      if(reactive_video_enc(get_hqenc_video_ch(hd_enc->channel))<0)                 
                      {
                          gtloginfo("keyframe�߳����¼�����Ƶ������%dʧ��!\n",get_hqenc_video_ch(hd_enc->channel));
                          printf("keyframe�߳����¼�����Ƶ������%dʧ��!\n",get_hqenc_video_ch(hd_enc->channel));
                      } 
                  }
              }
          }
      }
      else
      {
          hd_enc->keyframe_cnt=0;
      }

     //����ץͼ���
      if((get_takingpicflag(hd_enc->channel)==1)||(get_alarmpicflag(hd_enc->channel)==1)) //����ץͼ
      {
          if(hd_enc->pictime<hd_enc->timemax)
            {
         hd_enc->pictime++;
       }
          else
          {
              intid=(int)hd_enc->takepic_thread_id;
              if(intid>0)
                {
                    sem_post(&hd_enc->sem);
                    ret=pthread_cancel(hd_enc->takepic_thread_id);
           printf("    CANCEL ==========================\n");
                    hd_enc->takepic_thread_id=-1;//added by shixin
                }
              hd_enc->pictime=0;
          }
      }
#endif      
    //����¼��״̬�����������
        
        if(hd_enc->state!=0)
        {
            if(hd_enc->aviinfo != NULL)
            {
                if(++hd_enc->filelen>=hd_enc->max_len)//�з�
                {
                    hd_enc->cutflag=1;  
                }   
            }
            if(hd_enc->remote_trig_time>0)
            {
                if((--hd_enc->remote_trig_time)==0)
                {     
                    if(hd_enc->recordlen==0)
                    {
                        if((hd_enc->rec_type==0)&&(hd_enc->pre_rec !=0))
                        {
                            hd_enc->state=1;
                        }
                        else
                        {
                            hd_enc->state=0;            
                        }
                    }
                    else
                    {
                      hd_enc->cutflag=1;
                    }
                    hd_enc->remote_trigged=0;
                }
            }

            if(hd_enc->recordlen>0) //����ֹͣ¼��
            {
                if((--hd_enc->recordlen)==0)
                {     
                    if(hd_enc->remote_trigged==0)
                    {
                        if((hd_enc->pre_rec>0)&&(hd_enc->rec_type==0))//||(hd_enc->pre_rec==0))//wsy fixed from &&
                        {
                            hd_enc->state=1;
                        }
                        else
                        {
                            hd_enc->state=0;
                        }
                    }
                    else
                    {
                        hd_enc->cutflag=1;
                    }
                    hd_enc->trig=0;
                }
            }
        }
        pthread_mutex_unlock(&hd_enc->mutex);
    }
}



int lock_recent_file(struct hd_enc_struct *hd)//��index.dbȡ������
{
    time_t timenow;
    
    if(hd==NULL)
        return -EINVAL;
        
    timenow=time((time_t *)NULL);
    fileindex_lock_by_time(hd->partition,1, timenow-hd->pre_rec,timenow,-1,hd->channel);
    return 0;
}

/*����һ��¼���¼�
  *hd:����¼�񼰲ɼ��豸�����ݽṹ
  *trig:�����¼�(¼��ԭ��)
  *reclen:ϣ�����ж೤ʱ���¼��(ʵ��¼��ʱ���������ʱ¼��),��0����
  */
int trig_record_event(struct hd_enc_struct *hd,WORD trig,int reclen)
{
    if(hd==NULL)
        return -1;

    
#ifdef SHOW_WORK_INFO
    printf("we want to record %d seconds 'record',trig = 0x%x!\n",reclen,trig);
#endif
    if((trig==0)&&(hd->rec_type==0))
        return 0;
    
    if(hd->enable == 0)//not enabled
    {
        gtloginfo("%d·��Ƶ����,����¼����Ч\n",hd->channel);
        return -1;
    }   

    pthread_mutex_lock(&hd->mutex);

    if(trig==0)//��Ч����������¼��
    {
        if(hd->recordlen<MOTION_DLY_REC+reclen)
            hd->recordlen=MOTION_DLY_REC+reclen;
        if(hd->state==0)
            hd->state=1;
    }
    else
    {
        if((hd->state==2)&&(hd->trig!=trig))
            hd->cutflag=1;//�������״̬��һ�����и��ļ�
        else
            hd->state=2;
        hd->trig=hd->trig|trig;
        reclen+=hd->dly_rec;
        if(hd->recordlen<reclen)
            hd->recordlen=reclen;
    }
    //gtloginfo("test,trig_record_event�����д������\n");
    pthread_mutex_unlock(&hd->mutex);
    return 0;
}

//����Զ�̷����ĸ�����¼��ָ��
int remote_start_record(struct hd_enc_struct *hd,int reclen)
{
    if(hd==NULL)
        return -1;
    if(reclen==0)
        reclen=65535;
    pthread_mutex_lock(&hd->mutex);
    gtloginfo("Զ��Ҫ���ֹ�¼��%d��\n",reclen);
    hd->remote_trig_time=reclen;
#ifdef SHOW_WORK_INFO
    printf("recv rmt record reclen=%d hd->recordlen=%d \n",reclen,hd->recordlen);
#endif
    
    if((hd->remote_trigged!=1)&&(hd->state!=0))
        hd->cutflag=1; 
    hd->remote_trigged=1;
    hd->state=2;
    
    pthread_mutex_unlock(&hd->mutex);
    dump_saveinfo_to_log(1,hd->channel,reclen);
    return 0;
}
//����Զ�̷�����ֹͣ������¼��ָ��
int remote_stop_record(struct hd_enc_struct *hd)
{
    if(hd==NULL)
        return -1;
    gtloginfo("Զ��Ҫ��%d·hqstop\n",hd->channel);
    pthread_mutex_lock(&hd->mutex);
    /*trig=hd->trig;
    rec_len=0;
    (trig!=0)//�б�����Ϣ
    {
        for(i=0;i<TOTAL_TRIG_IN-1;i++)//��������ʱ�Ĵ˴�
        {
            if((trig&(1<<i))!=0)
            {
                newlen=get_ch_alarm_rec_time(i);
                if(rec_len<newlen)
                    rec_len=newlen;//����б����ź���Ӧ��¼�������ź�ָ����ʱ��
            }
        }       
    }
    if(rec_len==0) //Ŀǰû��δ��ı��ش���
    {*/
    if(hd->remote_trigged==1)
        hd->cutflag=1;
    hd->remote_trigged=0;
    if(hd->trig==0)
    {
        if((hd->pre_rec==0)||(hd->rec_type==1))
            hd->state=0;
        else
            hd->state=1;
        hd->recordlen=0;
    }
    
    hd->remote_trig_time=0;
    
    
    /*
    else //Ŀǰ����δ��ı��ش���
    {
        hd->trig=hd->trig&0x3FF;
        if(hd->recordlen<rec_len)
        {
            hd->recordlen=rec_len;
        }
    }*/
    pthread_mutex_unlock(&hd->mutex);
    dump_saveinfo_to_log(0,hd->channel,0);
    return 0;
}

#ifdef RECORD_PS_FILE
    static FILE* OUT_FP = NULL;
    static PS_handle_t *ps_fd;
    static unsigned char head_buf[1024*256]={0};
    int ps_len;
#endif


void fix_adts_header(char* data,int len)
{
	unsigned int obj_type=0;
	unsigned int num_data_block = (len-8)/1024;
	char * adts_header = data+1;
	char channels = 2;
	int rate_idx = 8;

	int frame_length = len-8+7;
	adts_header[0]=0xff;
	adts_header[1]=0xf9;
	adts_header[2]=obj_type<<6;
	adts_header[2]|=(rate_idx<<2);

	adts_header[2]|=(channels&0x4)>>2;
	adts_header[3]=(channels&0x3)<<6;
	adts_header[3]|=(frame_length&0x1800)>>11;
	adts_header[4]=(frame_length&0x1ff8)>>3;
	adts_header[5]=(frame_length&0x7)<<5;

	adts_header[5]|=0x1f;
	adts_header[6]=0xfc;
	adts_header[6]|=num_data_block&0x03;

}


//����hd����Ϣ����һ���ļ�
avi_t * create_record_file(struct hd_enc_struct *hd)
{
    char *p;
    char tname[128];
    int ret;
    char    *v_avitag=NULL;     ///<��Ƶ�����ʽ��AVI���
    avi_t *aviinfo=NULL;
    media_attrib_t *media_attrib=NULL;
    media_format_t *media_format=NULL;
    video_format_t *v_fmt=NULL;
    audio_format_t *a_fmt=NULL;
    int file_errno=0;

    
    if(hd==NULL)
        return NULL;

    if(make_file_name(hd,hd->filename)<0)
    {
        printf("create_record_file make_file_name error!\n");
        return NULL;
    }
    
    
    p=strstr(hd->filename,IMG_FILE_EXT);
    if(p!=NULL)
    {
        strcpy(tname,hd->filename);
        *p='\0';
        strncat(hd->filename,RECORDING_FILE_EXT,100); 
        //rename(tname,hd->filename);
    }
        

    ret=hdutil_create_dirs(hd->filename);//changed by shixin
    if(ret<0)
    {
        file_errno=errno;
        gtlogerr("�޷�Ϊ%s����Ŀ¼,����%d: %s\n",hd->filename,file_errno,strerror(file_errno));
        fix_disk(hd->filename, file_errno);
        return NULL;
    }
        
        //ret=AVIFileOpen(hd->filename,&hd->avi);
    media_attrib=get_venc_attrib_record(hd->channel);
    media_format=&media_attrib->fmt;
    v_fmt=&media_format->v_fmt;
    
    media_attrib=get_aenc_rec_attrib(hd->audiochannel);
    if(media_attrib!=NULL)
    {
        media_format=&media_attrib->fmt;
        a_fmt=&media_format->a_fmt;
  
    }
    //��������
    ret = fileindex_add_to_partition(hd->partition,hd->filename);
    aviinfo=AVI_open_output_file(hd->filename);
    if(aviinfo==NULL)
    {
        
        file_errno=errno;
        printf("can't create avi file %s\n",hd->filename);
        gtlogerr("�޷�����avi�ļ�%s,fopenʧ�� %s\n",hd->filename,strerror(file_errno));
        fix_disk(hd->filename,file_errno);
        return NULL;
    }   
    else 
    {
        
        
        //��������Ƶ����
/*      
        gtloginfo("v_fmt->v_frate is %d\n",v_fmt->v_frate);
        gtloginfo("v_width is %d\n",v_fmt->v_width);
        gtloginfo("v_fmt->v_height is %d\n",v_fmt->v_height);
        gtloginfo("format is %d\n",a_fmt->a_wformat);
        gtloginfo("channel is %d\n",a_fmt->a_channel);
        gtloginfo("sampling is %d\n",a_fmt->a_sampling);
        gtloginfo("nr_frame is %d\n",a_fmt->a_nr_frame);
        gtloginfo("bitrate is %d\n",a_fmt->a_bitrate);
        gtloginfo("bits is %d\n",a_fmt->a_bits);
*/

            switch(v_fmt->format)
            {
                case VIDEO_H264:
                    v_avitag="H264";
                break;
                case VIDEO_MJPEG:
                    v_avitag="MJPG";
                break;
                case VIDEO_MPEG4:
                    v_avitag="divx";
                break;
                default:
                    v_avitag="divx";
                break;

            }

        AVI_set_video(aviinfo,v_fmt->v_width,v_fmt->v_height,v_fmt->v_frate,v_avitag);//"divx" for compressor
        if(a_fmt!=NULL)
        {
            
                #ifdef USE_FFMPEG_LIB   
                      if(audio_save_fmt==4)
                     {
                        AVI_set_audio(aviinfo,2,a_fmt->a_bitrate,a_fmt->a_bits,0x50,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                     }
                      else
                        AVI_set_audio(aviinfo,a_fmt->a_channel,a_fmt->a_bitrate,a_fmt->a_bits,a_fmt->a_wformat,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                #else
            if(get_audio_num()>0)
                    AVI_set_audio(aviinfo,a_fmt->a_channel,a_fmt->a_bitrate,a_fmt->a_bits,a_fmt->a_wformat,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                #endif
                }
        //���avi header
        AVI_fix_header(aviinfo,0);
#ifdef RECORD_PS_FILE
        char mpgname[200];
        sprintf(mpgname,"%s-ps.mpg",hd->filename);
        OUT_FP=fopen(mpgname,"wb");
        if(OUT_FP==NULL)
        {
            gtloginfo("fopen %s failed \n",mpgname);
            return aviinfo;
        }
        ps_fd=ps_require();
        if(ps_fd==NULL)
        {
            gtloginfo("open ps_fd failed\n");
            return aviinfo;
        }
        if(v_fmt->format == VIDEO_H264)
            ret=ps_set_video(ps_fd, V_DECODE_H264, v_fmt->v_width, v_fmt->v_height, v_fmt->v_frate, 300000);
        else    
            ret=ps_set_video(ps_fd, V_DECODE_MPEG4, v_fmt->v_width, v_fmt->v_height, v_fmt->v_frate, 300000);
        if(ret<0)
        {
            gtloginfo("ps_set_video failed\n");
            return aviinfo;
        }
    //zw-add 2012-04-23---->
    ret=fileindex_add_to_partition(hd->partition,mpgname)
    if(ret<0)
    {
      gtlogerr("[%s:%d]error ret=%d,file add [%s] into db error\n",__FILE__,__LINE__,ret,mpgname);
      return aviinfo;
    }
    //zw-add 2012-04-23<----
#endif
        return aviinfo;
    }
}

//�޶��ѱ���ѯ���ļ���,��������Ϊname��newname
//��ʽΪHQ_C00_D20060621142504_L120_T00.ING
int modify_queried_filename(char *name,char *newname)
{
    struct hd_enc_struct *hd;
    int cut_len;
    char *lp,*lk,*lm;
    char buf[100];

    if((name==NULL)||(newname==NULL)) 
        return -1;
    lm=strstr(name,"_C");
    if(lm==NULL)
        return -2;
    hd=get_hdch(atoi(lm)); //��"_C00_"��ȡ
    if(hd==NULL)
        return -1;
    cut_len=hd->max_len;
    lp=strstr(name,"_L");
    if(lp==NULL)
        return -1;
    lp=lp+2;
    lk=index(lp,'_');
    memcpy(buf,lk,30);
    *lp='\0';
    sprintf(newname,"%s%02d%s",name,cut_len,buf);

    return 0;
}

//����hd����Ϣ�ر�һ���ļ�
int close_record_file(struct hd_enc_struct *hd)
{

    struct file_info_struct finfo;
    char newname[100];
    
    avi_t *aviinfo;
    int error=0;

#ifdef ZW_DB_TEST
    struct timeval tv;
    gettimeofday(&tv,NULL);
    gtloginfo("[%ld:%ld]entering [%s:%d]\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
#endif


    if(hd==NULL)
        return -1;
            
    if(hd->filename[0]=='\0')
        return 0;
#ifdef RECORD_PS_FILE
    ps_write_end(ps_fd, head_buf, ps_len, 256*1024);
    printf("ps_len in end is  %x\n", ps_len);
    fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
    fflush(OUT_FP);
    fclose(OUT_FP);
    ps_release(ps_fd);
#endif
    aviinfo=hd->aviinfo;
    if(aviinfo == NULL)
    {
        return 0;
    }
    if(aviinfo->fdes>0)
    {
        AVI_close(aviinfo);
        hd->aviinfo=NULL;   ///added by shixin
#ifdef SHOW_WORK_INFO
        printf("closing avi file:%s\n",hd->filename);
#endif
        if(hdutil_filename2finfo(hd->filename,&finfo)<0)//��fnameת��Ϊfinfo
        {
            printf("close_record_file->filename2finfo error!\n");
            gtloginfo("�ر��ļ�%s����:%s\n",hd->filename,strerror(errno));  
        }
        else
        {
            finfo.len=hd->filelen;
        
            if(finfo2filename(&finfo,newname)<0)//���ļ���Ϣת����newnameʧ��
            {
                perror("close_record_file & make new name \n");
                gtloginfo("�ر��ļ�%s�������ļ�������:%s\n",hd->filename,strerror(errno));  
            }
            else
            {
                if((hd->state==2)||(finfo.trig!=0)||(finfo.remote==1))//�ֹ�¼��򴥷�����ʱ
                {
                    hdutil_lock_filename(newname,newname);//��ס�ļ�
                }
                    
                if(hd->queryflag==1) ////wsy,���б���ѯ�����֮��ı��ļ����ĳ��Ȳ���
                {       
                    modify_queried_filename(newname,newname);
                    hd->queryflag=0;
                }
            
                if(fileindex_rename_in_partition(hd->partition,hd->filename,newname)<0)
                {
                    error=errno;
                    perror("rename file:");
                    gtlogerr("������avi�ļ�%s->%sʧ��,%s\n",hd->filename,newname,strerror(error));
                    fix_disk(hd->filename,error);
                    return -1;
                }

                #ifdef ZW_DB_TEST
                gettimeofday(&tv,NULL);
                gtloginfo("[%ld:%ld]entering [%s:%d]���������ݿ����ļ������\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
                #endif
                
            }
        }
            
    }
    hd->filename[0]='\0';
    hd->cutflag=0;
    hd->filelen=0;
    hd->aviinfo = NULL;

#if ZW_DB_TEST
    gettimeofday(&tv,NULL);
    gtloginfo("[%ld:%ld]leaving [%s:%d]\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
#endif


    return 0;
    
}

//����¼���߳�
int start_audio_thread(struct hd_enc_struct *hd_new)
{
    pthread_attr_t  thread_attr,*attr;
    int ret;

    if(hd_new==NULL)
        return -1;
    if((int)(hd_new->audio_thread_id)>0)//�Ѿ�����
    {
        return 0;
    }
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    ret=pthread_create(&hd_new->audio_thread_id,attr,record_audio_thread,(void*)hd_new);    //������0·¼���߳�
    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    if(ret==0)
    {
        gtloginfo("�ɹ�����%dͨ��¼���߳�,thread_id=%d\n",hd_new->audiochannel,(int)(hd_new->audio_thread_id));
        return 0;
    }
    else 
    {
        printf("start record_audio_thread  problem!!\n");
        gtlogerr("����%dͨ��¼���߳�ʧ��\n",hd_new->audiochannel);
        return -1;
    }   
}


//����������¼���߳�
int start_recordfilethread(struct hd_enc_struct *hd_new)
{

    int ret;
    int intid;
    pthread_attr_t  thread_attr,*attr;
    
    if(hd_new==NULL)
        return -1;
    
    intid=(int)hd_new->thread_id;
    if(intid>0)//�Ѿ�����
        return 0;
    
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    
    pthread_mutex_lock(&hd_new->mutex);
    hd_new->hdsave_mode=1;
    pthread_mutex_unlock(&hd_new->mutex);

    hd_new->threadexit = 0;
    ret=pthread_create(&hd_new->thread_id,attr,record_file_thread,(void*)hd_new);   //������0·ѹ���߳�
    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    if(ret==0)
    {
        gtloginfo("�ɹ�����%dͨ��¼���̹߳�����%s,thread_id=%d\n",hd_new->channel,hd_new->partition,(int)hd_new->thread_id);
        return 0;
    }
    else 
    {
        printf("start record_file_thread  problem!!\n");
        gtlogerr("����%dͨ��¼���߳�ʧ��\n",hd_new->channel);
        return -1;
    }   
}

//ֹͣ������¼���߳�
int stop_recordfilethread(struct hd_enc_struct *hd_new)
{
    int ret;
    int intid;
    int i;
    
    if(hd_new == NULL)
        return -1;
    intid = (int)hd_new->thread_id;
    if(intid <= 0)//�Ѿ���ֹͣ
        return 0;
    
    hd_new->threadexit = 1;



    /*���������˳�*/
    usleep(500000);
    intid = (int)hd_new->audio_thread_id;
    if(intid > 0)
    {
        ret=pthread_cancel(hd_new->audio_thread_id);
        if (ret!=0)
        {
            printf("cancel audio !\n");
            gtlogerr("ȡ����Ƶͨ��%d¼���߳�%dʧ��%d\n",hd_new->audiochannel,hd_new->audio_thread_id,ret);
        }
        printf("ǿ�ƹر���Ƶ %d  !!!!!!\n",hd_new->channel);
    }
    else
    {
        printf("��Ƶ %d  �����˳�!!!!!!\n",hd_new->channel);
    }

        
    intid = (int)hd_new->thread_id;
    if(intid <= 0)
    {
         printf("��Ƶ %d  �����˳� !!!!!!\n",hd_new->channel);
         return 0;
    }
    printf("��Ƶ %d  ǿ���˳�!!!!!!\n",hd_new->channel);
    /*�޷�����ֹͣ����Ҫǿ�ƹر�*/
    pthread_mutex_lock(&hd_new->file_mutex);
    close_record_file(hd_new);
    pthread_mutex_unlock(&hd_new->file_mutex);  
    
    ret=pthread_cancel(hd_new->thread_id);
    if (ret!=0)
    {
        printf("cancel record_file_thread error!\n");
        gtlogerr("ȡ��ͨ��%d¼���߳�%dʧ��%d\n",hd_new->channel,intid,ret);
        return -2;
    }   

    disconnect_video_record_enc(hd_new->channel);
    pthread_mutex_lock(&hd_new->mutex);
    hd_new->thread_id=-1;
    hd_new->hdsave_mode=0;
    pthread_mutex_unlock(&hd_new->mutex);
    gtloginfo("�ɹ�ȡ��¼���߳�");

    return 0;   
}

//ֹͣ������¼���߳�
int stop_Allrecordfilethread()
{
    int ret;
    int intid;
    int i;
    struct hd_enc_struct *hd_new;

    //�����豸һ���м�������¼��ͨ����3022ϵ����2����3024��1��,ip1004��1��
    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    {
        hd_new = get_hdch(i);
        if(hd_new == NULL)
            return -1;
        intid = (int)hd_new->thread_id;
        if(intid <= 0)//�Ѿ���ֹͣ
            return 0;
        
        hd_new->threadexit = 1;

    }

    /*���������˳�*/
    usleep(500000);
    for(i = 0; i < MAX_RECORD_CHANNEL_M; i++)
    {
        printf("����%dͨ������Ƶ�Ƿ�ر� \n",i);
        hd_new = get_hdch(i);
        if(hd_new == NULL)
            return -1;

        intid = (int)hd_new->audio_thread_id;
        if(intid > 0)
        {
            ret=pthread_cancel(hd_new->audio_thread_id);
            if (ret!=0)
            {
                printf("cancel audio !\n");
                gtlogerr("ȡ����Ƶͨ��%d¼���߳�%dʧ��%d\n",hd_new->audiochannel,hd_new->audio_thread_id,ret);
            }
            printf("ǿ�ƹر���Ƶ %d  !!!!!!\n",hd_new->channel);
            hd_new->audio_thread_id = -1;
        }
        else
        {
            printf("��Ƶ %d  �����˳�!!!!!!\n",hd_new->channel);
        }

        
        intid = (int)hd_new->thread_id;
        if(intid <= 0)
        {
             printf("��Ƶ %d  �����˳� !!!!!!\n",hd_new->channel);
             continue;
        }
        printf("��Ƶ %d  ǿ���˳�!!!!!!\n",hd_new->channel);
        /*�޷�����ֹͣ����Ҫǿ�ƹر�*/
        pthread_mutex_lock(&hd_new->file_mutex);
        close_record_file(hd_new);
        pthread_mutex_unlock(&hd_new->file_mutex); 
        printf("׼��kill ��Ƶ %d !!!!!!\n",hd_new->channel);
        ret=pthread_cancel(hd_new->thread_id);
        if (ret!=0)
        {
            printf("cancel record_file_thread error!\n");
            gtlogerr("ȡ��ͨ��%d¼���߳�%dʧ��%d\n",hd_new->channel,intid,ret);
//            continue;
        }   

        printf("�Ͽ���Ƶ %d ������!!!!!!\n",hd_new->channel);
        disconnect_video_record_enc(hd_new->channel);
        pthread_mutex_lock(&hd_new->mutex);
        hd_new->thread_id=-1;
        hd_new->hdsave_mode=0;
        pthread_mutex_unlock(&hd_new->mutex);
        gtloginfo("�ɹ�ȡ��¼���߳�");

    }

    return 0;   
}

//����������ֹ�̵߳ĺ���
//�������ĽṹΪ�ջ���cancel�����г����򷵻�-1;�ɹ��򷵻�0;����start�����г�����-2
int restart_recordfilethread(struct hd_enc_struct *hd_new)
{
    int ret;
    
    if(hd_new==NULL)
        return (-1);
    
    ret=stop_recordfilethread(hd_new);
    if(ret<0)
        return -1;
    sleep(1);       //safe
    hd_new->threadexit = 0;
    start_audio_thread(hd_new);
    usleep(500000);
    ret=start_recordfilethread(hd_new);
     /*�������õĲ���������Ƶ¼��*/

    if(ret<0)
        return -2;
    return 0;    
} 
//�����õ��峡����,¼���߳�
void record_audio_thread_cleanup(void *para)
{
    disconnect_audio_rec_enc(0);
    get_hdch(0)->audio_thread_id=-1;
}
//�����õ��峡����
void record_file_thread_cleanup(void *para)
{
    struct hd_enc_struct *hd_enc;
    struct msg dmsg;
    int qid;

    gtloginfo("test,�峡\n");
    if(para==NULL)
        return;
    
    hd_enc=(struct hd_enc_struct*)para;
    qid=hd_enc->qid;
    //��ն���
    while((msgrcv(qid,&dmsg,100,1,IPC_NOWAIT))>0)
    {       
    }
    //�رյ�ǰ�ļ�
    pthread_mutex_lock(&hd_enc->file_mutex);
    close_record_file(hd_enc);
    pthread_mutex_unlock(&hd_enc->file_mutex);
    hd_enc->watchcnt=0;

    disconnect_video_record_enc(hd_enc->channel);
    change_thread_id(hd_enc->channel,-1);//added by shixin 
    
}

//�ı��߳�id�ĺ���
void change_thread_id(int channel,int id) 
{
    struct hd_enc_struct *hd;
    hd=get_hdch(channel);
    pthread_mutex_lock(&hd->mutex);
    hd->thread_id=id;
    pthread_mutex_unlock(&hd->mutex);
}


//����I֡�����,��ץͼʹ�õ��߳�
void *keyframe_pool_thread(void *hd)
{
    struct hd_enc_struct    *hd_enc=NULL;
    int ret=0;
    struct stream_fmt_struct *frame=NULL;
    int buflen;
    int first_flag=1;
    int old_seq=-1,new_seq=-1;
    int video_flag;
    int stat_err_cnt=0;
    struct pool_ele_struct *freerm=NULL;
    struct pool_ele_struct *recdata=NULL;
    struct timeval *tm=NULL;
    
    if(hd==NULL)
    {
        //remed by shixin change_thread_id(hd_enc->channel,-1);
        return NULL;
    }
    hd_enc=(struct hd_enc_struct*)hd;
    gtloginfo(" start channel %d keyframe_pool_thread...\n",hd_enc->channel);   

//  frame=(struct stream_fmt_struct*)hd_enc->keyframebuf;
//  buflen=hd_enc->fblen-sizeof(struct stream_fmt_struct);//��ֹ���������
    frame=get_video_read_keyframe_buf(get_hqenc_video_ch(hd_enc->channel));
    if(frame==NULL)
    {
        //remed by shixin change_thread_id(hd_enc->channel,-1);
            gtloginfo("ͨ��%d(I)frameΪ�գ�����NULL.......\n",hd_enc->channel);
            sleep(1);
            return NULL;
    }
    buflen=get_video_read_keyframe_buf_len(get_hqenc_video_ch(hd_enc->channel));
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //�ӳ�ȡ����ĳʱ���
//remed by shxin ������¼���̵߳��峡����pthread_cleanup_push(record_file_thread_cleanup,(void *)hd_enc);//�����峡����
    pthread_testcancel();

    /*������ӻ����ֱ���ɹ�*/
    while(1)
    {
        ret=connect_video_enc_keyframe(get_hqenc_video_ch(hd_enc->channel),"hdmodule-pool");
        if(ret==0)
        {
            hd_enc->picerrorcode=0;
#ifdef SHOW_WORK_INFO
            printf("connect videoenc success\n");
#endif
            gtloginfo("%dͨ��I֡������̳߳ɹ�������Ƶ������%d\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            break;
        }
        else
        {
            if(hd_enc->picerrorcode==0)
                {
                    gtloginfo("%dͨ��I֡������߳�������Ƶ������%dʧ��\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
                    hd_enc->picerrorcode=ERR_EVC_NOT_SUPPORT;
                }
            printf("connect video enc failed, ret=%d!!\n",ret);
            sleep(1);
        }
    }

    while(1)
    {
        ret=get_venc_stat_keyframe(get_hqenc_video_ch(hd_enc->channel));
        if(ret==ENC_STAT_OK)
        {
            printf("%dͨ��I֡�������Ƶ������%d״̬����!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            gtloginfo("%dͨ��I֡�������Ƶ������%d״̬����!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            break;
        }
        else
        {
            if(++stat_err_cnt==15)
            {
                printf("%dͨ��I֡�������Ƶ������%d״̬�쳣,stat=%d!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel),ret);
                gtlogerr("%dͨ��I֡�������Ƶ������%d״̬�쳣,stat=%d!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel),ret);
            }
            printf("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret); 
            //gtloginfo("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret);    
        }
        sleep(1);
    }
    
    while(1)
    {
        do  /*����I֡Ϊֹ*/
        {
            ret=read_video_keyframe(get_hqenc_video_ch(hd_enc->channel),frame,buflen,&new_seq,&video_flag);
            if(ret<0)
            {
                printf("read_video_keyframe failed ret=%d\n",ret);
                usleep(100000);
                continue;   
            }
            hd_enc->keyframe_cnt=0;
            if(video_flag==FRAMETYPE_I)
            {
                first_flag=0;
                old_seq=new_seq;
            }
            else
            {
                old_seq++;
                if(old_seq!=new_seq)
                {                   
                    printf("I thread read_video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",old_seq,new_seq,video_flag);
                    //shixin remed if(old_seq!=0)
                    //  gtloginfo("I thread read_video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",old_seq,new_seq,video_flag);
                    old_seq=new_seq;                    
                    first_flag=1;
                    continue;
                }
            }
        }while(first_flag);


        if(is_keyframe(frame))
        {
            if((frame->len+14) > MAX_FRAME_SIZE)//�������,̫���I֡�Ͳ�Ҫ��
            {
                gtloginfo("��I֡���������󣬲����뻺���\n");
                continue;
            }
            
            tm=(struct timeval *)&frame->tv;
            //��I����ڶ���β��
            freerm=get_free_eleroom(get_stream_pool(hd_enc->channel));
            if(freerm==NULL)//changed by shixin �������������ӵ���һ������
            {
                recdata=get_active_ele(get_stream_pool(hd_enc->channel));//ȡ����һ��Ԫ��
                if(recdata==NULL)
                {
                    printf("get eletype failed!!!\n");
                    gtloginfo("�ӻ����%d�л�ȡԪ��ʧ��!\n",hd_enc->channel);
                }
                else
                {
                    ret=free_ele(get_stream_pool(hd_enc->channel),recdata); //�Żػ����                    
                }
                freerm=get_free_eleroom(get_stream_pool(hd_enc->channel));
            }
                
            if(freerm!=NULL)
            {
                set_ele_type(freerm,FRAMETYPE_I);
                memcpy(freerm->element,frame,frame->len+sizeof(struct stream_fmt_struct));//14);    //shixin modified
                ret=put_active_ele(get_stream_pool(hd_enc->channel),freerm);//�ŵ���ЧԪ�ض���β��
                if((ret==0)&&(hd_enc->semflag==1))
                    sem_post(&hd_enc->sem);
            } 

        }
    }
//  pthread_cleanup_pop(0);
    //remed by shixin change_thread_id(hd_enc->channel,-1);
    disconnect_video_enc_keyframe(get_hqenc_video_ch(hd_enc->channel));
    //gtloginfo("test,-1 here\n");
    return NULL;
}

#if 0
/*��������Ƶ�����.����prerec��ʾ�ӵ�ǰʱ����ǰ������
  ��������0,���������ظ�ֵ*/
int connect_media_pre(struct hd_enc_struct *hd_enc,int prerec)
{
    int ret;
    int stat_err_cnt=0;
    int connect_enc_failed=0;
    if(hd_enc==NULL)
        return -1;

    /*���������Ƶ�����ֱ���ɹ�*/
    while(1)
    {
        ret=connect_video_enc(get_hqenc_video_ch(hd_enc->channel),"hdmodule",prerec);
        if(ret==0)
        {
            printf("connect videoenc success\n");
            //gtloginfo("%dͨ��¼���̳߳ɹ�������Ƶ������\n",hd_enc->channel);
            break;
        }
        else
        {
            if(connect_enc_failed==0)
            {
                    gtloginfo("������Ƶ������ʧ��\n");
            }
            connect_enc_failed++;
            printf("connect video enc failed(%d), ret=%d!!\n",connect_enc_failed,ret);
            if(connect_enc_failed==40)
                {
                    gtlogerr("������Ƶ������ʧ��%d��!!!",connect_enc_failed);
                    
                }
            sleep(2);
        }
    }
    while(1)
    {
        ret=get_venc_stat(get_hqenc_video_ch(hd_enc->channel));
        if(ret==ENC_STAT_OK)
        {
            printf("��Ƶ������%d״̬����!\n",get_hqenc_video_ch(hd_enc->channel));
            //gtloginfo("��Ƶ������%d״̬����!\n",get_hqenc_video_ch(hd_enc->channel));
            
            break;
            
        }
        else
        {
            if(++stat_err_cnt==15)
            {
                printf("��Ƶ������%d״̬�쳣,stat=%d!\n",get_hqenc_video_ch(hd_enc->channel),ret);
                gtlogerr("��Ƶ������%d״̬�쳣,stat=%d!\n",get_hqenc_video_ch(hd_enc->channel),ret);
            }
            printf("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret); 
            //gtloginfo("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret);    
        }
        sleep(1);
    }

    //������Ƶ
    ret=connect_audio_rec_enc(0,"hdmodule",prerec);
    if(ret==0)
    {
        printf("connect audioenc success\n");
        //gtloginfo("������Ƶ�������ɹ�\n");
    }
    else
    {
        gtloginfo("������Ƶ������ʧ��\n");
        return 0;
    }
    ret=get_aenc_rec_stat(0);
    if(ret==ENC_STAT_OK)
    {
        printf("��Ƶ������%d״̬����!\n",0);
        //gtloginfo("��Ƶ������%d״̬����!\n",0);
    }
    else
    {
        printf("��Ƶ������%d״̬�쳣,stat=%d!\n",0,ret);
        gtlogerr("��Ƶ������%d״̬�쳣,stat=%d!\n",0,ret);
    }       
    return 0;
}
#endif


static int a_connect_enc[4]={0};//��ǰ�Ƿ���������Ƶ�����
static int connect_enc[4]={0};//��ǰ�Ƿ���������Ƶ�����

#ifdef USE_FFMPEG_LIB
#include <avcodec.h>
#include <avformat.h>
#include <audiofmt.h>
//������������ת��Ϊ˫���������ٳ�4
int conv_8km_2_32ks(char *source,char *target,int srclen)
{   
        unsigned short *s=(unsigned short*)source,*t=(unsigned short*)target;
        int i,j;
        int len=srclen/2;
        int conv_len=0;
        for(i=0;i<len;i++)
        {
                for(j=0;j<8;j++)
                {
                        *t=*s;
                        t++;
                }
                s++;
                conv_len++;
        }
        return conv_len*2*8;
}
static unsigned char write_buf[1024*150];
static int write_place=0;
static int read_place=0;
static int usage=0;
int ffmpeg_write_audio_file(AVCodecContext *c,avi_t *AVI,char *data,int len)
{
    int ret;
    int pkt_size=c->frame_size*4;//һ��ת�����ֽ���
    unsigned char temp_buf[1024];
//    printf("call ffmpeg_write_audio_file len=%d usage=%d write_place=%d read_place=%d\n",len,usage,write_place,read_place);
    if(usage<=0)
    {
        //printf("usage=%d write=%d read=%d!!\n",usage,write_place,read_place);
        memcpy(write_buf,data,len);
        read_place=0;
        write_place=len;
        usage=len;
    }
    else
    {
        memcpy(&write_buf[write_place],data,len);
        write_place+=len;
        usage+=len;
    }
    while(usage>=pkt_size)
    {
        ret = avcodec_encode_audio(c, temp_buf, sizeof(temp_buf), ( const short *)&write_buf[read_place]);
        ret=AVI_write_audio(AVI,temp_buf,ret);
        usage-=pkt_size;
        read_place+=pkt_size;        
    }
    return 0;
}
#endif
//¼�������߳�
void *record_audio_thread(void *hd)
{
#ifdef USE_FFMPEG_LIB
    AVCodecContext *avctx_opts=NULL;
    AVCodec *enc=NULL;
    unsigned char comp_buf[10240];
    unsigned char temp_buf[20480];
    int         frame_size;
#endif
    struct hd_enc_struct    *hd_enc=NULL;
    struct stream_fmt_struct *frame;
    int buflen;
    int ret;
    int old_seq=-1,new_seq=-1;
    int audio_flag;
    int aenc_fail_cnt=0;
    int stat_err_cnt=0;
    int audio_channel;
	int *phead=NULL;

    if(hd==NULL)
    {
        return NULL;
    }
    

    hd_enc=(struct hd_enc_struct*)hd;

    gtloginfo(" start audio thread channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
#ifdef SHOW_WORK_INFO
    printf("enter record audio channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
#endif


    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //�ӳ�ȡ����ĳʱ���
    pthread_cleanup_push(record_audio_thread_cleanup,(void *)hd_enc);//�����峡����
    pthread_testcancel();

    audio_channel = hd_enc->audiochannel;
    frame=get_audio_rec_read_buf(audio_channel);
    buflen=get_audio_rec_read_buf_len(audio_channel);//��ֹ���������
    if(frame==NULL)
    {
        gtloginfo("��Ƶ������%dframeΪ�գ�����NULL.......\n",0);
        sleep(1);
        hd_enc->audio_thread_id=-1;
        return NULL;
    }
#ifdef USE_FFMPEG_LIB
    if(audio_save_fmt==4)
    {
        /* must be called before using avcodec lib */
            avcodec_init();

        /* register all the codecs (you can also register only the codec
           you wish to have smaller code */
            avcodec_register_all();
        /* find the MP2 encoder */
        enc = avcodec_find_encoder(CODEC_ID_MP2);//);
        if (!enc) {
            fprintf(stderr, "codec not found\n");
            exit(1);
        }
        avctx_opts=avcodec_alloc_context();
        avctx_opts->bit_rate = 64000;
        avctx_opts->sample_rate = 32000;
        avctx_opts->channels = 2;
        frame_size= avctx_opts->frame_size;
        /* open it */
        if (avcodec_open(avctx_opts, enc) < 0) {
            fprintf(stderr, "could not open codec\n");
            exit(1);
        }
    }
        printf("audio_save_fmt=%d!!\n",audio_save_fmt);
#endif
    while(!hd_enc->threadexit)
    {   

        //printf("audio_start \n");

        printf("audio wait audio channel %d video:%d,!!!!!!!!!!!!!!!!!!!!\n",hd_enc->audiochannel,hd_enc->channel); 
        pthread_mutex_lock(&hd_enc->audio_mutex);
        pthread_cond_wait(&hd_enc->audio_cond,&hd_enc->audio_mutex);
        pthread_mutex_unlock(&hd_enc->audio_mutex);

        //printf("audio_ connect_audio_rec_enc\n");
        printf("audio connect channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
        /*���������Ƶ�����ֱ���ɹ�*/
        while((!hd_enc->threadexit) && (!a_connect_enc[hd_enc->channel]))
        {//������Ƶ������
            ret=connect_audio_rec_enc(audio_channel,"hdmodule",hd_enc->pre_connect);
            if(ret==0)
            {
                printf("connect audioenc%d success\n",0);
                gtloginfo("������Ƶ������%d �ɹ�!\n",0);
                break;
            }
            else
            {
                if(aenc_fail_cnt==0)
                {
                        gtloginfo("������Ƶ������%dʧ��\n",0);
                }
                aenc_fail_cnt++;
                printf("connect audio enc failed(%d), ret=%d!!\n",aenc_fail_cnt,ret);
                if(aenc_fail_cnt==40)
                {
                    gtlogerr("������Ƶ������ʧ��%d��!!!",aenc_fail_cnt);
                    
                }
                sleep(2);
            }
        }  

        //printf("audio_ connect_audio_rec_enc over\n");
        while(!hd_enc->threadexit)
        {//�ȴ���Ƶ����������
            ret=get_aenc_rec_stat(audio_channel);
            if(ret==ENC_STAT_OK)
            {
                gtloginfo("��Ƶ������%d״̬����!\n",0);
                printf("��Ƶ������%d״̬����!\n",0);
                break;
            }
            else
            {
                if(++stat_err_cnt==15)
                {
                    printf("��Ƶ������%d״̬�쳣,stat=%d!\n",0,ret);
                    gtlogerr("��Ƶ������%d״̬�쳣,stat=%d!\n",0,ret);
                }
                printf("audioenc%d state=%d!!!\n",0,ret);   
            }
            sleep(1);
        }

        printf("read audio frame channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
        old_seq=-1;
        while(connect_enc[hd_enc->channel]&&(!hd_enc->threadexit))//����������Ƶ������
        {

            ret=read_audio_rec_frame(audio_channel,frame,buflen,&new_seq,&audio_flag);
            if(ret<0)
            {
                printf("test,read_audio_frame failed ret=%d\n",ret);
                usleep(100000);
                continue;   //changed by shixin break;
            }
            hd_enc->audio_cnt=0;
            old_seq++;
            if(old_seq!=new_seq)
            {
                printf("read_audio_frame old_seq+1=%d newseq=%d\n",old_seq,new_seq);
                old_seq=new_seq;                    
                continue;
            }
            
            //printf("read_audio_frame ret=%d,len:%d hd_enc->threadexit:%d \n",ret,frame->len,hd_enc->threadexit);

            pthread_mutex_lock(&hd_enc->file_mutex);
            if(hd_enc->aviinfo!=NULL)                   ///added by shixin
            {
#ifdef USE_FFMPEG_LIB   
            if(audio_save_fmt==4)
            {
                ret=conv_ulaw2raw(comp_buf,frame->data,frame->len);
                ret=conv_8km_2_32ks(comp_buf, temp_buf,ret);   
                ffmpeg_write_audio_file(avctx_opts,hd_enc->aviinfo,temp_buf,ret);
            }
            else
                ret=AVI_write_audio(hd_enc->aviinfo,frame->data,frame->len);
#else

			phead = (int*)frame->data;
			if (*phead == 0x77061600)
			{
				//lc add 20150402  aac raw data ,add adts header for avi to play
				fix_adts_header(frame->data,frame->len);
				
				ret=AVI_write_audio(hd_enc->aviinfo,frame->data+8-7,frame->len-8+7);
			}else{
                ret=AVI_write_audio(hd_enc->aviinfo,frame->data,frame->len);
			}
#endif
            }
            pthread_mutex_unlock(&hd_enc->file_mutex);
    
        }
        //��Ƶ�������Ѿ��Ͽ�

        //printf("disconnect_audio_rec_enc hd_enc->threadexit:%d \n",hd_enc->threadexit);
        //ret=disconnect_audio_rec_enc(audio_channel);    //Ҳ�Ͽ���Ƶ������
    }
    printf("audio thread %d run over\n",hd_enc->channel);
    pthread_cleanup_pop(0);
    pthread_mutex_lock(&hd_enc->audio_mutex);
    hd_enc->audio_thread_id=-1;
    pthread_mutex_unlock(&hd_enc->audio_mutex);
    disconnect_audio_rec_enc(audio_channel);
    //printf("audio over here\n");
    return NULL;
}

//�ļ���¼�߳�
void *record_file_thread(void *hd)
{       
    struct hd_enc_struct    *hd_enc=NULL;
    struct stream_fmt_struct *frame=NULL;
    int buflen;
    int first_flag=1;
    int state=0,closeflag=0,newstate=0;//state=0��ʾû��¼�� 1��ʾԤ¼ 2��ʾ����¼��
    int ret;
    int old_seq=-1,new_seq=-1;
    int video_flag;
    int err,disknum;
#ifdef RECORD_PS_FILE
    int frametype;
#endif
    time_t end_last_file=0; //�ϸ��ļ�������ʱ���
    time_t timenow=0;
    avi_t *aviinfo=NULL;


    if(hd==NULL)
    {
        return NULL;
    }

    hd_enc = (struct hd_enc_struct*)hd;
    /*���û��Ӳ�̣�¼���߳�ҲҪ�˳�*/
    disknum =  get_sys_disk_num();
    if(disknum ==  0)
    {
        printf("no disk ,record thread return...\n");
        gtloginfo("û���ҵ�Ӳ�̣�¼���߳��������˳�...");     
        return NULL;
    }

    hd_enc = (struct hd_enc_struct*)hd;
    gtloginfo(" start channel %d record_file_thread...\n",hd_enc->channel);     

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //�ӳ�ȡ����ĳʱ���
    pthread_cleanup_push(record_file_thread_cleanup,(void *)hd_enc);//�����峡����
    pthread_testcancel();
    frame=get_video_record_buf(hd_enc->channel);
    buflen=get_video_record_buf_len(hd_enc->channel);//��ֹ���������
    
    if(frame==NULL)
    {
        change_thread_id(hd_enc->channel,-1);
        gtloginfo("ͨ��%d frameΪ�գ�����NULL.......\n",hd_enc->channel);
        sleep(1);
        return NULL;
    }   
    
start:

    //printf("1111111111111111111111channel %d threadexit=%d\n",hd_enc->channel,hd_enc->threadexit);
    while(!hd_enc->threadexit)
    {   

        newstate=hd_enc->state;
        switch(state)
        {
            case 0://ʲôҲ����Ҫ����
                if(newstate!=0)
                {
                    if(hd_enc->rec_type==0)
                    {
                        hd_enc->pre_connect=0;
                    }   
                    else
                    {
                        timenow=time((time_t *)NULL);
                        if((timenow-end_last_file)>=MOTION_PRE_REC)
                            hd_enc->pre_connect=MOTION_PRE_REC;
                        else
                            hd_enc->pre_connect=(timenow-end_last_file);
                    }
                    ret = connect_video_record_enc_succ(hd_enc->channel,"hdmodule",hd_enc->pre_connect);
                    ret = connect_audio_rec_enc(hd_enc->audiochannel,"hdmodule",hd_enc->pre_connect);//������Ƶ������,�쳣������¼���߳���
                    if(ret == 0)
                    {
                        a_connect_enc[hd_enc->channel] = 1;
                    }
                   
                    state=newstate;
                    if(newstate == 2)
                    {
                        lock_recent_file(hd_enc);
                        
                    }
                    connect_enc[hd_enc->channel] = 1;
                    pthread_mutex_lock(&hd_enc->audio_mutex);
                    pthread_cond_signal(&hd_enc->audio_cond);
                    pthread_mutex_unlock(&hd_enc->audio_mutex);
                }

            break;
            case 1://Ԥ¼ 
                    if (newstate==2) //��������
                        lock_recent_file(hd_enc);
            break;
            case 2://����¼��
            break;
            default:
            break;
        }
        if(connect_enc[hd_enc->channel] != 1)
        {
            sleep(1); //��û�����ӻ���ص������,�����ѭ��ռ��̫��cpu
            continue;
        }
        
        do
        {

            hd_enc->readenc_flag = 1;
            ret=read_video_record_frame(hd_enc->channel,frame,buflen,&new_seq,&video_flag);
            if(ret<0)
            {
                printf(" read video %d record frame failed ret=%d\n",hd_enc->channel,ret);
                usleep(100000);
                continue;
            }
            //printf("video_frame,read:%d,framelen:%d seq:%x,video_flag:%d!\n",ret,frame->len,new_seq,video_flag);
            ret=get_video_record_enc_remain(hd_enc->channel);
            if((ret>=100)&&((ret%10) == 0))//ÿ10����¼һ��
            {
                printf("warn:%d record_file_thread ��%d֡��Ƶû�ж�ȡ!\n",hd_enc->channel,ret);
                gtlogwarn("warn:record_file_thread ��%d֡��Ƶû�ж�ȡ!\n",hd_enc->channel,ret);
            }

            hd_enc->readenc_flag = 0;
            hd_enc->watchcnt=0;

            if(video_flag==FRAMETYPE_I)
            {
                //���ｫfirst_flag=0��Ŀ�������˳�while(first_flag),���򽫻᷵���ظ�����read_video_frame(),ֱ����ȡ����I֡Ϊֹ
                first_flag=0;
                old_seq=new_seq;
            }
            else
            {
                old_seq++;
                if(old_seq!=new_seq)
                {                   
                    printf("read %d video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",hd_enc->channel,old_seq,new_seq,video_flag);
                    old_seq=new_seq;                    
                    first_flag=1;
                    continue;
                }
            }
        }while(first_flag&&(!hd_enc->threadexit));
        /*������I֡*/

        hd_enc->watchcnt=0;     

        closeflag=0;
        if(hd_enc->cutflag)
        {
            //�鿴�ļ��Ƿ���Ҫ�з��ˣ����ļ�����Ԥ���С����Ҫͨ���з����ر��ļ�
            closeflag=1;    
            //gtloginfo("test,cutflag close\n");
        }
        else if ((state!=newstate)&&(state!=0))
        {
            /*¼���״̬�ı䣬��Ӧ���ļ�ҲҪ���Ÿı�*/
            closeflag=1;
            //gtloginfo("test,state close,state=%d,newstate =%d\n",state,newstate);
        }
    
        state=newstate; 

         /*����йرյĶ������������ر�*/
         pthread_mutex_lock(&hd_enc->file_mutex);    
        if((closeflag)&&(hd_enc->aviinfo != NULL))
         {
             close_record_file(hd_enc);
         }
         pthread_mutex_unlock(&hd_enc->file_mutex); 
    
        //Ϊɶʹ������ӿڣ�ʹ��video_flag�򵥵��жϲ�����?
        if(is_keyframe(frame)&&(!hd_enc->threadexit))
        {
            /*��ǰ��֡��Ҫ¼��*/
            if(newstate!=0)
            {
                pthread_mutex_lock(&hd_enc->file_mutex);    
                if(hd_enc->aviinfo == NULL)
                {
                        //�����Ƕ����
                    if(get_disk_free(hd_enc->partition) < get_hd_minval()) 
                    {
#if 0                    
                        /*��ȡ��һ��¼�����*/
                        if(disk_get_next_record_partition(hd_enc->partition) < 0)
                        {
#endif                        
                            /*����ĸ��̶���������Ӳ��û��mount�ϣ��˳�¼�����*/
                            hd_enc->threadexit = 1;
                            pthread_mutex_unlock(&hd_enc->file_mutex); 
                            gtloginfo("ͨ��%d �����ռ䶼���㣬ֹͣ¼���˳�¼���߳�\n", hd_enc->channel);
                            goto start;
#if 0                            
                        }
#endif                        
                        
                    }

                    printf("create_record_file channel %d  path:%s \n",hd_enc->channel,hd_enc->partition);

                    //�����ǰ״̬���ڷǿ���״̬���رպ���Ҫ���¿�һ���ļ�����¼��
                    errno=0;
                    aviinfo=create_record_file(hd_enc);
                    hd_enc->aviinfo = aviinfo;
                    if(aviinfo == NULL)
                    {
                        err=errno;
                        printf("ͨ��%d ����¼���ļ�ʧ��,����avi_tΪ�գ�error = %s\n",hd_enc->channel,strerror(err));
                        if(err!=ENOSPC)
                        {
                            gtloginfo("ͨ��%d ����¼���ļ�ʧ��,����avi_tΪ�գ�error = %s\n",hd_enc->channel,strerror(err));
                        }
                        pthread_mutex_unlock(&hd_enc->file_mutex);  
                        disconnect_video_record_enc(hd_enc->channel);  //added by shixin 
                        sleep(1);
                        state=0;
                        goto start;
                    }
                }

                //printf("read_record_file channel %d  file:%s \n",hd_enc->channel,hd_enc->filename);
                //����ǰ�����I֡д��AVI�ļ���ȥ
                //printf("AVI_write_frame,write framelen:%d seq:%x,video_flag:%d!\n",frame->len,new_seq,video_flag);
                ret=AVI_write_frame(aviinfo,frame->data,frame->len,is_keyframe(frame)); //shixin added 
                pthread_mutex_unlock(&hd_enc->file_mutex); 
#ifdef RECORD_PS_FILE
                if(is_keyframe(frame))
                    frametype = I_FRAME;
                else
                    frametype = P_FRAME;
                ps_len=ps_write_frame(ps_fd, V_STREAM, frame->data, frame->len, head_buf,1024*256);
                //printf("wsytest,ps_len = %d\t,framelen %d\n",ps_len,frame->len);
                fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
                fflush(OUT_FP);
#endif
                continue;
            }
            else
            {
                if(get_venc_attrib_record(hd_enc->channel)!=NULL)
                {
                    ret=disconnect_video_record_enc(hd_enc->channel);
                    connect_enc[hd_enc->channel]=0;
                    end_last_file=time((time_t *)NULL);                         
                    old_seq=-1;
                    new_seq=-1;
                }
            }
                
        }
        

//printf("3333333333333333333333  channel %d threadexit=%d\n",hd_enc->channel,hd_enc->threadexit);        
      //���ﴦ�����һ����������keyframe��������ﱻд�뵽avi�ļ���ȥ
        if((state!=0)&&(!hd_enc->threadexit))
        {
        
            //printf("AVI_write_frame,write framelen:%d seq:%x,video_flag:%d!\n",frame->len,new_seq,video_flag);
            pthread_mutex_lock(&hd_enc->file_mutex);
            if(hd_enc->aviinfo != NULL)
            {
                ret=AVI_write_frame(aviinfo,frame->data,frame->len,is_keyframe(frame)); 
            }
            pthread_mutex_unlock(&hd_enc->file_mutex);
#ifdef RECORD_PS_FILE
            if(is_keyframe(frame))
                frametype = I_FRAME;
            else
                frametype = P_FRAME;
            ps_len=ps_write_frame(ps_fd, V_STREAM, frame->data, frame->len, head_buf,1024*256);
            //printf("wsytest,ps_len = %d\t,framelen %d\n",ps_len,frame->len);
            fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
            fflush(OUT_FP);
#endif
        }
                   
    }
    /*�ر�¼���ļ�*/
    pthread_mutex_lock(&hd_enc->file_mutex); 
    close_record_file(hd_enc);
    pthread_mutex_unlock(&hd_enc->file_mutex); 
    
    pthread_cleanup_pop(0);
    change_thread_id(hd_enc->channel,-1);
    disconnect_video_record_enc(hd_enc->channel);
    printf("record file thread %d run over\n",hd_enc->channel);
    return NULL;
}


