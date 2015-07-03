/*
	lsk 2010-2-11 GPS����֧��
	
*/
#include "ipmain.h"
#include <commonlib.h>
#include "watch_process.h"
#include "netcmdproc.h"
#include "ipmain_para.h"
#include "alarm_process.h"
#include <sys/types.h>
#include "netinfo.h"
#include "gate_connect.h"
#include "maincmdproc.h"
#include "watch_board.h"
#include "devstat.h"
#include "infodump.h"
#include "hdmodapi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <gate_cmd.h>
//#include <gpsapi.h>
#include <iniparser.h>
#include <csvparser.h>
#include "gpsupport.h"
//#ifdef  GPS_SUPPORT


/*
	int gps_state; 
*	      0:    ������λ
*	    -1:     ͨѶ�˿�δ��ʼ��
*         -2:     δ��λ
*         -3:     ��gpsģ��ͨѶʧ�� ��û���յ��κ�gps���ݣ�
*	    -4:	��GPS��ͨѶЭ�鲻ƥ�䣨����gps����ʧ�ܻ����ʲ�ƥ�䣩
*/

struct GPS_contrl
{
	int gps_fd;
	int enable;
	int port;
	int baud;
	int parity;
	int stop;
	int datebit;
	int gps_state; 
	pthread_t thread_id_send;
	pthread_t thread_id_recv;
	pthread_mutex_t mutex_gps ;                        //����һ���̻߳�����
	struct usr_req_position_struct sendinfo;
};

static int gps_fd=-1;
static struct GPS_contrl gps_para;
static struct dev_position_return_struct pos_info;

void clear_position_info(void)
{
	memset(&pos_info,0,sizeof(struct dev_position_return_struct));
	pos_info.state=1;
}

void clear_gps_para(void)
{
	memset(&gps_para,0,sizeof(struct GPS_contrl));
	pthread_mutex_init(&gps_para.mutex_gps,NULL);
	gps_para.thread_id_recv=-1;
	gps_para.thread_id_send=-1;
	gps_para.gps_fd=-1;
}
/*****************************************************************************************************
* ������		 :gps_close_module()
* ����               :�رմ��ں��߳�
* ����ֵ :  
                   0:  �رմ��ںʹ����̳߳ɹ�
                 -1:  �ر�ʧ��
  ***********************************************************************************************************/ 
int gps_close_module(void)
{
	int ret;
	if(gps_fd < 0)
	{
		return 0;
	}
//	clear_position_info();
	ret = close(gps_fd);
	gps_fd = -1;
	return ret;
}

void swap_word(int len, void* dValue)
{
	int i;
	unsigned char temp;
	for(i=0;i<len/2;i++)
	{
		temp=((unsigned char*)dValue)[i];
		((unsigned char*)dValue)[i]= ((unsigned char*)dValue)[len/2+i];
		((unsigned char*)dValue)[len/2+i]=temp;
	}
}

//// gps ģ�����ݷ���1��һ��
/** 
 *   @brief  �����豸��gpsλ����Ϣ��ָ��������������
 *   @param  fd Ŀ��������
 *   @param  info �豸��λ����Ϣ
 *   @return   ��ֵ��ʾ����,�Ǹ���ʾ���ͳɹ�
 */ 

