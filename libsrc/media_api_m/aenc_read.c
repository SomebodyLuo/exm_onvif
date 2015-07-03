/*by wsy,for audio_pool connect and read etc @Dec 2006*/

#include <stdio.h>
#include <devinfo.h>
#include <devres.h>
#include <errno.h>
#include <mshmpool.h>
#include <media_api.h>
#include <aenc_read.h>
#include <syslog.h>
//����־��¼����
//name��ʾ��־��Ϣ�е�����
#define gtopenlog(name) openlog(name,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );//LOG_USER);

//#define gtlog  syslog		//ϵͳ��־��Ϣ��¼
#define gtlog syslog
//һ������Ϣ
#define gtloginfo(args...) syslog(LOG_INFO,##args)	//��¼һ����Ϣ
//���صĴ�����Ϣ
#define gtlogfault(args...) syslog(LOG_CRIT,##args)	//
//������Ϣ
#define gtlogerr(args...) syslog(LOG_ERR,##args)	//
//������Ϣ
#define gtlogwarn(args...) syslog(LOG_WARNING,##args)

#define gtlogdebug(args...) syslog(LOG_DEBUG,##args)

static media_source_t audio_enc[MAX_AUDIO_ENCODER];			//��Ƶ������ʵ��
static media_source_t audio_enc_playback[MAX_AUDIO_ENCODER];
static media_source_t audio_dec[MAX_AUDIO_DECODER];	
static media_source_t audio_dec_usr[MAX_AUDIO_DECODER];
static media_source_t audio_enc_rec[MAX_AUDIO_ENCODER];
const int audio_fmt=50;
/**********************************************************************************************
	 * ������	:init_audio_enc()
 * ����	:��ʼ����Ƶ����������ؽṹ
 * ����	:��
 * ���	:�ڿ����Ѿ�������һ����̬��������Ƶ�������ṹ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:�ڳ�������ʱ����һ��
 **********************************************************************************************/
int init_audio_enc(void)
{
	
	int i,ret,total;
	total = get_audio_num();
	if(total>MAX_AUDIO_ENCODER)
	{
		total=MAX_AUDIO_ENCODER;
		syslog(LOG_ERR,"get_audio_num=%d MAX_AUDIO_ENCODER=%d!!!\n",get_audio_num(),MAX_AUDIO_ENCODER);
		printf("get_audio_num=%d MAX_AUDIO_ENCODER=%d!!!\n",get_audio_num(),MAX_AUDIO_ENCODER);
	}
	
	for(i=0;i<total;i++)
	{
		ret=init_media_rw(&audio_enc[i],MEDIA_TYPE_AUDIO,i,1024);
		ret=init_media_rw(&audio_enc_playback[i],MEDIA_TYPE_AUDIO,i,1024);

	}
	
	return 0;	
}

/**********************************************************************************************
 * ������	:init_audio_dec()
 * ����	:��ʼ����Ƶ����������ؽṹ
 * ����	:��
 * ���	:�ڿ����Ѿ�������һ����̬��������Ƶ�������ṹ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:�ڳ�������ʱ����һ��
 **********************************************************************************************/
int init_audio_dec_usr(void)
{
	

	init_media_rw(&audio_dec_usr[0],MEDIA_TYPE_AUDIO,0,1024);

	return 0;	
}
/** 
 *   @brief     ��ʼ����Ƶ���뻺�����ر���
 *   @return   0��ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int init_audio_dec_pool(void)
{
    int i;
    int ret;
    media_source_t *adec=&audio_dec[0];
    init_media_rw(adec,MEDIA_TYPE_AUDIO,i,200);
    ret=create_media_write(adec,get_audio_dec_key(0),"rtimage",128*1024);
    //if(ret>=0)
    //{
   //     set_audio_dec_attrib(&adec->attrib->fmt.a_fmt);
   // }
    
    return ret;

}

/**********************************************************************************************
 * ������	:get_audio_enc()
 * ����	:��ȡ��Ƶ������������Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,������NULL
 **********************************************************************************************/
media_source_t *get_audio_enc(IN int no)
{
	if(no>=MAX_AUDIO_ENCODER)
		return NULL;	
	return &audio_enc[no];
}
media_source_t * get_audio_enc_playback(IN int no)
{
	if(no>=MAX_AUDIO_ENCODER)
		return NULL;

	return &audio_enc_playback[no];

}
/**********************************************************************************************
 * ������	:get_audio_dec()
 * ����	:��ȡ��Ƶ������������Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,������NULL
 **********************************************************************************************/
