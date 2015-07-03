#include "ipmain.h"
#include "hdmodapi.h"
#include "maincmdproc.h"
#include "gate_cmd.h"
#include "mod_com.h"
#include "mod_cmd.h"


/*****************************************************************************
 *������: alarm_snapshot()
 *����:   ��hdmoduleģ����б���ץͼ
 *����:   takepic,Ϊtakepic_struct�ṹ,����ʱ���,�����������ͨ��
 *����ֵ: 0��ʾ�ɹ�����ֵ��ʾʧ��
 ****************************************************************************/
int alarm_snapshot(struct takepic_struct *takepic,WORD env,WORD enc)
{
	DWORD send_buf[200];
	mod_socket_cmd_type *send;
	struct takepic_struct *pic;
	
	if(takepic==NULL)
		return -1;

	//gtloginfo("test,alarm_snapshot��,%d interval , %d number, %d channel\n",takepic->interval,takepic->takepic,takepic->channel);
	
	send = (mod_socket_cmd_type *)send_buf;
	send->cmd=ALARM_SNAPSHOT;
  //zw-add -2012-05-15
  send->gate.dev_no=0;
  send->gate.env=env;
  send->gate.enc=enc;
  //zw-add 2012-05-15

	pic=(struct takepic_struct*)send->para;
	send->len = sizeof(struct takepic_struct);
 	memcpy(pic,takepic,sizeof(struct takepic_struct));
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);

}


/*
 *	������¼�����
 * 	ch:Ҫ���Ƶ�ѹ��ͨ����
 * 	mode:	0: ��ʾֹͣ¼���߳�
			1: ��ʾ����¼���߳�
			2: ��ʾ��������¼���߳�
*/
static int hdrecord_ctrl(int ch,int mode)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	
	struct hdrecord_ctrl_struct *ctrl;

	send=(mod_socket_cmd_type *)send_buf;	
	send->cmd=HDRECORD_CTRL;
	ctrl=(struct hdrecord_ctrl_struct*)send->para;
	ctrl->channel=ch;
	ctrl->mode=mode;
	send->len = sizeof(struct hdrecord_ctrl_struct);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
	
}
/**********************************************************************************************
 * ������	:refresh_hdmodule_para()
 * ����	:��hdmodule���¶�ȡ����
 * ����	:��	 
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int refresh_hdmodule_para(void)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_REFRESH_PARA;
	send->len = 0;
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}
/**********************************************************************************************
 * ������	:trig_record_event()
 * ����	:����һ��¼���¼�
 * ����	:ch¼��ͨ��
 *			 trig:�����¼�(¼��ԭ��)
 *			reclen:ϣ�����ж೤ʱ���¼��(ʵ��¼��ʱ���������ʱ¼��),��0����
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int trig_record_event(int ch,int trig_flag,int reclen)
{
	DWORD send_buf[30];
	struct trig_record_event_struct *trig;
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	
	send->cmd=TRIG_RECORD_EVENT;
	trig=(struct trig_record_event_struct *)send->para;
	trig->channel=ch;
	trig->trig_flag=trig_flag;
	trig->reclen=reclen;
	send->len = sizeof(struct trig_record_event_struct);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);

}

/**********************************************************************************************
 * ������	:clear_hdmod_trig_flag()
 * ����	:���������¼��ģ��Ĵ���״̬
 * ����	:��
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int clear_hdmod_trig_flag(DWORD trig)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=CLEAR_TRIG_FLAG;
	//lc 2014-2-11 ��trig���͹�ȥ
	*((int *)send->para)=trig; //channel
	send->len = sizeof(int);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);		
}



/*********************************************************************************************
*������	:	alarm_playback()
*����		:	����¼��ط������tcprtimg
*����		:	��
*����ֵ	:	0��ʾ�ɹ�����ֵ��ʾʧ��
**********************************************************************************************/
int alarm_playback(int ch)
{
	DWORD	send_buf[50];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_IMG_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,RTIMAGE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

/*********************************************************************************************
*������	:	alarm_enc_playback()
*����		:	����¼��ط������videoenc
*����		:	��
*����ֵ	:	0��ʾ�ɹ�����ֵ��ʾʧ��
**********************************************************************************************/
int alarm_enc_playback(int ch)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_ENC_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,VIDEOENC_MOD_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}



/*********************************************************************************************
*������	:	alarm_hd_playback()
*����		:	����¼��ط������hdmodule
*����		:	��
*����ֵ	:	0��ʾ�ɹ�����ֵ��ʾʧ��
**********************************************************************************************/
int alarm_hd_playback(void)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_ENC_CMD;
	send->len = 0;
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}
/*********************************************************************************************
*������	:	alarm_stop_playback()
*����		:	����ֹͣ¼��ط������videoenc
*����		:	��
*����ֵ	:	0��ʾ�ɹ�����ֵ��ʾʧ��
**********************************************************************************************/
int alarm_cancel_playback(void)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=RTIMG_PLAYBACK_STOP_CMD;
	send->len = 0;
	return main_send_cmd(send,VIDEOENC_MOD_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

/*********************************************************************************************
*������	:	alarm_cancel_playback_rtimg()
*����		:	����ֹͣ¼��ط������tcprtimg
*����		:	ch �л��ص�ͨ����
*����ֵ	:	0��ʾ�ɹ�����ֵ��ʾʧ��
*��ע		:	�˺���������ֹͣ¼��ط������tcprtimgģ��
**********************************************************************************************/
int alarm_cancel_playback_rtimg(int ch)
{
	DWORD	send_buf[50];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=RTIMG_PLAYBACK_STOP_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,RTIMAGE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}


/**********************************************************************************************
 * ������	:restart_hd_record()
 * ����	:��������ָ��ͨ���ĸ�����¼��
 * ����	:ch:Ҫ���������ĸ�����¼��ͨ����
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int restart_hd_record(int ch)
{
	return hdrecord_ctrl(ch,2);
}

 /**********************************************************************************************
 * ������	:start_hd_record()
 * ����	:����ָ��ͨ���ĸ�����¼��
 * ����	:ch:Ҫ�����ĸ�����¼��ͨ����
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int start_hd_record(int ch)
{
	return hdrecord_ctrl(ch,1);
}

/**********************************************************************************************
 * ������	:stop_hd_record()
 * ����	:ָֹͣ��ͨ���ĸ�����¼��
 * ����	:ch:Ҫֹͣ�ĸ�����¼��ͨ����
 * ����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
 **********************************************************************************************/
int stop_hd_record(int ch)
{
	return hdrecord_ctrl(ch,0);
}





