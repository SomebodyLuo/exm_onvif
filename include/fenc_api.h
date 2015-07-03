#ifndef FENC_API_H
#define FENC_API_H
#include "typedefine.h"
#include <sys/time.h>
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef IO
#define IO
#endif

struct video_enc;

#ifndef struct_HDR
#define struct_HDR
struct NCHUNK_HDR {	//avi��ʽ�����ݿ�ͷ��־�ṹ
#define IDX1_VID  		0x63643030	//AVI����Ƶ�����
#define IDX1_AID  		0x62773130	//AVI����Ƶ���ı��
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
#endif


typedef struct{
    ///ѹ�������Ƶ֡
    ///ʹ������ṹʱҪ�ȷ���һ���󻺳���,Ȼ�󽫱��ṹ��ָ��ָ�򻺳���
    
#define MEDIA_VIDEO		0x01		//��Ƶ����
#define MEDIA_AUDIO	0x02		//��Ƶ����

#define FRAMETYPE_I		0x0		// frame flag - I Frame
#define FRAMETYPE_P		0x1		// frame flag - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// frame flag - Audio Frame
#define FRAMETYPE_AAC	0x7		// frame flag - Audio Frame



	struct timeval           tv;			   ///<���ݲ���ʱ��ʱ���
	unsigned long	           channel;	          ///<ѹ��ͨ��
	unsigned short           media;		   ///<media type ��Ƶ����Ƶ
	unsigned short           type;		          ///<frame type	I/P/����...
	long                          len;	                 ///<frame_buf�е���Ч�ֽ���
	struct NCHUNK_HDR chunk;                ///<���ݿ�ͷ��־��Ŀǰʹ��avi��ʽ
	unsigned char            frame_buf[4];    ///<��ű�������Ƶ���ݵ���ʼ��ַ
}enc_frame_t;

typedef struct motion_cfg
{
    unsigned int x_left_up;
    unsigned int y_left_up;
    unsigned int x_right_down;
    unsigned int y_right_down;	//���ĸ������޶��ƶ����ķ�Χ
    unsigned int threshold; /* threshold of motion vector */
    unsigned int sad_threshold;//sensitivity
    unsigned int mv_number;		//��Ҫ����16*16 block��
} motion_cfg;

typedef struct 
{

    ///�������Ͷ���
    #define     FENC_MJPEG          0
    #define     FENC_MPEG4          1
    #define     FENC_H264             2
    #define     FEBC_UNKNOW      -1
    unsigned int enc_type;           ///<��������

    unsigned int width;                      //length per dma buffer
    unsigned int height;
    unsigned int frame_rate;            ///<֡��     
    unsigned int frame_rate_base;  ///<Ӧ�ó�����Ҫ����,�ɿ⺯�����
    unsigned int gop_size;                ///<I֡���

    ///����ģʽ����
    #define FENC_CBR        1       ///<������
    #define FENC_VBR        2       ///<���������޵ı�����
    unsigned int enc_mode;              ///<����ģʽ1:CBR 2:VBR(����������)

    unsigned int bit_rate;                   ///<���� bps  CBRʱ��ʾ������С

    unsigned int quant;                     ///<��������VBRʱ��Ч
    unsigned int min_bitrate;           ///<VBRʱ��Ч
    unsigned int max_bitrate;          ///<VBRʱ��Ч
    
   //unsigned int qmax;      
   //unsigned int qmin;       
    //unsigned int intra;
    unsigned int roi_enable;            ///<��������ʹ�� ,Ϊ1ʱ roi_x,roi_y,roi_width,roi_height��Ч
    unsigned int roi_x;                     ///<�����������ʼλ��
    unsigned int roi_y;	
    unsigned int roi_width;              ///<����������
    unsigned int roi_height;	           ///<��������߶�


    ///����Ϊmotion detect ����
    unsigned int motion_sen;				//�ƶ����������
   	struct motion_cfg	md_config;			
   
   	//����blind detect ����
   	unsigned int blind_sen;					//�ڵ�����������
   	unsigned int blind_alarm_time;
   	unsigned int blind_cancelalarm_time;	//ȡ������ʱ��
    //TODO

  //  unsigned int fmpeg4_qtbl;          ///<ʹ�������������
    //enc_frame_t *coded_frame;
    //char *priv;
} fenc_param_t;


typedef struct fenc{
   
    fenc_param_t      param;            ///<��Ƶ�������
    
    void                   *encoder;        ///<video_enc_t  �ṹ,������Ƶ�������Ľṹ
    
    int                       priv_size;
    void                    *priv_data;

    int (*mdtect_callback)(struct fenc *handle,int motion_stat,int blind_stat);///TODO ���Ը�����Ҫ����motion_stat�ṹ
	
}fenc_t;



/**********************************************************************************
 *      ������: fenc_open()
 *      ����:   ��ָ���ı����ʽ�򿪱�����
 *      ����:   enc_type:�����ʽ
 *      ���:   ��
 *      ����ֵ: ָ��������ṹ��ָ��,NULL��ʾʧ�ܣ��������errno
 *********************************************************************************/
fenc_t *fenc_open(IN unsigned int enc_type);

/**********************************************************************************
 *      ������: fenc_set_param()
 *      ����:   �趨�������Ĺ�������
 *      ����:   enc: �Ѿ���fenc_open ���صľ��
 *                     param:�����õĲ���
 *      ���:   enc:�����²���ֵ�Ľṹָ��
 *      ����ֵ: 0��ʾ�ɹ�,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int fenc_set_param(IO fenc_t *enc,IN fenc_param_t *param);

/**********************************************************************************
 *      ������: fenc_encode_frame()
 *      ����      :   �趨�������Ĺ�������
 *      ����      :enc: �Ѿ���fenc_open ���صľ��
 *                        buf_size:����������Ĵ�С
 *                        data:���ԭʼ��Ƶ���ݵĻ�����(��ѹ��������Сƥ��)
 *      ���      :frame: ���õ�ѹ�������Ƶ֡(tv�ֶ�û�����)
 *      ����ֵ: ��ֵ��ʾ��Ч��ѹ�������ֽ�����,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int fenc_encode_frame(IN fenc_t *enc,OUT enc_frame_t *frame, IN int buf_size, IN void * data);


/**********************************************************************************
 *      ������: fenc_close()
 *      ����:   �رձ�����
 *      ����:   enc: �Ѿ���fenc_open ���صľ��
 *      ���:   enc:�ͷ����ڴ棬��Ч��ָ��
 *      ����ֵ: 0��ʾ�ɹ�,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int fenc_close(IO fenc_t *enc);

/**********************************************************************************
 *      ������: fmjpeg_encoder_sj()
 *      ����:   ��һ֡ԭʼ��Ƶ���ݱ����jpeg��ʽ��ͼƬ�ļ�
 *      ����:   width:ͼƬ���
 *                     height:ͼƬ�߶�
 *                     data:ԭʼ��Ƶ������
 *                     outfile:����ļ���
 *      ���:   outfile�ļ�:���ѹ���õ�jpg�ļ�
 *      ����ֵ: ��ֵ��ʾ�ļ���С,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int fmjpeg_encoder_sj(IN int width,IN int height,IN void *data,IN char *outfile);


int fenc_set_motion_detect_callback(fenc_t *enc,int (*mdtect_callback)(fenc_t *handle,int motion_stat,int blind_stat));


/**********************************************************************************
 *      ������: fenc_read_para_file()
 *      ����:   ��ָ���������ļ��ж�ȡ�������
 *      ����:   filename: �����ļ���
 *                     section:�������������Ϣ��ini����
 *      ���:   enc_param:�����²���ֵ�Ľṹָ��
 *      ����ֵ: 0��ʾ�ɹ�,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int fenc_read_para_file(IN char *filename,IN char *section, OUT fenc_param_t  *enc_param);

/**********************************************************************************
 *      ������: motion_blind_read_para_file()
 *      ����:   ��ָ���������ļ��ж�ȡ�ƶ�������Ƶ�ڵ�����
 *      ����:   enc_no: ������ͨ����
 *				 filename: �����ļ���
 *      ���:   enc_param:�����²���ֵ�Ľṹָ��
 *      ����ֵ: 0��ʾ�ɹ�,��ֵ��ʾʧ�ܣ��������errno
 *********************************************************************************/
int motion_blind_read_para_file(IN int enc_no, IN char *filename, OUT fenc_param_t  *enc_param);



int fenc_reset_timeout(int no,int num);

#endif