media_source_t *get_audio_dec(IN int no)
{
	if(no>=MAX_AUDIO_DECODER)
		return NULL;

	return &audio_dec[no];
}
media_source_t *get_audio_dec_usr(IN int no)
{
	if(no>=MAX_AUDIO_DECODER)
		return NULL;

	return &audio_dec_usr[no];
}

/**********************************************************************************************
 * ������	:get_audio_read_buf()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ������ָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,������NULL
 **********************************************************************************************/
void *get_audio_read_buf(IN int no)
{
	if(no>=1)//fixme ���ж�·��ƵоƬʱ
		return NULL;	
	return audio_enc[no].temp_buf;
}
/**********************************************************************************************
 * ������	:get_audio_read_buf_len()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ����������
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:��ʱ�������ĳ���
 **********************************************************************************************/
int get_audio_read_buf_len(IN int no)
{
	if(no>=1)//fixme ���ж�·��ƵоƬʱ
		return -EINVAL;	
	return audio_enc[no].buflen;
}

/**********************************************************************************************
 * ������	:connect_audio_enc()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_enc(IN int no,int type,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	char debug_name[20];
	media_attrib_t *attrib=NULL;
	media_format_t *media=NULL;
	audio_format_t *audio=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	sprintf(debug_name,"%s_%#x",name,get_audio_enc_key(no));
	ret=connect_media_read(&audio_enc[no], get_onvif_pool_key(no,type)+0x20000,debug_name,MSHMPOOL_NET_USR);
	
//	ret=connect_media_read(&audio_enc[no], get_audio_enc_key(no),debug_name,MSHMPOOL_NET_USR);
	if(ret>=0)
	{
		stat=get_aenc_stat(no);
		if(stat==ENC_STAT_OK)
		{//������״̬����
			attrib=get_aenc_attrib(no);
			if(attrib!=NULL)
			{
				media=&attrib->fmt;
				audio=&media->a_fmt;
				frate=(audio->a_frate);
				rc=move_media_place(&audio_enc[no],-(frate*pre_sec));
			}
		}
	}
	gtloginfo("�û�[%s]���ӵ���[%d]����� key[%#x]\n",debug_name,no,get_audio_enc_key(no));
	return ret;
}
/**********************************************************************************************
 * ������	:connect_audio_enc_playback()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_enc_playback(IN int no,int type,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	char debug_name[20];
	media_attrib_t *attrib=NULL;
	media_format_t *media=NULL;
	audio_format_t *audio=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	sprintf(debug_name,"%s_%#x",name,get_audio_enc_key(no));
//	ret=connect_media_read(&audio_enc_playback[no], get_audio_enc_key(no),debug_name,MSHMPOOL_NET_USR);
	ret=connect_media_read(&audio_enc_playback[no], get_onvif_pool_key(no,type)+0x20000,debug_name,MSHMPOOL_NET_USR);
	
	if(ret>=0)
	{
		stat=get_aenc_stat(no);
		if(stat==ENC_STAT_OK)
		{//������״̬����
			attrib=get_aenc_attrib(no);
			if(attrib!=NULL)
			{
				media=&attrib->fmt;
				audio=&media->a_fmt;
				rc=move_media_place(&audio_enc_playback[no],-(audio_fmt*pre_sec));
			}
		}
	}
	gtloginfo("�û�[%s]���ӵ���[%d]����� key[%#x]\n",debug_name,no,get_audio_enc_key(no));
	return ret;
}

/**********************************************************************************************
 * ������	:connect_audio_dec()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_dec(IN int no,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	media_attrib_t *attrib=NULL;
	media_format_t *media=NULL;
	audio_format_t *audio=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	ret=connect_media_read(&audio_dec_usr[no], get_audio_dec_key(no),name,MSHMPOOL_NET_USR);

	if(ret>=0)
	{
		stat=get_adec_stat(no);
		if(stat==ENC_STAT_OK)
		{//������״̬����
			attrib=get_adec_attrib(no);
			if(attrib!=NULL)
			{
				media=&attrib->fmt;
				audio=&media->a_fmt;
				frate=(audio->a_frate);
				rc=move_media_place(&audio_dec_usr[no],-(frate*pre_sec));
			}
		}
	}
	return ret;
}

/** 
 *   @brief     ������Ƶ����ֱ���ɹ�
 *   @param  no ��Ƶ���������
 *   @param  name Ӧ�ó�����
 *   @return   �Ǹ���ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int connect_audio_enc_succ(int no,int type,char *name)
{
    int ret;
    int fail_cnt=0;         //ʧ�ܴ���
    if(no>get_audio_num())
        return -EINVAL;

    ///������Ƶ������
    while(1)
    {
    	ret=connect_audio_enc(no,type,name,0);
    	if(ret==0)
    	{
    		printf     ("connect auidoenc%d success\n",no);
              gtloginfo("connect auidoenc%d success\n",no);
    		break;
    	}
    	else
    	{
    	    if(fail_cnt==0)
    	    {
    	        printf     ("������Ƶ������%dʧ��\n",no);
    	        gtloginfo("������Ƶ������%dʧ��\n",no);
    	        
    	    }
           fail_cnt++;
           printf("connect auido enc(%d) failed(%d), ret=%d!!\n",no,fail_cnt,ret);
           if(fail_cnt==40)
           {
                printf    ("������Ƶ������%dʧ��%d��!!!",no,fail_cnt);
                gtlogerr("������Ƶ������%dʧ��%d��!!!",no,fail_cnt);
           }
    		sleep(2);
    	}
    }	

    ///�ȴ���Ƶ����������
    fail_cnt=0;
    while(1)
    {
    	ret=get_aenc_stat(no);
    	if(ret==ENC_STAT_OK)
    	{
    		printf("��Ƶ������%d״̬����!\n",no);
    		break;
    	}
    	else
    	{
    		if(++fail_cnt==15)
    		{
    			printf    ("��Ƶ������%d״̬�쳣,stat=%d!\n",no,ret);
    			gtlogerr("��Ƶ������%d״̬�쳣,stat=%d!\n",no,ret);
    		}
    		printf("auidoenc%d state=%d!!!\n",no,ret);	
    	}
    	sleep(1);
    }    
    return ret;

}
/** 
 *   @brief     ������Ƶ������ֱ���ɹ�
 *   @param  no ��Ƶ���������
 *   @param  name Ӧ�ó�����
 *   @return   �Ǹ���ʾ�ɹ�,��ֵ��ʾʧ��
 */ 
