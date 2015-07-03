#include "blocks.h"
#define BUFFER_SIZE 400*1024 //���4Mbit������֡Ӧ�ò��ᳬ��100K
static char frame_buffer[BUFFER_SIZE];
//./sg_dd if=sda.img of=/dev/sda oflag=sgio seek=1m bs=1024 count=1


#if 1
#include "pthread.h"

pthread_t ntid;
void *aa()
{
	while(1)
	{
		sleep(1);
		printf("ok\n\n\n");
	}
}

int
main(int argc, char * argv[])
{
	int ret;
	ret = pthread_create(&ntid, NULL, aa, "new thread: ");
	if (ret != 0) {
		fprintf(stderr, "can't create thread: %s\n", strerror(ret));
		exit(1);
	}
//sleep(10);


	   init_sda();


		/*��ʼ����飬�Ƿ�Ҫ��ʽ��*/
		if(0!=year_init())
		{
			printf("year init error\n");
			exit(1);
		}
		DP("year over \n\n\n\n\n");



		/*��ʼ����*/
		if( 0!=day_init())
		{
			printf("day init error\n");
			exit(1);
		}

		DP("day over \n\n\n\n");



		sec_write_data();




		free_sda();
}

#else

int main()
{
	struct hd_frame
	{
		char data_head[8];			/*֡����ͷ�ı�־λ 0x5345434fa55a5aa5*/
		short frontIframe;			/*ǰһ��I֡����ڱ�֡��ƫ�Ƶ�ַ   ���ǰһ֡��Ӳ�̵�����棬���ֵ�����Ǹ�ֵ*/
		short is_I;		/*��֡�ǲ���I֡*/
		unsigned int size;			/*��֡��Ƶ���ݿ�Ĵ�С*/
	};
	printf("::size:%d",sizeof(struct hd_frame));
}


#endif
