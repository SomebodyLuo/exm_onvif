/*	GT1000ϵ����Ƶ�������鲥����(UDP)Э��ײ�⺯��
 *		 2006.9 shixin
 *    ���������ṩ��ʵ��Ƕ��ʽϵͳ��Զ�̼����ʹ���鲥��ʽͨѶ�ĵײ�ӿ�
 *    �ϲ��Ӧ�ýӿڼ����ݽṹ�����ڱ����ʵ�ַ�Χ
 *    ���������ṩ�Ĺ�����
 *    1.���������ݱ�����GTϵ����Ƶ���������鲥ͨѶЭ���ʽ���д�������ܣ�crc32У�飬֮���͸�ָ���ĵ�ַ
 *    2.��ָ����socket�н������ݰ�������crcУ�飬�����ı任�����ķ��ظ����ý��̣��������ݰ��ķ�������Ϣ���ص�����
 *    
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
	//windows
	#include <winsock2.h>
#else
	//linux
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include "gt_dev_api.h"
#include "tab_crc32.c"


/////////////////////////////////////////////////////////////////////////////////////////
static const unsigned char	broadcast_addr[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//static __inline__ void print_buffer(unsigned char *buf,int len)
static void print_buffer(unsigned char *buf,int len)
{
    int i;
    for(i=0;i<len;i++)
        printf("%02x,",buf[i]);
    return ;
}


/***************Э��ͷ��β����*******************************/
//�������Լ�ʹ�õĶ���
typedef struct{
	unsigned long	head;				//ͷ��־
	unsigned char	version;			//Э��汾��
	unsigned char	reserve;			//����
	unsigned short	enc_type;			//���ݰ���������
	unsigned char	target_id[GUID_LEN];//Ŀ���ַ(guid)
	unsigned char	source_id[GUID_LEN];//Դ��ַ(guid)
	unsigned short	reserve1;			//����
	short			pkt_len;			//���ݰ�����(data�е���Ч�ֽ���)
	unsigned long	xor32;				//��ͷ��32bit���У����
	unsigned char	data[4];			//���д���ʵ������
}gtdev_pkt_struct;

static const unsigned long 	pkt_head=0x55aa5540;	//��ͷ
static const unsigned char	proto_version=1;	//Э��汾��

/********************************************************************/
#define	BUF_SIZE		(2048)	//�ײ��Ļ�������С

/**********************************************************************************************
* ������	:send_dev_pkt()
* ����	:��һ�����İ���ָ���ļ��ܷ�ʽ�γ��������ݰ�,���͵�ָ����socket��ַ
* ����	:	fd:Ŀ��socket�ļ�������
* 		sin:����Ŀ���ַ��Ϣ�Ľṹָ��
* 		target_id:Ŀ��id��
* 		source_id:������id��
* 		buf:Ҫ���ͳ�ȥ�����ݻ�����
* 		buflen:�������е���Ч�ֽ���
* 		enc_type:Ҫʹ�õļ��ܷ�ʽ()
* 		flag:����ѡ���������ʱд0
* ����ֵ:	
* 		��ֵ��ʾ���ͳ�ȥ���ֽ�������ֵ��ʾ����
**********************************************************************************************/
int send_dev_pkt(IN int fd,IN struct sockaddr_in *sin,IN unsigned char *target_id,IN unsigned char *source_id,IN void *buf,int buflen,IN int enc_type,IN int flag)
{
	unsigned long 		send_buf[BUF_SIZE/4];
	int					pkt_len;
	unsigned long		*dw=NULL;
	unsigned long		*crc32=NULL;
	int					ret;
	int					addon;
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)send_buf;

	if((fd<0)||(buflen<=0)||(buf==NULL))
		return -EINVAL;
	if(buflen>=(BUF_SIZE-sizeof(gtdev_pkt_struct)))
	{
		return -EINVAL;
	}
	flag=flag;
	pkt->head=pkt_head;
	pkt->version=proto_version;
	pkt->reserve=0;
	pkt->enc_type=enc_type;
	memcpy((void*)pkt->target_id,target_id,GUID_LEN);
	memcpy((void*)pkt->source_id,source_id,GUID_LEN);
	pkt->reserve1=0;

	if(enc_type==GTDEV_ENC_NONE)
	{
		memcpy((void*)pkt->data,buf,buflen);
		pkt_len=buflen;
	}
	else
	{
		printf("can't support enc mode %d\n",enc_type);
		return -EINVAL;
	}

	if((pkt_len%4)!=0)//��4�ֽڶ���
	{
		addon=4-(pkt_len%4);
		memset((char*)pkt->data+pkt_len,0,addon);
	}
	else
		addon=0;

	pkt_len+=addon;
	pkt->pkt_len=pkt_len;

	dw=(unsigned long*)&pkt->version;
	pkt->xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
	crc32=(unsigned long*)((char*)pkt->data+pkt_len);
	*crc32=tab_crc32((void*)pkt->data,pkt_len);
	ret=sendto(fd,(void*)pkt,(sizeof(gtdev_pkt_struct)-sizeof(pkt->data)+pkt_len+4),MSG_DONTROUTE,(struct sockaddr *)sin, sizeof(*sin));	
	if(ret<0)
		ret=-errno;
	return ret;
}



