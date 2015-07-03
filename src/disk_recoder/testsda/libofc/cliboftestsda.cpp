/*
 * cliboftestsda.cpp
 *
 *  Created on: 2014-2-8
 *      Author: yangkun
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <string>

#include "gtlog.h"

#include "RWlogic.h"

#include "cliboftestsda.h"

using namespace gtsda;
gtsda::RWlogic *rdl;
gtsda::RWlogic::ReadDisk *rd[SDA_READ_NUM];
void *thr_fn(void *arg)
{
	static int i;
	memcpy(&i, arg, 4);
	std::cout << "sda pthread  " << i << " create success!!!" <<  std::endl;
	rd[i]->read_proc();
	rd[i]->thenum = i;
	return NULL;
}
//�ж��û����Ƿ�����
bool inline is_userno(int userno)
{
	if(userno>=0 && userno <SDA_READ_NUM)
		return true;
	else
		return false;
}

//��ʼ����Ӳ�̿�
int sda_init()
{
	rdl= gtsda::RWlogic::newRW(false/*read*/);
	rdl->Init();
	//���ȴ���READ_NUM�����߳�
	int i,*j;
	int err;
	for(i=0; i<SDA_READ_NUM; i++)
	{
		j = new (int);
		*j = i;
		rd[i] = new gtsda::RWlogic::ReadDisk(0);
		err = pthread_create(&rd[i]->ntid, NULL, thr_fn, j);
		if (err != 0) {
			fprintf(stderr, "can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}
	return 0;
}

//��ȡ¼��ʱ���
int query_record_section(int channel,int time_start,int time_end,\
		/*IN OUT*/char * time_sec_buf,int time_sec_buf_size)
{
	string str;
	int ret;
	ret=rdl->get_time_index(time_start,time_end,str);
	cout << "query record section: " << str << endl;
	int size=str.size();
	size=size<time_sec_buf_size?size:time_sec_buf_size;
	memcpy(time_sec_buf, str.c_str(), size);
	return ret;
}
//��ȡ����ͨ��
int sda_get_idle_user()
{
	int i;
	for(i=0; i<SDA_READ_NUM; i++)
	{
		if(!rd[i]->isRead)
			//break;
			return i;
	}
	return -1;
}

//��ʼ����
int sda_startplay(int usrno,int channel,int starttime, int playtime)
{
	fprintf(stderr, "come in startplay!!\n");
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;
	long long seek;
	fprintf(stderr, "come in startplay!!1\n");
	if( ( seek=rdl->is_in(starttime) ) == 0)
	{
		fprintf(stderr, "get seek err\n");
		cout << "starttime is wrong\n\n\n" << endl;
		//ʱ�䲻�ڿ��У������ڿ����Ҳ�����ʱ��
		return BLOCK_ERR_NOT_IN;
	}
	fprintf(stderr, "starttime:%d,get seek:%lld!!\n\n\n\n\n\n",starttime,seek);
	//bug fix ���´�����ܲ���ȫ
	rd[usrno]->setread(channel,seek );
	//rd[usrno]->isRead = true;
	rd[usrno]->start_play();
	fprintf(stderr, "over\n");
	return 0;
}

//ֹͣ����
int sda_stopplay(int usrno)
{
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;
	rd[usrno]->clear();
	rd[usrno]->isRead = false;
	return 0;
}

//��ȡ����Ƶ����
int sda_get_media(int usrno,int channel,int media_type,/*IN OUT*/char *buf,unsigned int buf_size, \
		/*OUT*/unsigned int *media_size, /*OUT*/stream_format_t *sft)
{
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;

	gtsda::FrameQueue<Blocks *> *video_queue=NULL;
	gtsda::FrameQueue<Blocks *> *audio_queue=NULL;
	gtsda::Blocks *bs=NULL; 	//���ؽṹ
	unsigned int size;			//���ص����ݴ�С

	if(TYPE_VIDEO == media_type)
	{
		//��ȡ��Ƶ����
		video_queue = rd[usrno]->get_video();
		//cout << "before video queue size: " << video_queue->size() << endl;
		//�������е����ݴ�С
		while(video_queue->size()<10)
		{
			//cout << "no enough video data!,size: " << video_queue->size()<< endl;
			rd[usrno]->start_play();
			rd[usrno]->set_notful();
		}
		//cout << "after video queue size: " << video_queue->size() << endl;
		//��ȡ��ǰ�������
		bs = video_queue->pop();
		size = bs->GetSize();
		//if(bs->GetType() == noblock)
		//	*video_type = TYPE_VIDEO_I;
		//else
		//	*video_type = TYPE_VIDEO_P;
	}
	else if(TYPE_AUDIO == media_type)
	{
		//��ȡ��Ƶ����
		audio_queue = rd[usrno]->get_audio();
		if(audio_queue->size()<=0)
		{
			//cout << "no audio data!!!" << endl;
			//rd[fParams.channel]->set_notful();
			return ERR_NO_DATA;
		}
		//cout << "audio queue size: " << audio_queue->size() << endl;
		bs 	=audio_queue->pop();
		size= bs->GetSize();
	}
	else return ERR_TYPE;

	//��������
	//cout << "size: " << size << "buf_size: " << buf_size << endl;
	size = buf_size<=size?buf_size:size;
	//cout << "size: " << size  << endl;
	memcpy(buf, bs->GetBuf(), size);//�ĸ�Сѡ�ĸ�
	*media_size=size;
	delete bs;
	bs=NULL;
	return 0;
}

//�ر�sda
void sda_free()
{
	free(rdl);
}
