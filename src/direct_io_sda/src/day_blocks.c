/*********************************************************************************************
 * ���Ĵ���������д��
 *
 ********************************************************************************************/
#include "blocks.h"
/*����һ���֡��ַ
 * 3600*24*3=86400*12=1036800   <= DATE_HEAD_BLOCK_SIZE*BLOCKSIZE=1037312
 * ���е�����ÿ�붼�����ڴ�������ҵ�����һ����Ӧ��ϵ������˵1���е�һ��I֡��Ӧһ���ڣ��������ʱû�����ݻ�û��I֡������Ϊ0
 * index���������һ���루֡)��day_block�е�λ�ã���������size��������һ����(֡)ռ�Ŀ�������С����
 * day_buff=0x4d4f4e545aa5a55a+int index,int size+struct day_block{ int time; long long blocks;}[3600*24];
 * ע��day_block.time time�������ص�ֵ day_block.blocksָ���洢��Ӳ���ϵ�block��seek
 *
 *
 * */
static unsigned char day_buff[DATE_HEAD_BLOCK_SIZE*BLOCKSIZE];
/*day_block��4�����Ϳ�ʼ��Ҳ����16���ֽ�*/
struct day_block *day_block=(struct day_block *)(day_buff + 4*4);

struct day_block_data{/*β*/int index;int size;};

static unsigned char day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
/*��ǰ�����д��Ŀ����(Ҳ�����һ����д������)���ɴ˱�������֪����ǰд��Ӳ�̵�ʲôλ���ˣ�\
 * �ǲ���д������λ���ˣ�Ҫ��ͷ��ʼ�ˡ�*/
long long current_block;


/*��ǰ�����ڵ�Ӳ�̿�*/
static long long current_day_block;









/*******debug1*******************/
long long seek_tmp_1;



/*******************************************************************
 * ��ȡ��ǰ������
 *******************************************************************/
inline long long get_current_block()
{
	//if(current_blocks==0)???????????
	return current_block;
}
inline long long get_current_day_block()
{
	DP("GET_CURRENT_DAY_BLOCK:%lld\n\n\n\n",current_day_block);
	//if(current_blocks==0)???????????
	return current_day_block;
}
static inline int day_save(long long seek)
{
	DP("SVAE DAY::::seek:%lld\n",seek);
	if(seek<0){printf("seek<0\n");exit(1);}
	int tmp=get_my_time()%SECOFDAY;
	printf("111index:%d,block::%lld,%lld,%lld\n",tmp,day_block[tmp].seek,day_block[tmp-1].seek,day_block[tmp-2].seek);
	/*�������ݵ�Ӳ��*/
	int ret=hd_write(get_hd_fd(), seek, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		DP("write error ,plear resove the probram,and i will exit()\n");
		return -1;
	}


	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);

	printf("222index:%d,block::%lld,%lld,%lld\n",tmp,day_block[tmp].seek,day_block[tmp-1].seek,day_block[tmp-2].seek);


	return 0;
}

inline void day_save1(){day_save(get_current_day_block());}

/*******************************************************************
 * ���ܣ��µ�һ��
 *������blocks��ģ����ã������룻��һ��Ҫд��Ŀ�
 *���أ�Ҫд�������λ��
 ******************************************************************/