int connect_audio_dec_succ(int no,char *name)
{
    int ret;
    int fail_cnt=0;         //ʧ�ܴ���
    init_audio_dec_usr();
    if(no>get_audio_num())
        return -EINVAL;

    ///������Ƶ������
    while(1)
    {
    	ret=connect_audio_dec(no,name,0);
    	if(ret==0)
    	{
    		printf     ("connect auidodec%d success\n",no);
              gtloginfo("connect auidodec%d success\n",no);
    		break;
    	}
    	else
    	{
    	    if(fail_cnt==0)
    	    {
    	        printf     ("������Ƶ������%dʧ��\n",no);
    	        gtloginfo("������Ƶ������%dʧ��\n",no);
    	        
    	    }
           fail_cnt++;
           printf("connect auido dec(%d) failed(%d), ret=%d!!\n",no,fail_cnt,ret);
           if(fail_cnt==40)
           {
                printf    ("������Ƶ������%dʧ��%d��!!!",no,fail_cnt);
                gtlogerr("������Ƶ������%dʧ��%d��!!!",no,fail_cnt);
           }
    		sleep(2);
    	}
    }	

    ///�ȴ���Ƶ����������
    fail_cnt=0;
    while(1)
    {
    	ret=get_adec_stat(no);
    	if(ret==ENC_STAT_OK)
    	{
    		printf("��Ƶ������%d״̬����!\n",no);
    		break;
    	}
    	else
    	{
    		if(++fail_cnt==15)
    		{
    			printf    ("��Ƶ������%d״̬�쳣,stat=%d!\n",no,ret);
    			gtlogerr("��Ƶ������%d״̬�쳣,stat=%d!\n",no,ret);
    		}
    		printf("auidodec%d state=%d!!!\n",no,ret);	
    	}
    	sleep(1);
    }    
    return ret;

}

/**********************************************************************************************
 * ������	:disconnect_audio_enc()
 * ����	:�Ͽ���ָ��������������
 * ����	:no:��Ҫ�Ͽ�����Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int disconnect_audio_enc(IN int no)
{
	if(no>=get_audio_num())
		return -EINVAL;	
	return disconnect_media_read(&audio_enc[no]);
}

/**********************************************************************************************
 * ������	:reactive_audio_enc()
 * ����	:���¼����Ƶ������������
 * ����	:no:��Ҫ���¼������Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:Ӧ�ó���Ӧ���ڵ���,��ֹ�����Լ�һ��ʱ��û����Ӧ����
 *			 audioenc����Ͽ�
 **********************************************************************************************/
