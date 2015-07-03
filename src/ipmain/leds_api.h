#ifndef LEDS_API_H
#define LEDS_API_H
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#define LEDS_MAJOR     255 	//changed by shixin 254
#define LEDS_NAME      "LEDS"
#define LEDS_VERSION   "Version 0.7"

//0.7 ����IP1004 LED��ģʽ Ŀǰ3����: ����/����/�������ҽ�֧��2Ԫģʽ
//�����: ע��������������������  �����:�κδ���������������   ������:�б���ʱ��������λ����
//0.6 ���������һ����˸��ʽ,2��1��; �������е��û���������֤ʧ��
//0.5 ������ں�2.6��֧��
//0.4�����˼������ӹ��ϵ�err����ʾ
//0.3��д�󲿷֣�������net��ָʾ����adsl����Ĺ���
//0.2������state��ָʾ����״̬����

#ifdef __KERNEL__

#if HZ==100
#define RETURN_TIME 25//��λ:1/100��
#elif HZ==200
#define RETURN_TIME 50
#else
	#error "unsupport HZ value"
#endif

#endif

#define LONGSIG 		5// ��λ:RETURN_TIME/100��
#define LONGPAUSE   	3
#define SHORTSIG 	1 //100ms
#define SHORTPAUSE 	1 //100ms
#define CYCLEPAUSE  	10 //���һ��cycle����ж�

#define MAX_DISPLAY_TYPE  33 //��ʾ�������ֵ

struct ts_struct {
	WORD longtime; //��
	WORD shorttime; //��
};

struct led_polling_struct 
{
	int cycle_done;	//cycle��ɱ�־
	int long_count;	//���ź�����м�����
	int short_count; //���ź�����м�����
	int long_done;   //���ź���������
	int short_done;	 //���ź���������
	int long_num;    //���źŸ���
	int short_num;   //���źŸ���
	int pause_count; //�̼��������
	int long_pause_count; //�����������
	int cycle_pause_count; //ѭ�����������
};

#define SET_NET_LED 		1 
#define SET_ERROR_LED   	12
#define SET_ALARM_LED		3

#define NET_LED				1
#define ALARM_LED			2
#define ERR_LED				3

#define NET_DISPLAY_TYPE	8//7  //�Ƶ���˸��ʽ����
#define ALARM_DISPLAY_TYPE	4
#define ERR_DISPLAY_TYPE	33

#define NET_NO_ADSL 	0
#define NET_NO_MODEM	1
#define NET_INVAL_PASSWD 2
#define NET_LOGINED	   4
#define NET_INVAL_USR  3
#define NET_ADSL_OK		5		
#define NET_GATE_CONNECTED	6	
#define NET_REGISTERED	7
#define NET_PAP_FAILED	8	

/*
#define STATE_RESET		0
#define STATE_UPDATING	1
#define STATE_ALARMING  2
#define STATE_ACK_ERR	3
#define STATE_ACKED		4
*/
#define STATE_ALARMING 2

int init_leds(void);
#if 0
int set_errorled_flash_mode(struct ts_struct *tsnew, int count); //countΪ�ֽ���,ӦΪ132
int set_stateled_flash_mode(struct ts_struct *tsnew, int count);
int set_netled_flash_mode(struct ts_struct *tsnew, int count);
#endif

int set_error_led_state(void);//0���������ļ���
int set_net_led_state(int state);	//0����1һ��һ�̣�2һ�����̣�3һ������..����
int set_state_led_state(int state); //�������ּ���
int get_current_netled(void); //����Ŀǰ��net��״̬
int get_current_alarmled(void);

#endif