long long day_new(long long blocks)
{

/*ͷ��ǰβ�ں�β�����µ�
 *
 *
 */

	/**********************************
	 * bug!!!
	 * �Ƿ�Ҫ�жϣ���ռ�Ŀ����Ƿ�����飿�������ж�����һ������飬��᲻��ռ�����������أ�
	 * ������ڰ����������ʱ��Ӧ�ü����һ������λ���ǲ��������ġ��������������ɾȥ��ʲô
	 * �ǲ���������һ�������blocks��ǰһ��Ļ�Ҫ��Ӳ��ǰ
	 *
	 * ********************************/





	/*��¼��ǰ����λ�� */
	current_day_block = blocks;

	int year_tail_block;

	/*������ô���Ȱ�ͷ�������µ�*/
	memset(day_buff, 0,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	memcpy(day_buff, day_head, sizeof(day_head));
	/*�����������һ����0*/

	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	day_head_queue->index=0;
	day_head_queue->size=0;
	day_block=(struct day_block *)(day_buff + 4*4);
	day_block[day_head_queue->index].time=get_my_time();
	day_block[day_head_queue->index].seek= blocks+DATE_HEAD_BLOCK_SIZE;

	/*�ж����һ���ǲ�����Ӳ�̵����,��Ҫ������λ�ã������Ӳ�̵����λ�ã������ӿ�ʼд��*/
	/*��ô�ж��ǲ������λ���أ�
	 * 0x4d4f4e545aa5a55a���24*3600*12/512=2025��(��2026��)
	 * ���ʣ�µ�û��2026�顣�Ǿʹ�ͷ��ʼд
	 * */
	if(get_hd_max_blocks()-year_get_tail_blocks_of_hd()< DATE_HEAD_BLOCK_SIZE )
	{
		DP("hd is full???\n");
		//������Щ��������˵��Ӳ������
		if(!hd_full_mark_get())
			hd_full_mark_set(TRUE);
		/*�ѵ�һ�飨ͷ����ɾ�ˣ��ڶ���ͳɵ�һ��ͷ�����ˣ������еĴ�Сû�иı�*/
		/*���ͷ����β,˵����ת��һȦ�ˡ�*/
		/*��Ҫ�ѵ�ǰ�Ķ�ͷɾ��*/
		year_insert_new_day_block(DATE_HEAD_BLOCK_SIZE);

		/*�������һ���λ��*/
		current_block =YEAR_HEAD_BLOCK_SIZE+DATE_HEAD_BLOCK_SIZE;
		day_save(current_block);
		/*�������*/
		return current_block;
	}

		DP("hd is not full\n");
//	else if( year_get_tail_block()<year_get_head_block() )
//	{
		//year_insert_new_day_block(year_get_head_address()->blocks);
		year_insert_new_day_block(blocks);
//	}
//	else
//		/*Ӳ��¼��ʱ�䲻����Ӳ�̿ռ仹û������*/
//	{

//	}
		/*��ʽ���ڴ�*/
		day_save(blocks);
		current_block = blocks+DATE_HEAD_BLOCK_SIZE;

		return current_block;


	return 0;
}
/****************************************************************************
 * ���ܣ���ȡ��ǰʱ��Ӧ��д������λ��
 ****************************************************************************/
inline long long day_get_sec_blocks()
{
	/*1���������ҳ����죻2���ڽ������ҳ�����д��ĵ�ַ*/
	/*20130805 yk maybe bug  һ���д������һ�룬��һ��������Ǽ�¼�����һ���λ�ã�������Ϊһ�����кܶ�֡*/

	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	int index=day_head_queue->index ;

	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	long long last_blocks = day_block[index].seek;

	/*��Ҫд��Ŀ��λ�ã���д�����һ�鿪ʼ��λ�� + ��д�����һ��Ĵ�С*/
	long long current_blocks=last_blocks + day_head_queue->size;
	return current_blocks;
}

/******************************************************************************
 * ���ܣ���һ���루֡����д��Ӳ��ʱ��ͬʱ��������ж�Ӧ�Ĵ���λ�Ϳ�λ��
 * ������blocks:��֡��Ӳ����λ�ã���������size����֡��Ӳ���������Ŀ���
 ******************************************************************************/
inline int day_set_sec_block(long long seek,int size,time_t current_time)
{/*ע�⣺����������������е㲻̫��*/
	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	/*�ҵ���֡������е�λ��*/
	int index= day_head_queue->index;
	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);
#if 0//yk del 20130805
	/*��һ������*/
	day_block[index+1].seek=seek;
	day_block[index+1].time=get_my_time();
	/*������Ҫ������� int seek,int size*/
#else
	int tmp=current_time%SECOFDAY;
	day_block[tmp].seek=seek;
	day_block[tmp].time=current_time;
	DP("index:%d,last_seek:%lld,%lld\n\n\n\n\n\n\n\n",tmp,day_block[tmp-1].seek,day_block[tmp].seek);
#endif
	day_head_queue->index=tmp;/*index+1; yk change 20130805*/
	day_head_queue->size=size;
	return 0;
}
/******************************************************************************
 * ���ܣ����������һ֡���룩����д���ʱ��
 * ʹ��������п���һ���Ӧ��֡������ǰ��һ֡��Ч������д����Ӳ�̱������˼��
 * ���أ�ʱ��
 ******************************************************************************/
