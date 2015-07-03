/******************************************************
 *  ���ܣ�ͨ�����Ѱ��ÿ���λ��
 *  �ӿڣ���һ�γ�ʼ����
 *
 ******************************************************/
#include "blocks.h"

static BOOL disk_format_mark=FALSE;



/*�����ݿ�*/
static unsigned char buf_head[YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE];
/*��������������ݣ������������Ǹ�ѭ������,�ṹ������һ��
 * 	struct year_queue_buf
 * 	{
 * 		unsigned int queue_size ,queue_head,queue_tail;
 * 		struct year_block{ unsigned time; long long seek;}[MAXDAY];
 * 	}
 * 	������MAXDAY��ô��Ŀ��У�ֻ�д�queue_head��ʼ����queue_tail������quque_size������Ч
 * */
static unsigned char *year_queue_buf;

struct year_queue_data
{
	unsigned int queue_size ,queue_head,queue_tail;
};


/**********************************************************
 * �����д��Ӳ��
 *********************************************************/
void year_save()
{
	DP("SVAE YEAR:::\n");
	long long seek=1;
	/*�������ݵ�Ӳ��*/
	int ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		DP("write YEAR BLOCK error ,plear resove the probram,and i will exit()\n");
		exit(1);
	}
}
/************************************************************
 * ���������صĴ���
 ************************************************************/
int year_error()
{
	//free (buf_head);
	return 0;
}
/**********************************************************
 * ��ʼ�������ݿ�
 * fd:Ӳ���ļ�������
 * block����ʼ�Ŀ���
 * ���أ�0�ɹ���
 * ********************************************************/
int year_init()
{
	int ret;
	/**************************************************
	 * ��ʼ��Ӳ�̴洢�ṹ
	 *ԭ���鿴Ӳ�̵�һ���ǰ�����ֽ��ǲ���0x59454152a5a5a5a5
	 * ************************************************/

	/*���ݿ�ӵ�һ�鿪ʼ����0����boot*/
	long long seek=1;
	/*********************************************************
	 * ����Ǻ����������ڴ��
	 *
	 *********************************************************/
	disk_format_mark=FALSE;
    memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	//ret=sg_read(outfd, buf_head, blocks, seek,blk_sz, &iflag, &dio_tmp, &blks_read);
    /*��ǰ��ʮ�飬���*/
	ret=hd_read(get_hd_fd(),  seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret==0)
	{
		printf("read block 1 ok\n");
	}
	else
	{
		printf("read block 1 error,and i will exit()");
		myprint(buf_head,512);
		return -1;
	}
	//myprint(buf_head,512*10);


	char buf_head1[YEAR_OFFSET+3*4 +4]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5, \
			0,0,0,0/*��ǰ���еĳ���*/,0,0,0,0/*��ǰ���еĶ�ͷ*/,0,0,0,0/*��ǰ���еĶ�β*//*��ͷ����β����ͬһ��λ��*/};
#if 0
	/*��Ӳ��ͷ�ƻ�����ͷ��ʼд*/
	buf_head[5]=0;
