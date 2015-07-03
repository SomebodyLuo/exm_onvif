/*
 * RWlocic.cpp
 *
 *  Created on: 2013-11-6
 *      Author: yangkun
 */

#include "RWlogic.h"
#include "wrap.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <cstdio>
# include <stdio.h>
#include "MultMediaBlocks.h"
#include "FrameQueue.h"
namespace gtsda
{
DataWrite *RWlogic::dw=NULL;


FrameQueue<Blocks *> RWlogic::frame_queue_read_video1;
FrameQueue<Blocks *> RWlogic::frame_queue_read_audio1;
unsigned int RWlogic::ReadDisk::num=0;
RWlogic::RWlogic(bool bMarkWR)
:bMarkWR(bMarkWR),bIsRun(false)
{
	// TODO Auto-generated constructor stub
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&start_again,NULL);
	dbread = NULL;
}

RWlogic::~RWlogic()
{
	ttt();
	//pthread_cancel(write_pid );
	//pthread_cancel(read_pid );
	//pthread_cancel(cmd_pid );
	//pthread_join(write_pid, NULL);
	//pthread_join(read_pid, NULL);
	//pthread_join(cmd_pid, NULL);
	ttt();
	if(yb)
	{
		delete yb;
		yb = NULL;
	}
	if(dbread)
	{
		delete dbread;
		dbread = NULL;
	}
	if(dbbac)
	{
		ttt();
		delete dbbac;
		dbbac = NULL;
	}
	if(db)
	{
		ttt();
		delete db;
		db = NULL;
	}
/*	if(dr_video1)
	{
		delete dr_video1;
		dr_video1 = NULL;
	}
	if(dr_audio1)
	{
		delete dr_audio1;
		dr_audio1 = NULL;
	}*/
	ttt();
	free_sda();
	ttt();
}
//���,���Ҫ����鸲���ˣ����������������ɾ�ˣ�������顣
/*****************************************************************
 * blocks:	�˿�ռ�Ŀ������
 * seek:	 �˿鿪ʼ��λ��
 *****************************************************************/