/**********************************************************************************************
* ������	:recv_dev_pkt()
* ����	:	��ָ���ӿڽ���һ�����ݣ����û������������
* ����	:	fd:Ҫ�����ļ���socket�ļ�������
*		selfid:�Լ���id��
*		buflen:������Ϣ�Ļ���������
*		flag:���ձ�־����������ʱд0
* ���	:	
* 		sin:���������ߵ�ַ��Ϣ�Ľṹָ��
* 		source_id:������id��
* 		msgbuf:���յ�����Ϣ
* 		enc_type:���յ������ݰ��ļ�������
* ����ֵ:	
* 		��ֵ��ʾ���յ�msgbuf�е���Ч�ֽ�������ֵ��ʾ����
*		-ENOMEM ��ʾ����������
**********************************************************************************************/
int recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid,OUT unsigned char *sourceid,OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag)
{
	unsigned long 		recv_buf[BUF_SIZE/4];//���ջ�����
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)recv_buf;
	int			ret;
	int			slen; 
	unsigned long		*dw=NULL;
	unsigned long		xor32,crc32,*pkt_crc32=NULL;
	if((fd<0)||(sin==NULL)||(selfid==NULL)||(sourceid==NULL)||(msgbuf==NULL)||(buflen<=0)||(enc_type==NULL))
		return -EINVAL;
	flag=flag;
	while(1)
	{
		slen=sizeof(*sin);
		ret=recvfrom(fd,(void *)pkt,sizeof(recv_buf),0,(struct sockaddr *)sin,&slen);
		if(ret<0)
		{
			if(errno==EINTR)
				continue;
			else
				return ret;
		}
		else if(ret==0)
		{
			continue;
		}
		//printf("recv %d bytes!!\n",ret);
		//print_buffer(recv_buf,ret);
		//printf("\n");
		if(pkt->head!=pkt_head)
			continue;	//����Э���ͷ
		dw=(unsigned long*)&pkt->version;
		xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
		if(xor32!=pkt->xor32)
		{
			//printf("xor32 0x%x != 0x%x\n",xor32,pkt->xor32);
			continue;	//��ͷУ�鲻��
		}
		if(memcmp(pkt->source_id,selfid,GUID_LEN)==0)
			continue;	//������Լ����͵İ��򲻴���
		if((memcmp(pkt->target_id,broadcast_addr,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid,GUID_LEN)!=0))
		{
			continue;	//���������ĵ�ַ
		}
		pkt_crc32=(unsigned long*)((char*)pkt->data+pkt->pkt_len);
		crc32=tab_crc32(pkt->data,pkt->pkt_len);
		if(crc32!=*pkt_crc32)
		{
		//	printf("crc32 0x%x != 0x %x\n",crc32,*pkt_crc32);
			continue;	//crc�����
		}
		*enc_type=pkt->enc_type;
		memcpy(sourceid,pkt->source_id,GUID_LEN);
		if(pkt->enc_type==GTDEV_ENC_NONE)
		{
			if(buflen<pkt->pkt_len)
				return -ENOMEM;
			memcpy(msgbuf,pkt->data,pkt->pkt_len);
			return pkt->pkt_len;
		}
		else
		{
			printf("receive unknow enc_type:%d pkt",enc_type);
			continue;	//����ʶ�ļ��ܸ�ʽ
		}
	}
	return 0;
}

