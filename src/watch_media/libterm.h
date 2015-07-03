#ifndef _LIBTERM_H
#define	_LIBTERM_H
#include "ansi_tty.h"
#define	C_WHITE		WHT	/* ��ɫ */
#define	C_RED			RED	/* ��ɫ */
#define	C_GREEN		GRN	/* ��ɫ */
#define 	C_YELLOW		YEL	/* ��ɫ */
#define 	C_BLUE			BLU	/* ��ɫ */
#define 	C_MAGENTA		MAG	/* ��ɫ */
#define	C_CYAN			CYN	/* ��ɫ */
#define	C_BLACK		BLK	/* ��ɫ */

#define	C_HWHITE		HIW	/* ����ɫ */
#define	C_HRED			HIR	/* ����ɫ */
#define	C_HGREEN		HIG	/* ����ɫ */
#define 	C_HYELLOW		HIY	/* ����ɫ */
#define 	C_HBLUE		HIB	/* ����ɫ */
#define 	C_HMAGENTA	HIM	/* ����ɫ */
#define	C_HCYAN		HIC	/* ����ɫ */

#define C_NORMAL        NOM

int 	InitTerminal(void);
void	RestoreTerminal(void);
void	ClearTermScr(void);

//CColor:�ַ���ɫ
//Attr:	0��ʾ������ʾ 1��ʾ����
#define	WriteTermStr(CColor,Attr,Arg...)	do { printf(CColor);if(Attr){printf(REV);} printf(Arg);printf(NOM);}while(0)//don't use printf(##Arg) in gcc 3.4.1
//����
#define	ClearTermScr()		do{printf("%s%s",CLR,HOME);}while(0)

#endif
