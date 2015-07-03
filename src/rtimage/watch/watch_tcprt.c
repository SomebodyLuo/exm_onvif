/** @file	       watch_tcprt.c
 *   @brief 	�鿴tcprtimageģ��ʵʱ��Ϣ�Ĺ��߳���
 			0.02 ���3022��ͬʱ�ۿ���·guid����״��
 			0.03t�޸�21��24�鿴�û�������
 *   @date 	2007.03
 */
 #include "../rtimage2.h"
#include <devinfo.h>
#include <libterm.h>
#include <signal.h>
#include <commonlib.h>
#include <mshm.h>
#include "../serv_info.h"
#include <calc_bitrate.h>
//#include <mcheck.h>
#include <stdlib.h>

const static char watch_version[]="0.03t";     ///<�汾��


static tcprtimg_svr_t     *p_tcprtimg=NULL;                                             ///<ָ������ṹ��ָ��
static int				virdev_num=-1;								    ///<
static BIT_RATE_T        netav_bitrate[TCPRT_MAX_VIRAVUSR_NO+1];    ///<������������Ƶ�ϴ�������Ϣ�Ľṹ,��Ϊ��һ���û����ڴ���æ��Ϣ����Ҫ+1
static BIT_RATE_T        net_aplay_bitrate[TCPRTIMG_MAX_APLAY_NO+1];
void init_bitrates(void)
{
    BIT_RATE_T   *br=NULL;
    int                  total=sizeof(netav_bitrate)/sizeof(BIT_RATE_T);
    int                  i;
    for(i=0;i<total;i++)
    {
        br=&netav_bitrate[i];
        memset((void*)br,0,sizeof(BIT_RATE_T));
        SetCalcAvgTime(br,3);
    }
}

///��tcprtimageģ��ı����ṹ
///0��ʾ�ɹ���ֵ��ʾʧ��
static int attach_tcprtimage_para(void)
{
    MEM_CORE	*mc=NULL;           ///<��Ϊ֮����Ҫ�ͷš������Ȳ�������ʹ�þֲ������Ϳ�����
    mc=MShmCoreAttach(get_tcprtimg_stat_key(),(void**)&p_tcprtimg);
    if(mc==NULL)
        return -ENOMEM;
    return 0;
}
static void print_av_usr(av_usr_t *usr,BIT_RATE_T *br)
{
    stream_send_info_t *send_info=&usr->send_info;
    socket_attrib_t *sock_attr=&usr->sock_attr;
    WriteTermStr(C_WHITE,0,"%02d ",usr->no);
    WriteTermStr(C_WHITE,0," %02d ",usr->venc_no);
    WriteTermStr(C_WHITE,0," %02d ",usr->serv_stat);
    WriteTermStr(C_WHITE,0," %-4.1f ",br->PeakBitrate/1024);
    WriteTermStr(C_WHITE,0," %-4.1f ",br->AvgBitrate/1024);
    WriteTermStr(C_WHITE,0,"%7d/%-7d ",sock_attr->send_buf_remain,sock_attr->send_buf_len);
    WriteTermStr(C_WHITE,0,"%2d %2d ",send_info->map.v_frames,send_info->map.a_frames);
    WriteTermStr(C_WHITE,0," %s/%s",inet_ntoa(usr->addr.sin_addr),usr->name);
  
    WriteTermStr(C_WHITE,0,"\n");
}
static void dump_av_usr(av_usr_t *usr)
{
	stream_send_info_t *send_info=&usr->send_info;
	 WriteTermStr(C_WHITE,0,"drop_a_flag     %02d\n",send_info->drop_a_flag);
	 WriteTermStr(C_WHITE,0,"drop_a_frames   %02d\n",send_info->drop_a_frames);
	 WriteTermStr(C_WHITE,0,"drop_p_flag     %02d\n",send_info->drop_p_flag);
	 WriteTermStr(C_WHITE,0,"drop_v_flag     %02d\n",send_info->drop_v_flag);
	 WriteTermStr(C_WHITE,0,"drop_v_frames   %02d\n",send_info->drop_v_frames);
	 WriteTermStr(C_WHITE,0,"require_i_flag  %02d\n",send_info->require_i_flag);
	 WriteTermStr(C_WHITE,0,"send_i_flag     %02d\n",send_info->send_i_flag);
	 WriteTermStr(C_WHITE,0,"first_flag      %02d\n",send_info->first_flag);
	 WriteTermStr(C_WHITE,0,"\n");


}
/**
 *   @param no��ӡ�ĸ������豸�ķ�����Ϣ
 */
