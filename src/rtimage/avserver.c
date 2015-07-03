/** @file	       avserver.c
 *   @brief 	�ṩ����Ƶ�������غ���ʵ��
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <media_api.h>
#include <venc_read.h>
#include <gtthread.h>
#include <ime6410api.h>     ///FIXME stream_fmt_struct�ṹ����ŵ������ط�!
#include <AVIEncoder.h>
#include <commonlib.h>
#include <gt_errlist.h>
#include "rtnet_cmd.h"
#include "net_avstream.h"
#include <soundapi.h>
#include <audiofmt.h>
#include "audio_pool.h"
#include "maincmdproc.h"
#include <gtsocket.h>
#include <devinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<dirent.h>
#include <time.h>
#include "avserver.h"
#include <hddbuf.h>
#include <gtsf.h>
//zsk add for getticketcount
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <nvp1918.h>
#include "play_back.h"
#include "avilib.h"
#include <onviflib.h>
//#define DEBUG
#define TCP_SEND_RESERVE_AUDIO      32*1024                                 ///<Ϊ������Ƶ���ݱ����Ļ�����
#define TCP_SEND_ADJUST_STEP         128*1024                                ///<ÿ�ε���tcp���ͻ�������С�Ĳ���
#define TCP_SEND_BUF_MAX                1024*1024                               ///<tcp���ͻ����������ֵ
static unsigned char    avi_head_buf[MAX_VIDEO_ENCODER][1024*3];   ///<���aviͷ�Ļ�����
static  int             avi_head_len[MAX_VIDEO_ENCODER];                ///<avi_head_buf��ÿ��Ԫ�ص���Ч�ֽ���
static  unsigned char frame_head_buf[MAX_VIDEO_ENCODER][28];
extern unsigned long  net_pkts_sta;
struct timeval  venc_last_tv[MAX_VIDEO_ENCODER];       ///<��Ƶ������0���һ֡��ʱ���
struct timeval  venc_pb_tv[MAX_AUDIO_ENCODER];
 ///����8180����Ƶ��ʽ����ǰ��̫һ��������ʱ��ת��һ��
static __inline__ void convert_intra_frame(unsigned long *flag)
{
    #define IFRAME_HEAD         0xb0010000
    *flag=IFRAME_HEAD;
}
 
/** 
 *   @brief     ��ʼ������Ƶ�����õ�����ر���
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
static int init_avserver(void)
{
    init_video_enc();                   ///<��ʼ����Ƶ�����������
    //init_audio_enc_pool();          ///<��ʼ����Ƶ�����������//zsk del
    init_audio_enc();
//#ifdef USE_ADEC_POOL
	init_audio_dec_pool();
//#endif
    return 0;
}
/** 
 *   @brief     ����ָ����ű�������aviͷ��Ϣ
 *   @param  no ��Ƶ���������
 *   @param  attrib ��Ƶ���������Խ��ָ��
 *   @return   0��ʾ�ɹ���ֵ��ʾʧ��
 *                 avi ͷ��Ϣ�����avi_head_buf������
 */
static int gen_venc_avi_head(IN int no)
{
    int     ret;
    struct defAVIVal davi;
    video_format_t	*vfmt=NULL;
	audio_format_t  *afmt=NULL;
	media_attrib_t * a_attrib=NULL;
	media_attrib_t * v_attrib=NULL;

    if(no>=MAX_VIDEO_ENCODER)
        return -EINVAL;

    memset((void*)&davi,0,sizeof(struct defAVIVal));

	v_attrib=get_venc_attrib(no);
	if(v_attrib!=NULL)
	{
		vfmt=&v_attrib->fmt.v_fmt;
		switch(vfmt->format)
		{
			case VIDEO_H264:
				sprintf(davi.v_avitag,"H264");
			break;
			case VIDEO_MJPEG:
				sprintf(davi.v_avitag,"MJPG");
			break;
			case VIDEO_MPEG4:
				sprintf(davi.v_avitag,"divx");
			break;
			default:
				sprintf(davi.v_avitag,"H264");
			break;

		}
		davi.ispal=vfmt->ispal;				    //�Ƿ���pal����Ƶ
		davi.nr_stream=2;					    //�м���ý���� 1��ʾ����Ƶ 2��ʾ��Ƶ��Ƶ����
		davi.v_width=vfmt->v_width;		        //ͼ����
		davi.v_height=vfmt->v_height;		    //ͼ��߶�
		davi.v_frate=vfmt->v_frate;			    //֡��
		davi.v_buffsize=vfmt->v_buffsize;	    //���黺������С


	}

	a_attrib=get_aenc_attrib(no);
	if(a_attrib!=NULL)
	{
		afmt=&a_attrib->fmt.a_fmt;
		davi.a_sampling=afmt->a_sampling;
		davi.a_channel=afmt->a_channel;
		davi.a_bitrate=afmt->a_bitrate;
		davi.a_wformat=afmt->a_wformat;	         //��Ƶ��ʽ
		davi.a_nr_frame=afmt->a_nr_frame;
		
	}

    ret=FormatAviHeadBufDef(&davi,avi_head_buf[no], sizeof(avi_head_buf[no]));
    avi_head_len[no]=ret;

    return 0;
}
int preflag[MAX_VIDEO_ENCODER]={0};
int sendflag=0;
int  write_rawaudio2file_send(void *buf,int len)
 {
	 static FILE *afp=NULL; 	 ///<�ļ�ָ��
	 char filename[50];
	 int ret=0;
		
	 if(afp==NULL&&sendflag==0)
	 {
	 		sendflag=1;
		 sprintf(filename,"/mnt/zsk/send.g711u");
		 afp=fopen(filename,"wb");
		 if(afp==NULL)
		 {
			 printf("can't create file:%s errno=%d:%s!!\n",filename,errno,strerror(errno));
			 return -errno;
		 }
	 }
	 if(afp!=NULL)
	 {
		 ret=fwrite(buf,1,len,afp);
		 fflush(afp);
	 }
	 return ret;
	 
 }
 static int  write_rawaudio2file_pre(void *buf,int len,avi_t **AVI)
 {
	 //static FILE *afp=NULL; 	 ///<�ļ�ָ��
	 char filename[50];
	 int ret=0;
	 if(*AVI==NULL||preflag[0]==0)
	 {
	      preflag[0]=1;
		 sprintf(filename,"/mnt/zsk/pre.g711u");
		 *AVI=AVI_open_output_file(filename);
		 if(*AVI==NULL)
		 {
			printf("AVI_OUTPUT_FILE open failed!! name[%s]\n",filename);
			close((*AVI)->fdes);
		 
			return -1; 
		 }
		 AVI_set_audio(*AVI,2,16000,16,WAVE_FORMAT_MULAW,0);
		 (*AVI)->mode=AVI_MODE_WRITE;
		 /*afp=fopen(filename,"wb");
		 if(afp==NULL)
		 {
			 printf("can't create file:%s errno=%d:%s!!\n",filename,errno,strerror(errno));
			 return -errno;
		 }
		 */
		 
	 }
	 if(*AVI!=NULL)
	 {
		 ret=AVI_write_audio(*AVI,buf,len);
		 //ret=fwrite(buf,1,len,afp);
		 //fflush(afp);
	 }
	 return ret;
	 
 }

 /** 
 *   @brief     ����ý�����ݽṹ������������
 *   @param  fd �Լ��򿪵�����������
 *   @param  frame ָ��Ҫ���͵����ݽṹָ��
 *   @param  v_fmt ��Ƶ�����ʽVIDEO_MPEG4, VIDEO_H264 ,VIDEO_MJPEG
 *   @return  ��ֵ��ʾ���ͳ�ȥ���ֽ���,��ֵ��ʾ����
 */ 
