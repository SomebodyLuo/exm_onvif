/***********************************************************************
 * ���ܣ�����������
 *
 **********************************************************************/
#include "blocks.h"
//debug1
time_t time_tmp_1;
static	long long thisseek;

struct hd_frame
{
	char data_head[8];			/*֡����ͷ�ı�־λ 0x5345434fa55a5aa5*/
	short frontIframe;			/*ǰһ��I֡����ڱ�֡��ƫ�Ƶ�ַ   ���ǰһ֡��Ӳ�̵�����棬���ֵ�����Ǹ�ֵ*/
	short is_I;		/*��֡�ǲ���I֡*/
	unsigned int size;			/*��֡��Ƶ���ݿ�Ĵ�С*/
};

/*�洢���ĵĻ��棬 */
static char frame_buffer[BUFFER_SIZE];	//֡��������С400k

/*��ͷ*/
static char framehead[8]={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
/*��Ӳ������ָ��ͷ*/
char *hd_frame_buff=NULL;
/*�����ǰһ��I֡��ƫ�Ƶ�ַ*/



//��������
static int		seq=-1;                 ///<ý���������
static int 	flag;
/*����֡I֡ʱ�ſ�ʼ��*/
int bool_start=0;
/***********************************************************************
 * ���ܣ���ȡ���Ĵ�С
 * ԭ����seek����ȡһ��
 **********************************************************************/
int sec_get_block_size(long long seek)
{
	int ret;
	long long blocks=1;
	char *p=frame_buffer;
	ret=hd_read( get_hd_fd(),  seek, blocks, p,blocks*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read block 1 error,and i will exit() \n");
		return -1;
	}
	//myprint(date_buf,DATE_HEAD_BLOCK_SIZE);
	if(memcmp(p,framehead,8)!=0)
	{
		/*����������ô�죿*/
		/*������ǵڼ���������*/
		printf("2seek:%lld\n",seek);
		printf("wrong data\n");
		myprint((unsigned char *)p,BLOCKSIZE);

		long long  tmp1=0;
		int tmp2=1;
		/*��ʼ������*/
		while(1)
		{
			p=frame_buffer;
			/*�ȶ�700��*/
			blocks=700;
			memset(p,0,BUFFER_SIZE);
			ret=hd_read(get_hd_fd(),  seek, blocks, p,blocks*BLOCKSIZE);
			if(ret!=0)
			{
				printf("read block 1 error,and i will exit() \n");
				return -1;
			}
			/*��������һ�飬ǰ�漸�����ݶ��ˣ��Ǿͽ�����*/
			for(tmp1=0; tmp1<=blocks; tmp1++)
			{
				/*�ҵ�һ��õ��ˡ�*/
				if(memcmp(p+BLOCKSIZE*tmp1,framehead,8)==0)
				{
					printf("\n\n\naaaa:tmp1:%lld\n",tmp1);
					/*�´δ��⿪ʼ*/
					seek = seek + tmp1;
					printf("1seek:%lld\n",seek);
					tmp2=0;
					//break;
					return -1;
				}
			}
			seek += blocks;
			printf("2seek:%lld\n",seek);
		}
	}
	/*
	else
		printf("the frame size:%d\n",*(int *)(p+12));
	*/
	//myprint((unsigned char *)p,BLOCKSIZE);
	int tmp_size= *(int *)(p+12);
	return tmp_size;

}

/**********************************************************
 * ���ܣ����Ӳ�����ռ䲻��д�ˣ���Ҫ��Ӳ�̿�ͷд
 *      1�������ʱӲ���ʼ�������
 *      2�������ʱӲ���ʼ�����루֡����
 *���أ�����ҪдӲ�̵Ŀ�����
 *
 **********************************************************/
long long  turn_to_hdhead()
{
	struct year_block *tmp=(struct year_block *)year_get_head_address();
	/*���Ӳ�̿�ʼ��һ�������*/
	if( tmp->seek==YEAR_HEAD_BLOCK_SIZE )
	{
		DP("it is day block\n");
		/*������в����µ�һ�飬ͬʱ�ѰѶ��е�ͷ��β�ֱ��ƶ���*/
		year_insert_new_day_block(YEAR_HEAD_BLOCK_SIZE+1);//yk add +1 20130802
		/*�Ǿͷ�����飬����д��*/
		return YEAR_HEAD_BLOCK_SIZE+1;
	}
	/*�����*/
	DP("it is sec block\n");
	return YEAR_HEAD_BLOCK_SIZE+1;
}
/***********************************************************
 * ���ܣ���֡д��Ӳ��
 * ע�⣺�����д˺���ǰҪ��ʼ���ڴ��
 * ����:seeksд���λ�ã�buff,д��֡��������buffsize,д�����ݴ�С
 * ���أ�
 **********************************************************/
int sec_write_frame(long long seeks,char *buff,unsigned int buffsize)
{

	return 0;
}

/**********************************************************
 * �ϲ�ӿ�
 * ���ܣ�д�ļ�
 *
 *
 **********************************************************/
int sec_write_data()
{
	int ret;


	/*��ǰ8���ַ��óɹ涨��ֵ*/
	struct hd_frame framehead={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};

	/*�����ǰһ��I֡��ƫ�Ƶ�ַ*/
	long long  frontIseek=day_get_sec_blocks();;
	/*��ȡ��ǰӦ��д���λ��*/
    int64_t seek =day_get_sec_blocks();

    printf("\n\n\nget seek:%lld\n",seek);


    /*!!!!!!!!bug   �������治�����ʱ����������*/
    if(seek < YEAR_HEAD_BLOCK_SIZE+DATE_HEAD_BLOCK_SIZE)
    {
    	printf("error seek and exit\n");
    	return -1;
    }

	//��ʼ������
	memset(&media ,0, sizeof(media_source_t));
	media.dev_stat= -1; //��ʾû������


	ret=connect_media_read(&media ,0x30000, "video", /*MSHMPOOL_LOCAL_USR*/1);
	if(ret<0)
	{
		printf("error in connect media read and exit\n");
		return -1;
	}


	static int i_times=0;

	/*������һ�쿪ʼ����һ��*/
	time_t in_new_day;

#ifdef DEBUG_LOG
	/*���˼���ʱ��*/
	static int allday_times=0;
	static long long all_frames=1;
	int tmp_time=get_my_time();
#endif /*DEBUG_LOG*/
	//debug
	printf("now we begin read media pool\n");
	while(1)
	{
		//debug1
		//time_tmp_1=get_my_time();
		//day_cmp_date("11",time_tmp_1);

#ifdef DEBUG_LOG
	//printf("this frame :%lld\n",all_frames++);
#endif /*DEBUG_LOG*/

	    /*���һ�µ�ǰʱ�䣬���һ������ˣ��µ�һ�쿪ʼ����ô���أ�*/
		if((get_my_time()%SECOFDAY==0)&&(in_new_day!=get_my_time()) )
		{
			/*��1s�ڿ����кܶ�֡*/
			in_new_day=get_my_time();
			printf("1day:seek:%lld\n",seek);
			seek=day_new(seek);// seek�����룬�϶�Ҫ���ˡ���ô������
			printf("2day:seek:%lld\n\n\n",seek);

#ifdef DEBUG_LOG
			allday_times++;
			printf("this is day:\t\t%d\n\n\n\n\n\n",allday_times);
			//if(allday_times>2)return 0;
#endif /*DEBUG_LOG*/
		}

		//��ȡ֡������ֵΪ֡��С ����kframe��ʲô�أ����ѵ�flag �����ǲ���I֡�����
		memset(frame_buffer, 0, BUFFER_SIZE);
		seq=-1;flag=-1;
		ret=read_media_resource(&media,frame_buffer, BUFFER_SIZE, &seq, &flag);
		if(ret<0)
		{
			printf("error in read media resource\n");
			exit(1);
		}
		enc_frame_t* the_frame_buffer=(enc_frame_t *)frame_buffer;

		int is_i;
		if(flag==1)
			is_i=0;
		else
			is_i=1;
		/*����һ��I֡ʱ�ſ�ʼִ������Ĵ��룬�洢��Ƶ����*/
		if(is_i){bool_start=1;}
		if(!bool_start)continue;

		/*����������Ƶ֡�ĵ�ַ��ǰƫ��16λ����Ŵ���Ӳ�����֡ͷ�����������ֲ���Σ��*/
		hd_frame_buff=the_frame_buffer->frame_buf-sizeof(struct hd_frame);

		/*����ĳ�Ҫ+��ͷ*/
		//int blocks=	(( (the_frame_buffer->len +sizeof(struct hd_frame)) + BLOCKSIZE -1)/BLOCKSIZE);
		int blocks=	(( (the_frame_buffer->len +sizeof(struct hd_frame)) + BLOCKSIZE -1)/BLOCKSIZE);//yk change 20130731

		/*���Ӳ��ʣ�µĿռ��ǲ��ǹ���*/
		if(blocks > get_hd_max_blocks() - seek)
		{
			//printf("11seek=%lld\n",seek);
			//printf("22the ---:%lld\n",(get_hd_max_blocks() - seek));
			//printf("33the hd don't have enough blocks---blocks:%d,seek:%lld\n",blocks,seek);
			/***********************************************
			 ��ô����??????????????????��seekֵ���ˡ����ؾ�ok��
			***********************************************/
			seek =turn_to_hdhead();
			printf("44seek=%lld\n",seek);
		}

		/*�����ǰһI֡ƫ�Ƶ�ַΪ*/
		framehead.frontIframe=seek-frontIseek;				//��᲻������أ�������
		framehead.size=the_frame_buffer->len;


		/*����֡��I֡ʱ���ѱ�֡�Ŀ�seek����*/
		if(is_i)
		{

			frontIseek =seek;
			framehead.is_I=1;
		    /*���һ����û������I֡�����´��루֡�����������������Ϣ*/
			time_t current_time=get_my_time();
		    if(day_get_lastsec_block()!=current_time)
		    {
		    	DP("set sec block\n");
		    	day_set_sec_block(seek,blocks,current_time);/*ע�⣺����������������е㲻̫��*/
		    }
		    /*��100��I֡ʱ�����д��Ӳ��*/
		    i_times++;
		    //printf("i_time:%d\n",i_times);
		    if(i_times%20==0)
			{
		    	DP("write day\n");
		    	day_save1();
		    	//return 0;
			}
			 printf("I\n\n\n");
		}

		//day_cmp_date("22",time_tmp_1);
		/*��֡����ͷд��hd_frame_buff��*/
		memcpy(hd_frame_buff,&framehead,sizeof(struct hd_frame));
		/*��Ӳ��*/
		ret=hd_write(get_hd_fd(), seek, blocks, hd_frame_buff,blocks*BLOCKSIZE);
	    if(ret!=0)
	    {
	    	printf("write date buf error ,plear resove the probram,and i will exit()\n");
	    	exit(1);
	    }
		//day_cmp_date("33",time_tmp_1);



#ifdef DEBUG_LOG
	    printf("currnet second:%d,second:%d,:sec:%d:seek:%lld\n",get_my_time(),get_my_time()-tmp_time,get_my_time()%60,seek);
	    //printf("len:%d,blocks:%d\n",(unsigned int)(framehead.size +sizeof(struct hd_frame)),blocks);
#else
	    printf("seek:%lld\n",seek);
#endif

	    /*��һ���λ��*/
	    seek += blocks;


	}
	return 0;
}




inline void set_seek(long long seek)
{
	thisseek=seek;
}

/**********************************************************
 * �ϲ�ӿ�
 * ���ܣ���֡
 *
 *����-1������ -2��������֡ͷ������ -3�µ�һ����û������ -4δ֪�Ĵ���
 **********************************************************/
int sec_read_data()
{
	/*
	//��һ֡��ʱ�� �Ƚ�����
	time_t last_frame_time;
	*/
	long long *read_seek=&thisseek;
	long long *seek=read_seek;
	int ret;
	int blocks;
	char *p=frame_buffer;
	//while(1)
	{


		//usleep(40000);		//25֡
begin:
		blocks=1;
		memset(p,0,BUFFER_SIZE);
DP("1\n");
		ret=hd_read(get_hd_fd(),  *read_seek, blocks, p,blocks*BLOCKSIZE);
		if(ret!=0)
		{
			printf("read block 1 error,and i will exit() \n");
			//*seek=read_seek;/*���������ͨ��ָ��ѵ�ǰ��seek����ȥ*/
			return -1;
		}
DP("2\n");
		//myprint(date_buf,DATE_HEAD_BLOCK_SIZE);
		if(memcmp(p,framehead,8)!=0 )
		{
			DP("21\n");
			long long ret1;
			/*����������ô�죿*/
			/*������ǵڼ���������*/
			printf("2seek:%lld\n",*read_seek);
			printf("wrong data\n");
			myprint((unsigned char *)p,/*BLOCKSIZE*/8);
			/*������ͷ*/
			if( day_head_memcmp(p)==0)
			{
				ret1=day_read_block(*read_seek);
				if(ret1>0)
				{
					printf("new day begin!!!\n");
					/*�µ�һ�죬���µ�һ��ĵ�һ����֡��ʼ��*/
					*read_seek =ret1;
					DP("211\n");
					goto begin;
				}

				printf("ERR:not new day!!!may be it's bad block ret=%lld\n",ret1);
				/*���һ����Ϳ*/
				//*seek=read_seek;/*���������ͨ��ָ��ѵ�ǰ��seek����ȥ*/

				return (int )ret1;
			}
			/*****************************************************************************************
			 * ��������һ֡���ݼȲ�����֡��Ҳ������֡��A��������Ӳ���õ����û�ռ��ˣ��ִ�ͷ��ʼд�ˣ�����Ҫ��ͷ��ʼ��
			 * B�������һ������֡���������������Ӧ�ò�����֣�����sgio�����⣩
			 * ****************************************************************************************/
			else
			{
				DP("22\n");
				long long ret1=-1;
				ret1=day_getnextbkad_formcur(*read_seek);

				if(ret1<YEAR_HEAD_BLOCK_SIZE+1)
				{
					if(1==ret1)
					{
						printf("WARRING::day is over,and sec frame is over!!!\n");
						*read_seek=YEAR_HEAD_BLOCK_SIZE+1;
						goto begin;
					}
					else
					{
						printf("ERR:UNKNOW ERROR!!!   \
								day_getnextbkad_formcur:::ret1=%ld",ret1);
						return ret1;
					}
				}

				printf("ret1:%lld\n",ret1);
				*read_seek=ret1;
				goto begin;
			}/*end of if( day_head_memcmp*/

		}



		//struct hd_frame
		int tmp_size= *(int *)(p+12);
DP("3\n");
		/*�ٴζ�ȡ��һ֡ʣ�ಿ��*/
		blocks=(tmp_size+16 +512 -1)/512 -1;
		*read_seek = *read_seek+1;
		ret=hd_read(get_hd_fd(),  *read_seek, blocks, p+BLOCKSIZE,blocks*BLOCKSIZE);
		if(ret!=0)
		{
			printf("read block 1 error,and i will exit() \n");
			*seek=*read_seek-1;
			return -1;
		}
DP("4\n");
#if 1
		ret= fifo_write(p+16, tmp_size);
		if(ret<0)
		{
			perror("write error\n");
			exit(1);
		}
#endif
		printf("ret=%d\n",ret);
		*read_seek += blocks;
		printf("read seek:%lld\n",*read_seek);
	}
	return 0;
}

