#include "mp_hdmodule.h"
#include "hdmod.h"
#include <signal.h>
#include <devinfo.h>
#include "process_modcmd.h"
#include <commonlib.h>
#include <venc_read.h>
#include <gtthread.h>
#include <aenc_read.h>
#include "diskinfo.h"
#include "mpdisk.h"
#include "hdutil.h"


//��ȡһ��Ĭ�����Խṹ
//������Ӧ���ͷ�
int get_gtthread_attr(pthread_attr_t *attr)
{
    int rc;
    if(attr==NULL)
        return -1;
    memset((void*)attr,0,sizeof(pthread_attr_t));
    rc=pthread_attr_init(attr);
    if(rc<0)
        return -1;
    rc=pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);//����״̬
    rc=pthread_attr_setschedpolicy(attr,SCHED_OTHER);
    return 0;
    
}


void second_proc(void)
{
    while(1)
    {
        sleep(1);
        hd_second_proc();
    }
}



void dump_sysinfo(void)
{
    return;
    //FIXME ���Բ���
}

//����־�ϼ�¼�˳�����
static void exit_log(int signo)
{
    switch(signo)
    {
        case SIGPIPE:
            printf("hdmodule process_sig_pipe \n"); 
            return ;
        break;
        case SIGTERM:
            gtloginfo("hdmodule ��kill,�����˳�!!\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGKILL:
            gtloginfo("hdmodule SIGKILL,�����˳�!!\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGINT:
            gtloginfo("hdmodule ���û���ֹ(ctrl-c)\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGUSR1://���ϵͳ��Ϣ��ָ���ļ�
            dump_sysinfo();
        break;
        case SIGSEGV:
            gtloginfo("#########hdmodule �����δ���#########\n");
            stop_Allrecordfilethread();
            close_all_res();
            printf("hdmodule segmentation fault\n");
            exit(0);
        break;
    }
    return;
}

#ifdef FOR_PC_MUTI_TEST
#include "pc_multi_test.c"
#endif
int process_opt_h(void)
{
    printf("¼��������version:%s\n",VERSION);
    printf("�÷�:hdmodule [OPTION] [argument]\n");
    printf("OPTION ѡ��˵��\n");
    printf("-h:��ʾ������Ϣ\n");
       printf("-v:��ʾ�汾��Ϣ���˳�����\n");
    return 0;
}
int process_argument(int argc,char **argv)
{
    int oc;
    if(argc<2)
    {
        return 0;
    }
    printf("*************************************************\n");
    while((oc=getopt(argc,argv,"hv"))>=0)
        {
                switch(oc)
                {
            case 'h':
                process_opt_h();
                exit(0);
            break;
                     case 'v':
                            printf("hdmodule version:%s\n",VERSION);
                            create_lockfile_save_version(HDMOD_LOCK_FILE,VERSION);
                            printf("*************************************************\n");
                            exit(0);
                     break;
            default:
            break;
                }
        }

    printf("*************************************************\n\n\n");
    return 0;
}

int main(int argc,char *argv[])
{

    pthread_attr_t  thread_attr,*attr;
    int lock_file;
    char pbuf[100];
    int i,ret;

    
#ifdef FOR_PC_MUTI_TEST
    gen_multi_devices();
#endif
    gtopenlog("hdmodule");
    setsid();
    process_argument(argc,argv);    
    lock_file=create_and_lockfile(HDMOD_LOCK_FILE);
    if(lock_file<=0)
    {
        printf("hdmodule module are running!!\n");
        gtlogerr("hdmoduleģ�������У���������Ч�˳�\n");       
        exit(0);
    }   
    sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
    write(lock_file,pbuf,strlen(pbuf)+1);//�����̵�id�Ŵ������ļ���
    //��������ж�ģ���Ƿ��Ѿ�ִ�еĹ���
    #ifdef FOR_PC_MUTI_TEST
    SetOutputTty(lock_file);// �����Ӱ�䵽��һ��������
    #endif

    signal(SIGKILL,exit_log);
    signal(SIGTERM,exit_log);
    signal(SIGINT,exit_log);
    signal(SIGSEGV,exit_log);
    signal(SIGPIPE,exit_log);
    printf("����hdmodule(ver:%s).......\n",VERSION);
    gtloginfo("����hdmodule(ver:%s).......\n",VERSION);
#if EMBEDED
    system("rm -f /tmp/*.widx");//����֮ǰ����ʱ�ļ�
#endif
    init_devinfo();     //��ȡ�豸��Ϣ��guid��
    ret = init_video_record_enc_all();//��ʼ����Ƶ�����
    if(ret != 0)
    {
        gtloginfo("init_video_record_enc error\n");
    }
    ret = init_audio_rec_enc_all();
    if(ret != 0)
    {
        gtloginfo("init_video_record_enc error\n");
    }
    init_com_channel();
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;


    //fileindex_init_filelock();  //zw-add
    InitAllDabase();

    creat_hdmodule_modsocket_thread();


    init_hdenc();   //Ӧ����init_tw2824()֮ǰ���� ����·������¼�����Ƶͨ����ʼ�������
  
    get_trig_status(); //ȡ�ò�����trig״̬
    get_save_status(); //ȡ�ò�����save״̬



    //��ת��ing�ļ���Ȼ���ٽ���Ӳ�̿ռ��ת���������������ʣ��ռ��С�Ļ�hdmodule�ͻ������������ѭ���У���diskman�Ǳ��ֲ��ܴ����ݿ��ļ���
    //�ҳ�����ing=0�Ŀ�ɾ���ļ�����Ϊ��ʱ������кܶ�ing=1���������������Ҫ�Ƚ���Щing�ļ�ת��Ϊavi�ļ���Ȼ����ɾ����
    convert_old_ing_files();


    //�����豸һ���м�������¼��ͨ����3022ϵ����2����3024��1��,ip1004��1��
    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    {
        if(get_hdch(i)->enable == 1)
        {

             /*�������õĲ���������Ƶ¼��*/
            start_audio_thread(get_hdch(i));
            usleep(500000);
            start_recordfilethread(get_hdch(i));
            usleep(500000);
        }
    }

    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    signal(SIGUSR1,exit_log);
    second_proc();
    exit(1);
}