int gt_send_position_info(int fd,struct dev_position_return_struct *info,int ack, int env, int enc,int dev_no)
{
//	int rc;
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
//	struct sockaddr_in peeraddr;
//	int addrlen=sizeof(struct sockaddr);	
	DWORD send_buf[50];			
//	char buf[50];
	struct dev_position_return_struct gpsinfo;
//	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_POSITION_RETURN;
	cmd->en_ack=ack;
	cmd->reserve0=0;
	cmd->reserve1=0;
#ifdef GPS_DEBUG
//// lsk 2010-8-11 test gps 
	printf(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
//	printf("lon= 0x%016x   lat = 0x%016x alt = 0x%016x\n", info->lon, info->lat, info->altitude);
//	gtloginfo(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
//	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
//	if((info->reserved[0])>=4)
//	info->state=2;
#endif
//// lsk 2010-8-11 test gps  ֻ�� ARM ��double ������Ҫswap
	memcpy(&gpsinfo,info,sizeof(struct dev_position_return_struct));
	swap_word(sizeof(double),&gpsinfo.altitude);
	swap_word(sizeof(double),&gpsinfo.lat);
	swap_word(sizeof(double),&gpsinfo.lon);
	swap_word(sizeof(double),&gpsinfo.speed);
	swap_word(sizeof(double),&gpsinfo.direction);
	
//	memcpy(&cmd->para,info,sizeof(struct dev_position_return_struct));
	memcpy(&cmd->para,&gpsinfo,sizeof(struct dev_position_return_struct));
	virdev_get_devid(dev_no,cmd->para);
	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+sizeof(struct dev_position_return_struct);
	return send_gate_pkt(fd,modcom,cmd->len+2,dev_no);	
}
/**************************************************************************************************
 * ������: gps_get_position_info
 * ����:  ����gps��״̬�Ͷ�λ��Ϣ
  * ����ֵ:
 *           0 :    ������λ
 *	     -1:     ͨѶ�˿�δ��ʼ��
 *         -2:     δ��λ
 *         -3:     ��gpsģ��ͨѶʧ�� ��û���յ��κ�gps���ݣ�
	     -4����GPS��ͨѶЭ�鲻ƥ�䣨����gps����ʧ�ܻ����ʲ�ƥ�䣩
 *        ����ֵ����
 *
 *         pos:���ص��Ѿ����õ����ݻ�����
 * ����:
 *         pos:  ָ��Ҫ��Ž������ݵĻ�����(4�ֽڶ���)
 * ˵��:   
 * 	   γ�ȣ���γΪ��ֵ����γΪ��ֵ
 *	  ���ȣ�����Ϊ��ֵ������Ϊ��ֵ 
 *	   �ٶȣ��Ǿ���ֵ
 ******************************************************************************************************/
int gps_get_position_info(struct dev_position_return_struct* pos)
{
	int ret;
#ifdef GPS_DEBUG
	struct dev_position_return_struct* info=&pos_info;
	printf(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
#endif
	pthread_mutex_lock(&gps_para.mutex_gps);
	memcpy(pos, &pos_info, sizeof(struct dev_position_return_struct));  // �ڴ渴��
	ret = gps_para.gps_state;
	pthread_mutex_unlock(&gps_para.mutex_gps);
	if(pos->reserved[0]<4)
	{
		pos->state=2;
		ret= -2;
	}
	else
	{
		pos->state=0;
		ret= 0;
	}
    return ret ;
}

int dev_position_info_return(void)
{
	int ret;
	struct dev_position_return_struct gpsinfo;
	
	memset(&gpsinfo,0,sizeof(gpsinfo));
	ret=gps_get_position_info(&gpsinfo);
#if 1
	switch(ret)
	{
		case 0:
//		printf("GPS module OK\n");	
		break;
		case -1:
		printf("GPS module ͨѶ�˿�δ��ʼ��\n");	
		gtlogerr("GPS module ͨѶ�˿�δ��ʼ��\n");	
		break;
		case -2:
		printf("GPS module δ��λ\n");	
		gtlogerr("GPS module δ��λ\n");	
		break;
		case -3:
		printf("��GPS module ͨѶʧ�� \n");	
		gtlogerr("��GPS module ͨѶʧ�� \n");	
		break;
		case -4:
		printf("GPS module ��ͨѶЭ�鲻ƥ��\n");	
		gtlogerr("GPS module ��ͨѶЭ�鲻ƥ��\n");	
		break;
		default:
		gtlogerr("gps_get_position_info return err %d \n",ret);
		break;
	};
#endif	
	gt_send_position_info(1,&gpsinfo,0,0,0,0);
	return ret;
}


static void gps_stop_send(void)
{
	pthread_mutex_lock(&gps_para.mutex_gps);
	clear_position_info();
	gps_para.sendinfo.enable=0;
	gps_para.thread_id_send=-1;
	pthread_mutex_unlock(&gps_para.mutex_gps);
}
static void gps_clean_up(void)
{
	gps_stop_send();
	gps_close_module();
}

/**************************************************************************************************
������:parse_gps_gpgga
����      : ����gps_gpggaһ�е�����
����ֵ:
                   0  : ��λ
                   -2: δ��λ
 ����:
               CSV_T* csv :ָ��һ�����ݵ��׵�ַ

**************************************************************************************************/
int parse_gps_gpgga(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	int number_ver = 0 ;                 //  ���Ǹ���
	int csv_get_position = 0;            //��λ��Ϣ   ����� "$GPGGA"
	const char *sptr=NULL;
	const char * csv_get_NS = NULL ;     //�����γ���Ǳ�γֵ
	const char * csv_get_EW  = NULL;     //��ö�����������ֵ
	double lat = 0;                         //���ά��ֵ����������
	double lon = 0;                         //��þ���ֵ����������
	
	
	csv_get_position = csv_get_int(csv,GPS_GPGGA_POSITION_FIX_INDICTOR,0);  //��0��Ϊ��λ
	if(csv_get_position == 0) 
	{
	   pthread_mutex_lock(&gpsctrl->mutex_gps);
	   clear_position_info();
	   pthread_mutex_unlock(&gpsctrl->mutex_gps);
	   printf("GGA not vailid\n");
	   return -2;
	}
	else
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		gpsctrl->gps_state = 0;           //״̬0 ����λ
	 	clear_position_info();
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	sptr = csv_get_str(csv,GPS_GPGGA_ALTITUDE,DEF_VAL_CHAR);
	if(sptr!=NULL)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.altitude= atof(sptr);  //���θ߶�
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else
	return -1;

	number_ver = csv_get_int(csv,GPS_GPGGA_SATELLITES_USED,DEF_VAL_INT);   //���Ǹ���
	pthread_mutex_lock(&gpsctrl->mutex_gps);
	pos_info.reserved[0] = number_ver;	////������
 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	
	csv_get_NS =  csv_get_str(csv,GPS_GPGGA_NORTH_SOUTH,DEF_VAL_CHAR);//ֱ�ӻ�õ�ά�ȵ�λ��:�ȶȷַ֣��ַַַ�		   
       sptr = csv_get_str(csv,GPS_GPGGA_LATITUDE ,DEF_VAL_CHAR);
	if(sptr!=NULL)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.lat = (atof(sptr))*0.01;  //Ҫ���ַ���ת����double
		lat = pos_info.lat;
		pos_info.lat = (int)lat + (lat - (int)lat)*100/60;   //ת�����ά�ȵ�λΪ:�ȶȡ��ȶȶȶ�
		if(strncmp(csv_get_NS, "S",strlen("S")) == 0)   //γ�ȣ���γΪ��ֵ����γΪ��ֵ
		{
		   pos_info.lat = -pos_info.lat; 
		}
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	} 
	else
	return -1;

	csv_get_EW  = csv_get_str(csv,GPS_GPGGA_EAST_WEST,DEF_VAL_CHAR); //ֱ�ӻ�õľ��� ��λ��:�ȶȶȡ��ȶȶȶ�
	sptr=csv_get_str(csv,GPS_GPGGA_LONGITUDE,DEF_VAL_CHAR);
	if(sptr)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.lon = (atof(sptr))*0.01; //Ҫ���ַ���ת����double
		lon = pos_info.lon;
		pos_info.lon = (int)lon + (lon-(int)lon)*100/60;
		if(strncmp(csv_get_EW,"W",strlen("W")) == 0)    // ���ȣ�����Ϊ��ֵ������Ϊ��ֵ 
		{
		   pos_info.lon = -pos_info.lon; 
		}
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else
	return -1 ;
//	printf("all GGA data parsed\n");
	return 0;
}
/**************************************************************************************************
������:parse_gps_gprmc
����      : ����gps_gprmcһ�е�����
����ֵ:
                   0  : ��λ
                   -2: δ��λ
 ����:
               char* line_buf :ָ��һ�����ݵ��׵�ַ

**************************************************************************************************/
int parse_gps_gprmc(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	const char *csv_get_position = NULL;      //��λ��Ϣ����� "$GPRMC"
	const char * sptr=NULL;
	
	csv_get_position = csv_get_str(csv,GPS_GPRMC_STATUS,DEF_VAL_CHAR);  //��0��Ϊ��λ
	if(strncmp(csv_get_position,"V",strlen("V")) == 0) 
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		clear_position_info();
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
		return -2;
	}

	if(strncmp(csv_get_position,"A",strlen("A")) == 0) 
	{
		sptr=csv_get_str(csv,GPS_GPRMC_POSITION_SPEED_KNOT,DEF_VAL_CHAR);
		if(sptr)
		{
		 	pthread_mutex_lock(&gpsctrl->mutex_gps);
			pos_info.speed	= (atof(sptr))*1.852;   //��λ��knot(��)�����km/h		   
			if(pos_info.speed < 0)   // �ٶȣ��Ǿ���ֵ
				pos_info.speed = -pos_info.speed;
		 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
		}
		else
		return -1;
		sptr=csv_get_str(csv,GPS_GPRMC_DIRECTION,DEF_VAL_CHAR);
		if(sptr)
		{
			pthread_mutex_lock(&gpsctrl->mutex_gps);
			pos_info.direction	= atof(sptr); 
			pthread_mutex_unlock(&gpsctrl->mutex_gps);
		}
		else
		return -1;
	}   
	return 0;
}
/**************************************************************************************************
������:parse_gps_gpvtg
����      : ����gps_gpvtgһ�е�����
����ֵ:
                   0  : ��λ
 ����:

**************************************************************************************************/
int parse_gps_gpvtg(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	const char* sptr=NULL;
	
	sptr = csv_get_str(csv, GPS_GPVTG_SPEED_KM_HOUR, DEF_VAL_CHAR);    //��λ��knot (��)�����km/h	
	if(sptr)
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.speed = atof(sptr);
		if(pos_info.speed < 0)   // �ٶȣ��Ǿ���ֵ
		pos_info.speed = -pos_info.speed;
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else 
	return -1;
	sptr=csv_get_str(csv,GPS_GPVTG_COURSE,DEF_VAL_CHAR);
	if(sptr)
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.direction  = atof(sptr); 
		pos_info.state = 0;   //ֻҪ�ǻ��GPVTG�������ݣ���˵���Ѿ���λ
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else 
	return -1;
	return 0;
}