static inline int write_frame_data2trans
	(IN int fd,IN char * head_buf,IN struct stream_fmt_struct *frame,int v_fmt)
{
    //static  unsigned long  iframe_head=0xb0010000; 
	unsigned char *p;
	int len;
	int ret=-1;
	//char	head_buf[4+sizeof(frame->tv)+4+8];
	memset(head_buf,0,GTSF_HEAD_SIZE);
	struct NCHUNK_HDR *chunk=&frame->chunk;
    if(fd<0)
            return -EINVAL;
       
	if(frame->media==MEDIA_VIDEO)
    {   
		chunk->chk_id=IDX1_VID;         
		/*
        if((v_fmt!=VIDEO_MJPEG)&&( frame->type==FRAMETYPE_I))
        {
	    //v2.25 ���ڿؼ�����֧��00000100�Ĺؼ���,�����ת�����ü���if(v_fmt!=VIDEO_MPEG4)				///<MPEG4����ʱҲ����I֡ͷ���б����,��Ϊ���������ܽ⾭������ת����GTVS3000��ͼ��,�°汾ת��Ҳ֧��00000100�Ĺؼ�֡��־��
                	convert_intra_frame((void*)frame->data);       ///����ʱ��������ת��
        }
		*/
	}
	else if(frame->media==MEDIA_AUDIO)
	{
		chunk->chk_id=IDX1_AID;
		//write_rawaudio2file_send(&frame->data,frame->len); 
	}	
	frame->chunk.chk_siz=frame->len;	
	/*
	if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
	{ 
		frame->chunk.chk_siz+=4;    
	}
	*/
    do
	{


	      ///��������ͷ
		memcpy(head_buf,&frame->chunk.chk_id,4);
		memcpy(&head_buf[4],&frame->tv,sizeof(frame->tv));
		memcpy(&head_buf[4+sizeof(frame->tv)],&frame->chunk.chk_siz,4);
		ret=fd_write_buf(fd,head_buf,4+sizeof(frame->tv)+4);
		if(ret!=(4+sizeof(frame->tv)+4))
		{			
			break;
		}
		/*
		if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
		{
	    	ret=fd_write_buf(fd,&iframe_head,sizeof(iframe_head));
	    	if(ret!=4)
	        	break;
		}
		*/


	  	///����������
		p=(unsigned char*)frame->data;
		//MAMAYATOTO
		len=(int)frame->len;
		if(frame->type==0)
		{
			//printf("frame I ------------timestamp[%d.%d]\n",frame->tv.tv_sec,frame->tv.tv_usec);
		}


		
		ret=fd_write_buf(fd,p,len);	
		//printf("���������� fd =%d,ret=%d,len=%d chk_size=%ld\n",fd,ret,len,frame->chunk.chk_siz);
		if(ret>0)
		{
			ret+=(4+sizeof(frame->tv)+4);
			if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
			{
				ret+=4;
			}
	    }
	}while(0);
	return ret;
}

static inline int write_frame_data2sdk
	(IN int fd,IN char * head_buf,IN struct stream_fmt_struct*frame,IN media_format_t * fmt)
{
	//int len;
	int ret=0;
	//unsigned short media;
	//unsigned short frame_type;
	//unsigned long  * head_tag=NULL;
	gtsf_stream_fmt	*pstream=NULL;;
	stream_video_format_t  *pV_fmt=NULL;
    stream_audio_format_t  *pA_fmt=NULL;
    stream_format_t  *pmedia_format=NULL;
	pstream = (gtsf_stream_fmt*)head_buf;
	pmedia_format = &pstream->media_format;
	//copy head
	
	//len=frame->len;
	//media=frame->media;
	//frame_type=frame->type;
	memset(pstream,0,GTSF_HEAD_SIZE);
	pstream->mark = GTSF_MARK;
	pstream->encrypt_type = 0;
	pstream->len = frame->len;
	if(frame->media==MEDIA_VIDEO)
   	{

		pstream->type=MEDIA_VIDEO;
		pV_fmt = (stream_video_format_t  *)&pmedia_format->v_fmt;
		pV_fmt->format = VIDEO_H264;
		pV_fmt->type=frame->type;
		if(( fmt->v_fmt.v_width==720)&&(fmt->v_fmt.v_height== 576))
		{
		pV_fmt->ratio = RATIO_D1_PAL;
		}
		else if(( fmt->v_fmt.v_width == 704)&&(fmt->v_fmt.v_height== 576))
		{
		pV_fmt->ratio = RATIO_D1_NTSC;
		}
		else if(( fmt->v_fmt.v_width == 352)&&(fmt->v_fmt.v_height== 288))
		{
		pV_fmt->ratio = RATIO_CIF_PAL;
		}
		else if(( fmt->v_fmt.v_width == 320)&&(fmt->v_fmt.v_height== 240))
		{
		pV_fmt->ratio = RATIO_CIF_NTSC;
		}    

   }//video
   else
   {
		pstream->type=MEDIA_AUDIO;
        pA_fmt = (stream_audio_format_t  *)&pmedia_format->a_fmt;
        pA_fmt->a_channel = 1;
        pA_fmt ->a_wformat = 7;
        pA_fmt->a_sampling = 8;
        pA_fmt->a_bits = 8;
        pA_fmt->a_bitrate = 64;

   }
	//audio
	//printf("mark[%#x]\ntype[%#x]\nencrypt_type[%#x]\nchannel[%#x]\nversion[%#x]\nlen[%d]\n",pstream->mark,pstream->type,pstream->encrypt_type,\
	pstream->channel,pstream->version,pstream->len);
	do
	{
		ret=fd_write_buf(fd,(char*)pstream,GTSF_HEAD_SIZE);
		if(ret!=GTSF_HEAD_SIZE)
		{			
			break;
		}
		if(frame->type==FRAMETYPE_I)
		{
			  
			convert_intra_frame((void*)frame->data); 	  ///����ʱ��������ת��
		}

		//head_tag=frame->data;
		//*head_tag=0x01000000;
		ret=fd_write_buf(fd,(char*)frame->data,frame->len);
		if(ret>0)
		{
			ret+=GTSF_HEAD_SIZE;
		
		}
	}while(0);
	return ret;

}

////////////////////////////map���////////////////////////////////////////

/** 
 *   @brief     ����ý����Ϣӳ��������һ������Ԫ����Ϣ
 *   @param  map ָ����ý����Ϣӳ��ṹ��ָ��
 *   @param  flag Ԫ�ص�����
 *   @param  bytes �¼�Ԫ�ص��ֽ���
 *   @return   0��ʾ�ɹ� ��ֵ��ʾ����
 */ 