int RWlogic::maybe_cover_dayblock(long long blocks,long long seek)
{
	int ret;
	struct seek_block sb;
	if( ( ret = yb->GetHead(sb)) < 0)//��ȡ��������
	{
		gtlogerr("@%s %d GetHead error:%d",__FUNCTION__ , __LINE__, ret);
		return ret;
	}
	if(sb.seek - seek>=0 &&\
			blocks>= sb.seek - seek)
	{
		gtloginfo("Warning:: the day block will be cover:seek:%lld,time:%d\n",sb.seek,sb.time);
		DayBlocks *db1 =new DayBlocks(sb.seek,false/*����ʱ����Ӳ��*/,false/*��д��Ӳ��*/);
		*dbbac = *db1;
		delete db1;
		dbbac->write();
		cout << "debug dbbac seek: " << dbbac->GetSeek()<< endl;
		if( (ret=yb->del(sb) ) < 0)//ɾ���������
		{
			gtlogerr("@%s %d yearblock del earlist block error:%d",__FUNCTION__ , __LINE__, ret);
			return ret;
		}
	}
	return 0;
}
RWlogic* RWlogic::newRW(bool bMarkWR)
{
	return new RWlogic(bMarkWR);
}
/*������д��Ӳ����*/
int RWlogic::writedata()
{
	gtloginfo("come to writedate");
	struct seek_block sb;//,earliest_day_block;
	int ret;
	int now,iOldTime;
	bIsRun = true;
	long long seek;
	//dr = new DataRead();
	//������л�ȡ����
	gtlogdebug("db.seek:%lld",db->GetSeek());
	yb->GetTail(sb);
	iOldTime = sb.time;
	if( (ret = db->GetTail(sb) ) != 0 ) //sb�����ǵ�ǰʱ���ƫ����
	{
		print_err(ret);
		gtlogerr("db getTail err:%d",ret);
		seek = db->GetNext();
		iOldTime = gettime();//����˲�����������������ܻ��������
	}
	else
	{
		seek = sb.seek;
		//iOldTime = sb.time;  //�ж��ǲ���ͬһ���ʱ��������ȡ����
	}

	//�Ѷ������Ƶ֡ȫ���ŵ�һ��Ļ�����
	MultMediaBlocks *mb_all;
	MultMediaBlocks *mb_video;
	MultMediaBlocks *mb_audio;

	SecBlocks *hb_head;/*һ������Ƶ�鼯�ϣ�ֻռһ��BLOCKSIZE,������������Ƶ��������ʹ�С*/
	SecBlocks *hb_video;/*��һ·��Ƶ����*/
	SecBlocks *hb_audio;/*��һ·��Ƶ����*/


	//��ӡ�����տ�ʼҪд���seek
	gtloginfo( " current seek: %lld" , seek);

	while(bIsRun)
	{
		pthread_mutex_lock(&lock);
		//fprintf(stderr, "start again 1\n");
		pthread_cond_wait(&start_again, &lock);
		//fprintf(stderr, "start again 2\n");
		pthread_mutex_unlock(&lock);

		cout << endl << endl;
		now = gettime();
		cout << "now: "<< now << endl<< endl<< endl;

		//�жϽ����ǲ��ǹ�ȥ��
		if(now/SECOFDAY != iOldTime/SECOFDAY )
		{
			now = gettime();
			long long seek_tmp= db->GetBlocks();
			delete db;
			db = NULL;
			sb.time=now;
			gtloginfo("the day is gone!!!");
			//�ж�ʣ�µĿռ��ǲ����ܹ�����������
			if(HdSize - seek < seek_tmp )
			{
				gtloginfo("the leave can't save one day blocks!!!");
				//�����棬������Ӳ�̿�ʼ��λ��
				seek = first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
						Blocks::get_block_num(sizeof(struct day_block))/*day back*/;
				//���,���Ҫ����鸲���ˣ����������������ɾ�ˣ�������顣
				if ( ( ret = maybe_cover_dayblock(Blocks::get_block_num(sizeof(struct day_block)),seek) ) <0 )
				{
					gtlogerr("@%s %d maybe_cover_dayblock error:%d",__FUNCTION__ , __LINE__, ret);
					return ret;
				}
			}
			ttt();
			//cout << " this day seek: " << seek << endl;
			db = new DayBlocks(seek,false/*����ʱ����Ӳ��*/,true/*Ҫд��Ӳ��*/);//�����ȥ�ˣ�����Ӳ���ж�ȡ��飬��ֱ�ӹ���
			//cout << "debug db.seek: " << db->GetSeek()<< endl;
#if 1
			db->write();
#else
			delete db;
			db = new DayBlocks(seek);
#endif
			sb.seek = seek;
			gtloginfo("year block add day block,seek:%lld,time:%d",sb.seek,sb.time);
			ret=yb->add(sb);
			ttt();
			seek += db->GetBlocks();
			//seek = db->GetNext();//����һ����ͬ�Ľ��
			//cout << "this db getblocks: " << db->GetBlocks()<<endl;
			print_err(ret);
		}

		//Ӳ���ǲ������ˣ�
		if( HdSize - seek < LEAVE_BLOCKS )
		{
			gtloginfo("Warning:: disk is full ");
			//Ӳ�����ˣ������ֻ��һ�죬˵��Ӳ�̲�����һ�졣
			if(1==yb->get_day_num())
			{
				gtloginfo("Warning:: capacity of disk can't save one day");
				*dbbac = *db;// warning
				dbbac->write();
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
									Blocks::get_block_num(sizeof(struct day_block)) + \
									Blocks::get_block_num(sizeof(struct day_block));
			}
			else
			{
				gtloginfo("Warning:: capacity of disk can't save one day, turn to head");
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
				Blocks::get_block_num(sizeof(struct day_block));
				//hb->SetSeek(seek);
			}
		}

		//���,���Ҫ����鸲���ˣ����������������ɾ�ˣ�������顣
		if ( ( ret = maybe_cover_dayblock(LEAVE_BLOCKS,seek) ) <0 )
		{
			gtlogerr("@%s %d maybe_cover_dayblock error:%d",__FUNCTION__ , __LINE__, ret);
			return ret;
		}


		sb.time = now;
		sb.seek = seek;
		//����seek����������ʱд��Ӳ��
		long long seek_mult_media = seek;
		//������һ��BLOCKSIZE��ͷ����
		mb_all = new MultMediaBlocks(BLOCKSIZE, multmedia);
		seek += 1;/* hb_head->GetBlocks() ��������ռһ�� */;


		unsigned int channel_mask=0;
		int queue_size;
		//�ֱ�ӣ�������Ƶͨ��������������ȡ����
		for(int i=0; i<MAX_CHANNEL; i++)
		{
			if( ( queue_size = mb_video_queue[i].size()) > 0 )
			{
				cout << "channel " << i << " video queue size: " << queue_size << endl;
				//������Ƶ��
				channel_mask |= (1<<i);
				mb_video = mb_video_queue[i].pop();
				hb_video = new SecBlocks(mb_video, mb_video->get_buff_size());
				delete mb_video;
				mb_video = NULL;
				//����Ӳ��д���seek
				hb_video->SetSeek(seek);
				//����Ƶ���ϵ�seekд��ͷ����
				mb_all->add_vidoe(seek,hb_video->GetSize()-sizeof(struct hd_frame),i);
				seek += hb_video->GetBlocks();
				//�����ݴ浽Ӳ����
				delete hb_video;
				hb_video = NULL;

				//������Ƶ��
				if( ( queue_size = mb_audio_queue[i].size() ) > 0 )
				{
					cout << "channel " << i <<  "audio queue size: " << queue_size << endl;
					mb_audio = mb_audio_queue[i].pop();
					hb_audio = new SecBlocks(mb_audio, mb_audio->get_buff_size());
					delete mb_audio;
					mb_audio = NULL;
					//����Ӳ��д���seek
					hb_audio->SetSeek(seek);
					//����Ƶ���ϵ�seekд��ͷ����
					mb_all->add_audio(seek,hb_audio->GetSize()-sizeof(struct hd_frame),i);
					seek += hb_audio->GetBlocks();
					//�����ݴ浽Ӳ����
					delete hb_audio;
					hb_audio = NULL;
				}
#ifdef DEBUG
				else
				{
					//������������������������������������Ƶû����Ƶ�����������
					gtlogerr("mb_audio_queue %d had no data!!!!!!!!!!!!!!!!!");
				}
#endif
			}
#ifdef DEBUG
			else
			{
				gtlogwarn("mb_video_queue channel %d had no data");
			}
#endif
		}

		mb_all->add_next_mult_media_block_seek(seek);
		sb.time = channel_mask;
		hb_head = new SecBlocks( mb_all, sizeof(mult_media_block));
		delete mb_all;
		mb_all = NULL;
		//���д��ͷ��
		hb_head->SetSeek(seek_mult_media);
		delete hb_head;
		hb_head = NULL;
		cout << " head block seek: "<< seek_mult_media<< " blocks: "  << seek -seek_mult_media \
			 << " channel_mask: " << channel_mask <<endl;

		//�������������
		if(1)/*��Ϊֻ���ǣ�֡ʱ��д���*/
		{
			ret = db->AddBlock(sb,now);
			if(ret<0  )
			{
				gtlogerr("BLOCK_ERR_DAY_SEC_MUT:%d",ret);
				print_err(ret);
				if(BLOCK_ERR_DAY_SEC_MUT != ret)
				{
					cout<< "debug:: here will exit" << endl;
					gtlogerr("debug:: here will exit");
					bIsRun = false;
				}
			}
		}
		iOldTime = now;
		//delete hb;//�ͷ��ڴ�
	}
	cout << "debug adfadf"<< endl;
	return 0;
}
/************************************************
 * ���ʱ���ǲ�����Ӳ������
 * ����Ӳ���з��أ�
 * ��Ӳ���з�����seek
 ***********************************************/
