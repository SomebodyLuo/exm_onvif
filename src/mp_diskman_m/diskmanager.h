#ifndef MP_DISKMANAGER_H
#define MP_DISKMANAGER_H
#include "gate_cmd.h"
#include "fileindex.h"
#include "mpdisk.h"


#define MPGDIR	"/hqdata/iframe" //�洢I��ͼƬ��Ŀ¼
#define MPGNUMBER         40   //��MPGDIR����������I��ͼƬ��
#define RM_MPG_ONCE       15    //һ��ɾ����MPG����

#define PICINDEXDIR "/hqdata/picindex" //�洢ͼƬ������Ŀ¼
#define PICINDEXNUMBER 		40 //��PICINDEXDIR�����������ı��ļ���
#define RM_PICINDEX_ONCE       15    //һ��ɾ����picindex����

#define INDEXDIR "/hqdata/index" //�洢¼���ļ�������Ŀ¼
#define INDEXNUMBER 40 //INDEXDIR���������ı��ļ���
#define RM_INDEX_ONCE       15    //һ��ɾ����index����

#define UPDATEDIR "/log/update" //�洢������Ϣ��Ŀ¼
#define UPDATENUMBER	30
#define RM_UPDATE_ONCE	10

#define DISK_MAN_LOOP		10 //ÿ��ô�������һ�δ��̹���

#define SHOW_WORK_INFO //��ӡprintf��Ϣ



struct lockfile_struct{
time_t 	start;
time_t 	stop;
int		trig;
int		ch;
int 	mode;
};
//��ϵͳ������ӡ��ָ���ļ�
void dump_sysinfo(void);
//��ȡ���̿�������,��λΪM
long	get_disk_free(char *mountpath);
//��ȡ����������,��λΪM
long	get_disk_total(char *mountpath);

int disk_full(char *mountpath, int partitionindex) ;
int init_diskman(void);
int creat_diskman_thread(pthread_attr_t *attr,void *arg);
int read_diskman_para_file(char *filename);
DWORD get_diskmanstatint(void);
int isdir(char *filename);
int usr_lock_file_time(char *path,struct usr_lock_file_time_struct *lockfile);
void set_cferr_flag(int flag);
int	remove_oldest_file(void);
#endif 



