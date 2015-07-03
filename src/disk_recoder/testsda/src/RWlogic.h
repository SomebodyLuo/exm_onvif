/*
 * RWlocic.h
 * Ӳ������Ķ�д�߼�����
 *  Created on: 2013-11-6
 *      Author: yangkun
 */

#ifndef RWLOCIC_H_
#define RWLOCIC_H_

#include <string>
#include "Blocks.h"
#include "YearBlocks.h"
#include "DayBlocks.h"
#include "SecBlocks.h"
#include "FrameQueue.h"
#include <queue>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <fstream>
#include "MultMediaBlocks.h"
using std::queue;
using  std::string;
namespace gtsda
{
class RWlogic;



enum rw_type
{
	read_type  = 1, /*��Ӳ�̶�������*/
	write_type = 2, /*������д��Ӳ��*/
	cmd_type   = 3, /*��������*/
	read_media_data  =4   /*�ӻ�����������Ҫ������Ƶ����*/
};
struct pthread_arg
{
	rw_type type;		//��д����
	int channel;		//ͨ����ֻ��read_media_dataʱ����
	RWlogic *wr;
};
//����˿�
#define CMD_PORT 60000
#define MSG_HEAD  0x5a5aa5a5
enum msg_type
{
	request_time = 0xa5a5a501,/*����¼��ʱ���*/
	cmd_play	 = 0xa5a5a502,/*���Ͳ�������*/
	cmd_ret_ok   = 0xa5a5a503,/*���سɹ�*/
	cmd_ret_err  = 0xa5a5a504  /*����ʧ��*/
};
struct msg_st
{
	unsigned msg_head;
	msg_type msgtype;
	unsigned int msg_len; 		/*��Ϣ����*/
}__attribute__ ((packed));
class RWlogic
{
public:
	static RWlogic* newRW(bool bMarkWR=true/*write*/);
	virtual ~RWlogic();
	void start(int starttime=0/*��ȡ��ʱ�򣬶��Ŀ�ʼʱ�䣬�˲���ֻ�ڶ���ʱ����Ч*/);
	int Init();
	void Stop(){bIsRun = false;};
	bool IsRun(){return bIsRun;}
	//д
	int writedata();
	int maybe_cover_dayblock(long long blocks, long long seek);
	void format();
	//��Ӳ��
	//�����������ݣ�������Ч��ʱ���
	int read_disk_print_record_time(string &timestring);
	//��ָ����ʱ����ڲ�ѯ��Ч���ݣ���������Ч��ʱ���
	int get_time_index(int timestart,int timeend, string &timeindex);
	//����һ�������
	static void printOneDay(const struct day_block *dDayData, int todaytime,int iStartTime , int iEndTime, string &timestring);
	long long  is_in(int starttime);
	//����������
	void cmd_proc();

	//������Ƶ��������������
	void read_pool_proc(int channel);
	static void write_for_rtsp(int )
	{

		static int times;
		cout << "size: " << frame_queue_read_video1.size()<< endl;
		if(frame_queue_read_video1.size()>0)
		{
			if( times++ > 3){cout << "write error!!\n";exit(1);}
			cout << "size: " << frame_queue_read_video1.size()<< endl;
			Blocks *bs = frame_queue_read_video1.pop();
			//ttt();
			cout << "buff size: "<< bs->GetSize() << endl;
			 dw->getdata( (const char *)bs->GetBuf(),bs->GetSize() );
			// ttt();
			dw->writedata();
			 times=0;
			 delete bs;
			 //ttt();
			 usleep(40000);
		}
		else
			ttt();
	}
	#define REALTIME
	void init_sigaction(void)
	{
		struct sigaction act;

		act.sa_handler = write_for_rtsp;
		act.sa_flags   = 0;
		sigemptyset(&act.sa_mask);
	#ifdef REALTIME
		sigaction(SIGALRM, &act, NULL);
	#else
		sigaction(SIGPROF, &act, NULL);
	#endif
	}
	/* init */
	void init_time(void)
	{
		struct itimerval val;

		val.it_value.tv_sec = 0;
		val.it_value.tv_usec = 40000;
		val.it_interval = val.it_value;
	#ifdef REALTIME
		setitimer(ITIMER_REAL, &val, NULL);
	#else
		setitimer(ITIMER_PROF, &val, NULL);
	#endif
	}
	//�߳����
	static void * create_thread(void *args);
	void printids(const char *s);

private:
	RWlogic(bool bMarkWR);
	YearBlocks *yb;
	DayBlocks *db;
	DayBlocks *dbread;
	DayBlocks *dbbac;//day_bac_block
	//SecBlocks *hb;
	//DataRead *dr;
	static DataWrite *dw;
	static char   *hd;
	bool bMarkWR;/*true write;  false read*/
	bool bIsRun;
	long long HdSize;//num of blocks
	static FrameQueue<Blocks *> frame_queue_read_video1;
	static FrameQueue<Blocks *> frame_queue_read_audio1;
	//��Ӳ��
	long long readseek;
	int starttime;
	void setreadseek(long long seek){readseek=seek;};
	//�߳����
	pthread_t read_pid,write_pid ,cmd_pid;
	//��
	pthread_mutex_t lock_start_read_again;
	pthread_cond_t start_read, stop_read;
	//��Ϣ����
	int msgid;