long long   RWlogic::is_in(int starttime)
{
	//1��������������ʱ��������һ�졣
	long long readseek = yb->TimeIn(starttime);
	if( readseek <= 0)
	{
		cout << "info:your input time not in year block"<< endl;
		readseek = dbbac->TimeIn(starttime);//Ҳ���ڱ��������
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in back day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in back day block,and i will exit!!");
			return 0;
		}
	}
	else
	{
		if(dbread)
		{
			delete dbread;
			dbread =NULL;
		}
		ttt();
		dbread = new DayBlocks(readseek,true,false/*����ʱ��Ӳ�̣�����ʱ��дӲ��*/);
		ttt();
		readseek = dbread->TimeIn(starttime);
		ttt();
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in  day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in  day block,and i will exit!!");
			return 0;
		}
	}
	return readseek;
}

void RWlogic::start(int starttime)
{
	static struct pthread_arg pa_write, pa_read, pa_cmd, pa_read_pool[MAX_CHANNEL];
	pa_write.wr = this;
	pa_read.wr  = this;
	pa_cmd.wr   = this;
	this->bIsRun = true;

	//��ʼ�������
	pthread_mutex_init(&lock_start_read_again,NULL);
	pthread_cond_init(&start_read,NULL);
	pthread_cond_init(&stop_read,NULL);
	//�ֱ𴴽��ӻ������������߳�
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		ttt();
		pa_read_pool[i].wr = this;
		pa_read_pool[i].type = read_media_data;
		pa_read_pool[i].channel = i;
		pthread_create(&write_pid, NULL, create_thread, &pa_read_pool[i]);
	}
	//����дӲ���߳�
	ttt();
	pa_write.type = write_type;
	pthread_create(&write_pid, NULL, create_thread, &pa_write);
	//������Ӳ���߳�
	//pa_read.type = read_type;
	//pthread_create(&read_pid, NULL, create_thread, &pa_read);
	//���������߳�
	//ttt();
	//pa_cmd.type = cmd_type;
	//pthread_create(&cmd_pid, NULL, create_thread, &pa_cmd);
}

