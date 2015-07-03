#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

static int init_flag=0;
static FILE *netfd=NULL;
unsigned long old_bytesout=0;

#define NET_FILE	("/proc/net/dev")

/*********************************************************
 *������:init_gtifstat();
 *����:  ��ʼ��
 *����ֵ:��ȷ����0�����󷵻ظ���
 *��ע: ��
 * ********************************************************/
int init_gtifstat(void)
{
	if ((netfd = fopen(NET_FILE, "r")) == NULL)
	{
    		printf("can't open %s: %s",NET_FILE,strerror(errno));
    		return -1;
  	}
	init_flag=1;
	return 0;
}


/**********************************************************
 *������:get_send_pkts();
 *����:��ȡָ���ӿڵķ��ͳ�ȥ�����ݰ�����
 *����ֵ:���ͳ�ȥ�����ݰ��ĸ���
 *��ע:����ǰΪֹ�����ͳ�ȥ�����ݰ��ĸ���
 **********************************************************/
unsigned long get_send_pkts(void)
{
	char buf[1024];
	char *iface=NULL;
	char *stats=NULL;
	int cnt;
	unsigned long bytesin=0;
	unsigned long bytesout=0;
	unsigned long tmp_out;

	//���»�����
	fflush(netfd);
	fseek(netfd,0,SEEK_SET);	
	/* check first lines */
	fgets(buf, sizeof(buf),netfd);	//filter first line
	fgets(buf, sizeof(buf),netfd);	//filter second line
 	fgets(buf, sizeof(buf),netfd);  //filter third line , lo

	//check the eth0
	if(fgets(buf, sizeof(buf),netfd) != NULL) 
	{
		if ((stats = strchr(buf, ':')) == NULL)
    			*stats++ = '\0';
    		iface = buf;
    		while (*iface == ' ')
      			iface++;
    		if (*iface == '\0')
			return -1;
		cnt=sscanf(&stats[1], "%*u %lu %*u %*u %*u %*u %*u %*u %*u %lu", &bytesin, &bytesout);

	}
	tmp_out=bytesout-old_bytesout;
	old_bytesout=bytesout;	
	//printf("[%s]....bytein=%ld,byteout=%ld,,cnt=%d..........[%ld/sec]\n",stats,bytesin,bytesout,cnt,tmp_out);
	
	return bytesout;
}

int close_gtifstat(void)
{
	return fclose(netfd);
}

#if 0
int main(void)
{
	int i=0;
	unsigned long snd_pkts;

	printf("��ʼ����...\n");
	init_gtifstat();	
	//for(i=0;i<40;i++)
	while(1)
	{
		snd_pkts=get_send_pkts();
		printf("zw-test ---->pkts_send=%ld\n",snd_pkts);
		sleep(1);
	}
	close_gtifstat();
	
}
#endif
