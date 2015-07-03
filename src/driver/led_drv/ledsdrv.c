/* leds driver, by lc 2013.4 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/types.h>

//#include <asm/hardware.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ioctl.h>
#include <asm/segment.h>
#include <asm/irq.h>
//#include <asm/arch/irqs.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/device.h>
//#include <linux/devfs_fs_kernel.h>
//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>




#include "leds.h"
#include "leds_api.h"
//#include "compile_info.h"

/* ԭ����ģ��
static void release_leds_dev(struct class_device *dev)
{
	return ;
}

static struct class leds_class = {
	.name		= "leds",
	.release	= &release_leds_dev,
};

struct class_device leds_class_dev;
*/

#define LEDS_DEV       "leds" //�豸�ڵ�����

static unsigned long errorstate=0;  //�����״̬
static int netstate=0; 				//�����״̬
static int alarmstate=0; 			//״̬��״̬	
static struct led_polling_struct err_polling;
static struct led_polling_struct alarm_polling;
static struct led_polling_struct net_polling;

//��ͬ�Ƶ�ts��
static struct ts_struct error_ts[ERR_DISPLAY_TYPE]; 
static struct ts_struct alarm_ts[ALARM_DISPLAY_TYPE]; 
static struct ts_struct net_ts[NET_DISPLAY_TYPE+1];
static void leds_poll(unsigned long arg);
/*
#ifndef KERNEL26
static struct timer_list leds_timerlist;
#ifdef CONFIG_DEVFS_FS
static devfs_handle_t leds_devfs_handle;
#endif

#else
static struct timer_list leds_timerlist=TIMER_INITIALIZER(leds_poll, 0, 0);
#endif
*/
static struct timer_list leds_timerlist=TIMER_INITIALIZER(leds_poll, 0, 0);

//�������״̬
void clear_states(void)
{
	netstate=0;
	alarmstate=0;
	errorstate=0;
}

//������м�����
static void clear_counters(void)
{
	memset(&err_polling,0,sizeof(struct led_polling_struct));
	memset(&alarm_polling,0,sizeof(struct led_polling_struct));
	memset(&net_polling,0,sizeof(struct led_polling_struct));
	err_polling.cycle_done=1;
	alarm_polling.cycle_done=1;
	net_polling.cycle_done=1;
}

//�����еƹر�,״̬���
void turn_off_all(void)
{
	set_leds_output();
	set_net_led_value(0);
#if 0	
	set_error_led_value(0);
	set_alarm_led_value(0);
#endif	
	clear_states();
	clear_counters();
}	

void turn_on_all(void)
{
	set_net_led_value(1);
#if 0	
	set_error_led_value(1);
	set_alarm_led_value(1);
#endif
}	

//�ڴ��������ҳ������ȵ�λ,�����Ϊ0,���ұ�Ϊ31,��û�д��󷵻�32
int find_err_mode(void) 
{
	int i;
	unsigned long tmp;
	tmp=errorstate;
	for(i=0;i<ERR_DISPLAY_TYPE-1;i++)
		{
			if(tmp & 0x80000000)  //��λ�д���
				return i;
			else
				tmp<<=1;
		}
	return 32;
}