int RWlogic::Init()
{
	ttt();
	int ret;
	if(init_sda()<0)
	{
		cout << "init sda error " << endl;
		exit(1);
		return 0;
	}
	else
		cout << "Init sda success!!!\n\n" ;
	//���

	try{
		if(bMarkWR/*write*/)
			yb = new YearBlocks()/*Ҫ��ҲҪд*/;
		else/*read*/
			yb = new YearBlocks(true, false)/*ֻ����д*/;
	}
	catch(int err)
	{
		gtlogwarn("catch error!!");
		print_err(err);
		if(BLOCK_ERR_DATA_HEAD == err )
		{
			gtlogwarn(" maybe this is new disk, and I will formate this disk!!!");
			if(bMarkWR)//дʱ���Ը�ʽ��
			{
				try{
				yb = new YearBlocks(false)/*��ʼ��ʱ����������ʱҪд*/;
				dbbac = new DayBlocks(first_block+Blocks::get_block_num(sizeof(struct year_block)),false/*����Ӳ��*/ , true/*Ҫд��Ӳ��*/ );
				db = new DayBlocks(first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
						Blocks::get_block_num(sizeof(struct day_block)), false/*����Ӳ��*/ , true/*Ҫд��Ӳ��*/);
				format();
				delete dbbac;
				dbbac = NULL;
				delete db;
				db = NULL;
				}
				catch(int err)
				{
					cout << "too much wrong !! and exit " << endl;
					exit(-1);
					cout << endl << endl;
				}
			}
			else//����ʱ�����ֱ�ӷ���-��;
			{
				cout << "read year block error!!" << endl;
				gtlogerr("read year block error");
				return -1;
			}
		}else
		{
			cout << "other error and I will exit" << endl;
			exit(1);
		}
	}



	//�������
	dbbac = new DayBlocks();
	//������л�ȡ�������
	struct  seek_block sb;
	if( ( ret = yb->GetTail(sb) ) < 0)
	{
		print_err( ret );//�쳣
		gtlogerr("year gettail err:%d\n",ret);
		//����ǿյ�
		if( ( BLOCK_ERR_EMPTY == ret ) && bMarkWR)
		{
			gtlogerr("year block is empty!!\n" );
			sb.seek = first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
					Blocks::get_block_num(sizeof(struct day_block));
			sb.time = gettime();
			if( ( ret = yb->add(sb) ) < 0 )
			{
				print_err( ret );//�쳣
				gtlogerr("year add err:%d\n",ret);
				return ret;
			}
		}
		else
			return ret;
	}
	gtloginfo("current day seek:%lld,time:%d,%s",sb.seek ,sb.time, ctime((time_t *)&sb.time) );
	//��ȡ���
	db = new DayBlocks(sb.seek);
	HdSize = hd_getblocks();

	//init_audio_pool();

	return 0;
}
void RWlogic::format()
{
	gtloginfo("fromat");
	//formate year block
	if(NULL != yb)
	{
		gtloginfo("yearblock");
		struct year_block *yb = (struct year_block *)this->yb->GetBuf();
		memcpy(yb->year_head, YearBlocks::year_head, sizeof(YearBlocks::year_head));
		this->yb->write();
	}
	//formate day bac block
	if(NULL != dbbac)
	{
		gtloginfo("formate dbbac");
		struct day_block *dbbac = (struct day_block *)this->dbbac->GetBuf();
		memcpy(dbbac->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->dbbac->write();
	}
	//formate day block
	if(NULL !=db )
	{
		gtloginfo("formate db");
		struct day_block *db = (struct day_block *)this->db->GetBuf();
		memcpy(db->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->db->write();
	}
}


static int find(struct seek_block *seek_block_data ,int size, int value=0)
{
	int i;
	if(value==0)
	{
		for(i =0; i<size; i++)
		{
			if(seek_block_data[i].seek==0  && seek_block_data[i].time == 0)
				break;
		}
	}
	else
	{
		for(i =0; i<size; i++)
		{
			if(seek_block_data[i].seek!=0 || seek_block_data[i].time != 0)
				break;
		}
	}
	return i;
}
//#define NETSDK
static void write_str(string &timestring, time_t time,int is_start=1)
{
#ifndef NETSDK
	if(is_start==1)
				timestring += "start:\tblock: ";
	else
		timestring += "end:\tblock:";
				//timestring +=std::to_string( dDayData->seek_block_data[sec].seek) ;
				//timestring += "\ttime:  ";
				timestring += std::to_string( static_cast<long long>(time ));
	if(is_start==1)
				timestring += " --> ";
	else
		timestring += " --> ";
				timestring += ctime(&time  ) ;
#else
				struct tm *p=gmtime( &time  ) ;
				timestring +=std::to_string(static_cast<long long>(1900+p->tm_year)) ;
				char buf[5];
				//��
				sprintf(buf,"%02d",(1+p->tm_mon));
				timestring +=buf;
				//��
				sprintf(buf,"%02d",(p->tm_mday));
				timestring +=buf;
				//ʱ
				sprintf(buf, "%02d", (p->tm_hour));
				timestring += buf;
				//��
				sprintf(buf, "%02d", (p->tm_min));
				timestring += buf;
				//��
				sprintf(buf,"%02d",(p->tm_sec));
				timestring +=buf;
				if(is_start==1)
					timestring += '-';
				else
					timestring += '\n';
#endif
}
/***************************************************************************
 * ���ܣ���������ʱ���
 * ������dDayData���ָ��
 * 		iBeginTime��ʼʱ��
 ***************************************************************************/

#define VOIDBLOCK 2
void RWlogic::printOneDay(const struct day_block *dDayData,  int todaytime,int iBeginTime , int iEndTime,string &timestring)
{
	struct seek_block *seek_block_data = const_cast<struct seek_block *>(dDayData->seek_block_data);
	seek_block_data = seek_block_data + iBeginTime;
	int size = iEndTime-iBeginTime;
#define JG 3

	int num=0;
	int tmp;
	while(1)
	{
		if(num>=size)break;
		//�ҵ�һ��
		tmp = find(seek_block_data+num,size-num, 1);
		num += tmp;
		//cout << num << "-->" ;
		write_str(timestring, todaytime+iBeginTime+num,1);
		//cout << "1iBeginTime: " << iBeginTime << " num: " << num << "todaytime: " << todaytime << endl;
		fflush(stdout);
		//�����һ��
		while(1)
		{
			if(num>=size)break;
			/* 1 1 1 0*/
			int tmp1=find(seek_block_data+num,size-num, 0);
			num+=tmp1;

			/*0 0 0 1*/
			tmp=find(seek_block_data+num, size-num,1);
			num+=tmp;
			if(tmp>=JG
#if 1
					||num==size  //�����������Ҫ��˵�����ҵ�����������
#endif
					)
			{
				//cout << num-tmp-1 ;
				write_str(timestring, todaytime+iBeginTime+num-tmp,0);
				//cout << "2iBeginTime: " << iBeginTime << " num: " << num-tmp << "todaytime: " << todaytime << endl;
				break;
			}
		}
		cout << endl;
	}
}

/*************************************************************************
 * ���ܣ���Ӳ���������飬���¼��ʱ��
 *************************************************************************/
int RWlogic::read_disk_print_record_time(string &timestring)
{
	int tmp=0;
	int day, sec;
	int bool_tmp=0;
	if(yb==NULL){ttt();cout << "year block error" << endl;return -1;}

	//����dbbac���ǲ���������,�����
	//��dbbac������
#ifndef NETSDK
	timestring += "daydata_bak::\n";
#endif
	//printOneDay((const struct day_block *)(dbbac->GetBuf()), 0 ,timestring );
	cout << endl;


	//��������е����ݣ�
	const struct year_block *yearblock =  (struct year_block *)yb->GetBuf();
#ifndef NETSDK
	timestring +=  "day of yearblock:";
	timestring += std::to_string( static_cast<long long>(yearblock->year_queue_data.queue_size) );
	timestring += "\n";
#endif
	bool_tmp=0;
	for(day=yearblock->year_queue_data.queue_head; \
		day < yearblock->year_queue_data.queue_head + yearblock->year_queue_data.queue_size; \
		day++)
	{
		if( yearblock->seek_block_data[day].seek==0 || yearblock->seek_block_data[day].time == 0)
		{
			cout << "worning :\tBLOCK_ERR_YEAR_PRINT\n";
			gtloginfo("worning :\tBLOCK_ERR_YEAR_PRINT\n");
			return -1;
		}
		cout << day<< " day block seek: " << yearblock->seek_block_data[day].seek << " time: " << yearblock->seek_block_data[day].time << endl;
#ifndef NETSDK
		timestring += std::to_string( static_cast<long long>(day+1-yearblock->year_queue_data.queue_head));
		timestring +="st day's time is: " ;
		timestring +=std::to_string( static_cast<long long>( yearblock->seek_block_data[day].time ));
		timestring += "\t-->";
		timestring += ctime((const time_t*)&( yearblock->seek_block_data[day].time ) );
#endif
		//cout << "today's block:\n";
		//��ȡ�������ݵ��ڴ���
		DayBlocks dayblock(yearblock->seek_block_data[day].seek,true/*����ʱ��Ӳ��*/,false/*����ʱ��д��*/);
		//cout << "seek of begin: " << (yearblock->seek_block_data[day].time)%SECOFDAY << endl;
		//��������ڵ�������
		//if(day==yearblock->year_queue_data.queue_head+1)
#if 1
		printOneDay((const struct day_block *)(dayblock.GetBuf()),yearblock->seek_block_data[day].time/SECOFDAY*SECOFDAY, \
				/* (yearblock->seek_block_data[day].time)%SECOFDAY*/0 , SECOFDAY, timestring );
#else
		struct day_block *dbb=(struct day_block *)(dayblock.GetBuf());
		for(int i=0; i<SECOFDAY; i++)
		{
			if(dbb->seek_block_data[i].seek!=0)
				cout << "seek: " << dbb->seek_block_data[i].seek << " mask: " << dbb->seek_block_data[i].time << endl;
		}
#endif

	}

	return 0;
}
/*************************************************************************
* ���ܣ�	��ָ����ʱ����ڲ�ѯ��Ч���ݣ���������Ч��ʱ���
* 		ֻ�ܲ�һ�����ڵ�ʱ���
* ������
* ����:	0�ɹ�
* 		<0ʧ��
**************************************************************************/
int RWlogic::get_time_index(int timestart,int timeend, string &timeindex)
{
	long long seek = yb->TimeIn(timestart);
	if(seek<0)
	{
		gtlogerr("timestart no in yearblock!!\n");
		return BLOCK_ERR_NOT_IN;
	}
	gtloginfo("debug %s:%d",__FILE__,__LINE__);

	//��ȡ�������ݵ��ڴ���
	DayBlocks dayblock( seek,true/*����ʱ��Ӳ��*/,false/*����ʱ��д��*/);
	int iTmpTime = (timestart%SECOFDAY) > (timeend%SECOFDAY)?SECOFDAY:(timeend%SECOFDAY);
	printOneDay((const struct day_block *)(dayblock.GetBuf()),timestart/SECOFDAY*SECOFDAY,\
			timestart%SECOFDAY , iTmpTime, \
			timeindex );
	gtloginfo("debug %s:%d",__FILE__,__LINE__);
	return 0;
}
/*�����������*/
void RWlogic::cmd_proc()
{
	string returntring;
	msg_st msg,msg_s;
	char *msg_content=NULL;//��Ϣ����
	int ret;

#define MAXLINE 80
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int i, n;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(CMD_PORT);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, 1);

	printf("Accepting connections ...\n");
	while (bIsRun)
	{
		cliaddr_len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		while (bIsRun)
		{
			memset(&msg, 0, sizeof(msg));
			n = Read(connfd, &msg, sizeof(msg));
			if (n == 0)
			{
				printf("the other side has been closed.\n");
				break;
			}
			if(msg.msg_head != MSG_HEAD)
			{
				cout << "msg head error!\n";
				continue;
			}
			memset(&msg_s, 0 ,sizeof(msg_s));
			msg_s.msg_head = MSG_HEAD;
			msg_s.msgtype  = cmd_ret_ok;
			if(msg.msgtype == request_time)
			{
				cout << "request_time" << endl;
				if( ( ret = read_disk_print_record_time(returntring)) < 0 )
				{
					cout << "read print error!! " << endl;

				}
				msg_s.msg_len = returntring.length();
			}
			else if(msg.msgtype == cmd_play)
			{
				cout << "cmd_play" << endl;
				ttt();
				starttime = msg.msg_len;
				cout << "start time: " <<  starttime  << endl;
				if(!is_in(starttime))
				{
					//���û��������
					ttt();
					msg_s.msgtype  = cmd_ret_err;
					msg_s.msg_len = 0;
				}
				else
				{
					//1������в����̣߳��ر�֮
					// del yangkun if(isRead)
					{
						ttt();
						pthread_mutex_lock(&lock_start_read_again);
						// isRead = false;
						pthread_cond_wait(&stop_read, &lock_start_read_again); //�ȴ�ֹͣ�ź�
						ttt();
						pthread_mutex_unlock(&lock_start_read_again);
					}



					//2�����������߳�
					ttt();
					pthread_cond_signal(&start_read); //���Ͳ���

					ttt();


					returntring= "ok";
					msg_s.msg_len = 0;
				}
			}
			else
			{
				msg_s.msgtype  = cmd_ret_err;
				returntring = "your msg type is wrong";
				msg_s.msg_len = returntring.length();
				ttt();
			}
			Write(connfd, &msg_s, sizeof(msg_s));
			ttt();
			if(msg.msgtype != cmd_play)
			{
				ttt();
				cout << " len size: " << returntring.length() << " " << returntring.size() << endl;
				Write(connfd, returntring.c_str() ,returntring.length());
				//�����ڴ�
				returntring.clear();
			}
		}
		Close(connfd);
	}


	while(bIsRun)
	{


#if 0
		if(p_msg_r->msgtype == request_time)
		{
			ttt();
			if( ( ret = read_disk_print_record_time(returntring)) < 0 )
			{
				cout << "read print error!! " << endl;

			}
		}
		else if(p_msg_r->msgtype == cmd_play)
		{
			ttt();
			//1������в����̣߳��ر�֮
			pthread_mutex_lock(&lock_start_read_again);
			isRead = false;
			//pthread_cond_signal(&stop_read); //����ֹͣ
			pthread_mutex_unlock(&lock_start_read_again);
ttt();

			pthread_cond_wait(&stop_read, &lock_start_read_again);


			//2�����������߳�
			ttt();
			pthread_cond_signal(&start_read); //���Ͳ���

			ttt();

	        returntring = "i am hear!\n";
		}
		else
		{
			returntring = "your msg type is wrong";
			ttt();
		}
		cout << "buf size: " << returntring.size() << endl;
#endif
		//����
		//memcpy(p_msg_s->msg_content, returntring.c_str() ,returntring.size());







	}//


}

#if 0
/*******************************************************************************************
 *��ʼ�� �ڴ��
 *******************************************************************************************/
void RWlogic::init_audio_pool()
{
	int ret;
	char name[256]="send pool";	//���ӵ�����
	ret=init_media_rw(&media,MEDIA_TYPE_VIDEO,0,BUFFER_SIZE);
	if(ret<0)
	{
		printf("error in init_media_rw,and exit\n");
		exit(1);
	}
	//��ʼ���ڴ��
		ret=create_media_write(&media, 0x30008,name,VIDEO_POOL_SIZE_SAVE);

	if(ret<0)
	{
		printf("error in create_media_write\n");
		exit(1);
	}
	media.attrib->stat=ENC_STAT_OK;
	media.buflen=BUFFER_SIZE;

	media.attrib->fmt.v_fmt.format=VIDEO_H264;
	media.attrib->fmt.v_fmt.ispal=0;
	media.attrib->fmt.v_fmt.v_buffsize=BUFFER_SIZE;
	media.attrib->fmt.v_fmt.v_frate=25; //֡��
	media.attrib->fmt.v_fmt.v_height= 576;
	media.attrib->fmt.v_fmt.v_width= 704;


	if(media.temp_buf ==NULL)
	{
		printf("1111error in media tmemp_buf\n");
	}
	the_frame_buffer =(enc_frame_t *)media.temp_buf;		//�����Ǹ�avi�ṹͷ����������
	memset(the_frame_buffer, 0, sizeof(enc_frame_t));
	the_frame_buffer->channel=0;					//ͨ����
	the_frame_buffer->chunk.chk_id=IDX1_VID;		//��Ƶ
	the_frame_buffer->media=MEDIA_VIDEO;
}
void RWlogic::write_pool(char *buf, unsigned int buffsize,bool is_key)
{

	if (is_key)
		the_frame_buffer->type = FRAMETYPE_I;
	else
		the_frame_buffer->type = FRAMETYPE_P;
	memcpy((char *) the_frame_buffer->frame_buf, buf, buffsize);
	the_frame_buffer->len = buffsize;
	the_frame_buffer->chunk.chk_siz = buffsize;
	int ret=write_media_resource(&media, the_frame_buffer, buffsize+sizeof(enc_frame_t), is_key/*MEDIA_TYPE_VIDEO*/);
	if(ret<0)
	{
		gtlogerr("cat't write media resource and exit");
	}
}
#endif
void RWlogic::printids(const char *s)
{
	pid_t      pid;
	pthread_t  tid;

	pid = getpid();
	tid = pthread_self();
	printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid,
	       (unsigned int)tid, (unsigned int)tid);
}
//�߳����
void * RWlogic::create_thread(void *args)
{
	struct pthread_arg pa;
	struct pthread_arg *pa1 = static_cast< pthread_arg * >(args);
	ttt();
	memcpy(&pa, pa1, sizeof(struct pthread_arg));
	ttt();

	//cout << "isRun: " << pa.wr->IsRun() << endl;

	/************������д��Ӳ��*************************/
	if(pa.type==write_type)
	{
		cout <<"this is write"<<endl;
		pa.wr->printids("write pthread\n");
		pa.wr->writedata();
		cout <<"write process  exit"<<endl;
	}
	/************��Ӳ�̶�����Ҫ������*************************/
	else if(pa.type==read_type)
	{
		cout <<"this is read"<<endl;
		pa.wr->printids("read pthread\n");
		//pa.wr->read_proc();
		cout <<"read process  exit"<<endl;
	}
	/************��������*************************/
	else if(pa.type==cmd_type)
	{
		cout << "this is cmd" << endl;
		pa.wr->printids("cmd pthread\n");
		pa.wr->cmd_proc();
		cout <<"cmd process  exit"<<endl;
	}
	/************������Ƶ��������������*************************/
	else if(pa.type==read_media_data)
	{
		cout << "this is read media data" << endl;
		gtloginfo("read media pool channel: %d",pa.channel);
		pa.wr->printids("cmd pthread\n");
		pa.wr->read_pool_proc(pa.channel);
		cout <<"cmd process  exit"<<endl;
	}
	ttt();
	gtlogwarn("proc %d exit!\n",pa.type);
	cout<< "proc " << pa.type << " exit!\n";

	pthread_exit(NULL);
	//f->thread();
	return NULL;
}
//������Ƶ��������������
void RWlogic::read_pool_proc(int channel)
{
	bool read_pool_start=false;
	//��ʼ������
	DataRead *dr_video = new DataRead(0, video, 0+channel);
	DataRead *dr_audio = new DataRead(0, audio, 0x20000+channel);
	MultMediaBlocks *mb_video=NULL;
	MultMediaBlocks *mb_audio=NULL;
	bool is_i;
	while(!read_pool_start)
	{
		//������һ����֡ʱ�ſ�ʼ
		read_pool_start  = dr_video->readpool();
	}

	while(bIsRun)
	{
		//��Ƶ�֣��ʹ棱s������Ӧ�ù��ˣ���Ƶ��128�˴棱s������
		mb_video = new MultMediaBlocks(1224*1024 ,mult_video);
		mb_audio = new MultMediaBlocks(128*1024  ,mult_audio);
		//��ȡ��һ�Σ����һ����֡�������
		mb_video->add(dr_video,video);
		//��ȡ����Ƶ����
		while(bIsRun)
		{
			is_i  = dr_video->readpool();
			//������֡�˳�
			if(is_i)break;
			mb_video->add(dr_video,video);
			//һ����Ƶ��Ӧ������Ƶ
			while(dr_audio->get_remain_frame_num()>0)
			{
				//����Ƶ������
				dr_audio->readpool();
				//����Ƶ������������"���"
				mb_audio->add(dr_audio,audio);
			}
		}
		//�ѵ�������Ƶ���ϣ���Ƶ�������һ�����������Ƶ����
		//debug
		//cout <<"channel:\t"<< channel <<"\n video buff size: " << mb_video->get_buff_size() <<\
				"audio buff size : " << mb_audio->get_buff_size()<< endl;
		//myprint((unsigned char*)(mb_video->GetBuf()+sizeof(struct hd_frame)), 8);
		//myprint((unsigned char*)(mb_video->GetBuf()+sizeof(struct hd_frame)\
				+sizeof(struct media_block)), 8);

		//����Ϻõ�����Ƶ���ݵļ��ϷŷŶ�������
		mb_video_queue[channel].push(mb_video);
		mb_audio_queue[channel].push(mb_audio);
		mb_video=NULL;
		mb_audio=NULL;
		if(0==channel)
			pthread_cond_signal(&start_again);
	}
}