int reactive_audio_enc(IN int no)
{
	if(no>=1)//fixme ���ж�·��ƵоƬʱ
		return -EINVAL;	
	return reactive_media_usr(&audio_enc[no]);
}

/**********************************************************************************************
 * ������	:read_audio_frame()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&audio_enc[no],frame,buf_len,eleseq,flag);
}
/**********************************************************************************************
 * ������	:read_audio_playback()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_playback(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&audio_enc_playback[no],frame,buf_len,eleseq,flag);
}

/**********************************************************************************************
 * ������	:read_adec_frame()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_adec_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&audio_dec_usr[no],frame,buf_len,eleseq,flag);
}

//#define 	ENC_NO_INIT		0		//������û�г�ʼ��
//#define	ENC_STAT_OK		1		//��������������
//#define	ENC_STAT_ERR		2		//����������	
/**********************************************************************************************
 * ������	:get_aenc_stat()
 * ����	:��ȡָ����ŵ���Ƶ������״̬
 * ����	:no:��Ҫ��ȡ״̬����Ƶ���������
 * ����ֵ	:��ֵ��ʾ����-EINVAL:�������� -ENOENT:�豸��û������
 *					ENC_NO_INIT:δ��ʼ��
 *					ENC_STAT_OK:״̬����
 *					ENC_STAT_ERR:����������
 **********************************************************************************************/