//��ʼ��ts���ñ���ڸ�����ֵ�г���Ӧ�ĳ�����������
static void init_ts(void) {
	//error�Ʋ���
	error_ts[0].longtime=0; error_ts[0].shorttime=4;   //����,�ڴ�
	error_ts[1].longtime=1; error_ts[1].shorttime=9;   //flash
	error_ts[2].longtime=2; error_ts[2].shorttime=1;   //net_enc_err
	error_ts[3].longtime=2; error_ts[3].shorttime=2; 	//enc1_err
	error_ts[4].longtime=2; error_ts[4].shorttime=3;	//2
	error_ts[5].longtime=2; error_ts[5].shorttime=4; 	//3	
	error_ts[6].longtime=2; error_ts[6].shorttime=5; 	//4
	error_ts[7].longtime=4; error_ts[7].shorttime=1;    //audio_dnc_err
	error_ts[8].longtime=1; error_ts[8].shorttime=1;   //cf_err
	error_ts[9].longtime=1; error_ts[9].shorttime=2;   //hd_err
	error_ts[10].longtime=1; error_ts[10].shorttime=5; //kbd_err	
	error_ts[11].longtime=3; error_ts[11].shorttime=1;  //video_loss0
	error_ts[12].longtime=3; error_ts[12].shorttime=2;  //1
	error_ts[13].longtime=3; error_ts[13].shorttime=3; //2
	error_ts[14].longtime=3; error_ts[14].shorttime=4;  //3
	error_ts[15].longtime=1; error_ts[15].shorttime=3; //disk_full
	error_ts[16].longtime=1; error_ts[16].shorttime=4; //pwr_err
	error_ts[17].longtime=4; error_ts[17].shorttime=6; //
	error_ts[18].longtime=4; error_ts[18].shorttime=7; 
	error_ts[19].longtime=4; error_ts[19].shorttime=8; 
	error_ts[20].longtime=3; error_ts[20].shorttime=1; 
	error_ts[21].longtime=3; error_ts[21].shorttime=2; 
	error_ts[22].longtime=3; error_ts[22].shorttime=3; 
	error_ts[23].longtime=3; error_ts[23].shorttime=4; 
	error_ts[24].longtime=3; error_ts[24].shorttime=5; 
	error_ts[25].longtime=3; error_ts[25].shorttime=6; 
	error_ts[26].longtime=3; error_ts[26].shorttime=7; 
	error_ts[27].longtime=3; error_ts[27].shorttime=8; 
	error_ts[28].longtime=4; error_ts[28].shorttime=1; 
	error_ts[29].longtime=4; error_ts[29].shorttime=2; 
	error_ts[30].longtime=4; error_ts[30].shorttime=3; 
	error_ts[31].longtime=4; error_ts[31].shorttime=4; 
	error_ts[32].longtime=0; error_ts[32].shorttime=0;

	//alarm�Ʋ��� 
	alarm_ts[0].longtime=0; alarm_ts[0].shorttime=0; 
	alarm_ts[1].longtime=1; alarm_ts[1].shorttime=0; 
	alarm_ts[2].longtime=0; alarm_ts[2].shorttime=4;
	alarm_ts[3].longtime=1; alarm_ts[3].shorttime=1;

	//net�Ʋ���
	net_ts[0].longtime=0; net_ts[0].shorttime=0;
	net_ts[1].longtime=1; net_ts[1].shorttime=1;
	net_ts[2].longtime=1; net_ts[2].shorttime=2;
	net_ts[3].longtime=1; net_ts[3].shorttime=3;
	net_ts[4].longtime=1; net_ts[4].shorttime=4;
	net_ts[5].longtime=0; net_ts[5].shorttime=4;
	net_ts[6].longtime=1; net_ts[6].shorttime=0;
	net_ts[8].longtime=2; net_ts[8].shorttime=1; //wsyadd,����һ��

}


//��ָ�������ָ����ֵ
void set_led_type_value(int ledtype,int value)
{
	switch(ledtype)
		{
			//case(ERR_LED):set_error_led_value(value); break;
			//case(ALARM_LED):set_alarm_led_value(value); break;
			case(NET_LED):set_net_led_value(value); break;
		}
}

