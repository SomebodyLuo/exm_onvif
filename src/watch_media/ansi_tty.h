/***********************************************************************************************
 * file:ansi_tty.h
 * ��ɫ�ַ��ն˵Ŀ����ַ�����
 ***********************************************************************************************/

#ifndef _ANSI_H
#define	_ANSI_H

#define ESC "\e"	//"\033"
#define CSI ESC"["


/* ǰ��ɫ */


#define BLK ESC"[30m" /* ��ɫ */
#define RED ESC"[31m" /* ��ɫ */
#define GRN ESC"[32m" /* ��ɫ */
#define YEL ESC"[33m" /* ��ɫ */
#define BLU ESC"[34m" /* ��ɫ */
#define MAG ESC"[35m" /* ��ɫ */
#define CYN ESC"[36m" /* ��ɫ */
#define WHT ESC"[37m" /* ��ɫ */


/* ��ǿǰ��ɫ */


#define HIR ESC"[1;31m" /* ���� */
#define HIG ESC"[1;32m" /* ���� */
#define HIY ESC"[1;33m" /* ���� */
#define HIB ESC"[1;34m" /* ���� */
#define HIM ESC"[1;35m" /* ���� */
#define HIC ESC"[1;36m" /* ���� */
#define HIW ESC"[1;37m" /* ���� */


/* ��ǿ����ɫ */


#define HBRED ESC"[41;1m" /* ���� */
#define HBGRN ESC"[42;1m" /* ���� */
#define HBYEL ESC"[43;1m" /* ���� */
#define HBBLU ESC"[44;1m" /* ���� */
#define HBMAG ESC"[45;1m" /* ���� */
#define HBCYN ESC"[46;1m" /* ���� */
#define HBWHT ESC"[47;1m" /* ���� */


/* ����ɫ */


#define BBLK ESC"[40m" /*��ɫ */
#define BRED ESC"[41m" /*��ɫ */
#define BGRN ESC"[42m" /*��ɫ */
#define BYEL ESC"[43m" /* ��ɫ */
#define BBLU ESC"[44m" /*��ɫ */
#define BMAG ESC"[45m" /*��ɫ */
#define BCYN ESC"[46m" /*��ɫ */
// #define BWHT ESC"[47m" /* ��ɫ */


#define NOR ESC"[2;37;0m" /* ����ԭɫ */


/* ������Ansi��ɫ�����ַ����� Gothic april 23,1993 */
/* ע�⣺��Щ��������ΪVT100�ն���Ƶġ� */


#define BOLD ESC"[1m" /* �򿪴��� */
#define CLR ESC"[2J" /* ���� */
#define HOME ESC"[H" /* ���͹�굽ԭ�� */
#define REF CLRHOME /* ������������ */
#define BIGTOP ESC"#3" /* Dbl height characters, top half */
#define BIGBOT ESC"#4" /* Dbl height characters, bottem half */
#define SAVEC ESC"[s" /* Save cursor position */
#define REST ESC"[u" /* Restore cursor to saved position */
//#define REVINDEX ESC"M" /* Scroll screen in opposite direction */
#define SINGW ESC"#5" /* Normal, single-width characters */
#define DBL ESC"#6" /* Creates double-width characters */
#define FRTOP ESC"[2;25r" /* �������� */
#define FRBOT ESC"[1;24r" /* ����ײ�һ�� */
#define UNFR ESC"[r" /* ���к͵ײ�һ�нⶳ */
#define BLINK ESC"[5m" /* ��������ģʽ */
#define U ESC"[4m" /* �»���ģʽ */
#define REV ESC"[7m" /* �򿪷���ģʽ */
#define HIREV ESC"[1,7m" /* ��ɫ�ʷ�����ʾ */


/* Binary Li ��� */
#define NOM ESC"[0m" /* ���� */





#endif

