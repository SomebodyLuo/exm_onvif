#include "ipmain.h"
#include "devstat.h"
#include "ipmain_para.h"
#include "devinfo_virdev.h"

static struct ip1004_state_struct  ip1004_state[1];//�豸��״̬
/**********************************************************************************************
 * ������	:init_devstat()
 * ����	:��ʼ���豸״̬�ṹ
 * ����	:��
 * ����ֵ	:��
 **********************************************************************************************/
void init_devstat(void)
{
	int i;

	memset((void*)&ip1004_state[0],0,sizeof(struct ip1004_state_struct));
	pthread_mutex_init(&ip1004_state[0].mutex, NULL);//ʹ��ȱʡ����
	ip1004_state[0].reg_dev_state.link_err=1;
	ip1004_state[0].regist_timer=REGIST_TIMEOUT-5;

}

/**********************************************************************************************
 * ������	:get_ip1004_state()
 * ����	:��ȡ�豸��״̬��Ϣ�ṹָ��
 * ����	:��
 * ����ֵ	:�����豸״̬�Ľṹ��ָ��
 **********************************************************************************************/
struct ip1004_state_struct * get_ip1004_state(int dev_no)
{
	return &ip1004_state[dev_no];
}

/**********************************************************************************************
 * ������	:get_gate_connect_flag()
 * ����	:��ȡ��������ͨ�ı�־
 * ����	:��
 * ����ֵ	:����ʾ�Ѿ���������ͨ
 *		��   ����ʾ��û����������ͨ
 **********************************************************************************************/
int get_gate_connect_flag(int dev_no)
{
	struct ip1004_state_struct *stat;
	stat=get_ip1004_state(dev_no);
	return stat->gate_connect_flag;
}
/**********************************************************************************************
 * ������	:set_gate_connect_flag()
 * ����	:�����Ѿ���������ͨ��־
 * ����	:flag:Ҫ���õı�־��ֵ��1��ʾ�Ѿ���������ͨ
 * ����ֵ	:��
 **********************************************************************************************/
void set_gate_connect_flag(int dev_no, int flag)
{//������ͨ���ر�־(��ͨ���ز�����ע��ɹ�!!)
	struct ip1004_state_struct *stat;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	if(flag==0)
	{
		stat->gate_connect_flag=0;		
	}
	else
		stat->gate_connect_flag=1;
	pthread_mutex_unlock(&stat->mutex);
}
/**********************************************************************************************
 * ������	:set_regist_flag()
 * ����	:����ע��ɹ���־ 1��ʾ�ɹ�
 * ����	:flag:Ҫ���õ�ע���־��ֵ 1��ʾ�ɹ�
 * ����ֵ	:��
 **********************************************************************************************/
void set_regist_flag(int dev_no, int flag)
{
	struct ip1004_state_struct *stat;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	if(flag==0)
	{
		stat->regist_flag=0;		
	}
	else
	{
		stat->regist_flag=1;
	}
	stat->regist_timer=REGIST_TIMEOUT-5*(dev_no+1);
	pthread_mutex_unlock(&stat->mutex);
}

/**********************************************************************************************
 * ������	:set_reportstate_flag()
 * ����	:���ñ���״̬�ɹ���־ 1��ʾ�ɹ�
 * ����	:flag:Ҫ���õı���״̬��־��ֵ 1��ʾ�ɹ�
 * ����ֵ	:��
 **********************************************************************************************/
void set_reportstate_flag(int dev_no, int flag)
{
	struct ip1004_state_struct *stat;	
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	if(flag==0)
	{
		stat->reportstate_flag=0;		
	}
	else
	{
		stat->reportstate_flag=1;
		//stat->reportstate_timer=REPORTSTATE_TIMEOUT-5;
	}
	pthread_mutex_unlock(&stat->mutex);
}

/**********************************************************************************************
 * ������	:set_alarm_flag()
 * ����	:���ñ����ɹ���־ 1��ʾ�ɹ�
 * ����	:flag:Ҫ���õı�����־��ֵ 1��ʾ�ɹ�
 * ����ֵ	:��
 **********************************************************************************************/
void set_alarm_flag(int dev_no, int flag)
{
	struct ip1004_state_struct *stat;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	if(flag==0)
	{
		stat->alarm_flag=0;	
		
	}
	else
	{
		stat->alarm_flag=1;
		//stat->alarm_timer=ALARM_TIMEOUT-5;
	}
	printf("dev %d alarm_flag now %d\n", dev_no,stat->alarm_flag);
	pthread_mutex_unlock(&stat->mutex);
}