#endif

	/*�����һ������Ҫ��ʽ��*/
	if(memcmp(buf_head1,buf_head,YEAR_OFFSET) !=0 )
	{
		disk_format_mark=TRUE;
		DP("the year head is wrong!!!!!\n");
		/*��ʽ���ڴ�*/
		memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
		/*����ȷ��ͷ���Ƶ�buf�У����ҳ�ʼ������У�д������Ǹ�ʽ����*/
		memcpy(buf_head,buf_head1,YEAR_OFFSET+3*4);

		/*�����ռ��20�飻������һ��������Ŀ���Ϊ20 (��Ϊ�е�0��)*/
		int times;
		times=get_my_time();
		long long blocks=HD_START+YEAR_HEAD_BLOCK_SIZE;/*��ǰ��һ�����Ӧ����21�鿪ʼ*/
		printf("time:%d\n",time);
		/*������еĵ�һ���ֵд��*/
		memcpy(buf_head+YEAR_OFFSET+3*4,(unsigned char *)&times, 4);
		memcpy(buf_head+YEAR_OFFSET+3*4+4,(unsigned char *)&blocks, 8);

		myprint(buf_head,8);
		/*�ӵ�һ�鿪ʼд��512���ֽ�*/
	    //ret=sg_write(fd, buf_head, YEAR_HEAD_BLOCK_SIZE, 1, BLOCKSIZE, &oflag, &dio_tmp);
		ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	    if(ret!=0)
	    {
	    	DP("write error ,plear resove the probram,and i will exit()\n");
	    	return -1;
	    }

	     /*bug!! ���ͷ���󣬸�ʽ���������ʱ���ܻ����*/
	}
	else printf("we have formate,and we can write data\n");




	/*��ʽ������,�������Ӧ��д���족��λ��,Ҳ������Ŀ��� ,��->Сʱ*/
	year_queue_buf=buf_head+YEAR_OFFSET ;

	struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
	unsigned int date_address=	p->queue_tail;/*���һ��*/
	struct year_block *dayblock=(struct year_block *)(year_queue_buf+sizeof(struct year_queue_data));



	/*���϶�����飨YEAR_HEAD_BLOCK_SIZE�����*/
	if(dayblock[date_address].seek<YEAR_HEAD_BLOCK_SIZE)
	{
			printf("date_address:%d,seek:%lld\n",date_address,dayblock[date_address].seek);
			DP("day block in year is wrong!!!\n");
			/*�����������ˣ��ͼ���ӿ�ͷ��ʼ*/
			memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
			memcpy(buf_head,buf_head1,YEAR_OFFSET);
			struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
			p->queue_head=0;
			p->queue_size=0;
			p->queue_tail=0;
			dayblock[0].seek=HD_START+YEAR_HEAD_BLOCK_SIZE;
			dayblock[0].time=get_my_time();

			printf("seek:%lld\n\n",dayblock[0].seek);
			/*�������ٱ��浽Ӳ����*/
			//long long blocks=;
			ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
		    if(ret!=0)
		    {
		    	DP("write error ,plear resove the probram,and i will exit()\n");
		    	return -1;
		    }
	}


	/*******************************************************************
	 * ����Ӧ���в�����ȡget_current_block();
	 *
	 *******************************************************************/

	return 0;
}

/***********************************************************************
 * ������У�������飻�϶�����������һ��ĺ�һ��
 *
 * ����ֵ��-1����
 ***********************************************************************/
int year_insert(int times, long long blocks)
{
	/*�˺���Ӧ�ü���*/
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)year_queue_buf;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];

	//�ж��ǲ��Ǵ�������д�
	/*����Ŀ���̫��*/
	if(get_hd_max_blocks()<blocks)
		return -1;
	/**/





	return 0;
}

/***********************************************************************
 * �������ȡ�����飬����������һ��
 *������times:����ĳ���ʱ��
 ***********************************************************************/
int year_get_day_block(int times)
{
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)year_queue_buf;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];


	return 0;
}
/***********************************************************************
 * ��������ҳ����һ�������
 ***********************************************************************/
inline unsigned int year_get_tail_block(){return *(unsigned int *)(year_queue_buf+4*2);}
 /***********************************************************************
  * ��������ҳ���һ�������
  ***********************************************************************/
inline   unsigned int year_get_head_block(){return *(unsigned int *)(year_queue_buf+4*1);}
  /***********************************************************************
   * ���������еĴ�С
   ***********************************************************************/
inline   unsigned int year_get_size(){return *(unsigned int *)(year_queue_buf+4*0);}
/************************************************************************
 * ����������һ�����ĵ�ַ
 *************************************************************************/
inline struct year_block* year_get_tail_address(){return  \
		(struct year_block *)( (int *)year_queue_buf+4*3 + year_get_tail_block()); \
}
/************************************************************************
 * ��������һ�����(����ͷ)�ĵ�ַ
 *************************************************************************/
inline struct year_block* year_get_head_address(){return  \
		(struct year_block *)( (int *)year_queue_buf+4*3 + year_get_head_block()); \
}
/************************************************************************
 * ����������һ�������Ӳ���е�blocks
 *************************************************************************/
inline long long year_get_tail_blocks_of_hd(){
		// return year_get_tail_address()->seek;
		year_queue_buf=buf_head+YEAR_OFFSET ;

		struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
		unsigned int day_address=	p->queue_tail;/*���һ��*/
		struct year_block *dayblock=(struct year_block *)(year_queue_buf+sizeof(struct year_queue_data));
		return dayblock[day_address].seek;
}
/************************************************************************
 * ����в����µ�һ��
 * �����ĵ�һ�飨ͷ��������һ�飨β�������ڶ��飬��ɵ�һ�顣������µ�һ����˶�β
 * ���������������Ӳ�������
 *************************************************************************/