inline int day_get_lastsec_block()
{
	int index= *(int *)(day_buff + 4*2 + 4*0);
	return day_block[index].time;
}
inline int day_get_lastsec_block_index()
{
	int index= *(int *)(day_buff + 4*2 + 4*0);
	return index;
}


/*******************************************************************************
 * ��ʼ���죬�������ݶ����ڴ���
 *
 *����-1����
 *******************************************************************************/
int day_init()
{
	int ret;
	/*����Ӳ�������һ��*/
	long long last_day_block=year_get_tail_blocks_of_hd();
	DP("DAY,last_day_block:%lld\n",last_day_block);

	/*��ʼд������λ��*/
	current_day_block=last_day_block;


	/*ȡ�����������*/
	memset(day_buff, 0, DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	ret=hd_read( get_hd_fd(),  last_day_block, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read day error\n");
		return -1;
	}
	/*�ж��������������*/
	if(memcmp(day_head,day_buff,sizeof(day_head)) !=0 )
	{
		/**/
		printf("day head is wrong!!\n");
		day_new(last_day_block);

		return -1;
	}

	current_block=day_get_sec_blocks();
	printf("current_day_block:%lld,current_block:%lld\n",current_day_block,current_block);

	/*debug*/
	//if(current_block==0||current_block<current_day_block)
	//{
	//	printf("current_block error\n");
	//	exit(1);
	//}
	//debug

	if(get_disk_format_mark()==TRUE)
	{
		day_new(YEAR_HEAD_BLOCK_SIZE+1);
		return 0;
	}
	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);
	printf("save time:%d,current time:%d\n",day_block[day_head_queue->index].time,get_my_time());
	/*�ж����һ���ǽ�����           �������죬ǰ��*/
	if(day_block[day_head_queue->index].time/SECOFDAY == get_my_time()/SECOFDAY)
	{
		printf("it is today!\n\n\n");
		//�ǽ���
		return 0;
	}
	else
	{
		printf ("not today\n");
		int index=day_get_lastsec_block_index();
		//�����һ������λ��д���µ�һ��
		day_new(day_get_sec_blocks());
	}
	return 0;
}












/*******************************************************************************
 * ָ��seek������������
 *
 *����:��ֵ ���ص�һ�����  -1������ -2���ͷ���� -3��һ����û�ܱ�����Ƶ����
 *******************************************************************************/
