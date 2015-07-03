/*
 * MultMediaBlocks.cpp
 *
 *  Created on: 2013-12-23
 *      Author: yangkun
 */

#include "MultMediaBlocks.h"

namespace gtsda
{
const unsigned char MultMediaBlocks::audio_head[8] =      {0x61, 0x75, 0x64, 0x69, 0x6f, 0xa5, 0xaa, 0x5a};/*audio 0xa5,0xaa,0x5a*/
const unsigned char MultMediaBlocks::video_head[8] =      {0x76, 0x69, 0x64, 0x65, 0x6f, 0xa5, 0xaa, 0x5a};/*video 0xa5, 0xaa,0x5a*/
const unsigned char MultMediaBlocks::mult_audio_head[8] = {0x61, 0x75, 0x64, 0x69, 0x6f, 0x6d, 0x75, 0xaa};/*audio mu 0xaa*/
const unsigned char MultMediaBlocks::mult_video_head[8] = {0x76, 0x69, 0x64, 0x65, 0x6f, 0x6d, 0x75, 0xaa};/*video mu 0xaa*/
const unsigned char MultMediaBlocks::mult_media_head[8] = {0x6d, 0x75, 0x6c, 0x74, 0xaa, 0x5a, 0xa5, 0x55};/*mult 0xaa 0x5a 0xa5 0x55*/


MultMediaBlocks::MultMediaBlocks(unsigned int uBufSize,blocktype bt)
:Blocks(uBufSize, 0, bt)
{
	/**************************************************
	 * �������һ��Ϊʲô����д���ǲ��Ǹо����ܹ��죿
	 * д��ʱ���ǿ��ǵ�������<<ʹ��ʹ��SecBlocks��Ĵ���>>����������crc���㷨
	 * ������SecBlocks��struct hd_frameͷ��ռ��λ��
	 ***************************************************/
	if(bt == multmedia)//mult_media��ͬ�Ĵ���
	{
		mmb = (struct mult_media_block *)( (char *)GetBuf() + sizeof(struct hd_frame) );
		//���ݲ�ͬ���͸��Ʋ�ͬ��ͷ
		memcpy(mmb->data_head, MultMediaBlocks::mult_media_head, 8);
	}
	else
	{
		//cout <<"debug bt: " << bt<< endl;
		mb = (struct media_block *)( (char *)GetBuf() + sizeof(struct hd_frame) );
		mb->size = sizeof(struct media_block);
		//���ݲ�ͬ���͸��Ʋ�ͬ��ͷ
		add_head(mb,bt);
		//ttt();
		//myprint((const unsigned char  *)mb->data_head,8);
	}

	/***************************************************/


}
MultMediaBlocks::~MultMediaBlocks()
{
	// TODO Auto-generated destructor stub
}
//���ݲ�ͬ���͸��Ʋ�ͬ��ͷ
void MultMediaBlocks::add_head(struct media_block *m_b)
{
	//if(!m_b)throw(0);
	//��Ƶ
	if(GetType() == video)
		memcpy(m_b->data_head, MultMediaBlocks::video_head, 8);
	//��Ƶ����
	else if(GetType() == mult_video)
		memcpy(m_b->data_head, MultMediaBlocks::mult_video_head, 8);
	//��Ƶ
	else if(GetType() == audio)
		memcpy(m_b->data_head, MultMediaBlocks::audio_head, 8);
	//��Ƶ����
	else if(GetType() == mult_audio)
		memcpy(m_b->data_head, MultMediaBlocks::mult_audio_head, 8);
	//���ڷ�Χ��
	else
		throw(0); //�������ͱ��಻֧��

}
void MultMediaBlocks::add_head(struct media_block *m_b,blocktype bt)
{
	//if(!m_b)throw(0);
	//��Ƶ
	if(bt == video)
		memcpy(m_b->data_head, MultMediaBlocks::video_head, 8);
	//��Ƶ����
	else if(bt == mult_video)
		memcpy(m_b->data_head, MultMediaBlocks::mult_video_head, 8);
	//��Ƶ
	else if(bt == audio)
		memcpy(m_b->data_head, MultMediaBlocks::audio_head, 8);
	//��Ƶ����
	else if(bt == mult_audio)
		memcpy(m_b->data_head, MultMediaBlocks::mult_audio_head, 8);
	//���ڷ�Χ��
	else
	{
		cout << "ddddddddddddddddddddd bt: " << bt  << endl;
		throw(0); //�������ͱ��಻֧��
	}

}
/************************************************
 * ���ܣ���DataRead�е����ݸ��Ƶ������У����ܵĿ飺audio,video,mult_video,mult_audio
 * ������DataRead *dr ����������
 * ���أ����ڵ���0 ���Ƴɹ�
 * 		С��0    ʧ�ܣ������ǽ�Ҫ���
 ************************************************/
int MultMediaBlocks::add(DataRead *dr,blocktype bt)
{
	if(!dr)throw(0);
	//���ʣ�µĿռ仹���������뱾֡����
	if(dr->get_frame_buff_size() + sizeof(struct media_block) > this->GetSize() - sizeof(struct hd_frame) - mb->size)
	{
		ttt();
		cout << "the buff is small"<<endl;
		gtlogerr("the buff is small");
		return -1;
	}
	//����ͷ
	struct media_block *mb_local = ( struct media_block * )( (char *)mb + mb->size );
	add_head(mb_local,bt);
	mb_local->size = dr->get_frame_buff_size();


	mb->size = mb->size + sizeof(struct media_block);
	//��������
	memcpy( (char *)mb + mb->size, dr->get_the_frame_buffer()->frame_buf, dr->get_frame_buff_size());
	//cout << "size : "<< dr->get_frame_buff_size()<<endl;
	mb->size  = mb->size +  dr->get_frame_buff_size() ;
	//myprint((const unsigned char  *)mb_local->data_head,8);
	return 0;
}

} /* namespace gtsda */