int parse_gps_message(char* info)
{
	int ret=0;
  int alld;
	CSV_T* csv = NULL;
	const char* csv_get_first_str = NULL;               //���ڻ��һ�����ݶ���ǰ�ĵ�һ���ַ���
	
	csv = csv_create();
	if(csv==NULL)
	{
		gtlogerr("error parse GPS message can not create csv\n");
		return -1;
	}
	ret=csv_parse_string(csv,info);
	if(ret)
	{
		csv_get_first_str = csv_get_error_str((IN int)ret);
		gtlogerr("parse gps message error %s \n", csv_get_first_str);
		csv_destroy(csv);
		return -1;
	}
	//////// �ֱ���� GPGGA GPRMC GPVTG
//	printf("%s",info);
	csv_get_first_str = csv_get_str(csv,1,NULL); 
	  /////// $GPGGA
	if(strncmp(csv_get_first_str,"$GPGGA",strlen("$GPGGA")) == 0)
	ret=parse_gps_gpgga(&gps_para,csv);
	  /////// $GPRMC
	else if(strncmp(csv_get_first_str,"$GPRMC",strlen("$GPRMC")) == 0)
	ret=parse_gps_gprmc(&gps_para,csv);
	
	  /////// $GPVTG
	else if(strncmp(csv_get_first_str,"$GPVTG",strlen("$GPVTG")) == 0)	
	ret=parse_gps_gpvtg(&gps_para,csv);
	csv_destroy(csv);
	return ret;
}