	//����д��Ӳ��
private:
	//��·����Ƶ�������Ԥ�������ݰ�ͨ�����б���
	FrameQueue<MultMediaBlocks *> mb_video_queue[MAX_CHANNEL];
	FrameQueue<MultMediaBlocks *> mb_audio_queue[MAX_CHANNEL];
	pthread_mutex_t lock;
	pthread_cond_t start_again;



/*************************************************************************
 *	��·����Ƶ������
 *	ÿ·����Ҫ���������̣߳�һ����Ӳ�̶����ݷŵ������У�һ���Ӷ����ж����ݷ���
*************************************************************************/
public:
	class ReadDisk
	{
#define TESTFRAMERATE	//����֡��
//#define WRITEHD			//�Ѵ�Ӳ�̶�����������Ƶ���ݱ�����ļ�
//#define NETSEND			//�Ѵ�Ӳ�̶�����������Ƶ����ͨ�����緢����

	private:
		long long readseek;
		unsigned int timecount;
		//������Ƶ����
		FrameQueue<Blocks *> video_queue;
		FrameQueue<Blocks *> audio_queue;
		//MediaPoolWrite *audio_pool;
		//MediaPoolWrite *video_pool;
		//��¼�м���ʵ��
		static unsigned int num;
#ifdef TESTFRAMERATE
		//֡�ʿ���
		int framenum;
		struct timeval oldtime,nowtime;
#endif

		//�߳�����ͬ��
		pthread_mutex_t lock;
		pthread_cond_t notfull;
		pthread_cond_t startplay;
		//���ſ���
		int channel;
	public:
		void set_notful()
		{
			//pthread_mutex_lock(&lock);
			pthread_cond_signal(&notfull);
			//pthread_mutex_unlock(&lock);
		};
		void start_play()
		{
			//pthread_mutex_lock(&lock);
			isRead = true;
			pthread_cond_signal(&startplay);
			//pthread_mutex_unlock(&lock);
		};
	public:
		//��¼���ǵڼ���
		unsigned int thenum;

		int get_date(char *data, int *size,int type,int *isi);
		//����ʱ�����
		int framecount;
		int count;
		bool isRead;
#ifdef NETSEND

		int state; //��ǰ�ı�״̬0��ʾû��׼���ã�1ռ����Դ��2׼������
		int oper;
		int peeraddr[32]; //���ӵĶԶ˵�ַ
		short peerport; //���ӵĶԶ˶˿�
		int speed;
		int socket; //���ط��͵�socket
#endif





		//�߳����
		pthread_t ntid;
		ReadDisk(long long seek);
		~ReadDisk();
		//��
		void read_proc();
		int readdataprocess(SecBlocks *video, SecBlocks *audio);
		void setread(unsigned int channel,long long seek,int framenum=0)
		{
			this->channel=channel;
			readseek=seek;
			framecount=framenum;
		};
		FrameQueue<Blocks *> *get_audio(){return &audio_queue;};
		FrameQueue<Blocks *> *get_video(){return &video_queue;};
		//�Ѷ����ڴ������ͨ�����緢�ͳ�ȥ����ֱ��дӲ��
		void write_proc();

		//�������Ƶ������
		void clear(){video_queue.clear();audio_queue.clear();set_notful();};

	};

};

} /* namespace gtsda */
#endif /* RWLOCIC_H_ */