static inline int add_ele2map(IO stream_send_map_t *map,IN int flag,IN int size)
{
    map_frame_t *t_frame=&map->frame_map[map->tail];             ///<��βָ��

    t_frame->flag=flag;
    t_frame->size=size;
    if(++map->tail>=MAX_MAP_BUF_FRAMES)
        map->tail=0;
    if(map->tail==map->head)
    {///��������,Ӧ�ò��ᷢ������������������˵����Ҫ����MAX_MAP_BUF_FRAMES
        printf("tail=head=%d ",map->tail);
        showbug();
    }
    
    if(flag==FRAMETYPE_PCM)
        map->a_frames++;                ///<��Ƶ
    else
        map->v_frames++;                ///<��Ƶ
    
    return 0;
}





#if 0

/** 
 *   @brief     ����map�е���Ч�ֽ���
 *   @param  map ָ����ý����Ϣӳ��ṹ��ָ��
 *   @return   map�е���Ч�ֽ���
 *   ������
 */
static inline int calc_map(IO stream_send_map_t *map)
{///������,
    map_frame_t *frame;
    int     total=0;
    int     i;
    int     head=map->head;
    int     tail=map->tail;
    while(head!=tail)
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        if(head!=tail)
        {
            frame=&map->frame_map[head];
            total+=frame->size;
            if(++head>=MAX_MAP_BUF_FRAMES)
                head=0;
        }
        else
        {
            break;
        }
    }
    return total;
}
/** 
 *   @brief     ��ӡmap�е�Ԫ��
 *   @param  map ָ����ý����Ϣӳ��ṹ��ָ��
 *   @return   map�е���ЧԪ����
 *   ������
 */
static inline int print_map(IN stream_send_map_t *map)
{///������,��ӡmap�е�Ԫ��
    map_frame_t *frame;
//    int     i;
    int     total=0;
    int     head=map->head;
    int     tail=map->tail;
    printf("map:");
    while(head!=tail)
    {
        frame=&map->frame_map[head];
        printf("%d,",frame->size);
        if(++head>=MAX_MAP_BUF_FRAMES)
            head=0;
        total++;
    }
    printf("(%d)\n",total);
    return total;
}
#endif
 /** 
 *   @brief     �ӷ���ý����Ϣӳ����ɾ��ָ���ֽ���������
 *   @param  map ָ����ý����Ϣӳ��ṹ��ָ��
 *   @param  bytes Ҫɾ�����ֽ���
 *   @return   ��ֵ��ʾɾ����Ԫ����
 */ 
static inline int del_ele_from_map(stream_send_map_t *map,int bytes)
{
    int i;
    int remain=bytes;                 ///<����Ҫɾ�����ֽ���
    int dels=0;                             ///<ɾ����Ԫ����
    map_frame_t *frame;             ///<ָ��map��Ԫ�ص�ָ��
    if(map->head==map->tail)
        return 0;                           ///<���п�
    if(bytes<=0)
        return 0;
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        frame=&map->frame_map[map->head];
        if(frame->size<=remain)
        {
            remain-=frame->size;
            if(++map->head>=MAX_MAP_BUF_FRAMES)
                map->head=0;
            dels++;
            if(frame->flag==FRAMETYPE_PCM)
                map->a_frames--;
            else
                map->v_frames--;
            if(map->head==map->tail)
            {///�����Ѿ�����
                //printf("map is empty!! head=tail=%d! remain=%d vframe=%d aframe=%d\n",map->head,remain,map->v_frames,map->a_frames);
                break;
            }
        }
        else
        {
            frame->size-=remain;
            break;
        }
    }
    return dels;
}
///////////////////////////////////////////////////////////////////////////////////

#if 0
 /** 
 *   @brief     �����û��ķ���socket��������С
 *   @param  usr �û�ָ��
 *   @param  size Ҫ���ڵĴ�С ��ֵ��ʾ��Ҫ���� ��ֵ��ʾ��Ҫ��С
 *   @return   0��ʾ�ɹ� ��ֵ��ʾʧ��
 */ 
static int adjust_usr_sock_buf(av_usr_t *usr,int size)
{
    int ret;
    int send_len=net_get_tcp_sendbuf_len(usr->fd);
    tcprtimg_svr_t    *p=get_rtimg_para();
    
    send_len+=size;

    #if 0
    //if(send_len>TCP_SEND_BUF_MAX)
    if(send_len>200*1024)
    {
        return -ENOMEM;
    }
    #endif
    //��������������Ϊ1
    net_set_tcp_sendbuf_len(usr->fd,1);
    ////send_len=150*1024;

    //������������Ϊ���
    send_len=p->tcp_max_buff*1024;
    ret=net_set_tcp_sendbuf_len(usr->fd,send_len);
    if(ret<0)
    {
        return -errno;
    }
    send_len=net_get_tcp_sendbuf_len(usr->fd);
    //printf("adjuest_buf to %dk!!\n",send_len/1024);
    //����Ӧ�ó���ֻ��(max*1024)*80%�Ŀռ���ã�����ʵ�����õ���max*1024
    usr->sock_attr.send_buf_len=send_len*0.8;               ///<ֻ����80%�Ŀռ����ʹ��,�����Ļ�����������
    //zw-add
    // ������ǰû�з��ͳ�ȥ�����ݰ���û�дﵽ������������80%
    if(usr->sock_attr.send_buf_remain>usr->sock_attr.send_buf_len)
    {
        printf("ԭ����send_buf_remain[%d]̫��\n",usr->sock_attr.send_buf_remain);
        //����ﵽ�˵Ļ�������ʱ��δ���͵����ݰ����������ϼ��ٵ�send_buff_len��ô��,��ҪĿ���ǽ��������Ӱ��׶�
        //����send_buf_remain,ƭһ��Ӧ�ó���
        usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len;
       //// usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len*2/5;
        printf("����send_buf_remain=[%d]\n",usr->sock_attr.send_buf_len);
    }

    //zw-add

    return 0;
}
#endif
static int adjust_usr_sock_buf(av_usr_t *usr,int size)
{
    int ret;
    int send_len=net_get_tcp_sendbuf_len(usr->fd);
    tcprtimg_svr_t    *p=get_rtimg_para();
    
    send_len+=size;

    #if 0
    //if(send_len>TCP_SEND_BUF_MAX)
    if(send_len>200*1024)
    {
        return -ENOMEM;
    }
    #endif
    //��������������Ϊ1
    net_set_tcp_sendbuf_len(usr->fd,1);
    ////send_len=150*1024;

    //������������Ϊ���
    send_len=p->tcp_max_buff*1024;
    ret=net_set_tcp_sendbuf_len(usr->fd,send_len);
    if(ret<0)
    {
        return -errno;
    }
    send_len=net_get_tcp_sendbuf_len(usr->fd);
    //printf("adjuest_buf to %dk!!\n",send_len/1024);
    //����Ӧ�ó���ֻ��(max*1024)*80%�Ŀռ���ã�����ʵ�����õ���max*1024
    usr->sock_attr.send_buf_len=send_len*0.8;               ///<ֻ����80%�Ŀռ����ʹ��,�����Ļ�����������
    //zw-add
    // ������ǰû�з��ͳ�ȥ�����ݰ���û�дﵽ������������80%
    if(usr->sock_attr.send_buf_remain>usr->sock_attr.send_buf_len)
    {
        printf("ԭ����send_buf_remain[%d]̫��\n",usr->sock_attr.send_buf_remain);
        //����ﵽ�˵Ļ�������ʱ��δ���͵����ݰ����������ϼ��ٵ�send_buff_len��ô��,��ҪĿ���ǽ��������Ӱ��׶�
        //����send_buf_remain,ƭһ��Ӧ�ó���
        usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len;
       //// usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len*2/5;
        printf("����send_buf_remain=[%d]\n",usr->sock_attr.send_buf_len);
    }

    //zw-add

    return 0;
}


 /** 
 *   @brief     ����ָ��ý������������ݸ���Ӧ���û�
 *   @param  enc ָ��ý���������ָ��
 *   @param  frame ָ��Ҫ���͵����ݽṹָ��
 *   @param  usr    �����͵��û�����
 *   @param  seq    ý�����ݵ����к�
 *   @param  flag   ý�����ݵı��
 *   @return   ��ֵ��ʾ�ɹ���ֵ��ʾ����,0��ʾʲôҲû�з���
 */
 unsigned long switch_cnt_2=0;
 unsigned long switch_cnt_1=0;
  int send_i_flag=1;
  int send_i_interval=0;
  int old_i_interval=0;
  int drop_p_flag=0;

