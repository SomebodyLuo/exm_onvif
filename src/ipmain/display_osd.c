#include "osd_api.h"
#include "display_osd.h"
#include "time.h"
#include "ipmain.h"
#include "devinfo.h"
#include "ipmain_para.h"
#include "netcmdproc.h"
#include "commonlib.h"
#include "iniparser.h"


int write_scr_time(void)
{
	struct tm *ptime;
	//int posx,posy,color,len;
	//unsigned char pstr[100];
	int color = 0;
	time_t ctime;
	ctime=time(NULL);
	ptime=localtime(&ctime);	
	if(ptime==NULL)
		return -1;
#if 0	
	/*to be fixed below....wsy*/
	posx=25;//shixintest  40;
	posy=0; 
	color=0;
	len=0;
	sprintf(pstr,"%d-%02d-%02d %02d:%02d:%02d",
			ptime->tm_year+1900,	//��
			ptime->tm_mon+1,	//��
	 		ptime->tm_mday, 		//��
			ptime->tm_hour,		//ʱ
			ptime->tm_min,		//��
			ptime->tm_sec);		//��
	osd_write(OSD_NET_CH, posx,posy,pstr,len,color);
	osd_write(OSD_LOCAL_CH,posx,posy,pstr,len,color);

#endif
	osd_display_time(OSD_NET_CH,ptime,color);
 	osd_display_time(OSD_LOCAL_CH,ptime,color);
	return 0;
}



//���ֵĳ��Ⱥ���ĸ���ȶ���1
size_t hzstrlen( char *s)
{
        int len=0;
        char *p=s;
        if(s==NULL)
                return 0;
        while(*p!='\0')
        {
                len++;
                if(*p>0x7f)
                        p++;
                p++;
        }
        return len;
}

int get_full_cameraname_posx(int vidpicsize)
{
	if((get_devtype()==T_GTVS3024)&&(vidpicsize != 0)) //3024,��sizeΪcif/hd1
		return FULL_CAMERANAME_POSX-1;
	else
		return FULL_CAMERANAME_POSX;
}


//screen_ch : 0,1,2,3�ֱ�����ķָ���ĸ�λ��
int get_cameraname_display_place(IN int screen_ch, IN char* pstr, IN int vidpicsize, OUT int * posx, OUT int * posy)
{
	if((posx == NULL)||(posy == NULL)||(pstr == NULL))
		return -EINVAL;
		
	if(get_quad_flag()) //���ķָ���豸	
	{
		switch(screen_ch)
		{
			case(0):	*posx =	QUAD0_CAMERANAME_POSX-strlen(pstr);
						*posy = QUAD0_CAMERANAME_POSY;
						return 0;
			case(1):	*posx =	get_full_cameraname_posx(vidpicsize)-strlen(pstr);
						*posy = QUAD0_CAMERANAME_POSY;
						return 0;
			case(2):	*posx =	QUAD0_CAMERANAME_POSX-strlen(pstr);
						*posy = FULL_CAMERANAME_POSY;
						return 0;
			case(3):	*posx =	get_full_cameraname_posx(vidpicsize)-strlen(pstr);
						*posy = FULL_CAMERANAME_POSY;
						return 0;
			default:	return -EINVAL;
		}
	}
	else	//��·�豸
	{
		*posx = NOQUAD_CAMERANAME_POSX - strlen(pstr);
		*posy = NOQUAD_CAMERANAME_POSY;
		return 0;
	}
}


//osd_ch: OSD_NET_CH��OSD_LOCAL_CH
//video_ch: 0-3��ʾȫ����4��ʾ�ķָ�
//vidpicsize,��ʾ�ߴ磬0-D1,1-CIF,2-HD1
int write_cameraname_to_channel(IN int osd_ch, IN  int video_ch, IN int vidpicsize)
{
	int posx,posy;
	char *pstr;
	int ret;
	int i;
	int color = 0;
	
	
	if((osd_ch!=OSD_NET_CH)&&(osd_ch!=OSD_LOCAL_CH))
		return -EINVAL;
		
	if(video_ch >= (get_video_num() + get_quad_flag()))
		return -EINVAL;
			
	
	if(video_ch == 4)//�ķָ�
	{
		for(i=0;i<4;i++)
		{
			pstr = get_mainpara()->vadc.enc_front[i].name;
			get_cameraname_display_place(i,pstr,vidpicsize,&posx, &posy);
			ret = osd_write(osd_ch, posx, posy, pstr,0, color,1);
		}
	}
	else //ȫ��
	{
		pstr = get_mainpara()->vadc.enc_front[video_ch].name;
		get_cameraname_display_place(3,pstr,vidpicsize,&posx, &posy);
		ret = osd_write(osd_ch, posx, posy, pstr,0, color,1);
	}

	return ret;
}

