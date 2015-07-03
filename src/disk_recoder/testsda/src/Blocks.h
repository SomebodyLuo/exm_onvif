/*
 * Blocks.h
 *
 *  Created on: 2013-10-24
 *      Author: yangkun
 */

#ifndef BLOCKS_H_
#define BLOCKS_H_
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>

#include "gtlog.h"
#include <error.h>

#include "hdwr.h"
#include "err.h"

/****************************************************************
 * �ް汾0.01:ʵ�ֱȽ��ȶ��ĵ�·����Ƶ��д
 * 0.02:
 * 0.03:ʵ��1·����Ƶ��д��
 * 0.04:ʵ���ȶ�������Ƶ�Ķ�д
 * 0.05:��·����Ƶд�������
 ****************************************************************/
#define Version "0.05"
#define DEBUG
#define CRC

//ûд���߼�ʱ����ʱ�����Ƕ����߼�
//#define NOREAD

using std::cout;
using std::endl;
using std::ostream;
using std::istream;
namespace gtsda
{
#define MAX_CHANNEL 4
/*Ӳ��Ĭ�Ͽ��С512�ֽ�  ���ֵ����˵�ǲ��ܸĵ�*/
#define BLOCKSIZE 512
#define BUFSIZE  400*1024
/*����������ܹ���800��*/
#define MAXDAY 800
/*��ѭ��������ͷ��ƫ��*/
#define YEAR_OFFSET 8
#define SECOFDAY (24*3600)
#define first_block 1
//ÿ��д����������Ŀ飻�ж�Ӳ����ʱ������鱻����ʱ���߼��Ƚϼ򵥡�
//����8Mbit/s 4��ͨ��,������Ƶ�����������������ܲ���4.5Mbyte������
#define LEAVE_BLOCKS ((4*1024*1024 + 512*1024 + BLOCKSIZE -1)/BLOCKSIZE )

//#define get_block_num(buffsize) (( (buffsize) + BLOCKSIZE -1)/BLOCKSIZE )
typedef enum
{
	noblock		= 0, /*ʲô��Ҳû��*/
	block		= 1,
	collection	= 2,/*���������İ�*/
	year		= 3,
	day			= 4,
	day_bac		= 5,
	sec			= 6,
	audio		= 7,
	video		= 8,
	multmedia  = 9,/*�����������Ƶ��*/
	mult_video  = 10,
	mult_audio  = 11,

}blocktype;
typedef enum
{
	get_start = 1,
	get_tail   = 2
}get_type;
struct  seek_block{ int time; long long seek;}__attribute__ ((packed));
struct year_queue{ unsigned int queue_size ,queue_head,queue_tail;};
struct year_block
{
	unsigned char year_head[8];
	struct year_queue year_queue_data;
	struct seek_block seek_block_data[MAXDAY];
}__attribute__ ((packed));
//struct day_queue { int index, size;};
struct day_block
{
	unsigned char day_head[8];
//	struct day_queue day_queue_data;
	struct seek_block seek_block_data[SECOFDAY];
}__attribute__ ((packed));

struct hd_frame
{
	char data_head[8];			/*֡����ͷ�ı�־λ 0x5345434fa55a5aa5*/
	short frontIframe;			/*ǰһ��I֡����ڱ�֡��ƫ�Ƶ�ַ   ���ǰһ֡��Ӳ�̵�����棬���ֵ�����Ǹ�ֵ*/
	short is_I;		/*��֡�ǲ���I֡*/
	unsigned int size;			/*��֡��Ƶ���ݿ�Ĵ�С*/
	unsigned int crc;			//����λ
}__attribute__ ((packed));

//��Ƶ��,�������
//ֻ������ý��飺video,audio�飻vidoe,audio���Ͽ飻
struct media_block
{
	char data_head[8];
	unsigned int size;	//���ݳ���
	int parm;
	//����Ƶ���ԣ��Ժ�Ҫ��ӣ�����ֻ֧��h264,d1,25
	//......��������Ƶ��
}__attribute__ ((packed));


typedef struct{	//��Ƶ��ʽ��Ϣ�ṹ
	unsigned char format;		//�����ʽformat
	unsigned char  type;		//frame type	I/P/B...
	unsigned char ratio;  //�ֱ���
}video_format_t;
typedef struct{	//��Ƶ��ʽ��Ϣ�ṹ
	unsigned char  a_wformat;	//������ʽ
	unsigned char  a_sampling;	//����������
	unsigned char  a_channel;	//����ͨ��
	unsigned char  a_nr_frame;	//һ�����������м�������
	unsigned char  a_bitrate;		//��������
	unsigned char  a_bits;		//��Ƶ����λ��
	unsigned char  a_frate;		//��Ƶ���ݵ�֡��(û�����м�����Ƶ����)
}audio_format_t;
struct one_channel_media
{
	long long    video_seek;  //��Ƶ1��Ӳ����ƫ��
	unsigned int video_buf_size;
	video_format_t vft;
	long long    audio_seek;  //��Ƶ1��Ӳ����ƫ��
	unsigned int audio_buf_size;
	audio_format_t aft;
}__attribute__ ((packed));

//����Ƶ�鼯��
struct mult_media_block
{
	char data_head[8];
	struct one_channel_media all_media_head[MAX_CHANNEL];
	long long next_mult_media_block;//��һ��mult_media_block�ĵ�ַ
}__attribute__ ((packed));




class Blocks
{
public:
	Blocks(unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(char *buf,unsigned int uSizeOfBuf,unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(long long llSeek,unsigned int uBufSize);//read from disk��and get type
//ǳcopy
	Blocks(Blocks *bsBlocks,unsigned int uBufSize);
	unsigned int GetSize(){return uSize;};
	long long GetSeek(){return llSeek;};
	void SetSeek(long long llSeek)
	{
		if(llSeek<0)
		{
			cout << "set seek error" << endl;
			throw -1;
		}
		else
			this->llSeek=llSeek;

	};
	long long GetBlocks(){return uNumOfBlocks;};
	long long GetNext(){return llSeek + uNumOfBlocks;};//��һ��д���λ�ã����Ǳ������һ��
	virtual ~Blocks();
	blocktype GetType()const{return bType;};
	const void * const GetBuf()const  {return cBuff;};
	void SetBuf(char *buf,unsigned int uSizeOfBuf)
	{
		memcpy(this->cBuff, buf, (uSize<uSizeOfBuf?uSize:uSizeOfBuf));
	};
	static unsigned int get_block_num(unsigned int  buffsize){ \
		return (( (buffsize) + BLOCKSIZE -1)/BLOCKSIZE ); }
	static blocktype judge_type(const char *buf);
	friend ostream & operator <<(ostream &os,const Blocks &bk);

	int read();
	int write();
	Blocks & operator=(const Blocks &bs);
private:
	char *cBuff;
	blocktype bType;
	unsigned int uSize;			//�������ܴ�С
	unsigned int uNumOfBlocks;	//buf��ռ�Ŀ顣
	long long llSeek;			//�ڶ��ٿ�
	static pthread_mutex_t mutex_wr;
};

} /* namespace gtsda */
#endif /* BLOCKS_H_ */