static void print_server_info(int no)
{
    tcprtimg_svr_t *p=p_tcprtimg;
    av_server_t     *av_svr=&p->av_server;
    aplay_server_t *ap_svr=&p->aplay_server;
	unsigned char guid_1[20];
	unsigned char guid_2[20];
	
	if(virdev_num==2)
	{
		virdev_get_devid(0, guid_1);
		virdev_get_devid(1, guid_2);
	

   		 ///3022������·��Ϣ
    		if(no==0)
    		{
			WriteTermStr(C_WHITE,0,"�豸0(guid=%02x%02x%02x%02x%02x%02x%02x%02x)",guid_1[7],guid_1[6],guid_1[5],guid_1[4],guid_1[3],guid_1[2],guid_1[1],guid_1[0]);
			WriteTermStr(C_WHITE,0,"lan:%d/%d ",av_svr->vir0_lan_users,p->max_lan_usrs/virdev_num);
			WriteTermStr(C_WHITE,0,"wan:%d/%d ",av_svr->vir0_wan_users,p->max_wan_usrs/virdev_num);
			WriteTermStr(C_WHITE,0,"aplay:%d/%d ",ap_svr->usrs,p->max_aplay_usrs);
			WriteTermStr(C_WHITE,0,"aplay_port=%d ",p->audio_play_port);
		}
		else
		{///����Ƶ
			WriteTermStr(C_WHITE,0,"\n");
			WriteTermStr(C_WHITE,0,"�豸1(guid=%02x%02x%02x%02x%02x%02x%02x%02x)",guid_2[7],guid_2[6],guid_2[5],guid_2[4],guid_2[3],guid_2[2],guid_2[1],guid_2[0]);
			WriteTermStr(C_WHITE,0,"lan:%d/%d ",av_svr->vir1_lan_users,p->max_lan_usrs/virdev_num);
			WriteTermStr(C_WHITE,0,"wan:%d/%d ",av_svr->vir1_wan_users,p->max_wan_usrs/virdev_num);
		}
        
	}  
	else
	{
		WriteTermStr(C_WHITE,0,"lan:%d/%d ",av_svr->lan_usrs,p->max_lan_usrs);
   		WriteTermStr(C_WHITE,0,"wan:%d/%d ",av_svr->wan_usrs,p->max_wan_usrs);
    		WriteTermStr(C_WHITE,0,"aplay:%d/%d ",ap_svr->usrs,p->max_aplay_usrs);
    		WriteTermStr(C_WHITE,0,"aplay_port=%d ",p->audio_play_port);
	}
    
   
    
    
    WriteTermStr(C_WHITE,0,"av_port=%d ",p->av_svr_port);
    WriteTermStr(C_WHITE,0,"th_drop_p=%d ",p->th_drop_p);
    WriteTermStr(C_WHITE,0,"\n");

}

/**
 *   @param no��ӡ�ĸ������豸�ķ�����Ϣ
 */