long long  day_read_block(long long day_seek)
{
	int ret;
	/*ȡ�����������*/
	memset(day_buff, 0, DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	ret=hd_read( get_hd_fd(),  day_seek, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read day error\n");
		return -1;
	}
	/*�ж��������������*/
	if(memcmp(day_head,day_buff,sizeof(day_head)) !=0 )
	{
		/**/
		printf("day head is wrong!!\n");

		return -2;
	}
	struct day_block *sec_queue=(struct day_block *)(day_buff+sizeof(day_head)+sizeof(struct day_block_data));
	int loop;
	for(loop=1/* 0 yk change debug*/; loop<SECOFDAY; loop++)
	{
		printf("1day_read_block:index:%d time:%d    blocks:%lld\n",loop,sec_queue[loop].time,sec_queue[loop].seek);
		/*����ҵ���һ��֡������*/
		if(sec_queue[loop].seek!=0/*&&*/||sec_queue[loop].time!=0)
			return sec_queue[loop].seek;

	}
	/*����һ����û��д��֡*/
	return -3;
}

/*******************************************************************************
 * ��ʱ�䷵�ص�ǰ���λ��
 *
 *
 *******************************************************************************/
long long  day_read_init(time_t this_time)
{
	int ret;

	/*������з��ص�ǰ���seek�飬����λ�� */
	long long day_seek=year_get_day_from_time(this_time);
DP("day_seek:%lld\n",day_seek);

	//������������,��������ݶ����ڴ���
	ret=day_read_block(day_seek);
	if(ret<0)
	{
		printf("read error:%d\n",ret);
		return  ret;
	}

DP("1\n");
	/*��������ҵ�һ�� ʱ������ӽ��˿��֡��*/
	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	printf("day_read_init:1 %d   %lld\n",day_block[this_time%SECOFDAY].time,day_block[this_time%SECOFDAY].seek);
	if(day_block[this_time%SECOFDAY].time==0||day_block[this_time%SECOFDAY].seek==0)
	{
		/*�����ǰʱ���ǿյģ��Ҹ�����ġ�*/
		int loop;
		for(loop=-10; loop <10; loop ++)
		{
			int tmp=this_time+loop;
			printf("day_read_init:%d   %lld\n",day_block[tmp%SECOFDAY].time,day_block[tmp%SECOFDAY].seek);
			if(day_block[tmp%SECOFDAY].time!=0&&day_block[tmp%SECOFDAY].seek!=0)
				return day_block[tmp%SECOFDAY].seek;
		}
DP("2\n");
		/*���ʱ���ǰ��10����û���ҵ����ϵģ���ô�ͷ��ش����ˡ�*/
		return -1;
	}
	else
		return day_block[this_time%SECOFDAY].seek;


}

/**************************************************************************
 * �ж�����ͷ�ǲ�����ͷ
 * ����-1������ͷ��0����ͷ
 **************************************************************************/
inline int day_head_memcmp(char *buf)
{
	if(memcmp(buf,day_head,sizeof(day_head)) !=0 )
		return -1;
	else
		return 0;
}

/***************************************************************************
 * ���뵱ǰ֡��ַ����ȡ��֡��ַ
 *
 *���أ����ڵ���(YEAR_HEAD_BLOCK_SIZE+1)������ֵ    0ʧ�� 1��һ�������
 **************************************************************************/
long long  day_getnextbkad_formcur(long long seek)
{
	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	int loop;
	BOOL mark=FALSE;
	for(loop=0; loop <SECOFDAY; loop++)
	{
		/*�ҵ���ǰ���λ����*/
		if(day_block[loop].seek==seek)
			mark=TRUE;
		if(mark)
		{
			/*�ҵ���ǰλ�ú�������һ��λ����*/
			if(day_block[loop].seek!=0&&day_block[loop].time!=0)
				return day_block[loop].seek;
		}
	}
	/*������û���ҵ���һ���������һ֡*/
	if(loop==SECOFDAY)
	{
		/*һ�������ˣ�����*/
		return 1;
	}


	return 0;

}




//debug1
int day_cmp_date(char *p,time_t  this_time)
{
	if(p!=NULL)
		printf(p);
	int tmp=this_time%SECOFDAY;
	struct day_block *sec_queue=(struct day_block *)(day_buff+sizeof(day_head)+sizeof(struct day_block_data));
	int loop;
	if(tmp>10)
		loop=tmp-10;
	else
		loop=tmp;

//	printf(":::%d    ",sec_queue);
	for(; loop<tmp+10;loop++)
		printf(" %d, %lld ",sec_queue[loop].time,sec_queue[loop].seek);

	printf("\n");
	return 0;
}