inline int year_insert_new_day_block(long long seek)
{

	struct year_queue_data *p=(struct year_queue_data *)(buf_head+YEAR_OFFSET);
	/*Ӳ������*/
	if( hd_full_mark_get() )
	{
		DP("HD FULL\n");
		/*��ͷ��ʼд*/
		if(seek==DATE_HEAD_BLOCK_SIZE)
		{
			DP("DAY HEAD BLODK\n");
			/*��ͷ*/
			p->queue_head= 1;
			/*��β*/
			p->queue_tail=0;
			struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
			yb[p->queue_tail].seek=seek;
			yb[p->queue_tail].time=time(NULL);

			/*bug!! ,��Ӳ����ʱ��Ӧ�ð�����к���û���õı�ʾ��Ŀ���0*/

			year_save();
			return 0;
		}
	}
	DP("NOT FULL\n");
	/*һ��ո�ʽ����Ӳ�̣���û��   ������д����*/
	{
		/*���м�һ��Ҳ����������һ�죬*/
		/*��ͷ*/
		p->queue_head +=1;
		/*��β*/
		p->queue_tail +=1;
		struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
		yb[p->queue_tail].seek=seek;
		yb[p->queue_tail].time=time(NULL);
		year_save();
		/*bug!! ,���������е���һ�������blocks�ȵ�ǰ����blocks��Ҫ��ǰ���ǲ��ǿ���˵����һ������������أ�*/
	}
	return 0;
}















/********************************************************
 * ���ص�ǰ�ö���һ����
 * ������Ӳ��ǰ��ʮ�������
 * ******************************************************/
unsigned int get_date_address(unsigned char *buf_queue)
{
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)buf_queue;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];
	/*��Ϊ���еĴ�С��ͷβ����������������*/
	return p[queue_tail+3];
}



























/*****************************************
 * ��ȡ��飬
 *
 *******************************************/

int year_read_init()
{
	int ret;
	/**************************************************
	 * ��ʼ��Ӳ�̴洢�ṹ
	 *ԭ���鿴Ӳ�̵�һ���ǰ�����ֽ��ǲ���0x59454152a5a5a5a5
	 * ************************************************/

	/*���ݿ�ӵ�һ�鿪ʼ����0����boot*/
	long long seek=1;
	/*********************************************************
	 * ����Ǻ����������ڴ��
	 *
	 *********************************************************/

    memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	//ret=sg_read(outfd, buf_head, blocks, seek,blk_sz, &iflag, &dio_tmp, &blks_read);
    /*��ǰ��ʮ�飬���*/
	ret=hd_read(get_hd_fd(),  seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret==0)
	{
		printf("read block 1 ok\n");
	}
	else
	{
		printf("read block 1 error,and i will exit()");
		myprint(buf_head,512);
		return -1;
	}
	//myprint(buf_head,512*10);


	char buf_head1[YEAR_OFFSET+3*4 +4]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5, \
			0,0,0,0/*��ǰ���еĳ���*/,0,0,0,0/*��ǰ���еĶ�ͷ*/,0,0,0,0/*��ǰ���еĶ�β*//*��ͷ����β����ͬһ��λ��*/};

	/*�����һ������Ҫ��ʽ��*/
	if(memcmp(buf_head1,buf_head,YEAR_OFFSET) !=0 )
	{
		DP("the year head is wrong!!!!!\n");
		return -1;
	}
	return 0;
}

/***************************************************
 * �ɴ���ʱ�䣬�õ���Ŀ��λ��
 *
 ***************************************************/
/*ps��bug:��Ϊʱ����ܳ�������һ��ʱ����ܶ�Ӧ����飬��������ֻ���ص�һ��
 * 		  �㷨���ԸĽ�
 */
long long year_get_day_from_time(time_t this_time)
{
	struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
	//struct year_queue_data *p=(struct year_queue_data *)(buf_head+YEAR_OFFSET);

	printf("this_time:%d    %d\n",this_time,this_time/SECOFDAY);
	/*��ǰ���󣬲鵽��һ����Ӧ���췵�أ���ʵ���ܲ鵽�����Ӧ����*/
	int i;
	for(i=0; i<MAXDAY; i++)
	{
		//if(i<10)
			printf(":%d  :%d     :%lld\n",yb[i].time/SECOFDAY, yb[i].time,yb[i].seek);
		if(yb[i].time/SECOFDAY == this_time/SECOFDAY)
			return yb[i].seek;
	}
	return -1;
}

/****************************************************
 * ��ȡӲ�̸�ʽ�Ƿ��ʽ����־λ
 *
 ***************************************************/
inline BOOL get_disk_format_mark()
{
	return disk_format_mark;
}
