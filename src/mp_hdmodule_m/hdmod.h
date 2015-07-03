#ifndef HDMOD_H
#define HDMOD_H

 //// lsk 2009-9-28  test gtvs3000 real D1  
#define  REAL_D1_TEST_FOR_3000      


#include <gate_cmd.h>
#include "rtpool.h"
#include <ime6410api.h>
#include <AVIEncoder.h>
#include <iniparser.h>
#include <avilib.h>
#include "fileindex.h"
#include "hdutil.h"
#include "sqlite3.h"
#include "mod_socket.h"



#define ALARMLOG_FILE           "/log/alarmlog.txt"
#define ALARMLOG_FILE_0         "/log/alarmlog.txt.0"
#define ALARMLOGFILE_MAX_SIZE  128   //������Ϣ�ļ���󳤶ȣ�kΪ��λ


#ifndef IN
#define IN
#define IO
#define OUT
#endif

struct alarm_log_struct{
BYTE time[24]; //ʱ�䲿��,��<2005-12-14 13:13:13> 
BYTE type[8];  //���Ͳ��֣�ack or alarm
BYTE data[12]; //���ݼ��س����֣���trig:0x0001
};

struct hd_enc_struct{               //�������洢�������ݽṹ
    pthread_mutex_t mutex;              
    pthread_t       thread_id;          //¼���߳�id
    pthread_t       audio_thread_id;    //¼���߳�id
    pthread_t       keyframe_pool_thread_id; //I֡������߳�id
    pthread_mutex_t audio_mutex;        //�Ƿ�������Ƶ�Ļ�����
    pthread_cond_t audio_cond;          //�Ƿ���Ҫ������Ƶ����������
    pthread_mutex_t file_mutex;         //¼���ļ���
    struct takepic_struct current_takepic; //��¼��������ץͼ�Ľṹ
    int hdsave_mode;                //������¼����ģʽ 1��ʾ��Ҫ�������� 0��ʾ��Ҫֹͣ
    int audiochannel;               //��Ƶ¼��ͨ��
    int channel;                    //��¼ͨ����
    int rec_type;                   //Ԥ¼ģʽ,0ΪһֱԤ¼,1Ϊ�������ƶ�¼��ģʽ
    int pre_rec;                    //Ԥ¼ʱ��(��)
    int dly_rec;                        //��ʱ¼��ʱ��
    int max_len;                    //�ļ��зֳ���(��)
    int pre_connect;                    //��Ҫ��ǰ���ӵ�����(��ʱ����)
    int del_typ;                     //ɾ���ķ�ʽ��0Ϊһֱɾ��1Ϊ��һ��ɾһ��
    int state;                      //��ǰ״̬0��ʾ���� 1��ʾ����Ԥ¼2��ʾ���ڱ���¼��
    char filename[100];             //��ǰ¼���ļ���
    //AVIVarHeader avi;                 //��ǰ¼���ļ���Ϣ
    avi_t * aviinfo;                    //��ǰ¼���ļ���Ϣ
    int recordlen;                  //��ǰ¼���¼���Ҫ������ʱ��(��),û���Ӽ�1����0��ʾ¼��ֹͣ
    int filelen;                        //��ǰ�ļ��ĳ���(��)
    int trig;                       //��ǰ�Ĵ���״̬ 
    int cutflag;                        //�ļ��и��־
    int qid;                        //����id
    int keynumber;                  //���ڴ������е�key
    int bitrate;                    //¼������ʣ�kbpsΪ��λ
    int watchcnt;           //���ڼ���¼���̴߳ӻ����ȡ���ݵļ�����
    int keyframe_cnt;       //���ڼ���keyframe�̴߳ӻ����ȡ���ݵļ�����
    int audio_cnt;          //���ڼ�����Ƶ������״̬
    int readenc_flag;       //¼���߳����ڴӻ���ض����ݵı�־
    int picerrorcode;       //��¼������г��ֵı��������󣬹�ץͼ��
    int takingpicflag;      //��ͨ���Ƿ���ץͼ
    int pictime;             //ץͼ�����˵�����
    int timemax;             //ץͼ���������ж�����
    pthread_t takepic_thread_id;    //ץͼ�̵߳�id
    int queuenumber;             //��������Ŀ
    sem_t sem;
    int semflag;
    //int restart6410times;
    struct pool_head_struct streampool; //�����..
    char devname[30];       //�豸�ڵ�,��"/dev/IME6410_D1"
    int queryflag; //���ļ�����ѯ������1���ر�ʱ�ǵøı����ļ�����
    //int create_file_flag; //�����пռ䴴���ļ���־
    int remote_trig_time; //�ֹ�¼��ʱ��,Ϊ0��¼
    int remote_trigged; //�ֹ�¼�񴥷�״̬,1Ϊ����,0Ϊ�޴���
    int alarmpicflag;       //����ץͼ��������1������0
    int alarmpic_required;  //��Ҫ����ץͼ��1��������0
    char alarmpic_path[60];     //����ץͼ����·����"/picindex/alarmpic.txt"
    char partition[16];         //��ǰ¼���ļ����ڷ�������"/hqdata/hda3"
    int enable;                 //��·��Ƶ�Ƿ���Ч������Ч������¼��
    int threadexit;           //�߳��˳������������߳��˳�������1
};



