/* imemotion.h*/ 
#ifndef IME_MOTION_H
#define IME_MOTION_H
#define DEV_IME6410			//lsk 2007 -1-5 

/*
*************************************************************
 * ������	:set_ime_motion_sens()
 * ����	: ���� ime6410 �ƶ���������� 
 * ����	: 
 		struct compress_struct *enc  ����������
 		int ch ��Ƶͨ����
 		int sens ������  0 ��ֹ�ƶ����  5�ƶ������������� lsk 2007 ��6 ��27
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************
 */
int set_ime_motion_sens(struct compress_struct *enc, int ch, int sens);
/*
*************************************************************
 * ������	:get_ime_motion_stat()
 * ����	: ���� ime6410 �ƶ���������� 
 * ����	: 
 		struct compress_struct *enc  ����������
 		int ch ��Ƶͨ����
 * ����ֵ	:0 ��ʾû���ƶ�, 1��ʾ���ƶ�����ֵ��ʾʧ��
*************************************************************
 */
int get_ime_motion_stat(struct compress_struct *enc, int ch);
#ifndef DEV_IME6410
/*
*************************************************************
 * ������	:set_ime_motion_para()
 * ����	: ���� ime6410 �ƶ�������
 * ����	: 
 		struct compress_struct *enc  ����������
 		unsigned char ch ��Ƶͨ����
 		int sen	�����Ȳ���
 		unsigned short *area  �������������	!!Ŀǰû��ʵ���������Ĺ���
 		 int pic_size  ͼ��Ĵ�С (D1,  HD1, CIF)
 * ����ֵ	:0 ��ʾ�ɹ�����ֵ��ʾʧ��
*************************************************************
 */
int set_ime_motion_para(struct compress_struct *enc, unsigned char ch, int sen,unsigned short *area, int pic_size);

/*
*************************************************************
 * ������	:process_motion_pkt()
 * ����	: ����ime6410 �ƶ�������ݰ� 
 * ����	: 
  		int ch ��Ƶͨ����
		unsigned char *buf  ����������	!!Ŀǰû��ʵ���������Ĺ���
 		 int pic_size  ͼ��Ĵ�С (D1,  HD1, CIF)
 * ����ֵ	:0 ��ʾû���ƶ���⣬1��ʾ���ƶ���⣬ ��ֵ��ʾʧ��
*************************************************************
 */
int process_motion_pkt(unsigned char ch, unsigned char *buf, int pic_size);
//int init_ime_motion_para(struct compress_struct *enc, unsigned char ch, int sen, int pic_size);
#endif
#endif
