/*
 * DataWR.cpp
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#include "DataWR.h"
#include "fcntl.h"
#include <stdio.h>
namespace gtsda
{

DataWrite::DataWrite()
:Blocks(BUFSIZE,0,block)
{
	//buf = GetBuf();
	fifo_init();

}

DataWrite::~DataWrite()
{
	/*�ͷŹܵ�*/
		close(fd);
}
/*��ʼ���ܵ�*/
int DataWrite::fifo_init()
{
	mkfifo("test.264", 0777);
	int ret;
	fd = open("test.264", O_WRONLY);
	if (fd < 0)
	{
		perror("openfile error");
		return -1;
	}
	return 0;
}
/*д�ܵ�*/
int DataWrite::fifo_write(const char *buf, int size)
{
	int ret = ::write(fd, buf, size);
	if (ret < 0)
	{
		cout << " fifo write error!\n";
		return -1;
	}
	return ret;
}
int DataWrite::writedata()
{
	int ret;
	if( ( ret=fifo_write( ( char *)GetBuf(), buffsize) <0) )
	{
		cout << "fifo_write error!!"<< endl;
		return ret;
	}
	return 0;
}
DataWrite  &DataWrite::operator=(DataRead &dr)
{

	memcpy( (void *)(GetBuf()) , dr.get_the_frame_buffer()->frame_buf , dr.get_frame_buff_size());
	buffsize = dr.get_frame_buff_size();
	return *this;
}
int DataWrite::getdata(DataRead &dr)
{
	memcpy( (void *)(GetBuf()) , dr.get_the_frame_buffer()->frame_buf , dr.get_frame_buff_size());
	buffsize = dr.get_frame_buff_size();
	return 0;
}
int DataWrite::getdata(const char *buf, unsigned int buffsize)
{
	memcpy( (void *)(GetBuf()) , buf, buffsize);
	this->buffsize = buffsize;
	return 0;
}



#ifdef MEMIN
DataRead::DataRead(unsigned int uBufSize,long long llSeek, blocktype bt)
	:Blocks(uBufSize,llSeek,bt)
{
	//�����ݸ�ʽ����bblock�ĸ�ʽ
	unsigned int i;
	bs = (struct bblock *)GetBuf();
	bs->size = GetSize()/4 -2;
	bs->xxd = rand();
	unsigned int *p = (unsigned int *)(bs->p);
	for(i =0; i< bs->size; i++)
	{
		*(p+i) = bs->xxd;
	}
}
#else
DataRead::DataRead(long long llSeek,blocktype bt,unsigned int channel)
	:Blocks(BUFSIZE,llSeek,bt),seq(-1),flag(-1),the_frame_buffer(NULL)
{
	init_pool(channel);
}
/*
DataRead::DataRead(unsigned int channel)
	:Blocks(BUFSIZE,0,block),seq(-1),flag(-1),the_frame_buffer(NULL)
{
	init_pool(channel);
}
*/
#endif
DataRead::~DataRead()
{

}
unsigned int  DataRead::get_remain_frame_num()
{
	return read_media_remain(&media);
}
void DataRead::init_pool(unsigned int channel)
{
	int err;
	char name[]="record";
	//��ʼ��mediaapi
	//��ʼ������
	memset(&media ,0, sizeof(media_source_t));
	media.dev_stat= -1; //��ʾû������
	err=connect_media_read(&media ,0x30000+channel, name, /*MSHMPOOL_LOCAL_USR*/1);
	if(err!=0)
	{
		cout<< "1error in connect media read and exit: "<< err << endl;;
		gtlogerr("error in connect media read and exit\n");
		exit(1);
		//throw ret;
	}
	else
	{
		cout << "connect success\n";
		gtloginfo("connect success\n");
	}
}
//�ӻ�����������
int DataRead::readpool()
{
	memset((void *)GetBuf(), 0, BUFSIZE);
	seq=-1;flag=-1;
	int ret=read_media_resource(&media,(void *)GetBuf(), BUFSIZE, &seq, &flag);
	if(ret<0)
	{
		gtlogerr("error in read media resource %d\n",ret) ;
		return ret;
	}
	the_frame_buffer=(enc_frame_t *)GetBuf();
	buffsize = the_frame_buffer->len;
	if(flag==1)
		bIsI=false;
	else
		bIsI=true;
	return bIsI;
}



#ifdef MEMIN
int publictime;
#else
#include <time.h>
#endif
int gettime(bool add)
{
#ifdef MEMIN
	if(add)
		return publictime++;
	else
		return publictime;
#else
		return time(NULL);
#endif
}


MediaPoolWrite::MediaPoolWrite(int format)
{
	if(0==format)
		init_audio_pool();
	else
		init_video_pool();
}