static void print_net_av_info(av_server_t *av_svr, int no)
{
    int                 i;
    int                 total=sizeof(av_svr->av_usr_list)/sizeof(av_usr_t);
    av_usr_t        *usr=NULL;          ///<�û��ṹָ��
    BIT_RATE_T   *br=NULL;           ///<�û������ṹָ��

    WriteTermStr(C_HWHITE,0,"*************************����Ƶ���з���*********************************\n");
#if 0
    WriteTermStr(C_WHITE,0,"No=�û����Enc=��Ƶ��� stat=����״̬ peak=��ֵ���� avg=ƽ������\n");
    WriteTermStr(C_WHITE,0,"used/buffer=���û���/�ܹ����� vfs=��Ƶ֡�� afs=��Ƶ����\n");
#endif
    
    WriteTermStr(C_WHITE,1,"No Enc stat peak    avg     used/buffer vfs afs     ip��ַ/�û���      \n");
    for(i=0;i<total;i++)
    {
        usr=&av_svr->av_usr_list[i];
        if(usr->serv_stat>0)
        {
            br=&netav_bitrate[i];
            CalcBitRate(br,usr->send_info.total_out-br->LastCheckBytes);	 //���㷢������
	     br->LastCheckBytes=usr->send_info.total_out;				        //�����ֽ���Ϣ
	    ///3022
	    if(virdev_num==2)
	    {
	     	   ///�����豸0��Ϣ
		   if((usr->venc_no==0)&&(no==0))
		   {
		       print_av_usr(usr,br);
		   }
		   ///�����豸1��Ϣ
		   else if((usr->venc_no==1)&&(no==1))
		   {
			print_av_usr(usr,br);
		   }
	    }
	    ///3021��24ϵ��
	    else
	    {
	          print_av_usr(usr,br);
	     }

	     
        }
        else
        {///�û���Ч
            
        }
    }
    //zsk debug
    #if 0
    WriteTermStr(C_HWHITE,1,"*********************zsk dump��Ϣ*********************************\n");
    for(i=0;i<total;i++)
        {
            usr=&av_svr->av_usr_list[i];
            if(usr->serv_stat>0)
            	{
			        //�����ֽ���Ϣ
    	          dump_av_usr(usr);
            	}
        }
	#endif
}
static void print_aplay_usr(aplay_usr_t *usr,BIT_RATE_T *br)
{
//    stream_recv_info_t *recv_info=&usr->recv_info;
//    socket_attrib_t *sock_attr=&usr->sock_attr;
    WriteTermStr(C_WHITE,0,"%02d ",usr->no);
    WriteTermStr(C_WHITE,0," %02d ",usr->serv_stat);
    WriteTermStr(C_WHITE,0," %-4.1f ",br->PeakBitrate/1024);
    WriteTermStr(C_WHITE,0," %-4.1f ",br->AvgBitrate/1024);
    WriteTermStr(C_WHITE,0," %6d ",usr->sock_attr.recv_buf_remain);
    WriteTermStr(C_WHITE,0," %6d ",usr->recv_info.play_buf_used);
    WriteTermStr(C_WHITE,0," %s/%s",inet_ntoa(usr->addr.sin_addr),usr->name);
    WriteTermStr(C_WHITE,0,"\n");
}
static void print_net_aplay_info(aplay_server_t *ap_svr)
{
    int                 i;
    int                 total=sizeof(ap_svr->usr_list)/sizeof(aplay_usr_t);
    aplay_usr_t    *usr=NULL;          ///<�û��ṹָ��
    BIT_RATE_T   *br=NULL;           ///<�û������ṹָ��

    WriteTermStr(C_HWHITE,0,"*************************��Ƶ���з���***********************************\n");
#if 0
    WriteTermStr(C_WHITE,0,"No=�û����stat=����״̬ peak=��ֵ���� avg=ƽ������buffered=���������ֽ���\n");
#endif    
    WriteTermStr(C_WHITE,1,"No stat peak   avg sock_buf play_buf    ip��ַ/�û���                  \n");
    for(i=0;i<total;i++)
    {
        usr=&ap_svr->usr_list[i];
        if(usr->serv_stat>0)
        {
            br=&net_aplay_bitrate[i];
            CalcBitRate(br,usr->recv_info.total_recv-br->LastCheckBytes);	 //���㷢������
	     br->LastCheckBytes=usr->recv_info.total_recv;				        //�����ֽ���Ϣ

            print_aplay_usr(usr,br);
        }
        else
        {///�û���Ч
            
        }
    }   
}
/**
 *   @param no��ӡ�ĸ������豸������Ƶ��Ϣ
 */