static char gps_buf[2048];
void GPS_recv_thread(struct GPS_contrl* para)
{
	int i=0;
	int err_read_cnt=0;
	int ret=0;
	struct GPS_contrl *gps_info=para;
	if(para==NULL)
	{
		gtlogerr("gps recv proc input para =NULL\n");
		pthread_exit(NULL);
	}
	if(gps_fd<0)
	{
		gtlogerr("gps recv proc gps_fd = %d\n",gps_fd);
              pthread_mutex_lock(&gps_info->mutex_gps);
		gps_info->gps_state=-1;
              pthread_mutex_unlock(&gps_info->mutex_gps);
		pthread_exit(NULL);
	}
		
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);		//�����߳�Ϊ��ȡ����
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);		//ִ�е�������Ž��г���
	gtloginfo("get uer command :gps recv thread start \n");
	while(1)
	{
		pthread_testcancel();
		ret = read(gps_fd, &gps_buf[i++], 1);
		if(ret!=1)
		{
			if(err_read_cnt++>5*1024)
			{
//				err_read_cnt=0;
				gtlogerr("gps read error \n");
		              pthread_mutex_lock(&gps_info->mutex_gps);
				gps_info->gps_state=-3;
		              pthread_mutex_unlock(&gps_info->mutex_gps);
				break;
			}
			continue;
		}
		err_read_cnt=0;
		if(gps_buf[i-1]=='$')		///// start a message
		{
			memset(gps_buf,0,sizeof(gps_buf));
			i=1;
			gps_buf[0]='$';
			continue;
		}
		if((gps_buf[i-1]=='\r')||(gps_buf[i-1]=='\n')) 	//// end of a message
//		if((gps_buf[i-1]=='\r')) 	//// end of a message
		{
			gps_buf[i-1]='\n';
			if(gps_buf[0]=='$')	//// message received completely
			{
//				printf("0x%02x 0x%02x 0x%02x \n",gps_buf[0],gps_buf[1],gps_buf[2]);
				pthread_mutex_lock(&gps_info->mutex_gps);
				pos_info.state=2;
				pthread_mutex_unlock(&gps_info->mutex_gps);
				parse_gps_message(gps_buf);
			}
			i=0;
		}
		if(i>=2047)	////2048���ֽ�û���յ�'$'
		{
			gtlogerr("gps receive 2048 bytes without a vailid message\n");
	              pthread_mutex_lock(&gps_info->mutex_gps);
			pos_info.state=3;
			gps_info->gps_state=-4;
	              pthread_mutex_unlock(&gps_info->mutex_gps);
			i=0;
		}
	}
	////// can not be here 
	gps_close_module();
	gtlogerr("gps receive thread faild\n");
	pthread_exit(NULL);
}

