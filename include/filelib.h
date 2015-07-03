/**
	@file 	filelib.h
	@brief	rm_file_select() �� ftw_sort()��ͷ�ļ�
*/

#ifndef LIBFILE_H
#define LIBFILE_H

#include<sys/stat.h>
#include "gtlog.h"

#define FTW_SORT_ALPHA				(1)		///<����ĸ˳������(Ĭ�Ϸ�ʽ)

#define FN_RETURN_CONTINUE		(1)		///<fn���ش�ֵ�����
#define FN_SPACE_ENOUGH			2	//fn���ش�ֵ��ʾ����ÿռ��㹻������
#define FN_DISK_ERR				3	//fn���ش�ֵ��ʾ����̴��󷵻�

#define DIR_DEPTH			(100)	//Ŀ¼���

///  @brief  linux�⺯��
extern char *get_current_dir_name(void);


/** 
	@brief		��ftw_sort()�е��õ��ⲿ����ĺ��������û��Լ�����
	@param		*dir_file  ��ǰ·��Ŀ¼
	@param		errnum �����
	@return		�û��Լ����壬��������ֵ

	��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
	�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
	�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
*/
extern int err_fn(const char *dir_file,int errnum);


extern int err_fn_user(const char *dir_file,int errnum);

/** 
	@brief		��ftw_sort()�е��õ��ⲿ����ĺ��������û��Լ�����
	@param		file ��ǰ·��
	@param 		*sb  stat�ṹָ��
	@param 		flag ��꣬�����¼��ֿ��� FTW_F---һ���ļ� 
					FTW_D--һ��Ŀ¼  
					FTW_DNR---���ɶ�ȡ��Ŀ¼����Ŀ¼���½��������� 
					FTW_SL----�������� 
					FTW_NS----�޷�ȡ�õ�stat�ṹ���ݣ��п�����Ȩ������
*/
extern int fn(const char *file,const struct stat *sb,int flag);

extern int fn_user(const char *file,const struct stat *sb,int flag,void *user_data);



/**
	@brief		�ӿں������ڲ�����check_dir()	
	@param		*dir    Ҫ������Ŀ¼��
	@param		(*fn)      �����ļ���Ŀ¼ʱ���õĺ���,fn���ط�0ֵʱ�˳�����,  ftw_sort��
				��fn  ���ص�ֵ
	@param		dir_depth ������Ŀ¼��ȣ��������������ftw_sort����������һ��
				����
	@param		sort_mode    ����ʱ������ģʽ,ȡֵΪFTW_SORT_ALPHA...
	@param 		rm_empty_dir  1:������ֿ�Ŀ¼��ɾ��0:��ɾ����Ŀ¼
	@param		(*err_fn)  ��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
				�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
				�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
	@return		�����ж��򷵻�fn()�����ķ���ֵ,ȫ���������򷵻� 0.����
				�������򷵻�-1(��scandir����),�������errno

*/
int  ftw_sort(	const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag),
					int dir_depth,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum)
					
			);


/**
	@brief		�ӿں������ڲ�����check_dir()	
	@param		*dir    Ҫ������Ŀ¼��
	@param		(*fn)      �����ļ���Ŀ¼ʱ���õĺ���,fn���ط�0ֵʱ�˳�����,  ftw_sort��
				��fn  ���ص�ֵ
			user_data	�û�����
	@param		sort_mode    ����ʱ������ģʽ,ȡֵΪFTW_SORT_ALPHA...
	@param 		rm_empty_dir  1:������ֿ�Ŀ¼��ɾ��0:��ɾ����Ŀ¼
	@param		(*err_fn)  ��ִ��scandir����Ŀ¼����ʱ���õĺ������д�����
				�󷵻� err_fn���ص�ֵ,��ֹ����,dir_file��ʾ�����Ŀ¼��
				�ļ�,errnum��ʾ����ţ���ֵΪ����ʱ��errno
	@return		�����ж��򷵻�fn()�����ķ���ֵ,ȫ���������򷵻� 0.����
				�������򷵻�-1(��scandir����),�������errno

*/
int ftw_sort_user(      const char *dir,
                                        int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data),
                                        void *user_data,
                                        int sort_mode,
                                        int rm_empty_dir,
                                        int (*err_fn)(const char *dir_file,     int errnum)
                        );

///rm_file_select.c�ļ��ĺ궨��
#define ERROR_VAL					(-1)///<rm_file_select����ֵ 

/**
	@fn		int rm_file_select(const char *dir,const char *file_path)
	@brief	rm_file_select�ӿں���	
	@param	*dir������ Ŀ¼
	@param	*file_path ��Ҫ������Ŀ¼�����file_path = NULL ��ɾ��
	@return	ftw_no_chdir�ķ���ֵ
	
	�����ڲ��ٴε���ftw_no_chdir
	ָ��Ŀ¼�µ�����Ŀ¼,   ����file_pathָ����Ŀ¼��ɾ������
	��Ŀ¼���ļ���
  
 */
int rm_file_select(const char * dir, const char * file_path);






#endif