/**********************************************************************************************
 * * ������        :dual_id_recv_dev_pkt()
 * * ����  :       ��ָ���ӿڽ���һ�����ݣ����û������������
 * * ����  :       fd:Ҫ�����ļ���socket�ļ�������
 * *               selfid:�Լ���id�ţ�����Ϊ GUID_LEN
 * *		   selfid1:����һ��id�� ����ΪGUID_LEN
 * *               buflen:������Ϣ�Ļ���������
 * *               flag:���ձ�־����������ʱд0
 * * ���  :       
 * *               sin:���������ߵ�ַ��Ϣ�Ľṹָ��
 * *               source_id:������id�� ����ΪGUID_LEN
 * *               msgbuf:���յ�����Ϣ
 * *               enc_type:���յ������ݰ��ļ�������
 * *	           recv_id:���յ���id�� 
 * * ����ֵ:       
 * *               ��ֵ��ʾ���յ�msgbuf�е���Ч�ֽ�������ֵ��ʾ����
 * *               -ENOMEM ��ʾ����������
 * *	lsk 2009-2-9 ������һ�������id������id�κ�һ����ȷ���ܹ��������ݰ�
 * **********************************************************************************************/
EXPORT_DLL int dual_id_recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid, IN unsigned char *selfid1,OUT unsigned char *sourceid,OUT unsigned char *recv_id, OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag)
{
	unsigned long 		recv_buf[BUF_SIZE/4];//���ջ�����
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)recv_buf;
	int			ret;
	int			slen; 
	unsigned long		*dw=NULL;
	unsigned long		xor32,crc32,*pkt_crc32=NULL;
	//// lsk 2009 -2-9
	if((fd<0)||(sin==NULL)||(selfid==NULL)||(recv_id==NULL)||(selfid1==NULL)||(sourceid==NULL)||(msgbuf==NULL)||(buflen<=0)||(enc_type==NULL))
		return -EINVAL;
	flag=flag;
	while(1)
	{
		slen=sizeof(*sin);
		ret=recvfrom(fd,(void *)pkt,sizeof(recv_buf),0,(struct sockaddr *)sin,&slen);
		if(ret<0)
		{
			if(errno==EINTR)
				continue;
			else
				return ret;
		}
		else if(ret==0)
		{
			continue;
		}
		//printf("recv %d bytes!!\n",ret);
		//print_buffer(recv_buf,ret);
		//printf("\n");
		if(pkt->head!=pkt_head)
			continue;	//����Э���ͷ
		dw=(unsigned long*)&pkt->version;
		xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
		if(xor32!=pkt->xor32)
		{
			//printf("xor32 0x%x != 0x%x\n",xor32,pkt->xor32);
			continue;	//��ͷУ�鲻��
		}
		//// lsk 2009-2-9
		if((memcmp(pkt->source_id,selfid,GUID_LEN)==0)||(memcmp(pkt->source_id,selfid1,GUID_LEN)==0))
			continue;	//������Լ����͵İ��򲻴���2009 -2-9 lsk 
		if((memcmp(pkt->target_id,broadcast_addr,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid1,GUID_LEN)!=0))
		{
			continue;	//���������ĵ�ַ
		}
		else	//// lsk 2009 -2-9 
		{
			memcpy(recv_id,pkt->target_id,GUID_LEN);
		}
		pkt_crc32=(unsigned long*)((char*)pkt->data+pkt->pkt_len);
		crc32=tab_crc32(pkt->data,pkt->pkt_len);
		if(crc32!=*pkt_crc32)
		{
		//	printf("crc32 0x%x != 0x %x\n",crc32,*pkt_crc32);
			continue;	//crc�����
		}
		*enc_type=pkt->enc_type;
		memcpy(sourceid,pkt->source_id,GUID_LEN);
		if(pkt->enc_type==GTDEV_ENC_NONE)
		{
			if(buflen<pkt->pkt_len)
				return -ENOMEM;
			memcpy(msgbuf,pkt->data,pkt->pkt_len);
			return pkt->pkt_len;
		}
		else
		{
			printf("receive unknow enc_type:%d pkt",enc_type);
			continue;	//����ʶ�ļ��ܸ�ʽ
		}
	}
	return 0;
}

