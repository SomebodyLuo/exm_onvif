
#include <stdlib.h>
#include <stdio.h>
#include <typedefine.h>
#include "rtpool.h"
#include <pthread.h>
#include <string.h>
/**********************************************************************************************
 * ������	:mkrtpool()
 * ����	:����һ��Ԫ�س�Ϊplen,������num��Ԫ�صĻ����
 * ����	:plen:��������ɵ����Ԫ���ֽ���
 *			 num:������е�Ԫ������
 * ���	:head:��������ؽṹ��ָ�룬����ʱ�������
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 **********************************************************************************************/
int mkrtpool(struct pool_head_struct *head,int plen,int num)
{//���������
	int mem_len,i,ret,j;
	struct pool_ele_struct *newele,*oldele;
	struct pool_ele_struct **pool;
	pthread_mutex_init(&head->mutex, NULL);//ʹ��ȱʡ����
	head->total_size=num;
	head->count=0;
	head->plen=plen;
	head->free=NULL;
	head->active=NULL;
	head->pool=NULL;
	// Init Semaphore..
	ret = sem_init(&head->ssema, 0, 0);
	if(ret)
		return -1;
	oldele=NULL;
	mem_len=sizeof(struct pool_ele_struct)-4+plen;
	pool=(struct pool_ele_struct **)calloc(sizeof(struct pool_ele_struct *),num);//�����
	if(pool==NULL)
		return -1;
	
	for(i=0;i<num;i++)
	{
		newele=(struct pool_ele_struct*)calloc(4,mem_len/4);//change malloc(mem_len);
		if(newele==NULL)
		{
			for(j=0;j<i;j++)
				free(pool[j]);
			if(pool!=NULL)
				free(pool);
			head->pool=NULL;
			//not enough memory
			return -1;
		}
		pool[i]=newele;
		memset((void*)newele,0,mem_len);
		if(head->free==NULL)
		{
			head->free=newele;
			oldele=newele;			
		}
		else
		{
			oldele->next=newele;			
			oldele=newele;
		}
		
	}
	newele->next=NULL;	
	head->pool=pool;

	initrtpool(head);
	return 0;
	
}

/**********************************************************************************************
 * ������	:initrtpool()
 * ����	:��ʼ��һ�������,
 * ���	:head:��������ؽṹ��ָ�룬����ʱ�������
 * ����ֵ	:0��ʾ�ɹ���ֵ��ʾ����
 * ע		:������mkrtpool֮�����
 **********************************************************************************************/
int initrtpool(struct pool_head_struct *head)
{//NOT test
	int i;
	struct pool_ele_struct *cur;
	struct pool_ele_struct **pool;
	if(head==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);
	if(head->pool==NULL)
		return -1;
	pool=head->pool;
	head->active=NULL;
	head->free=NULL;
	for(i=0;i<head->total_size;i++)
	{
		cur=pool[i];
		if(cur==NULL)
			continue;
		cur->next=head->free;
		head->free=cur;
	}

	head->count=0;
	pthread_mutex_unlock(&head->mutex);
	return 0;
}

/**********************************************************************************************
 * ������	:get_free_eleroom()
 * ����	:�ӻ���ػ��һ�����еĿռ�
 * ����	:head:��������ؽṹ��ָ��
 * ����ֵ	:ָ��ʣ��ռ��ָ�� ����NULL��ʾû�п��пռ�
 **********************************************************************************************/
struct pool_ele_struct *get_free_eleroom(struct pool_head_struct *head)
{
	struct pool_ele_struct *pret;
	//int total,i;
	if(head==NULL)
		return NULL;
	pret=NULL;
	pthread_mutex_lock(&head->mutex);
	if(head->free==NULL)
	{
		pret=NULL;
	}
	else
	{
		pret=head->free;		
		head->free=pret->next;
		pret->next=NULL;
		pret->ele_type=0;
	}


	
	pthread_mutex_unlock(&head->mutex);
	return pret;		
}

/**********************************************************************************************
 * ������	:put_active_ele()
 * ����	:��Ԫ�ط�����ЧԪ�ض���β��
 * ����	:head:��������ؽṹ��ָ��
 *			 active:ָ��Ҫ�����Ԫ��ָ��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾ����
 **********************************************************************************************/