void GPS_send_thread(struct GPS_contrl* para)
{
	int i=0;
	unsigned long j=0;
//	int ret=0;
	if(para==NULL)
	{
		gtlogerr("gps send proc input err value\n");
		pthread_exit(NULL);
	}
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);		//�����߳�Ϊ��ȡ����
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);		//ִ�е�������Ž��г���
	gtloginfo("get uer command :gps send thread start \n");
       gtloginfo("GPS��ʱ��Ϊ[%ld]��\n",para->sendinfo.send_seconds);
	while(1)
	{
		for(i=0;i<para->sendinfo.interval;i++)
		{
			pthread_testcancel();
			sleep(1);
			j++;
		}
		dev_position_info_return();
		if(j>=para->sendinfo.send_seconds)
		{
			j=0;
			break;
		}
	}
	gps_stop_send();
       gtloginfo("��ǰ[%ld]�ѳ���GPS��ʱ�����ֵ[%ld]��GPSֹͣ����λ����Ϣ\n",j,para->sendinfo.send_seconds);
//	gps_clean_up();
	pthread_exit(NULL);
}

static int create_gps_send_thread(void)
{
	return GT_CreateThread(&gps_para.thread_id_send,(void*)&GPS_send_thread, (void*)&gps_para);
}

static int create_gps_recv_thread(void)
{
	return GT_CreateThread(&gps_para.thread_id_recv,(void*)&GPS_recv_thread, (void*)&gps_para);
}
/**********************************************************************************************
 * ������ :gps_init_module()
 * ���� :            ����gps���ڵĹ���ģʽ�����ͳ�ʼ��ָ���gpsģ��
 * ���� :
 *		  port             :����GPSͨѶ�Ĵ��ںţ�0��1��2......��
 *  		  baud            :Ҫ���õĲ����� 4800,9600,19200...                     
 *		  databits        :����λ        7,8
 *             stopbits        :ֹͣλ        1,2
 *              parity          :��żУ��λ'n','o','e','s'
 * ����ֵ       :  0 ��ʾ�ɹ�,��ֵ��ʾʧ��
 * ˵���������ʼ���ɹ����´��ٵ��øú�����ʱ��������������ͬ����ֱ�ӷ���0��
 *	 ��ͬ����Ҫ�������ô��ڣ����ͳ�ʼ�����
 **********************************************************************************************/
int gps_init_module(int port,int baud,int databits,int stopbits,int parity)
{    
	char port_number[100];  //����豸�˿ں�
	int ret ;

	memset(port_number,0,sizeof(port_number));	
	sprintf(port_number, "/dev/ttyS%d", port);
	
	if(gps_fd >= 0)   // �ڳ�ʼ��ʱ���Ѵ򿪵Ĵ����ȹر��ٴ� 
	{
        	ret = gps_close_module();
		if(ret < 0)
		{
			return -1;   
		}
	}	

	gps_fd = open(port_number, O_RDWR|O_NOCTTY);
	
	if(gps_fd  < 0)
	{
		gtlogerr("can not open com port %s\n",port_number);
		return -1;
	}

	ret = set_com_mode(gps_fd, databits, stopbits,parity);
	
	if(ret < 0)
	{
  		gtlogerr("errlor set gps com mode datebits=%d stopbits=%d parity=%c \n", 
			databits,stopbits,parity);
		close(gps_fd);
		gps_fd = -1;
		return -1;
	}

	ret = set_com_speed(gps_fd, baud);  
	
	if(ret < 0)
	{
  		gtlogerr("errlor set gps com speed baud=%d \n",baud);
	      	close(gps_fd);
		gps_fd = -1;
		return -1;
	}
	ret= create_gps_recv_thread();
       return ret;
}