//ʹ��Ӧ�ư�ָ��������˸
void poll_led(int ledtype, struct led_polling_struct *poll)
{
	int i;
	int value=5;
	if((poll==0)||(ledtype>ERR_LED)||(ledtype<NET_LED))
		return;
	if(poll->cycle_done)//���һ��cycle������load�µ�cycle
		{
			poll->cycle_done=0;
			poll->cycle_pause_count=0;
			switch(ledtype)
				{
					case(ERR_LED):	
									//i=find_err_mode();
									//lc to do Ŀǰֻ���д�����������жϴ���ţ������ɸĽ�
									if(errorstate!= 0)
										i = 31;
									else
										i = 32;
									poll->long_num=error_ts[i].longtime;
									poll->short_num=error_ts[i].shorttime;	
									//printk("ERR_LED i=%d\n",i);
									break;
					case(ALARM_LED): 
									if(alarmstate==ALARM_DISPLAY_TYPE)
									{
											//printk("ALARM_LED alarmstate=%d\n",alarmstate);
											//set_alarm_led_value(1);
											poll->cycle_done=1;
											return;
									}
									//printk("ALARM_LED alarmstate=%d\n",alarmstate);
									//�����4��Ϊ����
									 if(alarmstate>ALARM_DISPLAY_TYPE)
									 	 return;
									 //set_alarm_led_value(0);
									 poll->cycle_done=1;
									 return;
					case(NET_LED):   
								    if(netstate==7) //�����7��Ϊ����
									{
											//printk("NET_LED netstate=%d\n",netstate);
											set_net_led_value(1);
											poll->cycle_done=1;
											return;
									} 
									
									if(netstate>NET_DISPLAY_TYPE)
										return;
									
									 poll->long_num=net_ts[netstate].longtime;
									 poll->short_num=net_ts[netstate].shorttime;
									 break;
					default:		
									poll->long_num=0;
									poll->short_num=0;
									break;
				}	
			poll->long_count=0;
			poll->short_count=0;
			poll->long_done=0;
			poll->short_done=0;
		}
	if(poll->long_done<poll->long_num) //����������ź���
		{
			if(poll->long_count++<LONGSIG) //�������Ĺ���
					value=1;
			else //���ڰ��Ĺ���
				{
					value=0;
					if(poll->long_pause_count++==LONGPAUSE-1)
						{
							poll->long_pause_count=0;
						 	poll->long_count=0;	
				 			poll->long_done++;
				 		}						
				}
		}	
	else //����������ź���
		{
			if(poll->short_done<poll->short_num)
				{
					if(poll->short_count++<SHORTSIG)
						value=1;
					else 
						{
							value=0;
							if(poll->pause_count++==SHORTPAUSE-1)
								{
									poll->pause_count=0;
								 	poll->short_count=0;	
						 			poll->short_done++;
						 		}						
						}
				}
			else	//���ѭ��pause(��)
				{
					value=0;
					if(poll->cycle_pause_count++>=CYCLEPAUSE)
						poll->cycle_done=1;
				}
		}
		set_led_type_value(ledtype,value);
}
 

static void leds_poll(unsigned long arg)
{   
   	poll_led(ERR_LED,&err_polling);  
   	poll_led(ALARM_LED,&alarm_polling);
	poll_led(NET_LED,&net_polling);
   	leds_timerlist.expires = jiffies + RETURN_TIME; 
   	add_timer(&leds_timerlist);
}

/*
static void set_net_display(unsigned char *buf )
{
	//�û�����net_ts��
	int i;	
	unsigned char buffer[NET_DISPLAY_TYPE*4+20];
	memcpy(buffer,buf,NET_DISPLAY_TYPE*4);
	for(i=0;i<NET_DISPLAY_TYPE;i++)
		{
			net_ts[i].longtime=buffer[4*i];
			net_ts[i].shorttime=buffer[4*i+2];
		}
}

static void set_alarm_display(unsigned char *buf )
{
	//�û�����state_ts��
	int i;	
	unsigned char buffer[STATE_DISPLAY_TYPE*4+20];
	memcpy(buffer,buf,STATE_DISPLAY_TYPE*4);
	for(i=0;i<STATE_DISPLAY_TYPE;i++)
		{
			state_ts[i].longtime=buffer[4*i];
			state_ts[i].shorttime=buffer[4*i+2];
		}
}

static void set_error_display(unsigned char *buf )
{
	//�û�����error_ts��
	int i;	
	unsigned char buffer[ERR_DISPLAY_TYPE*4+20];
	memcpy(buffer,buf,ERR_DISPLAY_TYPE*4);
	for(i=0;i<ERR_DISPLAY_TYPE;i++)
		{
			error_ts[i].longtime=buffer[4*i];
			error_ts[i].shorttime=buffer[4*i+2];
		}
}
*/

//static int leds_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
static int leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long buffer[100];
	unsigned long bufferlong[2];
	unsigned char *buf;
	unsigned long *buflong;
	buf=(unsigned char *)buffer;
	buflong=(unsigned long *)bufferlong;

	//printk("leds_ioctl cmd is %d\n",cmd);
	
	switch (cmd){
		
		case	SET_ERROR_LED:
				//printk("leds_ioctl errorstate is %ld\n",*(unsigned long *)arg);
				copy_from_user(buflong,(unsigned long *)arg,4);
				errorstate=bufferlong[0];	
		break;
		case	SET_ALARM_LED:	
				copy_from_user(buf,(unsigned char *)arg,1);
				alarmstate=buf[0];
				//printk("leds_ioctl alarmstate is %d\n",alarmstate);
		break;
		case	SET_NET_LED:
				copy_from_user(buf,(unsigned char *)arg,1);
				netstate=buf[0];
				//printk("leds_ioctl netstate is %d\n",netstate);
		break;
		default:
			#ifdef DEBUG
				printk("\nledsdrv recv a unknow cmd :0x%08x\n",(int)cmd);
			#endif
		break;
		}
	return 0;
}

