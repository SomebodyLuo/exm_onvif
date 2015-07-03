#include <soundapi.h>	//����������ļ���ʹ��soundapi��
#include <audiofmt.h>
#include <error.h>	
#include <errno.h>
#include <typedefine.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "devinfo.h"
#include "gtlog.h"

#define		DEF_SAMRATE		8000	//ȱʡ������
#define		DEF_SAMSIZE		16		//ȱʡ����λ��
#define 	DEF_CHANNELS	2		//ȱʡͨ����
#define 	DEF_FRAGSIZE	10		//ȱʡfragment��С
#define		DEF_FRAGNB		16		//ȱʡfragment��



#define 	VERSION			"0.09"
//0.09		����豸�Ƿ����������Ϻ���������3024��ʱ����������������־Ϊ��Ƶ���ݾ�ulaw����󣬻���0ֵ���ݳ��֣���ʱ���´���Ƶ�豸����
//0.08		����ʱ��򿪣��رռ�����Ƶ�豸���Ϻ�������3024��������ʱ����ʱ��������
//0.07		�ָ�����GTVS1000��deafmute��ͬ��
//0.06		ͳһ���ȴ�rec�ٴ�open
//0.05 		����gtopenlog	
//0.04

FILE	*rec_file=NULL;

int main(void)
{
	int devtype;
	snd_dev_t *rec_dev = NULL; //�����ɼ������豸�ľ��
	snd_dev_t *play_dev = NULL; //����¼�������豸�ľ��

	
	#define REC_DATA_SIZE	1024
//	unsigned short playbuf[REC_DATA_SIZE];
	unsigned short rec_buf[REC_DATA_SIZE];
	char *prec=(char*)rec_buf;				///ָ��ɼ���������ָ��
	
	int i,j,ret;
	
	int repeat_cnt=0;						///���Դ���Ƶ�豸�Ĵ���
	int bad_cnt=0;							///��������Ƶ�����ݴ���(ulaw�����ֵΪ0��Ϊ����)


    gtopenlog("deafmute");   
   	init_devinfo();
   	devtype = get_devtype();
   	printf("deafmute version:%s working on %s\n",VERSION,get_devtype_str());
   	gtloginfo("deafmute version:%s ,working on %s\n",VERSION,get_devtype_str());

	while(1)
	{
		///���ڴ������Ƭ�ε��ļ�
        rec_file=fopen("/deafmuteu.raw","wb");
        if(rec_file==NULL)
                printf("can't create deafmuteu.raw!\n");


		///�򿪲ɼ��豸
		rec_dev = open_audio_rec_dev();
        if(rec_dev==NULL)//�д���
        {
                printf("error opening record device, errno %d:%s\n",errno,strerror(errno));
                return -1;
        }
		ret=reset_audio_inbuf(rec_dev);                           ///<����豸������
    	ret=set_audio_params(rec_dev,8000,16,2,10,16);  ///<������Ƶ����
    	ret=set_audio_in_gain(rec_dev,0);       ///<����mic����
    	ret=set_audio_ready(rec_dev);


        /*�򿪲����豸*/
        play_dev = open_audio_play_dev();
        if(play_dev==NULL)//�д���
        {
                printf("error opening play device, errno %d:%s\n",errno,strerror(errno));
                close_audio_rec_dev(&rec_dev);
                return -1;
        }

		set_playback_blocknr(play_dev,-1); ///<<����ģʽ

		//if(repeat_cnt<=0)
		{

			///�ɼ���Ƭ���ݣ����������Ƿ��д��(2009.07���Ϻ������İ��ӷ����е��豸��ʱ�����нϴ�����)
			///�������ݷ���u-law�������һЩ0ֵ�����ݣ��������豸û��

			printf("I'm deaf-and-mute, Grrrr!\n");
			for(i=0;i<10;i++)
			{
				ret = read_audio_data(rec_dev, (char*)rec_buf, REC_DATA_SIZE);
				printf("capture %d bytes audio data...\n",ret);
				ret = conv_stereo2right((char*)rec_buf,(char*)rec_buf,ret);			///<GTVS3000ʹ�õ���������
				ret=conv_raw2ulaw((char*)rec_buf,(char*)rec_buf,ret);
				if((rec_file!=NULL)&&(ret>0))
				{
					fwrite(rec_buf,ret,1,rec_file);
				}
				
				prec=(char*)rec_buf;
				for(j=0;j<ret;j++)
				{
					if(prec[j]==0)
					{
						bad_cnt++;			
						printf("audio bad_cnt=%d i=%d\t j=%d!!\n",bad_cnt,i,j);
					}
				}
				if(bad_cnt>10)
				{
					break;
				}
	        }

			if(rec_file!=NULL)
			{
				fclose(rec_file);
				rec_file=NULL;
			}
			if(bad_cnt>10)
			{
				printf("��⵽���������� bad_cnt=%d,���´���Ƶ�豸,repeat=%d...\n",bad_cnt,repeat_cnt);
				gtloginfo("��⵽����������bad_cnt=%d,���´���Ƶ�豸,repeat=%d...\n",bad_cnt,repeat_cnt);			
			}
			else
			{
				//һ������
				while(1)
	                sleep(1);
			}
		}

		//�������⣬���´�
        /*�ر��豸*/
        if( close_audio_rec_dev(&rec_dev) < 0 )
        {
                printf("close rec device failed, errno %d:%s\n",errno,strerror(errno));
        	close_audio_play_dev(&play_dev);
        	return -1;
        }

        /*�ر��豸*/
        if( close_audio_play_dev(&play_dev) < 0 )
        {
                printf("close play device failed, errno %d:%s\n",errno,strerror(errno));
	        return -1;
        }
		repeat_cnt++;
		if(repeat_cnt>5)	
		{//��β��к�Ͳ�����
			while(1)
				sleep(10);
		}
	}


	return 0;
}


