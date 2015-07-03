#include "hdctl.h"
#include "stdio.h"
#include "error.h"
#include "stdlib.h"

#include <unistd.h>
#include "string.h"

int main(int argc, char **argv)
{
	int diskno;
	int ret;
	int percent;
	
	if(argc ==1)
		diskno = 0;
	else
		diskno = *argv[1];
		
	printf("����%d�Ŵ��̵Ķ̲���!�����ĵȴ�2��������\n",diskno);
	ret = run_hd_smarttest(diskno,GT_SMART_SHORTTEST); //shorttest
	if(ret != 0)
	{
		printf("%d�Ŵ��̶̲���ʧ��,%d:%s\n",diskno,ret,strerror(-ret));
		return -1;
	}
	sleep(100);
	
	ret =get_hd_shorttest_result(diskno,&percent) ;
	if(ret == 0)
	{
		printf("%d�Ŵ��̶̲���ͨ�������д��̳�����!\n",diskno);
		run_hd_smarttest(diskno,GT_SMART_LONGTEST);
		printf("%d�Ŵ����ѿ�ʼ���ԡ�����5Сʱ����ú��ѯ���Խ����\n",diskno);
	}
	else
		printf("%d�Ŵ��̶̲��Խ�� %d:%s�������г�����\n",diskno,ret,get_testresult_str(ret));
	return 0;
}