struct msg
{
        long msg_type;
        char name[100];
};

typedef struct {
gateinfo                gate;
struct takepic_struct   takepic;
}takepic_info;

//¼�������߳�
void *record_audio_thread(void *hd);
//����¼���߳�
int start_audio_thread(struct hd_enc_struct *hd_new);
void get_trig_status(void);
void get_save_status(void);
void dump_clearinfo_to_log(void);
struct pool_head_struct *get_stream_pool(int channel);
int init_hdenc(void);
struct hd_enc_struct    *get_hdch(int channel);
void hd_second_proc(void);
void *record_file_thread(void *hd);
void *takepic_thread(void * takepic);
//int remote_cancel_alarm(DWORD trig);
//int trig_record_event1(struct hd_enc_struct *hd,WORD trig,int reclen);
//����Զ�̷����ĸ�����¼��ָ��
int remote_start_record(struct hd_enc_struct *hd,int reclen);
//����Զ�̷�����ֹͣ������¼��ָ��
int remote_stop_record(struct hd_enc_struct *hd);
int query_record_index(char *indexname,int ch,time_t start,time_t stop,int trig);
int start_recordfilethread(struct hd_enc_struct *hd_new);
int stop_recordfilethread(struct hd_enc_struct *hd_new);
int restart_recordfilethread(struct hd_enc_struct *hd_new);
int get_hd_minval(void);
int usr_take_pic(gateinfo *gate, struct takepic_struct *takepic);
void change_thread_id(int channel,int id);
struct compress_struct *get_encoder(int channel);
//��ȡ������ʾ��hdmod״̬
DWORD get_hdmodstatint(void);
int  get_time_before(struct timeval *timenow, int diff_in_msec,struct timeval *timebefore);
int clear_hdmod_trig_flag(int channel);
//��ȡ����ץͼ��־
int get_alarmpicflag(int channel);
//��ȡ��Ҫ����ץͼ��־
int get_alarmpic_required(int channel);
//��ȡ���ڴ���ץͼ��־
int get_takingpicflag(int channel);
int set_takingpicflag(int value, int ch);
int set_alarmpic_required(int value, int ch);
int set_alarmpicflag(int value, int ch);
/*����һ��¼���¼�
  *hd:����¼�񼰲ɼ��豸�����ݽṹ
  *trig:�����¼�(¼��ԭ��)
  *reclen:ϣ�����ж೤ʱ���¼��(ʵ��¼��ʱ���������ʱ¼��),��0����
  */
int trig_record_event(struct hd_enc_struct *hd,WORD trig,int reclen);
//�������ļ��ж�ȡ������¼����ز���
int read_hqsave_para_file(char *filename,char *section, int channel);
int init_hdenc_ch(int channel);
int set_sys_encoder_flag(int encno,int inst_flag,int errnum);

//����hd����Ϣ�ر�һ���ļ�
int close_record_file(struct hd_enc_struct *hd);
//����I֡�����,��ץͼʹ�õ��߳�
void *keyframe_pool_thread(void *hd);

avi_t * create_record_file(struct hd_enc_struct *hd);


int convert_old_ing_files(void);


void hd_playback_en(void);
void hd_playback_cancel(void);

#endif