static int leds_open (struct inode *inode, struct file *file)
{
	
	//��ʼ������
	printk("open leds!\n");
	//�������Ŷ���Ϊ�������ֹ��������ʾΪ�ߵ�ƽ������
/*
	//turn_off_all();
	*/
	init_ts();
	
	leds_timerlist.expires = jiffies + RETURN_TIME; //100����һ��  ???!
    	leds_timerlist.function = leds_poll; 
    	leds_timerlist.data = 0;

	add_timer(&leds_timerlist);
	
	printk("leds opened\n");
/*	
#ifndef KERNEL26
        MOD_INC_USE_COUNT;
#else
        try_module_get(THIS_MODULE);
#endif
*/
	return 0;
}


static int leds_release (struct inode *inode, struct file *file)
{
	//clean up
	turn_off_all();

	del_timer(&leds_timerlist);
	printk("released leds!\n");
/*	
#ifndef KERNEL26
        MOD_DEC_USE_COUNT;
#else
        module_put(THIS_MODULE);
#endif
*/
	return 0;
}

static struct file_operations leds_fops = {
	owner:THIS_MODULE,
	open:		leds_open,
    unlocked_ioctl:		leds_ioctl,/* linux3.0.y ����ʹ��ioctl������unlocked_ioctl */
	//ioctl:		leds_ioctl,
	release:    leds_release,
};

static struct miscdevice led_dev = {
    MISC_DYNAMIC_MINOR,
    "leddrv",
    &leds_fops,
};

static int __init leds_init(void)
{
	printk("start init the leds %s(%s)...\n",LEDS_VERSION,"lc led");
	int  ret=0;
/*
	if (register_chrdev(LEDS_MAJOR,LEDS_NAME, &leds_fops)) {
                        printk("leds: can't register devfs leds\n");
                        return -ENODEV;
                }
    class_register(&leds_class);	
    		
	//leds_class_dev.dev = &adap->dev;
	leds_class_dev.class = &leds_class;
	leds_class_dev.devt = MKDEV(LEDS_MAJOR,0);
	sprintf(leds_class_dev.class_id, "leds");
	class_device_register(&leds_class_dev);
	//class_device_create_file(&leds_class_dev, &class_device_attr_name);
	*/
	ret = misc_register(&led_dev);
    if (ret)
    {
        printk(KERN_ERR "register misc dev for leddrv fail!\n");
  		return ret;
 	}
	gpio_2_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_2_BASE_ADDR,0x10000);
 	if(!gpio_2_base_addr_virtual)
 	{
     		printk("ioremap gpio group0 failed!\n");
     		/*˵��gpio_0 ��gpio_4��ӳ��ɹ��ˣ���gpio_8ûӳ��ɹ������Խ�֮ǰӳ����ͷ�*/
     		return -1;
 	}

	hi3520d_button_pin_cfg();
	turn_off_all();

#if 0
	mdelay(3000);
	turn_on_all();
	mdelay(3000);
	turn_off_all();
	print_led_value();
	mdelay(3000);
	turn_on_all();
	print_led_value();
	mdelay(3000);
	turn_off_all();
	print_led_value();
	mdelay(3000);
	turn_on_all();
	print_led_value();
	mdelay(3000);
	turn_off_all();
#endif	
	return 0; 
}

static void __exit leds_exit(void)	
{
	turn_off_all();
	printk("leds driver %s(%s) removed!\n",LEDS_VERSION,"lc led");
/*	
	class_unregister(&leds_class);
	devfs_remove("leds");
	unregister_chrdev(LEDS_MAJOR,LEDS_NAME);
	*/
	misc_deregister(&led_dev);
   	iounmap((void*)gpio_2_base_addr_virtual);
}

MODULE_LICENSE("GPL");
module_init(leds_init);
module_exit(leds_exit);

