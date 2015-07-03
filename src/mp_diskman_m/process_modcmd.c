#include "mp_diskman.h"
#include <mod_com.h>
#include "diskmanager.h"
#include "gt_com_api.h"
#include <gt_errlist.h>
#include "devinfo.h"
#include "hdutil.h"
#include "process_modcmd.h"
#include "mod_socket.h"
#include "mod_cmd.h"


static int  com_fd= -1; //���ͺͽ��������udp socket
static pthread_t modsocket_thread_id=-1;

//��ʼ����������ͨѶ������ͨ��
int init_com_channel(void)
{
    com_fd  =   mod_socket_init(0,0);   
    return 0;
}
//����״̬��������
int send_state2main(void)
{
    
    DWORD *state;   
    DWORD socketbuf[20];
    mod_socket_cmd_type *cmd;
    pid_t *pid;
    
    cmd=(mod_socket_cmd_type *)socketbuf;
    cmd->cmd    =   DISKMAN_STATE_RETURN;
    cmd->len    =   4+sizeof(pid_t);
    pid=(pid_t*)cmd->para;
    *pid=getpid();
    state=(DWORD*)&cmd->para[sizeof(pid_t)];
    *state=get_diskmanstatint();

        #if 0
        //2011-06-21 zw-modified-back
    if((*state&0x01)==0x01)
    {
        //ӦlxҪ���޸ĵ�û��Ӳ��ʱ ������zw-modify-2010-12-21
        return 0;
    }
        #endif
    return mod_socket_send(com_fd,MAIN_PROCESS_ID,DISKMAN_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
}



/*
  *��diskman���¶�ȡ����
*/
int refresh_diskman_para(void)
{
    read_diskman_para_file(IPMAIN_PARA_FILE);
    return 0;
}




/*************************************************
�������ƣ�usr_lock_file_time_cmd
��Ҫ�������������ط����İ�ʱ�����ļ���������úͷ���
���룺    �ļ�������fd,��������ṹָ��*cmd
            env,enc,���ܺ�ǩ���㷨
�����    ����ֵΪ0��ʾ��ȷ��-1��ʾ����
�޸���־��wsy@Jan11,2006����
*************************************************/

static int usr_lock_file_time_cmd(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
    struct usr_lock_file_time_struct *locktime; //����������ṹ
    int ret,result; //��¼����ֵ
    if( (gate == NULL) || (cmd == NULL) || (cmd->cmd != USR_LOCK_FILE_TIME) )
    {
        return -1;  
    }
    locktime = (struct usr_lock_file_time_struct *)cmd->para;

    gtloginfo("�յ����ط����ӽ������%04d-%02d-%02d-%02d-%02d-%02d��%04d-%02d-%02d-%02d-%02d-%02d,ģʽ%d,ͨ��%d\n", locktime->start.year,locktime->start.month,locktime->start.day,locktime->start.hour,locktime->start.minute,locktime->start.second,locktime->stop.year,locktime->stop.month,locktime->stop.day,locktime->stop.hour,locktime->stop.minute,locktime->stop.second, locktime->mode, locktime->lock_ch);
    
    if(cmd->en_ack != 0)
    {
        send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,RESULT_SUCCESS,gate);
    }

    sleep(1);//�����ļ���û�з֣�������ӽ���
            
    //����lock_file_by_time�����������ؽ���͵�����
    ret = usr_lock_file_time(HDSAVE_PATH,locktime);
    if(ret >= 0)
    {
        result = RESULT_SUCCESS;
    }
    else
    {   
        result=ERR_DVC_INTERNAL;
    }
    gtloginfo("��ʱ��μӽ���������%d,%s\n", result,get_gt_errname(result));

    cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2;
    
    return 0;
    
}