int init_gps_para(char* filename)
{
	dictionary      *ini;	
	char *pstr=NULL;
	int ret=0;
	ini=iniparser_load(filename);
        if (ini==NULL) {
                printf("cannot parse ini file file [%s]", filename);
                return -1 ;
        }
	clear_gps_para();
	clear_position_info();
	gps_para.enable= iniparser_getint(ini,"GPS:enable",0);
	if(gps_para.enable==0)
	{
		iniparser_freedict(ini);
		printf("û�а�װGPSģ��\n");
		gtloginfo("û�а�װGPSģ��\n");
		return -2;
	}
	gps_para.port= iniparser_getint(ini,"GPS:port",0);
	gps_para.baud= iniparser_getint(ini,"GPS:baud",9600);
	gps_para.datebit= iniparser_getint(ini,"GPS:date",8);
	gps_para.stop= iniparser_getint(ini,"GPS:stop",1);
	pstr = iniparser_getstring(ini,"GPS:parity",NULL);
	if(pstr!=NULL)
		gps_para.parity=pstr[0];
	else
		gps_para.parity='N';
	iniparser_freedict(ini);
	printf("��װ��GPSģ��port %d baud %d %c %d %d \n",gps_para.port, gps_para.baud,
		gps_para.parity,gps_para.datebit,gps_para.stop);
	gtloginfo("��װ��GPSģ��port %d baud %d %c %d %d \n",gps_para.port, gps_para.baud,
		gps_para.parity,gps_para.datebit,gps_para.stop);
	ret=gps_init_module(gps_para.port, gps_para.baud, gps_para.datebit, gps_para.stop,gps_para.parity);
	if(ret<0)
	{
		gtlogerr("gps init err %d\n",ret);
//		ret= send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
//		return ret;
	}
	return ret;
}

int process_usr_query_position(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no)
{
	int ret;
	struct usr_req_position_struct *req_cmd;
//	BYTE dev_guid[20];
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);

	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_DEVICE_POSITION)
	return -EINVAL;

	req_cmd=(struct usr_req_position_struct *)cmd->para;

	printf("%s(fd=%d)������ȡ�豸λ����Ϣ����:enable=%d,target=%d,interval=%d,send_seconds=%ld\n",inet_ntoa(peeraddr.sin_addr),fd,req_cmd->enable,req_cmd->target,req_cmd->interval,req_cmd->send_seconds);
	if(gps_para.enable==0)		////û�а�װgpsģ��
	{
		ret= send_gate_ack(fd,cmd->cmd,ERR_EVC_NOT_SUPPORT,env,enc,dev_no);
		return ret;
	}
	if(gps_fd<0)
	{
		gtlogerr("did not initialize gps module\n");
		send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
		init_gps_para(IPMAIN_PARA_FILE);
		return -1;
	}
	
	if(req_cmd->enable==0)		//// ֹͣ����gps����
	{
		if(gps_para.sendinfo.enable==1)
		{
			if(gps_para.thread_id_send>=0)	
			{
				ret=pthread_cancel(gps_para.thread_id_send);
				if(ret>=0)
				{
					gtloginfo("get uer command :gps thread canceled \n");
				}
				else
				{
					gtlogerr("can not cancel gps thread\n");
				}
			}
			sleep(2);
			gps_stop_send();
//			gps_clean_up();
		}
	}
	else							////��ʼ����gps����
	{
		if(gps_para.sendinfo.enable==0)
		{
			memcpy(&gps_para.sendinfo,req_cmd,sizeof(struct usr_req_position_struct));
#if 0
			if(gps_para.thread_id_recv==-1)
			{
				ret=create_gps_recv_thread();
				if(ret>=0)
				{
					gtloginfo("get uer command :gps recv thread start \n");
					gps_para.sendinfo.enable=1;
				}
				else
				{
					gtlogerr("can not create  gps recv thread\n");
					gps_clean_up();
				}
			}
#endif			
			ret=create_gps_send_thread();
			if(ret>=0)
			{
				gtloginfo("get uer command :gps send thread start \n");
				gps_para.sendinfo.enable=1;
			}
			else
			{
				gtlogerr("can not create  gps send thread\n");
				gps_clean_up();
			}
		}
		
	}
	
	if(ret>=0)
	{
		ret= send_gate_ack(fd,cmd->cmd,RESULT_SUCCESS,env,enc,dev_no);
	}
	else
	{
		ret= send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
	}
	return ret;
}

//#endif