int MediaPoolWrite::write_pool(char *buf, unsigned int buf_size)
{

	int ret;
	//the_frame_buffer->tv.tv_sec=time111[channel]/1000000;
	//the_frame_buffer->tv.tv_usec=time111[channel]%1000000;
	the_frame_buffer->len=buf_size;
	the_frame_buffer->chunk.chk_siz=buf_size;
	the_frame_buffer->type=1;
	memcpy(the_frame_buffer->frame_buf,buf,buf_size);

	//д�ĳ���һ��Ҫ���Ͻṹ��ĳ���
	ret=write_media_resource(&media,the_frame_buffer,buf_size+sizeof(enc_frame_t),0);
	if(ret<0)
	{
		printf("cat't write media resource and exit");

	}
	//printf("write ok:size=%d\n\n\n\n",tmp);

	return 0;
}
void MediaPoolWrite::init_video_pool()
{
	int ret;
	ret=init_media_rw(&media,MEDIA_TYPE_VIDEO,0,BUFFER_SIZE);
	if(ret<0)
	{
		printf("error in init_media_rw,and exit\n");
		exit(1);
	}
	char *name = "record_video_write";
	ret=create_media_write(&media, 0x30008,name,VIDEO_POOL_SIZE);/*���һ·��ƴ�Ӵ洢��*/
	media.attrib->stat=ENC_STAT_OK;
	media.buflen=BUFFER_SIZE;

	media.attrib->fmt.v_fmt.format=VIDEO_H264;
	media.attrib->fmt.v_fmt.ispal=0;
	media.attrib->fmt.v_fmt.v_buffsize=BUFFER_SIZE;
	media.attrib->fmt.v_fmt.v_frate=25; //֡��
	media.attrib->fmt.v_fmt.v_height= 576;
	media.attrib->fmt.v_fmt.v_width= 704;
	//gtloginfo("encode media pool:height=%d,width=%d\n",get_para()->psize.height,get_para()->psize.width);
	//�û���ʱ
	//MSHmPoolSetMaxNum_t(1);


	//test
	if( media.temp_buf ==NULL)
	{
		printf("1111error in media tmemp_buf\n");
	}
	the_frame_buffer=(enc_frame_t *)media.temp_buf;		//�����Ǹ�avi�ṹͷ����������
	memset(the_frame_buffer, 0, sizeof(enc_frame_t));
	the_frame_buffer->channel=0;					//ͨ����
	the_frame_buffer->chunk.chk_id=IDX1_VID;		//��Ƶ
	the_frame_buffer->media=MEDIA_VIDEO;
}
void MediaPoolWrite::init_audio_pool()
{


	int ret;
	int i;


	//��ʼ��ý���д��Ϣ
	ret=init_media_rw(&media,MEDIA_TYPE_AUDIO,0,AUDIO_BUFFER_SIZE);
	if(ret<0)
	{
		printf("error in init_media_rw,and exit\n");
		exit(1);
	}
	//��ʼ���ڴ��
	/*�ڴ�ص�����Ҫ�䶯*/
	char *name = "record_audio_write";
	/*�ڴ�ص�idҪ�䶯*/
	ret=create_media_write(&media, 0x50008,name,AUDIO_ENC_POOL_SIZE_NET);
	if(ret<0)
	{
		printf("error in create_media_write\n");
		exit(1);
	}
	media.attrib->stat=ENC_STAT_OK;
	media.buflen=AUDIO_BUFFER_SIZE;

	media.attrib->fmt.a_fmt.a_wformat=7;		//WFORMAT_ULAW			0x0007//������ʽ
	media.attrib->fmt.a_fmt.a_sampling=8000;	//����������
	media.attrib->fmt.a_fmt.a_channel=1;		//����ͨ��
	media.attrib->fmt.a_fmt.a_nr_frame=1;		//һ�����������м�������
	media.attrib->fmt.a_fmt.a_bitrate= 8000;		//��������
	media.attrib->fmt.a_fmt.a_bits= 8;			//��Ƶ����λ��
	media.attrib->fmt.a_fmt.a_frate= 8;			//(8000/1024)��Ƶ֡�� FIXME
	//�û���ʱ



	//test
	if( &media.temp_buf ==NULL)
	{
		printf("1111error in media tmemp_buf\n");
	}
	the_frame_buffer=(enc_frame_t *)media.temp_buf;		//�����Ǹ�avi�ṹͷ����������
	memset((void *)the_frame_buffer, 0, sizeof(enc_frame_t));
	the_frame_buffer->channel=0;					//ͨ����
	the_frame_buffer->chunk.chk_id=IDX1_AID;		//��Ƶ
	the_frame_buffer->media=MEDIA_AUDIO;

	the_frame_buffer->len=160;// 20130717 320;//yk 1024;
	the_frame_buffer->chunk.chk_id=IDX1_AID;
	the_frame_buffer->chunk.chk_siz=160;// 20130717 320;//yk 1024;
	the_frame_buffer->type=FRAMETYPE_PCM;

}


} /* namespace gtsda */