/*************************************************
  ����Ļ����Ӧ��������ͷ������osd
  ����: osd_ch,ȡֵΪOSD_LOCAL_CH��OSD_NET_CH
  		video_ch,ͨ���ţ�0-3��ʾ��Ӧ����ͷȫ����4��ʾ�ķָ�	
  		vidpicsize,��ʾ�ߴ磬0-D1,1-CIF,2-HD1
  ע��:ȫ��ʱ�����ڴ���Ļ���½ǣ��ķָ�ʱ������ÿ��С��������½�
*************************************************/
int display_camera_name(int osd_ch, int video_ch,int vidpicsize)
{
	if((video_ch>4)||(video_ch <0))
		return -EINVAL;
	
	return write_cameraname_to_channel(osd_ch,video_ch,vidpicsize);
		
}


/*************************************************
	//wsyadd
	����Ļ�ϵ��Ӱ�װ�ص��ַ���osd
	����: 	osd_ch,ȡֵΪOSD_LOCAL_CH��OSD_NET_CH
		    posx,��������x��ƫ����,ȡֵ0-44; 
			posy,��������y��ƫ������ȡֵ0-17
*************************************************/
int display_install_place(int osd_ch, int posx, int posy)
{
	char inst_place[100];
	int color=0;
	int ret;
	if((virdev_get_virdev_number()==2)&&(osd_ch == OSD_LOCAL_CH))
		get_dev_sitename(1,inst_place);
	else
		get_dev_sitename(0,inst_place);
	ret = osd_write(osd_ch,posx,posy,inst_place,0,color,0);
	return ret;
}

//��install:osd_max_len�������õ�ip1004.ini�ļ�
int osd_set_maxlen_to_file(void)
{
	dictionary *ini = NULL;
	FILE *fp = NULL;
	int maxlen = 0;
	char entry[30];
	
	maxlen = osd_get_max_len();
	if(maxlen < 0)
	{
		gtloginfo("ȡosd��װ�ص�����д�ַ���ʧ�ܷ���%d\n",maxlen);
		return -1;
	}
	
	ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
		return -EINVAL;	
	}
	else
	{
		sprintf(entry,"install:osd_max_len");
		iniparser_setint(ini,entry,maxlen);		
		gtloginfo("osd��װ�ص�����д�ַ���Ϊ%d,д�������ļ�\n",maxlen);
		save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
		iniparser_freedict(ini);
		return 0;
	}

}


//����Ӧͨ��(net/local)дosd��Ϣ,�����ж��Ƿ���Ҫд���Լ����ݺ�����
//ֻ����clear_scr��camera_name
int write_osd_info(int osd_ch, int vidpicsize)
{

	struct ipmain_para_struct *mainpara;
	int video_ch = 0;
	int ret;
	if(get_osd_flag() == 1)
	{	
		mainpara=get_mainpara();
		osd_clear_scr(osd_ch);
		
		if(get_quad_flag()==1)//����Ƶ�ָ�������ʾ��ͷ����
		{
			if(osd_ch == OSD_NET_CH)
				video_ch = mainpara->vadc.quad.current_net_ch;
			//if(osd_ch == OSD_LOCAL_CH)
			//	video_ch = mainpara->vadc.quad.current_local_ch;	
			ret = display_camera_name(osd_ch,video_ch,vidpicsize);
		}
		
		return ret;
	}
	else
		return -EINVAL;

	return 0;
}

static int osd_display_cnt = 5; //��������ÿ10����ʾһ�ΰ�װ�ص�


static void * osd_proc_thread(void * para)
{
	
	gtloginfo("start osd_proc_thread...\n");
	printf("start osd_proc_thread...\n");
	
	write_osd_info(OSD_NET_CH,get_mainpara()->net_ch_osd_picsize);
	//write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);
	/*
	if(virdev_get_virdev_number()==2) //�����豸
	{
		if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
		{
			display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
		}
		if(get_mainpara()->devpara[1].osd.inst_place_display == 1)
		{
			display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[1].osd.inst_place_posx,get_mainpara()->devpara[1].osd.inst_place_posy);
		}
	}
	else
	*/
	{
		if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
		{
			display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);		
			display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
		}	
	}
	
	while(1)
	{
		//if(get_quad_flag()==0)//3021,22,��Ҫ��ͣ��д
		{
			if((osd_display_cnt <=30)&&(((++osd_display_cnt) % 10) == 0))
			{	

//				osd_display_cnt = 0;
				write_osd_info(OSD_NET_CH,get_mainpara()->net_ch_osd_picsize);
				//write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);
				if(virdev_get_virdev_number()==2) //�����豸
				{
					if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
					{
						display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
					}
					if(get_mainpara()->devpara[1].osd.inst_place_display == 1)
					{
						display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[1].osd.inst_place_posx,get_mainpara()->devpara[1].osd.inst_place_posy);
					}
				}
				else
				{
					if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
					{
						display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
						display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
					}	
				}
			}
		}	
		
		
		write_scr_time();
		
		//sleep(1);
		usleep(1000*100);
	}
	return NULL;
}	

pthread_t osd_thread_id = -1;

int creat_osd_proc_thread(pthread_attr_t *attr,void *arg)
{
	return pthread_create(&osd_thread_id,attr, osd_proc_thread, NULL);
}



void osd_second_proc(void)
{
	return;
}