int old_v_frames=0;
struct timeval last_tv;
int debug=0;

void begin_debug(int signo)
{
	gtloginfo("receive sigusr1,begin_debug\n");
	debug=1;

}

void end_debug(int signo)
{
	gtloginfo("receive sigusr2,end_debug\n");
	debug=0;

}
static inline int send_media_frame2usr
	(av_usr_t *usr,media_source_t *enc,struct stream_fmt_struct *frame,int seq,int flag)
{
    int ret;
    int send_bytes;
    stream_send_info_t  *send_info=&usr->send_info;         ///<������Ϣ
    socket_attrib_t         *sock_attr=&usr->sock_attr;         ///<socket��Ϣ
    int len;
	int no;
    tcprtimg_svr_t    *p=get_rtimg_para();
	
	no=enc->no;
    pthread_mutex_lock(&usr->u_mutex);
    do
    {
    	//netcmdavihead
        if(!send_info->send_ack_flag)
        {///<��û�з�����Ӧ��aviͷ

            if(enc->media_type==MEDIA_TYPE_VIDEO)
            {
				gen_venc_avi_head(enc->no);
                ret=send_rtstream_ack_pkt(usr->fd,RESULT_SUCCESS,(char*)avi_head_buf[enc->no],avi_head_len[enc->no]);
                if(ret>=0)
                {   ///������Ӧ���ɹ�
                    sock_attr->send_buffers=get_fd_out_buffer_num(usr->fd);
                    add_ele2map(&send_info->map,FRAMETYPE_P,sock_attr->send_buffers);   ///<��map�з�һ��������
                    send_info->total_put=sock_attr->send_buffers;               ///<��ʼ�������ֽ���
                    send_info->total_out=0;                                                 
                    send_info->send_ack_flag=1;                                         ///<�Ѿ����͹��ļ�ͷ
                    send_info->first_flag=1;
					send_info->require_i_flag=1;
                    ret=1;                                                                            ///<����һ֡���ݿ�ʼ����
                    break;
                }
            }
            else
            {
                ret=0;
                break;
            }
        }
        else
        {            
        		if(debug==1)
        		{
        			printf("pool_read_flag______[%d]\n",flag);
        			gtloginfo("pool_read_flag______[%d]\n",flag);
        		}
                if(send_info->first_flag)
                {
                    if(flag==FRAMETYPE_P)
                    {
	                    	if(get_playback_stat(no)<0&&send_info->require_i_flag==1)
	                    	{
	                    		require_videoenc_iframe(enc->no);
								send_info->require_i_flag=0;
	                    	}
                    
                        ret=0;
                        break;
                   
                    }
                    else if(flag==FRAMETYPE_I)
                    {
                    
                        send_info->first_flag=0;						
                    }
                }
            
        }

        //send_buf_remain��send_buffers���������ĺ�����Щ����
        //send_buf_remain��socket������ʵʱ��ȡ�����ĵ�ǰʣ����ֽ���
        //��Ϊsocket�������첽�ģ�send_buffers��¼�����ϴη�����֮��Ļ������������ֽ�������
        //���η�������ǰ��send_buffers��ȥ send_buf_remain���ʾ������֮�䷢���˶����ֽ���


        ///socket���ͻ������е�����
        sock_attr->send_buf_remain=get_fd_out_buffer_num(usr->fd);                    ///<socket���ͻ������ڵ��ֽ���
        send_bytes=sock_attr->send_buffers-sock_attr->send_buf_remain;              ///<�շ��ͳ�ȥ���ֽ���
        sock_attr->send_buffers=sock_attr->send_buf_remain;
        ret=del_ele_from_map(&send_info->map,send_bytes);
        send_info->total_out+=send_bytes;                                                             ///<�ܹ����ͳ�ȥ���ֽ���
        if(send_bytes<0)
        {
        	gtloginfo("send_bytes=%d\n",send_bytes);
            printf("send=%d sock_attr->send_buf_remain=%d!!!xxx!!!!!!!!!!!send_bytes=Ϊ����!!!!!!!!!!!!!!\n",send_bytes,sock_attr->send_buf_remain);
        }
        else
       {	
  			//printf("=============================================ramin=%d============��ǰ�������е�֡��=%d\n",sock_attr->send_buf_remain,send_info->map.v_frames);
		}

	
        //��ÿ�η�����Ƶ֡��ʱ�򶼻�ʵʱ���޸����ڼ�¼ͳ�����ݵı���
        //ͬʱ��ÿ�η��͵�ʱ��Ҳ��ͨ���ϴε�ͳ���������жϵ�ǰ�Ļ�������С�����
        if(enc->media_type==MEDIA_TYPE_VIDEO)
        {///��������Ƶ����
            //�����ǰ�Ļ���������Ƶ֡������Ƶ֡�ټӱ�����10֡����MAX_MAP_BUF_FRAMES�Ļ�
            //���ξͲ��������������ˣ�ֱ�Ӷ������ε���Ƶ֡����socket�������е����ݷ������
            //Ȼ��ʹ��drop_v_flag,send_i_flag��drop_v_frames��¼��ǰ�Ķ�֡����״̬
            //������������Աȣ������֡�����Ǹ���tcprtimg�Ļ������жϵģ�tcprtimgӵ���Լ��Ļ�����
            //��������������videoenc�Ļ������ж�ȡ���ݣ���������ȡ�ٶȱȽϿ�Ļ������¶�
            //���������ݳ�����tcprtimg�Լ�����󻺳��������޵Ļ���tcprtimg�Ͳ��ٽ���ȡ��������Ƶ��
            //��֡����socket�ķ��ͻ�����ȥ�����ˣ����ǲ��ö�����ǰ�Ķ������֡�����������Ż�
            //������ԭ�е�֡��ֱ��tcprtimg����������֡С��MAX_MAP_BUF_FRAMESΪֹ��Ȼ���ٽ�����������
            //֡���ŷ��͵�socket�ķ��ͻ����������ڸ���socket�ķ��ͻ�������������Ƿ���Ҫ��֡�Ļ�
            //���������ķ���
            if((send_info->map.v_frames+send_info->map.a_frames+10)>MAX_MAP_BUF_FRAMES)
            {///Ϊ��Ƶ����Ԥ��10֡�ռ�
                    printf("line:[%d]usr:%d not enough map drop video:%d!!break\n",__LINE__,usr->no,(int)frame->len);
                    		gtloginfo("line:[%d]usr:%d not enough map drop video:%d!!break\n",__LINE__,usr->no,(int)frame->len);
                    send_info->drop_v_flag=1;
                    send_info->send_i_flag=0;
                    send_info->drop_v_frames++;
                    ret=0;
                    break;                     
            }

            //��socket�������еĿ��ÿռ������ж��Ƿ���Ҫ������������С��������Сʱ�ѽ���ǰ����
            //һ֡������
            //if((sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO)<(frame->len+sizeof(struct stream_fmt_struct)*2))
            ////printf("line[%d] [%d]--[%d].....drop_len=[%d].............switch_cnt=%ld\n",__LINE__,sock_attr->send_buf_len,sock_attr->send_buf_remain,(sock_attr->send_buf_len)*3/5,switch_cnt);                

			   if(net_pkts_sta<p->pkts_limit)
                {              
 
                    //������ռ�С��80*1024ʱ����˴�������������Ϊ200*1024�����Ҷ�֡��������������
                    //�ȴ������Ǳ߰�Ŀǰʣ���֡�����ͳ�ȥ������⵽ʣ��ռ����80*1024ʱ��ת��
                    //���󻺳������ã�������������Ϊ800*1024,Ȼ�����������������
                    ret=adjust_usr_sock_buf(usr,TCP_SEND_ADJUST_STEP);  ///<socket���ͻ���������,���ڷ��ͻ�����
                    //if((sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO)<(frame->len+sizeof(struct stream_fmt_struct)*2))

                    //��������ӿ���Ҫ���adjust_usr_sock_buf()���жϡ�adjust_usr_sock_buf()���ж�remain�Ƿ�ﵽ
                    //������������80%,����ﵽ�ˣ���϶����������Ľӿڣ���Ϊ�Ѿ���remain���¸�ֵ��,remain��buf_len��ȣ����û�дﵽ80%����ô���ȼ���һ�£��Ѿ����ͳ�ȥ�����ݰ����Ƿ�ﵽ������������3/5,���û�дﵽ����Ҳ��Ҫ�ӵ��µ����ݰ�,���ٽӿ��ˣ��Ȱ�ʣ�µ�δ���ͳ�ȥ�İ������ź�������˵��
                     if((sock_attr->send_buf_len-sock_attr->send_buf_remain< ((sock_attr->send_buf_len) *3/5)))
                    { ///����û�з��䵽�㹻�Ļ�����
                        //printf("line[%d]usr:%d not enough buf!%d<%d drop video frame !!\n",__LINE__,usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO),(int)(frame->len+sizeof(struct stream_fmt_struct)*2));
                        printf("������line[%d]usr:%d not enough buf!%d<%d drop video frame !!\n",__LINE__,usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain),80*1024);
                        send_info->drop_v_flag=1;
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        ret=0;
                        break;                    
                    }               
                }
                else 
                {   

                    //�������û�800k
                    len=net_get_tcp_sendbuf_len(usr->fd);
                    if(len!=512*1024)
                    {
                        len=512*1024;
                        net_set_tcp_sendbuf_len(usr->fd,len);
                        sock_attr->send_buf_len=len*0.8;
                        //remainӦ�ò���Ҫ������
                       // printf("���ֻص�800*1024��\n");
                    }
                }
				

            ///�ж��Ƿ񵽶�����ֵ
            if(send_info->drop_v_flag)
            {///���ڶ�����Ƶ״̬
                if(send_info->drop_p_flag)
                {///���ڶ���֡״̬

			//�ָ�map.v_frames������;����һ����v_framesС��10��һ���������(v_frames+10)<th_drop_v
                    if(send_info->map.v_frames<10)
                    {///���Իָ���Ƶ����
                        send_info->drop_v_flag=0;
                    }
                    else
                    {
                        send_info->send_i_flag=0;           ///<��Ҫ����I֡�ſ��Իָ�
                        send_info->drop_v_frames++;     ///<�ֶ�ʧ��һ֡��Ƶ
                        ret=0;
                        printf("line:[%d] drop frame:%d\n",__LINE__,send_info->drop_v_frames);
                        break;
                    }
                }

                //���drop_v_flag�ڱ�����״̬������Ҫ����һ�����ᣬ�����������»�ȡ����
                if((send_info->map.v_frames+10)<usr->th_drop_v)
                {///���Իָ���Ƶ����
                    //ȡ���˶���drop_v_flag,��ζ��I֡��P֡���л��ᷢ���ˣ�ֻ���л��ᣬ����һ����
                    send_info->drop_v_flag=0;
                }
                else
                {
                    //ʲô����Ҫ���ˣ����Ǽ������������
                    printf("line:[%d]usr:%d %d frame > th_drop_v(%d) drop frame:%d!\n",__LINE__,usr->no,send_info->map.v_frames,usr->th_drop_v,(int)frame->len);
                    send_info->drop_v_frames++;
                    ret=0;
                    break;
                }
            }
 
            //�µ���������ǰ֡��I֡���Ǿͷ����˰�
            if(flag==FRAMETYPE_I)
            {
                send_info->send_i_flag=1;
                send_info->last_v_seq=seq;
			//printf("=======�ҷ�����һ��I֡\n");
            }
            else
            {///p֡
                if(send_info->drop_p_flag)
                {
                    if((send_info->map.v_frames+10)<usr->th_drop_p)
                    {//�ָ�����p֡
                        send_info->drop_p_flag=0;
                    }
                    else
                    {
                        printf("line[%d]usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        ret=0;
                        break;
                    }                    
                }
                else
                {
        
                    if((send_info->map.v_frames)>=usr->th_drop_p)
                    {
                        printf("line:[%d]usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        send_info->drop_p_flag=1;
                        send_info->drop_v_frames++;
                        ret=0;   
                        break;
                    }

                }
                if(send_info->send_i_flag)
                {
                    

                }
                else
                {///��û�з���i֡
                     printf("line[%d],usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);      ///<�����ȷ���i֡����ܷ���p֡
                    send_info->send_i_flag=0;
                    send_info->drop_v_frames++;
                    ret=0;                  
                    break;                                   
                }                
            }


      }//end if(enc->media_type==MEDIA_TYPE_VIDEO)
      else
        {///��Ƶ����

            if(send_info->map.a_frames>40)
            {///��໺��40����Ƶ
                    printf("usr:%d too many audio pkt, drop audio:%d!!\n",usr->no,(int)frame->len);
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                       
            }
            if((send_info->map.v_frames+send_info->map.a_frames+2)>MAX_MAP_BUF_FRAMES)
            {///Ԥ��2֡�ռ�
                    printf("usr:%d not enough map drop audio:%d!!\n",usr->no,(int)frame->len);
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                     
            }
            if((sock_attr->send_buf_len-sock_attr->send_buf_remain-1024)<(frame->len+sizeof(struct stream_fmt_struct)*2))
            {

                ret=adjust_usr_sock_buf(usr,TCP_SEND_ADJUST_STEP);      ///<socket����������,���ڻ�����
                if((sock_attr->send_buf_len-sock_attr->send_buf_remain-1024)<(frame->len+sizeof(struct stream_fmt_struct)*2))
                { ///����û�з��䵽�㹻�Ļ�����
                    printf("usr:%d not enough buf!%d<%d drop audio frame !!\n",usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain-1024),(int)(frame->len+sizeof(struct stream_fmt_struct)*2));
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                    
                }
               
            }
            if(send_info->drop_a_flag)
            {
                if(send_info->map.a_frames<10)
                {///�ָ���Ƶ����
                    send_info->drop_a_flag=0;
                }
                else
                {
                    printf("usr:%d drop a frame:%d!\n",usr->no,(int)frame->len);
                    ret=0;
                    break;
                }
            }
           
        
        }
    	if(1)
    	{
    		//printf("send_frame_type_____[%d]_____length[%d]\n",frame->type,frame->len);
    		//gtloginfo("send_frame_type_____[%d]_____length[%d]\n",frame->type,frame->len);
    	}
		if(usr->stream_idx!=-1)//sdk usr
		{
			ret=write_frame_data2sdk(usr->fd,(char*)frame_head_buf[no],frame,&(enc->attrib->fmt));
			if(ret>0)
			{
				sock_attr->send_buffers+=ret;									///<�ѷ��뻺�������ֽ���
				add_ele2map(&send_info->map,frame->type,ret);
				sock_attr->send_buf_remain+=ret;							///<�����������ֽ���
				send_info->total_put+=ret;			   
			}
       	 	else
       		{
	            printf("DBG write_frame_data2net ret=%d:%d:%s\n",ret,errno,strerror(errno));
				printf(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
	            gtloginfo(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
				pthread_mutex_unlock(&usr->u_mutex);
				rtnet_av_close_connect(usr->no);
				return ret;
        	}


			
		}
		else//trans usr
        {
        	ret=write_frame_data2trans(usr->fd,(char*)frame_head_buf[no],frame,enc->attrib->fmt.v_fmt.format);                ///<��ý�����ݰ����͵�����
        	if(ret>0)
        	{
	            sock_attr->send_buffers+=ret;                                   ///<�ѷ��뻺�������ֽ���
	            add_ele2map(&send_info->map,frame->type,ret);
	            sock_attr->send_buf_remain+=ret;                            ///<�����������ֽ���
	            send_info->total_put+=ret;             
        	}
       	 	else
       		{
	            printf("DBG write_frame_data2net ret=%d:%d:%s\n",ret,errno,strerror(errno));
				printf(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
	            gtloginfo(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
        	}
		}
    }while(0);
    pthread_mutex_unlock(&usr->u_mutex);
	
    return ret;

}
 /** 
 *   @brief     ����ָ��ý������������ݸ���Ӧ���û�
 *   @param  enc ָ��ý���������ָ��
 *   @param  frame ָ��Ҫ���͵����ݽṹָ��
 *   @param  seq  ý�����ݵ����к�
 *   @param  flag   ý�����ݵı��
 *   @return   ��ֵ��ʾ�û���ý�巢�͵��û���,��ֵ��ʾ����
 */ 
static int send_media_frames
	(media_source_t *enc,struct stream_fmt_struct *frame,int seq,int flag)
{

    tcprtimg_svr_t    *p=get_rtimg_para();
    av_usr_t            *usr=NULL;
    int                     total=sizeof(p->av_server.av_usr_list)/sizeof(av_usr_t);        ///<���û���
    int                     nums=0;                 ///<���͵��û�����
    int                     i;
    int                     ret;
	int 					no;
	stream_send_info_t  *send_info;         ///<������Ϣ
	no=enc->no;

    for(i=0;i<total;i++)
    {
        usr=&p->av_server.av_usr_list[i];
		send_info=&usr->send_info;
        if(usr->serv_stat<=0)
        {
            continue;                                                         ///<��û���յ���������
        }
        else
        {

            if(enc->media_type==MEDIA_TYPE_VIDEO)       ///<��Ƶ��Ϣ��������ģ����뷢��
            {


            	if(enc->no==usr->venc_no)       
                {               
                    ///<��Ƶ������������û����ĵ�ƥ��


					
                    ret=send_media_frame2usr(usr,enc,frame,seq,flag);
					//printf("send_media_frame2usr ret =%d\n",ret);
                    if(ret>0)
                        nums++;
                }
            }
            else
            {
            	
            	if(enc->no==usr->venc_no)       
                {       
                    if(usr->serv_stat==3)                               ///<��������Ƶ����
                    {///<Ŀǰֻ��һ����Ƶ������ ���Բ��ý��������ж�
                        ret=send_media_frame2usr(usr,enc,frame,seq,flag);
                        if(ret>0)
                            nums++;
                    }
            	}
               
            }
        }
        
    }
    return nums;
}





 unsigned long GetTickCount()
 {
		 static unsigned long s_mode = 0;
		 static unsigned long s_tt = 0;
		 if (s_mode == 0)
		 {
				 unsigned long tps = (unsigned long)sysconf(_SC_CLK_TCK);
				 printf("tps = %lu\r\n", tps);
				 if (1000 % tps == 0)
				 {
						 s_tt = 1000 / tps;
						 s_mode = 1;
				 }
				 else
				 {
						 s_tt = tps;
						 s_mode = 2;
				 }
		 }
		 struct tms t;
		 const unsigned long dw = (unsigned long)times(&t);
		 return (s_mode == 1 ? (dw * s_tt) : (unsigned long)(dw * 1000LL / s_tt));
 }

 /** 
 *   @brief     ��Ƶ�����߳�
 *   @param  para ָ��������Ƶ��������ָ��
 *   @return   ��������
 */ 
static void *venc_server_thread(void *para)
{
	media_source_t *enc=(media_source_t*)para;  
	int                     ret,ret2;
	struct stream_fmt_struct      *frame_buf=NULL;                    ///<�����Ƶ֡�Ļ�����
	struct stream_fmt_struct      *frame_buf_playback=NULL;
	int                     buflen;                                      ///<frame_buf�ĳ���
	int 					buflen_playback;
	int                     no;                                            ///<���������enc->no
	int                     old_seq=-1,seq=-1;                 ///<ý���������
	int                     old_seq_playback=-1,seq_playback=-1;
	int                     flag;                                          ///<ý�����ݵı��
	int						flag_playback;
	int play_back_no;
	tcprtimg_svr_t    *p=get_rtimg_para();
	playback_t * pb=get_playback_parm();
	play_back_no=enc->no;
	if(enc==NULL)
	{
		printf     ("venc_server_thread para=NULL exit thread!!\n");
		gtloginfo("venc_server_thread para=NULL exit thread!!\n");
		pthread_exit(NULL);
	}
	no=enc->no;
	media_source_t * enc_playback = get_video_enc_playback(no);
	printf     (" start venc_server_thread (%d)...\n",no);
	gtloginfo(" start venc_server_thread (%d)...\n",no);
	frame_buf=(struct stream_fmt_struct *)enc->temp_buf;                                            ///<��ʱ������
	frame_buf_playback=(struct stream_fmt_struct *)enc_playback->temp_buf;
	buflen=enc->buflen;                                                       //��ʱ����������
	buflen_playback=enc_playback->buflen;

	ret=connect_video_enc_succ(no,(int)STREAM_SEC ,"rt_usr",0);            ///<������Ƶ�����������
	//gen_venc_avi_head(no,get_venc_attrib(no));          

	pthread_mutex_lock(&pb->mutex);

	if(get_frate_from_venc(no)>0);
	{
		pb->pb_vct[no]=p->playback_pre*get_frate_from_venc(no); //���ֻ�����ȴ�ʱ��
	}
		
	pthread_mutex_unlock(&pb->mutex);
	while(1)
	{ 
			if(pb->pb_vct[no] == 0)
			{
						
					if(pb->pb_venc[no]==0) //����һ���Ե�flag
					{
						connect_video_enc_playback(no,(int)STREAM_SEC,"pb_usr",p->playback_pre);
						pthread_mutex_lock(&pb->mutex);
						pb->pb_venc[no]=-1;
						pthread_mutex_unlock(&pb->mutex);
					}
					ret=read_video_playback(no,(void*)frame_buf_playback,buflen_playback,&seq_playback,&flag_playback);
					old_seq_playback++;
					if(old_seq_playback!=seq_playback)
					{
						printf("[%s]read_playback_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq_playback,seq_playback);
						old_seq_playback=seq_playback;
					}

			}
				ret=read_video_frame(no,(void*)frame_buf,buflen,&seq,&flag);
				//printf("video_frame flag=%#x\n",flag);
				venc_last_tv[no].tv_sec=frame_buf->tv.tv_sec;
				venc_last_tv[no].tv_usec=frame_buf->tv.tv_usec;
				old_seq++;
				if(old_seq!=seq)
				{
					printf("[%s]read_video_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq,seq);
					old_seq=seq;
					continue;
				}

				if(pb->pb_vct[no]>0)
				{
						pthread_mutex_lock(&pb->mutex);
						pb->pb_vct[no]--;
						//printf("pb_vct[%d]=%d\n",no,pb->pb_vct[no]);
						pthread_mutex_unlock(&pb->mutex);
				}
				
				
				
				//ret=get_video_enc_remain(no);
				//printf("ʵʱͨ����дָ���[%d]\n",ret);


#ifdef DEBUG
		if(tmp_cnt++>750&&no==0&&playback==1)
	
		{
			set_playback_en(play_back_no);
			playback=0;

		}
		else
		{
			if(tmp_cnt>1500&&get_playback_stat(play_back_no)==1)
				set_playback_cancel(play_back_no);
		}

#endif

		
		if(ret>=0)
		{

			if(get_playback_stat(play_back_no)<0)
			{
				//��ȡʵʱ��Ƶ
				send_media_frames(enc,frame_buf,seq,flag);    



			}	
			else if(get_playback_stat(play_back_no)==no)//�ѳɹ����ӻطų�
			{
			
				if(get_playback_frame_adjust(no)==0)
				{	
					ret2=find_playback_iframe(no);
					if(ret2<0)
						gtlogerr("find_playback_iframe err\n");
					set_playback_frame_adjust(no,ret2);
				}
				//��ȡ�ط���Ƶ
					
				send_media_frames(enc_playback,frame_buf_playback,seq_playback,flag_playback);
				
			}
			//printf("������[%d]���͸��û�����[%d]\n",enc->no,ret);
		}
		else
		{
			///�����ݳ���
			printf("read_video_frame%d ret=%d!!\n",no,ret);
			if(ret!=-EINTR)
				sleep(1);
		}
	}  
	return NULL;
}

int posix_memalign(void **memptr, size_t alignment, size_t size);

/** 
 *   @brief     дһ����Ƶ���ݵ�ָ����ŵ���Ƶ�����������
 *   @param  aenc ��Ƶ����������ؽṹָ��
 *   @param  frame ָ����Ƶ֡��ָ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
static inline int write_adec_frame_pool(media_source_t *adec, struct stream_fmt_struct *frame)
{
	int s_len=frame->len+sizeof(struct stream_fmt_struct)-sizeof(frame->len);
	adec->attrib->stat=ENC_STAT_OK;
	return write_media_resource(adec,frame,s_len,frame->type);
}
static inline int read_audio_frame_pool(IN media_source_t *aenc,OUT void *buf,IN int buf_len,OUT int *eleseq,OUT int *flag)

{
	return read_media_resource(aenc,buf,buf_len,eleseq,flag);
}
//#ifdef SAVE_RAW_AUDIO
///��ԭʼ��Ƶ���ݴ�������ļ�,���ڲ���

//#endif



#if 0
/** 
 *   @brief     ����map�е���Ч�ֽ���
 *   @param  map ָ����ý����Ϣӳ��ṹ��ָ��
 *   @return   map�е���Ч�ֽ���
 *   ������
 */
static inline int calc_map(IO stream_send_map_t *map)
{///������,
    map_frame_t *frame;
    int     total=0;
    int     i;
    int     head=map->head;
    int     tail=map->tail;
    while(head!=tail)
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        if(head!=tail)
        {
            frame=&map->frame_map[head];
            total+=frame->size;
            if(++head>=MAX_MAP_BUF_FRAMES)
                head=0;
        }
        else
        {
            break;
        }
    }
    return total;
}


static inline int print_map(IN stream_send_map_t *map)
{///������,��ӡmap�е�Ԫ��
    map_frame_t *frame;
//    int     i;
    int     total=0;
    int     head=map->head;
    int     tail=map->tail;
    printf("map:");
    while(head!=tail)
    {
        frame=&map->frame_map[head];
        printf("%d,",frame->size);
        if(++head>=MAX_MAP_BUF_FRAMES)
            head=0;
        total++;
    }
    printf("(%d)\n",total);
    return total;
}
#endif



/** 
 *   @brief     ����Ƶ���з�����봦�����
 */
void avserver_second_proc(void)
{
    int                         i;
    int                         total_video=get_videoenc_num();
	int							total_audio=get_audio_num();
    tcprtimg_svr_t        *p=get_rtimg_para();
    av_server_t           *av_svr=&p->av_server;
    int                         av_usr_num=av_svr->wan_usrs+av_svr->lan_usrs;
    static int                 old_av_usrs=0;               ///<��һ�ε��û���
    static int                 av_freecnt=0;                ///<����Ƶ���з���Ŀ���ʱ��

    if(old_av_usrs!=av_usr_num)
    {
        if(old_av_usrs==0)
        {
            set_net_enc_busy(1);
        }
        old_av_usrs=av_usr_num;
        av_freecnt=0;
    }
    else
    {
        if(old_av_usrs==0)
        {
            if(++av_freecnt==30)//30��󱨸����״̬10)
            {
                set_net_enc_busy(0);
            }
        }
        else
            av_freecnt=0;
    }
        


    for(i=0;i<total_video;i++)
    {
		if(get_venc_stat(i)>=0)
		{
				
			if(reactive_video_enc(i)==1||reactive_video_enc_playback(i)==1)
			{
				gtloginfo("%d!\n",i);
				printf("%s %d!\n",__FUNCTION__,i);
				exit(0);

			}
		}

		

    }
	for(i=0;i<total_audio;i++)
	{
		if(get_aenc_stat(i)>=0)
		{

			if(reactive_audio_enc(i)==1||reactive_audio_enc_playback(i)==1)
			{
				gtloginfo("%d!\n",i);
				printf("%d!\n",i);
				exit(0);

			}
		}

	}
        
}

/** 
 *   @brief     ��Ƶ�����߳�
 *   @param  para ָ��������Ƶ��������ָ��
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 

static void *aenc_server_thread(void *para)
{
    tcprtimg_svr_t    *p=get_rtimg_para();
    struct stream_fmt_struct  *frame_buf=NULL;
	struct stream_fmt_struct  *frame_buf_playback=NULL;
    media_source_t *enc=(media_source_t*)para;      ///<������ָ��
    playback_t * pb=get_playback_parm();
    int                     ret;
    int                     buflen;                                       ///<frame_buf�ĳ���
    int 					buflen_playback;
    int                      no=enc->no;                                           ///<���������enc->no
    int                     seq=-1;                                     ///<ý���������
    int eleseq=-1,old_seq=-1,flag;
	int old_seq_playback=-1,seq_playback=-1,flag_playback;
	avi_t * AVI=NULL;
    if(enc==NULL)
    {
        printf     ("aenc_server_thread para=NULL exit thread!!\n");
        gtlogerr ("aenc_server_thread para=NULL exit thread!!\n");
        pthread_exit(NULL);
    }


   
    printf     (" start aenc_server_thread (%d)...\n",no);
    gtloginfo(" start aenc_server_thread (%d)...\n",no);

	media_source_t * enc_playback = (media_source_t*)get_audio_enc_playback(no);
	frame_buf=(struct stream_fmt_struct  *)enc->temp_buf;
	frame_buf_playback=(struct stream_fmt_struct *)enc_playback->temp_buf;
	buflen=enc->buflen; 			
	buflen_playback=enc_playback->buflen;
	ret=connect_audio_enc_succ(no,(int)STREAM_SEC,"rt_usr");			
	//pb->pb_act[no]=p->playback_pre*30; //�ݶ�30*30֡������

	if(get_frate_from_aenc(no)>0);
	{
		pb->pb_act[no]=p->playback_pre*get_frate_from_aenc(no); //���ֻ�����ȴ�ʱ��
	}

    while(1)
    {
    		if(pb->pb_act[no] == 0)
			{
					if(pb->pb_aenc[no]==0)
						
					{	
						connect_audio_enc_playback(no,(int)STREAM_SEC,"pb_usr",p->playback_pre);
						pthread_mutex_lock(&pb->mutex);
						pb->pb_aenc[no]=-1;
						pthread_mutex_unlock(&pb->mutex);
					}
					ret=read_audio_playback(no,(void*)frame_buf_playback,buflen_playback,&seq_playback,&flag_playback);
					old_seq_playback++;
					if(old_seq_playback!=seq_playback)
					{
						printf("[%s]read_playback_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq_playback,seq_playback);
						old_seq_playback=seq_playback;
						//continue;
					}
			}
			


		    ret = read_audio_frame(no,(void *)frame_buf,buflen,&eleseq,&flag);
		//	printf("audio_frame flag=%#x\n",flag);
		//	printf("__________buflen[%d]_____frame_len[%d]\n",buflen,frame_buf->len);
			
			frame_buf->tv.tv_sec=venc_last_tv[no].tv_sec;
			frame_buf->tv.tv_usec=venc_last_tv[no].tv_usec;
			old_seq++;
			if(old_seq!=eleseq)
			{
				printf("[%s]read_audio_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq,eleseq);
				old_seq=eleseq;
				continue;
			}

			if(pb->pb_act[no]>0)
			{
					pthread_mutex_lock(&pb->mutex);
					pb->pb_act[no]--;
					//printf("pb_act[%d]=%d\n",no,pb->pb_act[no]);
					pthread_mutex_unlock(&pb->mutex);
			}
		
			if(ret>=0)
			{
	
				if(get_playback_stat(no)<0)
				{
					//��ȡʵʱ��Ƶ
					ret=send_media_frames(enc,frame_buf,seq,flag);	  
				}	
				else
				{
					//��ȡ�ط���Ƶ
					ret=send_media_frames(enc_playback,frame_buf_playback,seq_playback,flag_playback);
				}

			}
			else
			{
				///�����ݳ���
				printf("read_audio_frame%d ret=%d!!\n",no,ret);
				if(ret!=-EINTR)
					sleep(1);
			}

		/*    if(ret<=0)
		    {
		        printf("read_audio_data ret=%d!\n",ret);
				gtlogerr("read_audio_data ret=%d!\n",ret);
				gtlogerr("get_audio_enc_remain=%d,no=%d\n",get_audio_enc_remain(no),no);
		        sleep(1);
		        continue;
		    }
		    */
	//printf("read adio data ret=%d\tlen=%d\t type=%d\t chkid=%x\n",ret,frame->len,frame->type,frame->chunk.chk_id);
//#define SAVE_RAW_AUDIO
#ifdef SAVE_RAW_AUDIO
			if(enc->no==0)
				write_rawaudio2file_pre(&frame_buf->data,frame_buf->len,&AVI);      ///<��������ԭʼ���ݴ������
#endif
		


   //     seq++;
  	//  	ret=send_media_frames(enc,frame,eleseq,FRAMETYPE_PCM);                     ///<���͸��û�


    
       //zsk del ret=write_audio_frame_pool(enc,frame);                                               ///<д�뻺���
    }
	AVI_close_output_file(AVI);
    return NULL;    
}



 /** 
 *   @brief     ��������Ƶ�����߳�
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int create_av_server(void)
{
  tcprtimg_svr_t      *p=get_rtimg_para();
 // av_server_t          *svr=&p->av_server;
  media_source_t     *venc=NULL;                  ///<��Ƶ������
  media_source_t     *aenc=NULL;
  int                         ret=0;
  int                         total;
  int                         i;
  init_avserver();                    ///<��ʼ������Ƶ���з����õ��ı���

  init_playback_parm();
  //��Ƶ���벿�ַ���videoenc��������rtimageֻȡ
  ///������Ƶ�����߳�
  total = get_audio_num();
  for(i=0;i<total;i++)//Ŀǰֻ��һ��������������3
  {
	  	aenc=get_audio_enc(i);
	  	ret=gt_create_thread(&aenc->thread_id,aenc_server_thread,(void*)aenc);
  }

  ///������Ƶ�����߳�
	total=get_video_num();
	for(i=0;i<total;i++)
	{

	//������Ƶ��������

		venc=get_video_enc(i);
		ret=gt_create_thread(&venc->thread_id, venc_server_thread, (void*)venc);

	}

    return 0;
}