/**********************************************************************************************
 * ������	:set_trigin_flag()
 * ����	:���ñ���������״̬�ɹ���־ 1��ʾ�ɹ�
 * ����	:flag:Ҫ���õı���������״̬��ֵ 1��ʾ�ɹ�
 * ����ֵ	:��
 **********************************************************************************************/
void set_trigin_flag(int dev_no,int flag)
{
	struct ip1004_state_struct *stat;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	if(flag==0)
	{
		stat->trigin_flag=0;		
	}
	else
		stat->trigin_flag=1;
	stat->trigin_timer=0;
	printf("dev %d trigin_flag now %d\n", dev_no,stat->trigin_flag);
	pthread_mutex_unlock(&stat->mutex);
}

/**********************************************************************************************
 * ������	:get_regist_flag()
 * ����	:��ȡע��ɹ���־ 
 * ����	:��
 * ����ֵ	:����ʾע��ɹ�����ʾû��ע��ɹ�
 **********************************************************************************************/
int get_regist_flag(int dev_no)
{
	struct ip1004_state_struct *stat;
	int i;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	i=stat->regist_flag;
	pthread_mutex_unlock(&stat->mutex);
	return i;
}
/**********************************************************************************************
 * ������	:get_reportstate_flag()
 * ����	:��ȡ����״̬�ɹ���־ 
 * ����	:��
 * ����ֵ	:����ʾ����״̬�ɹ�������ʾû�гɹ�
 **********************************************************************************************/
int get_reportstate_flag(int dev_no)
{
	struct ip1004_state_struct *stat;
	int i;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	i=stat->reportstate_flag;
	pthread_mutex_unlock(&stat->mutex);
	return i;
}	
/**********************************************************************************************
 * ������	:get_alarm_flag()
 * ����	:��ȡ�����ɹ���־ 
 * ����	:��
 * ����ֵ	:����ʾ�����ɹ�������ʾû�гɹ�
 **********************************************************************************************/
int get_alarm_flag(int dev_no)
{
	struct ip1004_state_struct *stat;
	int i;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	i=stat->alarm_flag;
	pthread_mutex_unlock(&stat->mutex);
	return i;
}
/**********************************************************************************************
 * ������	:get_trigin_flag()
 * ����	:��ȡ��������ӱ仯�ɹ���־ 
 * ����	:��
 * ����ֵ	:����ʾ��������ӱ仯�ɹ�������ʾû�гɹ�
 **********************************************************************************************/
int get_trigin_flag(int dev_no)
{
	struct ip1004_state_struct *stat;
	int i;
	stat=get_ip1004_state(dev_no);
	pthread_mutex_lock(&stat->mutex);
	i=stat->trigin_flag;
	pthread_mutex_unlock(&stat->mutex);
	return i;
}

/**********************************************************************************************
 * ������	:get_testd1_flag()
 * ����	:��ȡ����������������״̬
 * ����	:��
 * ����ֵ	:����ʾ���ڽ��и���������������
 *		��   ����ʾû�н��и���������������
 **********************************************************************************************/
int get_testd1_flag(int dev_no)
{
	return ip1004_state[dev_no].test_d1_flag;
}
/**********************************************************************************************
 * ������	:set_testd1_flag()
 * ����	:�������ڽ��и��������������Ա�־
 * ����	:flag:Ҫ���õı�־��ֵ��1��ʾ���ڲ��Ը�����ͨ�� 
 * ����ֵ	:��
 **********************************************************************************************/
void set_testd1_flag(int dev_no,int flag)
{
	ip1004_state[dev_no].test_d1_flag=flag;
}

/**********************************************************************************************
 * ������	:set_alarm_out_stat()
 * ����	:���������״̬���õ��豸״̬�ṹ��
 * ����	:stat:���ӵ����״̬
 * ����ֵ	:��
 **********************************************************************************************/
void set_alarm_out_stat(int dev_no,int stat)
{
	ip1004_state[dev_no].alarm_out_stat=stat;
}

/**********************************************************************************************
 * ������	:dump_ip1004_stat_to_file()
 * ����	:���豸��״̬��Ϣ���ַ�����ʽ�����һ���Ѵ򿪵��ļ�ָ��
 * ����	:fp:һ���Ѿ��򿪵��ļ�ָ��
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
 **********************************************************************************************/