static void show_net_av_info(int no)
{
    tcprtimg_svr_t *p=p_tcprtimg;
    av_server_t     *av_svr=&p->av_server;

	print_net_av_info(av_svr,no);
	///3022�ڶ�·û��������Ƶ
	if(no!=1)
	{
		
    		print_net_aplay_info(&p->aplay_server);
	}
	
		
}

static void refresh_screen(void)
{
    WriteTermStr(C_HWHITE,0,"\t\t%s ��������Ƶ������Ϣ(��Ӧrtimage�汾:%s)\n\n",get_devtype_str(),version);
    print_server_info(0);
    show_net_av_info(0);
	///3022ͬʱ��ӡ��·guid������Ƶ��Ϣ
	if(virdev_num==2)
	{
		print_server_info(1);
  		show_net_av_info(1);
	}
}
///����ctrl-c�ĺ���
static void process_ctrl_c(int signo)
{       
        if(signo!=SIGINT)
                return;
        WriteTermStr(C_HWHITE,0,"�˳��鿴����!!\n");
}       
int main(int argc,char *argv[])
{
		//setenv("MALLOC_TRACE", "/mnt/zsk/mtrace.log", 1);
		//mtrace();
        struct timeval    tv;                      ///<select��ʱ�����
        int                     ret;
        int                     sel=0;                 ///<select�ķ���ֵ          
        char                  c;                        ///<���ն˶�ȡ���ַ�����     
        fd_set                readfds;            ///<select�õĶ�������������
        int                     interval=1;        ///<Ĭ��ˢ�¼��
        init_devinfo();

	virdev_num=virdev_get_virdev_number();
	if(virdev_num<0)
	{
		printf("��ȡ�豸����ʧ��\n");
		exit(1);
	}


#if 0
	//�����µ�virdev���õ�
	sel=virdev_get_virdev_number();
	printf("virdev_number=%d\n",virdev_get_virdev_number());
	for(interval=0;interval<sel;interval++)
	{
		printf("-----------------------------------------\n");
		printf("guid=%s\n",virdev_get_devid_str(interval));
		printf("total com=%d\n",virdev_get_total_com(interval));
		printf("video num=%d\n",virdev_get_video_num(interval));
		printf("videoenc num=%d\n",virdev_get_videoenc_num(interval));
		printf("trigin num=%d\n",virdev_get_trigin_num(interval));
		printf("alarm out num=%d\n",virdev_get_alarmout_num(interval));
		printf("audio num=%d\n",virdev_get_audio_num(interval));
		
	}	

	exit(1);
#endif
        ///�ж��Ƿ����Ѿ�������tcprtimage����

	ret=create_and_lockfile(RT_LOCK_FILE);	//�����ļ�
	if(ret>=0)
	{
		printf("Ŀǰû���������е�rtimage�������!\n");
		exit(1);
	}
       else
       {
            close(ret);
       }
       init_bitrates();
       ret=attach_tcprtimage_para();
       if(ret<0)
        {
            printf("���ܴ�rtimage����Ĳ�����Ϣ!\n");
            exit(1);
        }
       
        tv.tv_sec = interval;
        tv.tv_usec = 0;
        FD_ZERO (&readfds);
        signal(SIGINT,process_ctrl_c);
        while(sel>=0)
        {
        
                ClearTermScr();                 //����
                refresh_screen();            
                

                tv.tv_sec = interval;
                tv.tv_usec = 0;
                FD_SET (0, &readfds);
                sel=select (1, &readfds, NULL, NULL, &tv);
                if(sel==0)
                {
                        continue;
                }
                else if(sel>0)
                {
                        if (FD_ISSET (0, &readfds))
                        {
                                if (read (0, &c, 1) <= 0)
                                {
                                        printf("read <=0\n");
                                        exit(1);
                                }
                        }
                }


        }

        exit(0);
}