RWlogic::ReadDisk::ReadDisk(long long seek):isRead(false),
framenum(0),framecount(0),channel(0)
{
	//ֻ������5���ͻ���
	if(num>5)
		throw(0);
	num++;
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&notfull,NULL);
	pthread_cond_init(&startplay,NULL);
}
RWlogic::ReadDisk::~ReadDisk()
{
	num--;
	/*
	if(audio_pool)
	{
		delete audio_pool;
		audio_pool = NULL;
	}
	if(video_pool)
	{
		delete video_pool;
		video_pool = NULL;
	}
	*/
}

/*************************************************************************
 * ���ܣ�	�����ȡ�����ݣ��ֽ⣬������ȥ
 * ������
 * video: ��Ƶ����
 * auido: ��Ƶ����
 *************************************************************************/
int RWlogic::ReadDisk::readdataprocess(SecBlocks *video, SecBlocks *audio)
{
	if(!video||!audio)
	{
		gtlogerr("readdataprocess video or audio si null");
		throw ;
	}
	//static unsigned int audio_count,video_count;
	//audio_count=0,video_count=0;
	bool isfirst=true;
	//��һ��ͷ
	struct media_block *mb_video = (struct media_block *)((char *)video->GetBuf() +sizeof(struct hd_frame));
	struct media_block *mb_audio = (struct media_block *)((char *)audio->GetBuf() +sizeof(struct hd_frame));

	unsigned int video_size = mb_video->size;
	unsigned int audio_size = mb_audio->size;

	//��Ƶͷ
	mb_video = (struct media_block *)((char*)mb_video + sizeof(struct media_block));
	video_size -= sizeof(struct media_block);
	//��Ƶͷ
	mb_audio = (struct media_block *)((char*)mb_audio + sizeof(struct media_block));
	audio_size -= sizeof(struct media_block);

	//����Ƶ���ݷŵ��ڴ������0x30008
	while(isRead&&video_size>0)
	{
		//video_count++;
			//cout << "video leave : " << video_size << "frame : " << mb_video->size << endl;
			//����
			video_size -= mb_video->size;
			//cout << "debug1111" << endl;
			//�����ݴ���
			//video_pool->write_pool((char*)mb_video+sizeof(struct media_block), mb_video->size);
			if(isfirst)
			{
				isfirst=false;
				//type=0�ǣ�֡, type=1�ǣ�֡
				video_queue.push((char*)mb_video+sizeof(struct media_block), mb_video->size,0);
			}
			else
				video_queue.push((char*)mb_video+sizeof(struct media_block), mb_video->size);
			mb_video = (struct media_block *)((char*)mb_video + \
								sizeof(struct media_block) +mb_video->size);
			video_size -= sizeof(struct media_block);
	}

	//�����Ƶû���꣬����

	//����Ƶ���ݷŵ��ڴ������0x50008
	//ubuntu����¼���ļ�:aplay  -t raw -c 1 -f MU_LAW -r 8000 1.pcm
	while(isRead&&audio_size>0)
	{
		//audio_count++;
			//cout << "audio leave : " << audio_size << "frame : " << mb_audio->size << endl;
			//����
			audio_size -= mb_audio->size;
			//cout << "debug1112" << endl;
			//�����ݴ���
			//audio_pool->write_pool((char*)mb_audio+sizeof(struct media_block), mb_audio->size);
			audio_queue.push(        (char*)mb_audio+sizeof(struct media_block), mb_audio->size);
			mb_audio = (struct media_block *)((char*)mb_audio + \
								sizeof(struct media_block) +mb_audio->size);
			audio_size -= sizeof(struct media_block);
	}
	//cout << "video cout: " << video_count << "\taudio count: " << video_count*2 << " " << audio_count << endl;
	return 0;
}
/*��Ӳ�̶�����Ҫ������*/
void RWlogic::ReadDisk::read_proc()
{
#ifndef NOREAD
	sleep(1);
	int ret;
	long long seek;
	//dw = new DataWrite;
	SecBlocks *sb;
	SecBlocks *sb_video;
	SecBlocks *sb_audio;
	timecount = 0;
	//video_pool = new MediaPoolWrite(1);
	//audio_pool = new MediaPoolWrite(0);
	while(1)
	{
start:
	fprintf(stderr, "startplay0\n");
	while(!isRead)
	{
		pthread_mutex_lock(&lock);
		fprintf(stderr, "startplay1\n");
		pthread_cond_wait(&startplay, &lock);
		fprintf(stderr, "startplay2\n");
		pthread_mutex_unlock(&lock);
	}
		if(isRead)
			cout << "read channel:" << channel << " seek: " << readseek << endl;
		seek = readseek;
		while(isRead)
		{
			cout << "debug!!!!!!!!" << endl;
#ifndef READTEST
			pthread_mutex_lock(&lock);
			cout << "video_queue.size(): " << video_queue.size() ;
			cout << "audio_queue.size(): " << audio_queue.size() ;
			while(video_queue.size()>24)
			{
				//cout << "  11video_queue.size(): " << video_queue.size() << endl;
				//cout << "continue" << endl;
				//usleep(10000);
				pthread_cond_wait(&notfull, &lock);
			}
			pthread_mutex_unlock(&lock);
			//��ֹ����
			if(!isRead)
			{
				fprintf(stderr, "goto\n\n\n");
				goto start;
			}
#endif
			//��ȡͷ����
			try
			{
				//�Ȱ�512��ͷ����
				sb = new SecBlocks( sizeof(struct mult_media_block) ,seek );
			}
			catch (int ret)
			{
				if(HD_ERR_READ==ret)
				{
					ttt();
					gtlogerr("read blocks err, seek:%lld",seek);
					//������ֶ�ȡ���󣬾��˳����ѭ����ͣס
					isRead=false;
					break;
				}
				else if(BLOCK_ERR_DATA_HEAD == ret)
				{
					ttt();
					gtlogerr("read blocks head err, seek:%lld,and exit",seek);
					//������ֶ�ȡ���󣬾��˳����ѭ����ͣס
					isRead=false;
					break;
				}
			}
			struct mult_media_block *mmb = (struct mult_media_block *)((char *)sb->GetBuf() +sizeof(struct hd_frame));
			//cout << "video seek: " << mmb->video1_seek << " block num: " << mmb->video1_buf_size <<\
					"\taudio seek: " << mmb->audio1_seek << " block num: " << mmb->audio1_buf_size <<endl;
			seek = mmb->next_mult_media_block;

			sb_video = new SecBlocks(mmb->all_media_head[channel].video_buf_size, \
					mmb->all_media_head[channel].video_seek );
			sb_audio = new SecBlocks(mmb->all_media_head[channel].audio_buf_size,\
					mmb->all_media_head[channel].audio_seek );

			if( ( ret = readdataprocess(sb_video, sb_audio) ) < 0 )
			{
				cout << "error: " << ret << endl<<endl<<endl;
				//������ֶ�ȡ���󣬾��˳����ѭ����ͣס
				isRead=false;
				break;
			}
			cout << "seek: " << seek<< endl;
			//�������
			if(sb)
			{
				delete sb;
				sb = NULL;
			}
			if(sb_audio)
			{
				delete sb_audio;
				sb_audio = NULL;
			}
			if(sb_video)
			{
				delete sb_video;
				sb_video = NULL;
			}

		}
		//pthread_cond_signal(&stop_read);
	}
#endif
}