int dump_ip1004_stat_to_file(FILE *fp)
{
	struct ip1004_state_struct *s;
	struct per_state_struct	*per;//��������״̬
	struct dev_state_struct *dev;//�豸����״̬
	struct trig_state_struct 	*trig;//�豸�ı�������״̬
	
	if(fp==NULL)
		return -1;
	int i;
	for(i=0;i<virdev_get_virdev_number();i++)
	{

	s=&ip1004_state[i];
	per=&s->reg_per_state;
	dev=&s->reg_dev_state;
	trig=&s->reg_trig_state;
	fprintf(fp,"ip1004_state_structstruct \n");
	fprintf(fp,"{\n");

	fprintf(fp,"\twatch_version=%s\n",s->watch_version);
	
	fprintf(fp,"\tgate_connect_flag=%d\n",s->gate_connect_flag);
	fprintf(fp,"\tregist_flag=%d\n",s->regist_flag);
	fprintf(fp,"\tregist_timer=%d\n",s->regist_timer);
	fprintf(fp,"\treportstate_flag=%d\n",s->reportstate_flag);
	fprintf(fp,"\treportstate_timer=%d\n",s->reportstate_timer);
	fprintf(fp,"\talarm_flag=%d\n",s->alarm_flag);
	fprintf(fp,"\talarm_timer=%d\n",s->alarm_timer);
	fprintf(fp,"\ttest_d1_flag=%d\n",s->test_d1_flag);
	fprintf(fp,"\talarm_out_stat=0x%02x\n",s->alarm_out_stat);


	
	
		fprintf(fp,"\tstruct per_state_struct\n");
		fprintf(fp,"\t{\n");

			fprintf(fp,"\t\tvideo_loss0=%d\n",per->video_loss0);
			fprintf(fp,"\t\tvideo_loss1=%d\n",per->video_loss1);
			fprintf(fp,"\t\tvideo_loss2=%d\n",per->video_loss2);
			fprintf(fp,"\t\tvideo_loss3=%d\n",per->video_loss3);
			fprintf(fp,"\t\tdisk_full=%d\n",per->disk_full);
			
		fprintf(fp,"\t}\n");	

		fprintf(fp,"\tstruct dev_state_struct\n");
		fprintf(fp,"\t{\n");
			fprintf(fp,"\t\tlink_err=%d\n",dev->link_err);
			fprintf(fp,"\t\thd_err=%d\n",dev->hd_err);
			fprintf(fp,"\t\tcf_err=%d\n",dev->cf_err);
			fprintf(fp,"\t\taudio_dec_err=%d\n",dev->audio_dec_err);
			fprintf(fp,"\t\tvideo_enc0_err=%d\n",dev->video_enc0_err);
			fprintf(fp,"\t\tvideo_enc1_err=%d\n",dev->video_enc1_err);
			fprintf(fp,"\t\tvideo_enc2_err=%d\n",dev->video_enc2_err);
			fprintf(fp,"\t\tvideo_enc3_err=%d\n",dev->video_enc3_err);
			fprintf(fp,"\t\tvideo_enc4_err=%d\n",dev->video_enc4_err);

		fprintf(fp,"\t}\n");	


		fprintf(fp,"\tstruct trig_state_struct\n");
		fprintf(fp,"\t{\n");
			fprintf(fp,"\t\ttrig0=%d\n",trig->trig0);
			fprintf(fp,"\t\ttrig1=%d\n",trig->trig1);
			fprintf(fp,"\t\ttrig2=%d\n",trig->trig2);
			fprintf(fp,"\t\ttrig3=%d\n",trig->trig3);
			fprintf(fp,"\t\ttrig4=%d\n",trig->trig4);
			fprintf(fp,"\t\ttrig5=%d\n",trig->trig5);
			fprintf(fp,"\t\ttrig6=%d\n",trig->trig6);
			fprintf(fp,"\t\ttrig7=%d\n",trig->trig7);
		
			fprintf(fp,"\t\tmotion0=%d\n",trig->motion0);
			fprintf(fp,"\t\tmotion1=%d\n",trig->motion1);
			fprintf(fp,"\t\tmotion2=%d\n",trig->motion2);
			fprintf(fp,"\t\tmotion3=%d\n",trig->motion3);

		fprintf(fp,"\t}\n");	
	

	fprintf(fp,"}\n");
	}
	return 0;
	
}