/*************************************************
�������ƣ�lock_file_time
��Ҫ������������ģ�鷢���İ�ʱ���ļ���������úͷ���
���룺    cmd,ͨ��socketͨ������������
�����    ����ֵΪ0��ʾ��ȷ��-1��ʾ����
�޸���־��wsy@Jan11,2006����
*************************************************/
int  lock_file_time(mod_socket_cmd_type *cmd)
{
    
    struct usr_lock_file_time_struct *locktime; //����������ṹ
    int ret,result; //��¼����ֵ
    
    if( (cmd == NULL) || (cmd->cmd != LOCK_FILE_TIME) )
    {
        return -1;  
    }

    locktime = (struct usr_lock_file_time_struct *)cmd->para;

    gtloginfo("�յ���ʱ�μӽ������%04d-%02d-%02d-%02d-%02d-%02d��%04d-%02d-%02d-%02d-%02d-%02d,ģʽ%d,ͨ��%d\n", locktime->start.year,locktime->start.month,locktime->start.day,locktime->start.hour,locktime->start.minute,locktime->start.second,locktime->stop.year,locktime->stop.month,locktime->stop.day,locktime->stop.hour,locktime->stop.minute,locktime->stop.second, locktime->mode, locktime->lock_ch);
    
    if(cmd->gate.gatefd > 0) //��Ҫ����
        send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,RESULT_SUCCESS,&cmd->gate);
    
    //����lock_file_by_time�����������ؽ���͵�����
    ret = usr_lock_file_time(HDSAVE_PATH,locktime);
    if(ret >= 0)
    {
        result = RESULT_SUCCESS;
    }
    else
    {   
        result=ERR_DVC_INTERNAL;
    }
    gtloginfo("��ʱ��μӽ���������%d,%s\n", result,get_gt_errname(result));

    return result;
    
}


/*************************************************************************
 * ������������ת����������������
*************************************************************************/

//������������ת����������������
static int process_gate_cmd(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
    switch(cmd->cmd)
    {
        case USR_LOCK_FILE_TIME://��ָ��ʱ��εĸ������ļ����������
            gtloginfo("recv a USR_LOCK_FILE_TIME cmd!\n");
            usr_lock_file_time_cmd(gate,cmd);
        break;
        default:
            printf("diskman recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
            gtloginfo("diskman recv a unknow bypass cmd 0x%04x\n",cmd->cmd);            
            send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT,gate);
        break;
    }
    
    return 0;
    
}


/*************************************************************************
 * ������������ת����������������
*************************************************************************/



static int process_modsocket_cmd(int sourceid, mod_socket_cmd_type *modsocket)
{

    int rc;
    rc=0;
    
    if(modsocket == NULL)
        return -EINVAL;
    
    switch (sourceid)
    {
        case MAIN_PROCESS_ID:   
            switch(modsocket->cmd)
            {
                case GATE_BYPASSTO_MOD_CMD://��������ת����������������
                    rc= process_gate_cmd(&modsocket->gate,(struct gt_usr_cmd_struct *)&modsocket->para);
                    //para�Ŀ�ͷ4���ֽڴ�ŵ��������̶����ص�������
                break;  
                case MAIN_QUERY_STATE:
                    gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
                    send_state2main();
                break;
                case MAIN_REFRESH_PARA:
                    gtloginfo("recv MAIN_REFRESH_PARA cmd !\n");
                    refresh_diskman_para();
                break;
                case UNLOCK_FILE:
                    gtloginfo("recv UNLOCK_ALL_FILES cmd !\n"); 
                    hdutil_lock_all_files(0);
                break;
                case LOCK_FILE_TIME:
                    gtloginfo("recv a USR_LOCK_FILE_TIME cmd!\n");
                    lock_file_time(modsocket);  
                break;
                default:
                    printf("diskman recv a unknown cmd 0x%x:",modsocket->cmd);
                    gtloginfo("diskman recv a unknown cmd 0x%x:",modsocket->cmd);
                break;
            }
            
        break;
        default:    break;
    }
    return 0;
}



int creat_diskman_modsocket_thread(void)
{
    return creat_modsocket_thread(&modsocket_thread_id,com_fd,DISKMAN_ID,"diskman",process_modsocket_cmd);
}