void RWlogic::ReadDisk::write_proc()
{
	sleep(1);
	ttt();
	cout << "cout : " << count<< endl;
#ifdef	WRITEHD
	using namespace std;
	char videoname[200];
	char audioname[200];
	sprintf(videoname,"/mnt/yk/video%d.264",count);
	sprintf(audioname,"/mnt/yk/audio%d.pcm",count);
	cout << "videoname :" << videoname << " audioname: " << audioname << endl;
	ofstream videoof(videoname);
	ofstream audioof(audioname);
#endif
	while(1)
	{
		if(!isRead)
		{
			if(count==0)ttt();
			sleep(1);
		}
		if(isRead)
		{

			if(video_queue.size()>0)
			{
#ifdef TESTFRAMERATE
				//֡�ʿ���
				if(framenum==0)
				{
					gettimeofday(&oldtime, NULL);
					framenum ++;
				}
				else
				{
					framenum ++;
					if(framenum==100)
					{
						gettimeofday(&nowtime, NULL);
						int tmp = (nowtime.tv_sec*1000000+nowtime.tv_usec)-(oldtime.tv_sec*1000000+oldtime.tv_usec);
						cout << "channel " << count << " frame rate: " << (float)(framenum*1000000)/tmp << endl;;
						framenum=0;
					}
				}
#endif

				//����ʱ�����
				framecount--;
				if(framecount<=0)
				{

					isRead=false;
					sleep(1);
					video_queue.clear();
					audio_queue.clear();
					continue;
				}


				//if(count==0)ttt();
				//cout << "video size: " << video_queue.size() << endl;
				Blocks *bs=video_queue.pop();
#ifdef	WRITEHD
				videoof.write((char *)bs->GetBuf(), bs->GetSize());
				videoof.flush();
#endif
#ifdef NETSEND

#endif
				delete bs;
				int i;
				for(i=0; i<2; i++)
				{
					//cout << "audio_queue size: " << audio_queue.size() << endl;
					if(audio_queue.size()<=0)break;
					Blocks *abs=audio_queue.pop();
#ifdef	WRITEHD
					audioof.write((char *)abs->GetBuf(), abs->GetSize());
					audioof.flush();
#endif
					delete abs;
				}


				usleep(10000);
			}
			else//û�����ݣ�˯��
			{
				usleep(40000);
			}

			while(audio_queue.size()>0)
			{
				//cout << "audio_queue size: " << audio_queue.size() << endl;
				//if(count==0)ttt();
				Blocks *abs=audio_queue.pop();
#ifdef	WRITEHD
				audioof.write((char *)abs->GetBuf(), abs->GetSize());
				audioof.flush();
#endif
#ifdef NETSEND

#endif
				delete abs;
			}
		}
	}




}
//type=1 ��Ƶ��type=2��Ƶ
//isi 1�ǣ�֡   0�ǣ�֡ ��Ƶ������
//���أ���ȷ����������û�����ݣ�������
int RWlogic::ReadDisk::get_date(char *data, int *size,int type,int *isi)
{
	if(type==1)
	{
		if(audio_queue.size()<=0)
			return 1;
		Blocks *abs=audio_queue.pop();
		memcpy(data,(char *)abs->GetBuf(), abs->GetSize());
		*size = abs->GetSize();
		return 0;
	}
	if(type==2)
	{
		if(video_queue.size()<=0)
			return 1;
		Blocks *abs = video_queue.pop();
		memcpy(data,(char *)abs->GetBuf(), abs->GetSize());
		*size = abs->GetSize();
		if(0 ==  abs->GetType())
			*isi = 1;
		else
			*isi = 0;
		return 0;
	}
	return 3;
}
} /* namespace gtsda */
