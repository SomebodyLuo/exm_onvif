#ifndef __SFCP_H
#define	__SFCP_H

//�汾
#define SFCP_VERSION		("2.02")
//ver:2.02 2008-08-27 �޸��жϷ��������ļ��Ƿ���ڵ�����
//ver:2.01 2008-08-27 �޸ĸ���ʧ��ʱ��¼��־����ʾ��Ϣ
//ver:2.00 2008-08-25 �޸İ汾�ţ�����ʱ����3�Σ�У��3�ζ����ϸ�ʱ�˳���
//ver:1.00 2008-08-14 ����Դ�ļ���


#define	DST_FILENAME_MAX_LEN		(512)		//Ŀ���ļ�����󳤶�
#define	SRC_FILENAME_MAX_LEN		(512)		//Դ�ļ�����󳤶�
#define	RD_BUF_MAX_LEN			(8192)		//��Դ�ļ����Ļ�������󳤶�
#define	FILENAME_MAX_LEN			(128)		//�ļ�������󳤶�
#define LNK_PATH_MAX_LEN		(128)		//link-path-max-len
#define DIR_MAX_DEEP			(5000)		//����Ŀ¼��������

//�����ļ�Ŀ¼��Ϣ�Ľṹ
typedef struct
{
	char src_path[SRC_FILENAME_MAX_LEN];			//Դ�ļ�·��
	char dst_path[DST_FILENAME_MAX_LEN];			//Ŀ���ļ�·��
	char dst_path_bak[DST_FILENAME_MAX_LEN];		//Ŀ��·���ı���
	//char sub_path[SRC_FILENAME_MAX_LEN];		//���·��
	//char old_sub_path[SRC_FILENAME_MAX_LEN];		//src����һ�ν����Ŀ¼
	char cur_dst_path[DST_FILENAME_MAX_LEN];		//��ǰ��Ŀ���ļ�·��
	//char file_name[FILENAME_MAX_LEN];
	char sub_path_nh[SRC_FILENAME_MAX_LEN];		//û��Դͷ��Ŀ¼
	char sub_path_nf[SRC_FILENAME_MAX_LEN];		//û���ļ�β��Ŀ¼
	char sub_path_nf_old[SRC_FILENAME_MAX_LEN];	//�ϴε�Ŀ¼
	unsigned long src_file_len;						//Դ�ļ�����
	unsigned long dst_file_len;						//Ŀ���ļ�����
	int	sub_file_total;							//��Ŀ¼�µ��ļ���
	int	arg_total;
	int	creat_ndir_flag;
	int	src_file_flag;
	int	opt_r_flag;				//������ѡ��r
	int	opt_f_flag;				//fѡ���־
	int	opt_d_flag;				//dѡ���־
	int 	opt_u_flag;				//��ʾ���Ʊ�־
}SFCP_PATH_T;







#endif
