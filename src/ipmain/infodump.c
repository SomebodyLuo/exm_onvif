#include "ipmain.h"
#include "ipmain_para.h"
#include "infodump.h"
#include "devstat.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "gate_cmd.h"
#include "ipmain.h"
#include "sqlite3.h"
#include "netcmdproc.h"

static sqlite3 *alarmdb;
pthread_mutex_t db_mutex;	//�������ݿ�Ҫ�õ�����

#define	DB_VERSION	2		//���ݿ�汾��
//  2	������version���Զ����ӵ�id
//	1	���һ�棬ֻ��timestr,type,state,time,triginid


#define TYPE_ALARMACK	"<ALARMACK>"
#define TYPE_ALARMINFO	"<ALARM>    "
#define TYPE_CHANGE		"<CHANGE>    "
#define TYPE_CHANGEACK	"<CHANGEACK>"
/**********************************************************************************************
 * ������	:dump_sysinfo()
 * ����	:��ϵͳ��Ϣ�����ָ���ļ�
 * ����	:��	 
 * ����ֵ	:��
 **********************************************************************************************/
void dump_sysinfo(void)
{
	struct timeval tv;
	struct tm *ptime;
	time_t ctime;
	FILE *dfp;
	char *filename=SYSINFO_DUMP_FILE;
	dfp=fopen(filename,"w");
	if(dfp==NULL)
		return ;
	fprintf(dfp,"\tipmain system runtime info\n\n");

	if(gettimeofday(&tv,NULL)<0)
	{
		fprintf(dfp,"\t��ȡϵͳʱ��ʱ����\n");
	}
	else
	{
		ctime=tv.tv_sec;
		ptime=localtime(&ctime);
		if(ptime!=NULL)
		{
			fprintf(dfp,"\ttime:%d-%d-%d %d:%d:%d.%03d\n\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000);	
		}
	}

	
	dump_ip1004_stat_to_file(dfp);
	dump_para_to_file(dfp);
	fclose(dfp);
}

int get_lognum_callback(void *para, int argc, char **value, char **name)
{
	int *lognum;
	lognum = (int *)para;
	
	*lognum = atoi(value[0]);
	return 0;
}

int get_indexcnt_callback(void *indexcnt, int argc, char **value, char **name)
{
	int *count;
	
	count = (int *)indexcnt;
	*count = atoi(value[0]);
	
	return 0;
}

int get_index_cnt(void)
{
	int lastidx=0;
	char *errmsg=NULL;
	char sqlcmd[100]={0};

	//lc 2014-3-25 �ı��¼��id���Ӳ��ԣ���������+1���������һ��id��+1
	sprintf(sqlcmd,"select id from alarm order by id desc, time desc limit 1");
	sqlite3_exec(alarmdb,sqlcmd,get_indexcnt_callback, &lastidx, &errmsg);
	//sqlite3_exec(alarmdb,"select count(*) from alarm",get_indexcnt_callback, &indexcnt, &errmsg);
	return lastidx;
}


//��һ��׼���õ�sqlcmd��¼�������ݿ�(���û�б��ʹ���)��ɾ��̫�ϵļ�¼
int insert_record_to_db(char *type, char *state,char *alarmid)
{
	int ret;
	char *errmsg=NULL;
	int lognum;
	int id;
	char sqlcmd[1000];
	time_t timep;
 	struct tm *p;
 		
	
	if((type == NULL)||(state == NULL)||(alarmid == NULL))
		return -EINVAL;
	
	time(&timep);
	p=localtime(&timep);
	
	pthread_mutex_lock(&db_mutex);
	ret = sqlite3_open(ALARM_DB, &alarmdb);
	if(ret != 0)
	{
		gtlogerr("�޷������ݿ�%s,ԭ��%s\n",ALARM_DB,sqlite3_errmsg(alarmdb));
		pthread_mutex_unlock(&db_mutex);
		return ret;	
	}

	//������
	ret = sqlite3_exec(alarmdb, "create table alarm( timestr nvarchar[32], type nvarchar[16], state nvarchar[32], time integer, triginid nvarchar[32],version integer , id integer)", NULL,NULL, &errmsg);
	
	//������
	ret = sqlite3_exec(alarmdb, "alter table alarm add  version integer default 1", NULL,NULL, &errmsg);
	ret = sqlite3_exec(alarmdb, "alter table alarm add  id integer default 0", NULL,NULL, &errmsg);
	
	//����sqlcmd
	id =get_index_cnt()+1;
	sprintf(sqlcmd,"insert into alarm values ('<%4d-%02d-%02d %02d:%02d:%02d>','%s','%s','%d','%s','%d','%d')",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,type,state,(int)timep,alarmid,DB_VERSION,id);
	printf("insert_record_to_db sqlcmd is %s\n",sqlcmd);
	//����
	ret = sqlite3_exec(alarmdb,sqlcmd, NULL,NULL, &errmsg);
	//gtloginfo("sqlcmd is %s\n",sqlcmd);
			
		
	//������־������̫��ʱɾ�����ϵ�100��	
	sprintf(sqlcmd,"select count(*) from alarm");
	sqlite3_exec(alarmdb,sqlcmd,get_lognum_callback, &lognum, &errmsg);
	
	if(lognum > MAX_LOG_NUMBER)
	{
		sprintf(sqlcmd,"delete from alarm where rowid in (select rowid from alarm order by id limit 100)");
		ret = sqlite3_exec(alarmdb,sqlcmd,NULL,NULL, &errmsg);
	}
	
	sqlite3_close(alarmdb);
	pthread_mutex_unlock(&db_mutex);
	return ret;
}




/**********************************************************************************************
 * ������	:dump_alarmack_to_log()
 * ����	:������ack��Ϣ�������ָ���ļ�
 * ����	:	result: ack���
 *			alarmid:������
 * ����ֵ	:��
 **********************************************************************************************/
void dump_alarmack_to_log(int result, char * alarmid)
{
	char state[100];
	
	sprintf(state,"0x%04x",result);
	insert_record_to_db(TYPE_ALARMACK,state,alarmid);
	
	return ;
}


/**********************************************************************************************
 * ������	:dump_alarminfo_to_log()
 * ����	:��������Ϣ�ִ������ָ���ļ�
 * ����	:	trig:��ǰ����״̬
 *			time:����ʱ��
 *			alarmid:������
 * ����ֵ	:��
 **********************************************************************************************/
void dump_alarminfo_to_log(DWORD trig,time_t time,char *alarmid)
{
	char state[100];
	
	sprintf(state,"0x%04lx",trig);
	insert_record_to_db(TYPE_ALARMINFO,state,alarmid);
	return ;


}

/**********************************************************************************************
 * ������	:dump_trigininfo_to_log()
 * ����	:��������ӱ仯״̬�������ָ���ļ�
 * ����	:oldtrigin:֮ǰ������״̬
 *		newtrigin:֮�������״̬
 *		time:�����ı��ʱ��
 *		trigin_id:������
 * ����ֵ	:��
 **********************************************************************************************/
void dump_trigininfo_to_log(DWORD oldtrigin, DWORD newtrigin, DWORD time, char *trigin_id)
{
	char state[100];

	sprintf(state,"0x%08lx->0x%08lx",oldtrigin, newtrigin);
	insert_record_to_db(TYPE_CHANGE,state,trigin_id);	
	
	return ;
}

/**********************************************************************************************
 * ������	:dump_triginack_to_log()
 * ����	:�����ط���������ӱ仯״̬��ACKд��ָ���ļ�
 * ����	:	result, ack���ؽ��
 *			trigin_id,������
 * ����ֵ	:��
 **********************************************************************************************/
void dump_triginack_to_log(int result, char *trigin_id)
{
	char state[100];

	sprintf(state,"0x%04x",result);
	insert_record_to_db(TYPE_CHANGEACK,state,trigin_id);
	
	return ;
}



int get_ackid_callback(void *acknumber, int argc, char **value, char **name)
{
	int *numberp;
	
	numberp = (int *)acknumber;
	*numberp = atoi(value[0]);
	
	return 0;
}



//Ѱ�������һ��ָ�����͵�ack,��������id���,
//�������ALARMLOG_VALID_HOURS֮ǰ,�򷵻�ALARMLOG_VALID_HOURS֮ǰ��ʱ��
int find_lastest_ack(char *acktypestr)
{
	char sqlcmd[300];
	char *errmsg = NULL;
	int acknumber = 0;
	int ret;
	
	//ֱ��exec,ʧ�ܾ�����
	pthread_mutex_lock(&db_mutex);
	ret = sqlite3_open(ALARM_DB, &alarmdb);
	if(ret != 0)
	{
		gtlogerr("�޷������ݿ�%s,ԭ��%s\n",ALARM_DB,sqlite3_errmsg(alarmdb));
		pthread_mutex_unlock(&db_mutex);
		return ret;	
	}
	
	//������
	ret = sqlite3_exec(alarmdb, "alter table alarm add  version integer default 1", NULL,NULL, &errmsg);
	ret = sqlite3_exec(alarmdb, "alter table alarm add  id integer default 0", NULL,NULL, &errmsg);
	
	sprintf(sqlcmd,"select id from alarm where type='%s' and state='0x0000' order by id desc, time desc limit 1",acktypestr);
	sqlite3_exec(alarmdb,sqlcmd,get_ackid_callback, &acknumber, &errmsg);
	//gtloginfo("acktime is %d for  %s\n",acktime,acktypestr);
	sqlite3_close(alarmdb);
	pthread_mutex_unlock(&db_mutex);
	
	
	return acknumber;
}


//���ڴ���ÿһ����Ҫ�ط��ı�����¼�Ļص�����
int resend_alarm_callback(void *number, int argc, char **value, char **name)
{
	DWORD trig;
	time_t time;
	int *no;
	struct tm *p;
	struct send_dev_trig_state_struct dev_trig;
	
	trig = atohex(value[2]);
	time = atoi(value[3]);	
	p=localtime(&time);
	
	dev_trig.alarmstate	=	trig;
	dev_trig.year		= 	1900+p->tm_year;
	dev_trig.month 		= 	1+p->tm_mon;
	dev_trig.day		= 	p->tm_mday;
	dev_trig.hour		=	p->tm_hour;
	dev_trig.minute		=	p->tm_min;
	dev_trig.second		=	p->tm_sec;
	
	no =(int *)number;
	*no = *no +1;
	return	send_dev_trig_state(-1,&dev_trig, 1, -1,-1,0);

}

//���ڴ���ÿһ����Ҫ�ط��Ķ�������״̬��¼�Ļص�����
int resend_trigin_callback(void *number, int argc, char **value, char **name)
{
	DWORD oldstate,newstate;
	time_t time;
	int *no;
	char *lp;
	char buf[20];
	
	lp=strstr(value[2],"->0x");
	if(lp == NULL)
		return 0;
	
	strncpy(buf,lp+2,strlen(lp));
	*lp = '\0';
	
	oldstate = atohex(value[2]);
	newstate = atohex(buf);	
	time = atoi(value[3]);	
	//gtloginfo("value[2] is %s, oldstate is %d, newstae is %d\n",value[2],oldstate,newstate);
	
	no =(int *)number;
	*no = *no +1;
	return	send_alarmin_state_change(oldstate,newstate,time,0);
}

//��ѯ��Ҫ�ط��ı�����¼
int process_latest_alarmlog(int ackid)
{
	int ret;
	char sqlcmd[300];
	char *errmsg = NULL;
	int number = 0;
	
	pthread_mutex_lock(&db_mutex);
	ret = sqlite3_open(ALARM_DB, &alarmdb);
	if(ret != 0)
	{
		gtlogerr("�޷������ݿ�%s,ԭ��%s\n",ALARM_DB,sqlite3_errmsg(alarmdb));
		pthread_mutex_unlock(&db_mutex);
		return ret;	
	}
	
	sprintf(sqlcmd,"select * from alarm where type='<ALARM>    ' and id > %d order by id ",ackid);
	//printf("process_latest_alarmlog sqlcmd %s\n",sqlcmd);
	sqlite3_exec(alarmdb,sqlcmd,resend_alarm_callback,&number, &errmsg);
	if(number > 0)
		gtloginfo("��%d��������־δ���ack,�ط����\n",number);
	else
		gtloginfo("û��δ���ack�ı�����־\n");
	sqlite3_close(alarmdb);
	pthread_mutex_unlock(&db_mutex);
	return 0;


}

//��ѯ��Ҫ�ط��Ķ���״̬�仯��¼
int process_latest_triginlog(int ackid)
{
	int ret;
	char sqlcmd[300];
	char *errmsg = NULL;
	int number = 0;
	
	pthread_mutex_lock(&db_mutex);
	ret = sqlite3_open(ALARM_DB, &alarmdb);
	if(ret != 0)
	{
		gtlogerr("�޷������ݿ�%s,ԭ��%s\n",ALARM_DB,sqlite3_errmsg(alarmdb));
		pthread_mutex_unlock(&db_mutex);
		return ret;	
	}
	
	sprintf(sqlcmd,"select * from alarm where type='<CHANGE>    ' and id > %d order by id",ackid);
	sqlite3_exec(alarmdb,sqlcmd,resend_trigin_callback,&number, &errmsg);
	if(number > 0)
		gtloginfo("��%d����������仯��־δ���ack,�ط����\n",number);
	else
		gtloginfo("û��δ���ack�Ķ�������仯��־\n");
	sqlite3_close(alarmdb);
	pthread_mutex_unlock(&db_mutex);
	return 0;
}

/**********************************************************************************************
 * ������	:check_alarmlog()
 * ����	:���δACK�Ľ��ڱ���,���������Է���
 * ����	:   
 * ����ֵ	:0
 **********************************************************************************************/
int check_alarmlog(void)
{
	
	int ackid;
	
	
	
	//ȡ�����һ����ack���,��
	ackid = find_lastest_ack(TYPE_ALARMACK);
	
	process_latest_alarmlog(ackid);
	
	return 0;
}

/**********************************************************************************************
 * ������	:check_triginlog()
 * ����	:���δACK�Ľ��ڶ��ӱ仯��Ϣ,���������Է���
 * ����	:   
 * ����ֵ	:0
 **********************************************************************************************/
int check_triginlog(void)
{
	int ackid;
	ackid = find_lastest_ack(TYPE_CHANGEACK);
	//printf("latest change acktime is %d\n",acktime);
	process_latest_triginlog(ackid);
	return 0;
}


