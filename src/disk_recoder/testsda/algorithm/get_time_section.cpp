/*
 * get_time_section.cpp
 *
 *  Created on: 2014-2-10
 *      Author: yangkun
 */

#include <iostream>
#include <stdio.h>
using namespace std;
/***********************************************************************************************************
 *  �㷨���������ȡʱ���
 *
 *  ԭ�����������bQueueһ�������������ҳ������ڵĶΣ�����3����(����������)��������θ����ˣ�
 *
 ***********************************************************************************************************/

#define JG 3
int  bQueue[]={/*0*/1,0,0,0, /*4*/1,0,0,0, /*8*/1,1,0,0, /*12*/0,0,0,0, /*16*/1,0,0,0, /*20*/0,1,1,0};

int print(int *bq,int size);
int main()
{
	print(bQueue,sizeof(bQueue)/sizeof(int));
}

int find(int *bq ,int size, int value)
{
	int i;
	for(i =0; i<size; i++)
	{
		if(bq[i]==value)
			break;
	}
	return i;
}

int print(int *bq,int size)
{
	int num=0;
	int tmp;
	while(1)
	{
		if(num>=size)break;
		//�ҵ�һ��
		tmp = find(bq+num,size-num, 1);
		num += tmp;
		cout << num << "-->" ;
		fflush(stdout);
		//�����һ��
		while(1)
		{
			if(num>=size)break;
			/* 1 1 1 0*/
			int tmp1=find(bq+num,size-num, 0);
			num+=tmp1;

			/*0 0 0 1*/
			tmp=find(bq+num, size-num,1);
			num+=tmp;
			if(tmp>=JG
#if 1
					||num==size  //�����������Ҫ��˵�����ҵ�����������
#endif
					)
			{
				cout << num-tmp-1 ;

				break;
			}
		}
		cout << endl;
	}

	return 0;
}