#include <file_def.h>//test !!!
int get_aenc_stat(IN int no)
{
	media_source_t 	*media=&audio_enc[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	if(attrib==NULL)
		return -ENOENT;
	return attrib->stat;
}

/**********************************************************************************************
 * ������	:get_adec_stat()
 * ����	:��ȡָ����ŵ���Ƶ������״̬
 * ����	:no:��Ҫ��ȡ״̬����Ƶ���������
 * ����ֵ	:��ֵ��ʾ����-EINVAL:�������� -ENOENT:�豸��û������
 *					ENC_NO_INIT:δ��ʼ��
 *					ENC_STAT_OK:״̬����
 *					ENC_STAT_ERR:����������
 **********************************************************************************************/
int get_adec_stat(IN int no)
{
	media_source_t 	*media=&audio_dec_usr[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	if(attrib==NULL)
		return -ENOENT;
	return attrib->stat;
}


/**********************************************************************************************
 * ������	:get_aenc_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_aenc_attrib(int no)
{
	media_source_t 	*media=&audio_enc[no];
	if(no>=get_audio_num())
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}
/**********************************************************************************************
 * ������	:get_aenc_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_adec_attrib(int no)
{
	media_source_t 	*media=&audio_dec_usr[no];
	if(no>=get_audio_num())
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}

int get_audio_enc_remain(IN int no)
{
    return read_media_remain(&audio_enc[no]);
}


/**********************************************************************************************
 * ������	:init_audio_rec_enc()
 * ����	:��ʼ����Ƶ����������ؽṹ
 * ����	:�ڼ�·��Ƶ
 * ���	:�ڿ����Ѿ�������һ����̬��������Ƶ�������ṹ
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:�ڳ�������ʱ����һ��
 **********************************************************************************************/
int init_audio_rec_enc_all()
{

	int i,ret;

	for(i=0;i<MAX_AUDIO_ENCODER;i++)
	{
		ret += init_media_rw(&audio_enc_rec[i],MEDIA_TYPE_AUDIO,i,AAC_AUDIO_BUFLEN);
	}
	
	return ret;	
}

/**********************************************************************************************
 * ������	:connect_audio_rec_enc()
 * ����	:���ӵ�ָ����ŵ���Ƶ������
 * ����	:no:��Ҫ���ӵ���Ƶ���������
 *			name:�û���
 *			pre_sec:��ǰ����pre_sec���λ�ÿ�ʼ����
 *					0:��ʾ�����µ�Ԫ������
 *				      >0:��ʾҪ��ǰ����
 *					   ���pre_sec��ֵ���ڻ�����е���֡����������Ԫ�ؽ�������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int connect_audio_rec_enc(IN int no,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	media_attrib_t *attrib=NULL;
	media_format_t *media=NULL;
	audio_format_t *audio=NULL;
    
	if(no >= MAX_AUDIO_ENCODER)
		return -EINVAL;
	ret=connect_media_read(&audio_enc_rec[no], get_audio_enc_key(no),name,MSHMPOOL_NET_USR);
	if(ret>=0)
	{
		stat=get_aenc_rec_stat(no);
		if(stat==ENC_STAT_OK)
		{//������״̬����
			attrib=get_aenc_rec_attrib(no);
			if(attrib!=NULL)
			{
				media=&attrib->fmt;
				audio=&media->a_fmt;
				frate=(audio->a_frate);
				rc=moveto_media_packet(&audio_enc_rec[no],-(frate*pre_sec));
			}
		}
	}
	gtloginfo("�û�[%s]���ӵ���[%d]����� key[%#x]\n",name,no,get_audio_enc_key(no));
	return ret;
}

/**********************************************************************************************
 * ������	:get_aenc_rec_attrib()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
media_attrib_t *get_aenc_rec_attrib(int no)
{
	media_source_t 	*media=&audio_enc_rec[no];
	if(no >= MAX_AUDIO_ENCODER)
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}

/**********************************************************************************************
 * ������	:get_audio_enc_rec_remain()
 * ����	:��ȡָ����Ƶ���������ڴ�صĸ���
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
int get_audio_enc_rec_remain(IN int no)
{
    return read_media_remain(&audio_enc_rec[no]);
}

/**********************************************************************************************
 * ������	:get_aenc_rec_stat()
 * ����	:��ȡָ����Ƶ�������ĸ�����Ϣ�ṹָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ��������Ƶ�������������ԵĽṹָ��
 *			 NULL��ʾ���� ���������󣬱�����δ����
 **********************************************************************************************/
int get_aenc_rec_stat(IN int no)
{
	media_source_t 	*media=&audio_enc_rec[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	if(attrib==NULL)
		return -ENOENT;
	return attrib->stat;
}

/**********************************************************************************************
 * ������	:read_audio_rec_frame()
 * ����	:��ָ����ŵı������ж�ȡһ֡����
 * ����	:no:��Ҫ��ȡ���ݵ���Ƶ���������
 *			:buf_len:frame�������ĳ���,Ҫ��һ֡���ݳ������������ᱨ��
 * ���	:frame:׼�������Ƶ֡�Ļ�����
 *			 eleseq:��Ƶ֡�����
 *			 flag:��Ƶ֡�ı�־
 * ����ֵ	:��ֵ��ʾ��ȡ�����ֽ�������ֵ��ʾ����
 **********************************************************************************************/
int read_audio_rec_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_packet(&audio_enc_rec[no],frame,buf_len,eleseq,flag);
}

/**********************************************************************************************
 * ������	:disconnect_audio_enc()
 * ����	:�Ͽ���ָ��������������
 * ����	:no:��Ҫ�Ͽ�����Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int disconnect_audio_rec_enc(IN int no)
{
	if(no >= MAX_AUDIO_ENCODER)
		return -EINVAL;	
	return disconnect_media_read(&audio_enc_rec[no]);
}

/**********************************************************************************************
 * ������	:reactive_audio_rec_enc()
 * ����	:���¼����Ƶ������������
 * ����	:no:��Ҫ���¼������Ƶ���������
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 * ע		:Ӧ�ó���Ӧ���ڵ���,��ֹ�����Լ�һ��ʱ��û����Ӧ����
 *			 audioenc����Ͽ�
 **********************************************************************************************/
int reactive_audio_rec_enc(IN int no)
{
	if(no >= MAX_AUDIO_ENCODER)//fixme ���ж�·��ƵоƬʱ
		return -EINVAL;	
	return reactive_media_usr(&audio_enc_rec[no]);
}


/**********************************************************************************************
 * ������	:get_audio_rec_read_buf()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ������ָ��
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:ָ���Ӧ�����Ƶ�������Ľṹָ��,������NULL
 **********************************************************************************************/
void *get_audio_rec_read_buf(IN int no)
{
	if(no>= MAX_AUDIO_ENCODER)//fixme ���ж�·��ƵоƬʱ
		return NULL;	
	return audio_enc_rec[no].temp_buf;
}
/**********************************************************************************************
 * ������	:get_audio_rec_read_buf_len()
 * ����	:��ȡָ�����������ڶ�ȡ���ݵ���ʱ����������
 * ����	:no:��Ҫ���ʵ���Ƶ���������
 * ����ֵ	:��ʱ�������ĳ���
 **********************************************************************************************/
int get_audio_rec_read_buf_len(IN int no)
{
	if(no >= MAX_AUDIO_ENCODER)//fixme ���ж�·��ƵоƬʱ
		return -EINVAL;	
	return audio_enc_rec[no].buflen;
}