int put_active_ele(struct pool_head_struct *head,struct pool_ele_struct *active)
{
	int i,total;
	int rc;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	if(active==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);

	total=head->total_size;
	cur=head->active;
	rc=0;
	for(i=0;i<total;i++)
	{
		if(head->active==NULL)
		{
			head->active=active;
			active->next=NULL;
			head->count=1;
			break;
		}		
		if(cur->next==NULL)
		{
			cur->next=active;
			active->next=NULL;
			head->count++;
			break;
		}
		cur=cur->next;
	}
	if(i==total)
	{
		rc=-1;
		//����
	}
	
	pthread_mutex_unlock(&head->mutex);
	return rc;
}
#define GET_ALL_ELE_TYPE	-1
//ȡ����ЧԪ�ض����е�һ��ָ�����͵�Ԫ��
static struct pool_ele_struct *get_active_type_ele(struct pool_head_struct *head,DWORD type)
{
	struct pool_ele_struct *pret,*prev,*cur;
	int i;
	if(head==NULL)
		return NULL;
	pret=NULL;
	pthread_mutex_lock(&head->mutex);
	prev=head->active;
	if(prev!=NULL)
	{
		if((type==GET_ALL_ELE_TYPE)||(prev->ele_type==type))
		{
			pret=head->active;
			head->active=head->active->next;
			if(head->count>0)
				head->count--;
			goto get_active_type_ele_end;
		}
	}
	else
		goto get_active_type_ele_end;
	for(i=0;i<head->total_size;i++)
	{
		cur=prev->next;
		if(cur==NULL)
			break;
		if(cur->ele_type==type)
		{
			pret=cur;
			prev->next=cur->next;
			if(head->count>0)
				head->count--;
			goto get_active_type_ele_end;				
		}
		prev=prev->next;
		if(prev==NULL)
			break;	
	}





get_active_type_ele_end:
	pthread_mutex_unlock(&head->mutex);	
	return pret;
}

/**********************************************************************************************
 * ������	:get_active_ele()
 * ����	:ȡ��������е�һ����ЧԪ��
 * ����	:head:��������ؽṹ��ָ��
 * ����ֵ	:ָ���һ����ЧԪ�ص�ָ�� NULL��ʾû����ЧԪ��
 **********************************************************************************************/
struct pool_ele_struct *get_active_ele(struct pool_head_struct *head)
{

	return get_active_type_ele(head,GET_ALL_ELE_TYPE);

}


/**********************************************************************************************
 * ������	:get_pool_active_num()
 * ����	:��ȡ���������ЧԪ�صĸ���
 * ����	:head:��������ؽṹ��ָ��
 * ����ֵ	:>=0��ʾ���������ЧԪ�صĸ��� ��ֵ��ʾʧ��
 **********************************************************************************************/
int	get_pool_active_num(struct pool_head_struct *head)
{
	int num,i;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	num=0;
	pthread_mutex_lock(&head->mutex);
	cur=head->active;
	for(i=0;i<head->total_size;i++)
	{
		if(cur==NULL)
			break;
		cur=cur->next;
		num++;
	}
	pthread_mutex_unlock(&head->mutex);
	return num;
}

/**********************************************************************************************
 * ������	:get_pool_free_num()
 * ����	:��ȡ�����ʣ��Ԫ�ؿռ�ĸ���
 * ����	:head:��������ؽṹ��ָ��
 * ����ֵ	:>=0��ʾ������е�ʣ��ռ����  ��ֵ��ʾʧ��
 **********************************************************************************************/
int	get_pool_free_num(struct pool_head_struct *head)
{	
	int num,i;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	num=0;
	pthread_mutex_lock(&head->mutex);
	cur=head->free;
	for(i=0;i<head->total_size;i++)
	{
		if(cur==NULL)
			break;
		cur=cur->next;
		num++;
	}
	pthread_mutex_unlock(&head->mutex);
	return num;
}

/**********************************************************************************************
 * ������	:free_ele()
 * ����	:��һ���ù���Ԫ�ط��뻺��صĿ��ж���
 * ����	:head:��������ؽṹ��ָ��
 *			 ele:ָ�����Ԫ�ص�ָ��
 * ����ֵ	:0��ʾ�ɹ� ��ֵ��ʾʧ��
 **********************************************************************************************/
int free_ele(struct pool_head_struct *head,struct pool_ele_struct *ele)
{
	//struct pool_ele_struct *cur;
	//int i;
	if(head==NULL)
		return -1;
	if(ele==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);
	
	ele->next=head->free;
	ele->ele_type=0;
	head->free=ele;
	pthread_mutex_unlock(&head->mutex);
	return 0;
}

/**********************************************************************************************
 * ������	:drop_ele_type()
 * ����	:����ЧԪ�ػ�������ɾ��num��type���͵�Ԫ��
 * ����	:head:��������ؽṹ��ָ��
 *			 type:Ҫ������Ԫ������
 *			 num:Ҫ������Ԫ������
 * ����ֵ	:��ֵ��ʾ�ɹ�������Ԫ�ظ�������ֵ��ʾ����
 **********************************************************************************************/
int drop_ele_type(struct pool_head_struct *head,DWORD type,int num)
{
	int i,ret;//,total;
	struct pool_ele_struct *cur;//,*prev,*erase;
	if(head==NULL)
		return -1;
	ret=0;

	for(i=0;i<num;i++)
	{

		cur=get_active_type_ele(head,type);
		if(cur==NULL)
			break;
		if(free_ele(head,cur)==0)
			ret++;

	}	
	return ret;

}

