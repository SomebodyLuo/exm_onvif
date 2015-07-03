#pragma once
#ifndef _RTSP_H_
#define _RTSP_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_RTSP_VALUE_SIZE            (128)        ///�������ͨ�������󳤶�

enum RtspContentsType
{
  PLAY,                 ///<���š���š����ŵ�
  PLAY_RESPONSE,        ///<���š���š����ŵȵ���Ӧ
  PAUSE,                ///<��ͣ
  PAUSE_RESPONSE,       ///<��ͣ��Ӧ
  TEARDOWN,             ///<ֹͣ
  TEARDOWN_RESPONSE,    ///<ֹͣ��Ӧ
  UNKNOWN_CONTENTS_TYPE ///<δ����
};

//��ǰ����֧�����ֻ�Ӧ:�ɹ����߲�����
enum RtspResponseCode
{
  RESPONSE_CODE_SUCCESS,                          ///<��Ӧ�Է�ִ�гɹ�
  RESPONSE_CODE_NOT_ACCEPTABLE,                   ///<��Ӧ�Է����������

  RESPONSE_CODE_UNKNOWN
};

enum RangeTimeType
{
  SMPTE,              ///<���ʱ��
  NPT,                ///<����ʱ��
  CLOCK,              ///<����ʱ��
  UNKNOWN_TIME_TYPE   ///<δ����
};

///�����RtspContentsType������������Ӧ��ֵ�����磬����ΪPLAYʱ��cseq��time_type��begin��end��scale������Ч��
///�����Ĳ��ù���
typedef struct RTSP
{
  enum RtspContentsType rtsp_type;
  enum RtspResponseCode response_code;
  char cseq[MAX_RTSP_VALUE_SIZE];                  ///���յ��û�����ʱ����Ҫ���˲�����������������Ӧʱ���õ���Ӧ��Ϣ����
  enum RangeTimeType time_type;                    ///�ο�����Ķ���,����PLAY��Ϣͷ��
  char begin[MAX_RTSP_VALUE_SIZE];                 ///��ʼʱ��
  char end[MAX_RTSP_VALUE_SIZE];                   ///����ʱ�䣬��begin��Ӧ
  int scale;                                       ///Ĭ��Ϊ1

  char seq[MAX_RTSP_VALUE_SIZE];                   ///����PLAY_RESPONSE��ͷ�ֶε�RTP_INFO��
  char rtp_time[MAX_RTSP_VALUE_SIZE];
  char url[MAX_RTSP_VALUE_SIZE];
} rtsp_t;


///Ӧ��Է��Ĳ��š���š����š��쵹����������֡��/�����������
///�˴���Ҫ����ص�����ʱ��CSeqֵ��urlָ����seq��rtp_time����Ӧ���Ǹ�����URL��seqָ�������ĵ�һ���������
///rtp_time ������յ���play�����еĿ�ʼʱ�������ʱ�������ɵ�RTPʱ�����Play����ķ����߸��ݸ�ʱ�������
///RTPʱ����NPTʱ���ӳ�䡣
///scaleΪ�Լ�ʵ�ʵĲ����ٶȡ�Scale Ϊ1���������ţ�������1��Ϊ�����������ʵı���������Ϊ���š�
rtsp_t * gtaua_make_answer_play(char * cseq, enum RtspResponseCode code, char * seq, char * rtp_time, char * url, int scale);

///Ӧ����ͣ����
rtsp_t * gtaua_make_answer_pause(char * cseq, enum RtspResponseCode code);

///Ӧ��ֹͣ����
rtsp_t * gtaua_make_answer_stop(char * cseq, enum RtspResponseCode code);

///�ͷ�ͨ�������make����������rtsp
void gtaua_free_rtsp(rtsp_t * contents);

//enum RtspContentsType gtaua_get_rtsp_contents_type(rtsp_t * contents);
//char * gtaua_get_cseq(rtsp_t * contents);
//enum RangeTimeType gtaua_get_time_type(rtsp_t * contents);               ///�ο�����Ķ���,����PLAY��Ϣͷ��
//char * gtaua_get_begin_time(rtsp_t * contents);                          ///��ʼʱ��
//char * gtaua_get_end_time(rtsp_t * contents);                            ///����ʱ�䣬��begin��Ӧ
//int gtaua_get_scale(rtsp_t * contents);                                  ///Ĭ��Ϊ1
//char * gtaua_get_seq(rtsp_t * contents);                                 ///����PLAY_RESPONSE��ͷ�ֶε�RTP_INFO��
//char * gtaua_get_rtp_time(rtsp_t * contents);
//char * gtaua_get_url(rtsp_t * contents);

#ifdef  __cplusplus
}
#endif

#endif

