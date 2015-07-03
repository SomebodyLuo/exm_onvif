/*
		����¼���ļ���������غ���  --wsy Dec 2007
*/


#include "fileindex.h"
#include <errno.h>
#include "gtlog.h"
#include "filelib.h"
#include "time.h"
#include "sqlite3.h"
#include "ftw.h"
#include "file_def.h"
#include "pthread.h"
#include "commonlib.h"
#include "hdutil.h"
#include "avilib.h"
#include "fixdisk.h"
#include <unistd.h>
#include <sys/time.h>
#define 	INDEX_FILE_NAME  "index.db"  //ÿ��������Ŀ¼�µ������ļ�����
#define		INDEX_TABLE_NAME	"aviindex"		//���ݿ��ڵı����


#define	AVIINDEX_DB_VERSION		1
//�汾	1	��ʼ��������汾�ȴ˸�����ؽ�db


#define DATABASE_NAME_LEN 64
#define DATABASE_MAX 4
typedef struct 
{
	 sqlite3 *db;
        char    dbname[DATABASE_NAME_LEN];
}DB_INFO;


DB_INFO db_info[DATABASE_MAX] = {0};


int GetDabase(char *mountpath, sqlite3 **pdb)
{
    int i;
    
    for(i = 0; i <DATABASE_MAX; i++)
    {
        /*mountpath��db_info[i].dbname����"/hqdata/sda1"
           ����Ϊ������ٶȣ�֮�Ƚϵ�10λ�͵�11λ*/
        if((db_info[i].dbname[10] == mountpath[10]) && (db_info[i].dbname[11] == mountpath[11]))
        {
            *pdb = db_info[i].db;
            return 0;
        }
    }
    
    //gtlogerr("[%s:%d] ���ݿ�%s ������!\n",__FILE__,__LINE__,mountpath);
    return -1;

}


/*��ʼ��Ӳ���ϵ����ݿ⣬ʹ��mpdisk_process_all_partitions�ȽϺ�������
mpdisk_process_all_partitions��mpdiskģ���У�mp_diskman��mp_hdmodule����ʱ̫����*/
int InitAllDabase()
{

    int             i = 0;
    int             ret;
    sqlite3      *pdb = NULL;
    

    memset(db_info, 0, sizeof(db_info));

    if(access("/hqdata/sda1/index.db", F_OK) == 0 )//������
    {
        
        ret = fileindex_open_db("/hqdata/sda1", &pdb);
        if(ret != 0)
        {
            gtlogerr("�޷���%s�µ����ݿ�,ԭ��%s\n","/hqdata/sda1/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda1");
            db_info[i].db = pdb;
            gtloginfo("��%s�ɹ�,db:%x\n","/hqdata/sda1/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("û��%s���ݿ�,Ӳ�̻���û�йҽ�\n","/hqdata/sda1/index.db");
    }    
 
    if(access("/hqdata/sda2/index.db", F_OK) == 0 )//������
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda2", &pdb);
        if(ret != 0)
        {
            gtlogerr("�޷���%s�µ����ݿ�,ԭ��%s\n","/hqdata/sda2/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda2");
            db_info[i].db = pdb;
            gtloginfo("��%s�ɹ�,db:%x\n","/hqdata/sda2/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("û��%s���ݿ�,Ӳ�̻���û�йҽ�\n","/hqdata/sda2/index.db");
    }
    


    
    if(access("/hqdata/sda3/index.db", F_OK) == 0 )//������
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda3", &pdb);
        if(ret != 0)
        {
            gtlogerr("�޷���%s�µ����ݿ�,ԭ��%s\n","/hqdata/sda3/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
        
            strcpy(db_info[i].dbname,"/hqdata/sda3");
            db_info[i].db = pdb;
            gtloginfo("��%s�ɹ�,db:%x\n","/hqdata/sda3/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("û��%s���ݿ�,Ӳ�̻���û�йҽ�\n","/hqdata/sda3/index.db");
    }


     
    if(access("/hqdata/sda4/index.db", F_OK) == 0 )//������
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda4", &pdb);
        if(ret != 0)
        {
            gtlogerr("�޷���%s�µ����ݿ�,ԭ��%s\n","/hqdata/sda4/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda4");
            db_info[i].db = pdb;
            gtloginfo("��%s�ɹ�,db:%x\n","/hqdata/sda4/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("û��%s���ݿ�,Ӳ�̻���û�йҽ�\n","/hqdata/sda4/index.db");
    }    

    return 0;

}

int CloseAllDabase()
{

    int            i = 0;

    for(i = 0; i <DATABASE_MAX; i++)
    {
        if(db_info[i].db != NULL)
        {
            sqlite3_close(db_info[i].db);
        }
    }
    
    return 0;

}




/*
		�µĹ����õĽӿ�
*/

/****************************************************************************************
 *��������:pathname2indexname()
 * ����:   ���������ص�������"/hqdata/hda2"���������������"/hqdata/hda2/index.db"
 *���� :   mountpath   ��Ҫ�޸ĵ�·���� ��:/hqdata/hda2
 *��� :   indexname   ���ص��ַ�������:/hqdata/hda2/index.db
 *���� :   ��ȷ����0 �����󷵻ظ�ֵ
 * **************************************************************************************/
int pathname2indexname(IN char *mountpath, OUT char* indexname)
{
    if((mountpath == NULL)||(indexname == NULL))
        return -EINVAL;
        
    sprintf(indexname, "%s/%s",mountpath,INDEX_FILE_NAME);  
    return 0;
}


/*******************************************************************************************
 *������:  exec_sqlcmd()
 *����  :  ִ��sql���
 *����  :  db         ��open���ص�ָ��db���ݿ��ļ��ľ���� 
 *         sqlcmd     ��Ҫִ�е�sql���
 *         callback   �ص�����
 *         para       ���ͻص������Ĳ���
 *����ֵ:  ��ȷ����0 �� ���󷵻ظ�ֵ
 *******************************************************************************************/
int exec_sqlcmd(sqlite3 *db, char *sqlcmd,  int (*callback)(void*,int,char**,char**), void * para)
{

    int ret = SQLITE_BUSY;
    char *errmsg = NULL;
    
    if((db==NULL)||(sqlcmd == NULL))
        return -EINVAL;

    while((ret==SQLITE_BUSY)||(ret==SQLITE_LOCKED))
    {
        ret = sqlite3_exec(db,sqlcmd, callback,para, &errmsg);
        if(ret == 0)
        {
            if(errmsg) 
            {
                sqlite3_free(errmsg);
            }
            break;
        }
        else
        {       
            gtlogerr("[%s:%d]sqlite3_exec()[%s]...error=%d:%s\n",__FILE__,__LINE__,sqlcmd,ret,errmsg);
            if(errmsg) 
            {
                sqlite3_free(errmsg);
            }
            usleep(10);
        }
    }   

    return ret;
    
}


/**********************************************************************************************
 *������: isdir()
 *����  : ���filenameָ����ļ��Ƿ�Ϊһ��Ŀ¼
 *����  : filename    �ļ���
 *���  : void
 *����ֵ: 0��Ŀ¼��1����Ŀ¼
 * ********************************************************************************************/
int isdir(char *filename)
{

    struct stat statbuf;

    if(stat(filename,&statbuf)==-1)
    {
            printf("Get stat on %s Error��o%s\n",filename,strerror(errno));
            return(-1);
    }

    if(S_ISDIR(statbuf.st_mode))
    {
    //printf("%s is dir\n",filename);
            return 0;
    }
    else
    {
    //printf("%s is not dir\n",filename);
            return 1;
    }
    
}


int fileindex_open_db(char *mountpath, sqlite3 **pdb)
{
	char indexname[100];
	
	if(mountpath == NULL)
		return -EINVAL;
		
	pathname2indexname(mountpath,indexname);
	return sqlite3_open(indexname,pdb);
      

}


int fileindex_close_db(IN char* mountpath, IN sqlite3* db)
{
	if(db == NULL)
		return -EINVAL;
	sqlite3_close(db);
	return 0;
}

//��ʼ��������������ֵ������
int fileindex_init_db(IN char * partition)
{
    int ret;
    char sqlcmd[200];
    sqlite3 *db;
    char *errmsg=NULL;
    
    if((partition == NULL)||(isdir(partition)))
        return -EINVAL;
    
    ret=fileindex_open_db(partition,&db);
    if(ret != 0)
    {
        gtlogerr("�޷���%s�µ����ݿ�,ԭ��%s\n",partition,sqlite3_errmsg(db));
        return ret; 
    }

    //������
    sprintf(sqlcmd,"create table %s(name nvarchar[80], start integer, stop integer, ch integer, trig integer,ing integer,lock integer, version integer)",INDEX_TABLE_NAME);
    ret = sqlite3_exec(db, sqlcmd, NULL,NULL, &errmsg);
    if(ret != 0)
    {
        gtlogerr("�޷�������%s,ԭ��%s\n",INDEX_TABLE_NAME,sqlite3_errmsg(db));
    }

    if(errmsg) 
    {
        sqlite3_free(errmsg);
    }
    
    fileindex_close_db(partition, db);
    return ret; 
    
}

/*************************************************************************
 * 	������:	fileindex_add()
 *	����:	��һ���ļ������������ڵķ����������ļ���ĩβ
 *	����:	db,���ݿ�ָ��
 *			filename,¼���ļ���
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_add(IN sqlite3 *db, IN char *filename)
{
    char sqlcmd[200];
    struct file_info_struct finfo;
    
    if((filename == NULL)||(db==NULL))
        return -EINVAL;
    
    hdutil_filename2finfo(filename, &finfo);
    //����
    sprintf(sqlcmd,"insert into %s values ('%s','%d','%d','%d','%d','%d','%d','%d')",INDEX_TABLE_NAME,filename,(int)finfo.stime,(int)finfo.stime+finfo.len,finfo.channel,finfo.trig,finfo.ing_flag,finfo.lock,AVIINDEX_DB_VERSION);
    return exec_sqlcmd(db, sqlcmd,NULL,NULL);
}

//sqlite3 *db3 = NULL;
int fileindex_add_to_partition(IN char* mountpath, IN char *filename)
{
    sqlite3 *db=NULL;
    int  ret;
    
    if((mountpath == NULL)||(filename == NULL))
        return -EINVAL;


    ret = GetDabase(mountpath,&db);
    //db = db_info[1].db;
    //printf("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
    if(ret != 0)
    {
        //gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    
    fileindex_add(db,filename);
    //fileindex_close_db(mountpath,db);

    return 0;
}

int fileindex_del_file(sqlite3* db, char *filename)
{
    char sqlcmd[200];
    
    if((db==NULL)||(filename==NULL))
        return -EINVAL;
    
    sprintf(sqlcmd,"delete from %s where name = '%s'",INDEX_TABLE_NAME,filename);
    return exec_sqlcmd(db,sqlcmd,NULL,NULL);
    
}




int del_file_callback(void *db, int argc, char **value, char **name)
{

    int ret;
    int retf;
    
    if(db == NULL)
        return 0;

    //printf("delete file %s \n",value[0]);

    //�鿴��Ҫɾ�����Ǹ�¼���ļ��Ƿ����    
    if(access(value[0],F_OK)!=0)//�Ѿ���������
    {
        gtlogerr("ɾ��%s �ļ������Ǹ��ļ��Ѿ���������",value[0]);
        return 0;

    }

    errno = 0;
    ret = remove(value[0]);
    if(ret != 0)
    {
        gtloginfo("ɾ��%sʧ��,�����%d:%s\n",value[0],errno,strerror(errno));
        fix_disk(value[0],errno);
    }
    
    return 0;
    
}


int get_version_callback(void *arg, int argc, char **value, char **name)
{

    int *version;
    
    if(arg == NULL)
        return -EINVAL;
    
    version = (int *)arg;
    *version =atoi(value[0]);
    return 0;
}

//����db version,����û��database�򷵻�0
int get_db_version(IN char *dbname)
{

    char sqlcmd[200];
    sqlite3 *db;
    char *errmsg=NULL;  
    int version=0;
    
    if(dbname == NULL)
        return -EINVAL;
    
    sqlite3_open(dbname,&db);
    sprintf(sqlcmd,"select version from aviindex order by start limit 1");
    exec_sqlcmd(db, sqlcmd,get_version_callback,&version);
    sqlite3_close(db);
    
    return version;
    
}

/*************************************************************************
 * 	������:	fileindex_del_oldest()
 *	����:	�ӷ���������ɾȥ���ϵ�δ������¼��ɾ����Ӧ�ļ�
 *	����:	mountpath,�������ƣ�����/hqdata/hda1
 *			no,��Ҫɾ�����ļ���Ŀ
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
//static sqlite3 *db1 = NULL;
int fileindex_del_oldest(IN char* mountpath, int no)
{
    char sqlcmd[200];
    int ret;
    sqlite3 *db= NULL;

    if((mountpath == NULL)||(no <=0))
    {
        printf("[%s:%d]mountpath==NULL or no <0\n",__FILE__,__LINE__);
        return -EINVAL;
    }

//     if(db1 == NULL)
//     {
//        fileindex_open_db(mountpath,&db);
//        db1 = db;
//    }
//    else
//        db = db1;
    ret = GetDabase(mountpath,&db);
    if(ret != 0)
    {
        gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    //�Ȳ�ѯ��ɾ�������ļ�
    sprintf(sqlcmd,"select name from %s where lock = 0 and ing = 0 order by start limit %d",INDEX_TABLE_NAME,no);
    ret=exec_sqlcmd(db,sqlcmd,del_file_callback,db);
    if(ret!=0)
    {
        gtlogerr("sqlite3ִ��ɾ������¼���ļ�ʧ��,ret=%d\n",ret);
    }

    sprintf(sqlcmd,"delete from %s where name in (select name from %s where lock = 0 and ing = 0 order by start limit %d)",INDEX_TABLE_NAME,INDEX_TABLE_NAME,no);
    ret=exec_sqlcmd(db,sqlcmd,NULL,NULL);
    if(ret!=0)
    {
        gtlogerr("sqlite3ִ��ɾ������¼���ļ�ʧ��,ret=%d\n",ret);
    }

    //fileindex_close_db(mountpath,db);
    return ret;
}

/*************************************************************************
 * 	������:	fileindex_rename_in_partition()
 *	����:	�����������ļ���ָ���ļ����ĳɸ������ļ���
 *	����:	mountpath,��������
 *			oldname,������
 			newname,������
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_rename_in_partition(IN char*mountpath, IN char *oldname, IN char* newname)
{
    sqlite3 *db= NULL;
    int ret;
    
    if((mountpath == NULL)||(oldname == NULL)||(newname == NULL))
        return -EINVAL;

    //fileindex_open_db(mountpath,&db);
    ret = GetDabase(mountpath,&db);
    if(ret != 0)
    {
        gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    ret = fileindex_rename(db, oldname,newname);
    //fileindex_close_db(mountpath,db);

    return ret;
}



int fileindex_rename(IN sqlite3 *db, IN char *oldname, IN char *newname)
{
    int ret;
    if((db == NULL)||(oldname == NULL)||(newname == NULL))
        return -EINVAL;
    
    if(access(oldname,F_OK)!=0)
    {
        printf("��������%sΪ%s��ǰ���Ѳ�����\n",oldname,newname);
        fileindex_del_file(db,oldname);
        return -EPERM;
    }
    ret = fileindex_add(db, newname);
    if(ret!=0)
        gtloginfo("in rename fileindex_add %s return %d\n",newname,ret);
    ret = rename(oldname,newname);
    //sprintf(sqlcmd,"update %s set name ='%s' and stop = '%d' and ing = '%d' and lock ='%d' where name='%s'",INDEX_TABLE_NAME,newname,finfo.stime+finfo.len,finfo.ing_flag,finfo.lock,oldname);
    
    ret =fileindex_del_file(db,oldname);
    
    if(ret!=0)
        gtloginfo("in rename fileindex_del %s return %d\n",oldname,ret);
    
    return ret;
}


int lock_file_callback(void *indexname, int argc, char **value, char **name)
{
    char oldname[200];
    char *filename; 
    int mode;
    FILE *fp= NULL;
    int ret;
    
    if(indexname==NULL)
        return 0;
    sprintf(oldname,value[0]);
    
    //�����ɵ��ļ���д���ļ�
    fp=fopen(indexname,"a+");
    if(fp!=NULL)
    {
        fprintf(fp,"%s\n",oldname);
        fclose(fp);
    }
    
    return 0;
}




int fileindex_lock_by_time(IN char* mountpath, IN int flag,IN int  starttime, IN int stoptime, IN int  trig, IN int ch)
{
	char sqlcmd[400];
	char indexname[200];
	char cmd[200];
	char oldname[200];
	char tname[200];
	FILE *fp=NULL;
	struct timeval tv;
       struct timezone tz;	
	sqlite3* db;
       int ret;
	
	if(mountpath==NULL)
		return -EINVAL;
	//fileindex_open_db(mountpath,&db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
	//�Ȳ�ѯ����������ļ�
	sprintf(sqlcmd,"select name from %s where lock = %d and ing = 0",INDEX_TABLE_NAME,1-flag); 
	if(starttime!= -1)
	{
		sprintf(cmd," and start <= %d ",stoptime);
		strcat(sqlcmd,cmd);
	}
	if(stoptime != -1)
	{
		sprintf(cmd,"and stop >= %d ",starttime);
		strcat(sqlcmd,cmd);
	}
	if(trig != -1)
	{
		sprintf(cmd,"and trig =  %d ",trig);
		strcat(sqlcmd,cmd);
	}
	if(ch != -1)
	{
		sprintf(cmd,"and ch =  %d",ch);
		strcat(sqlcmd,cmd);
	}
	//���������ļ�
	gettimeofday(&tv,&tz);
	sprintf(indexname,"/hqdata/%ld-%06d-lockindex.txt",tv.tv_sec,tv.tv_usec);
	exec_sqlcmd(db,sqlcmd,lock_file_callback,indexname);

	//�������ļ�ִ�о������
	fp=fopen(indexname,"r");
	if(fp!=NULL)
	{
		while(fgets(oldname,200,fp)!=NULL)
		{
			oldname[(strlen(oldname)-1)] = '\0';
			if(flag == 0)
				hdutil_unlock_filename(oldname,tname);
			else
				hdutil_lock_filename(oldname,tname);
			fileindex_rename(db, oldname,tname); 
		}
		
	}
	//ɾ�������ļ�
	//remove(indexname);--��ʱ�����ڴ���
	//fileindex_close_db(mountpath,db);
	return 0;
}



//��fileindex_create_index�е�ftw���ã�Ϊһ���ļ���������
int create_index_for_file_fn (char *file, struct stat *sb, int flag, void *user_data)
{
	sqlite3 *db;
	int ret;
	if(user_data==NULL)
		return 0;
	
	db = (sqlite3 *)user_data;
	if((flag==FTW_F)&&((strstr(file,IMG_FILE_EXT)!=NULL)||(strstr(file,RECORDING_FILE_EXT)!=NULL))) 
 	{
 		ret = fileindex_add(db, file); 
  	}	
 	return 0;
} 

/*************************************************************************
 * 	������:	fileindex_create_index()
 *	����:	Ϊ���������µ�¼���ļ���������
 *	����:	path��������������,��"/hqdata/hda2"
 *			forced: 0��ʾ��ǰû��������������ʱ�Ŵ���,1��ʾ������ζ����´�����1
 *	���:	
 * 	����ֵ:	�ɹ�����0,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_create_index(IN  char *path, IN int forced)
{
	sqlite3 *db;
	char cmd[200];
	char indexname[200];
	int ret;
	struct stat statbuf;

	if((path==NULL)||(get_disk_total(path)<200))
		return -EINVAL;
	
  //Generating the database index file /hqdata/hda1/index.db	
	pathname2indexname(path, indexname);		
	if(forced == 0) //û������ʱ�Ŵ���
	{
		if(access(indexname,F_OK|W_OK|R_OK)==0)//������
		{
			stat(indexname,&statbuf);
			if(statbuf.st_size!=0)	//������С��Ϊ0
			{
				//�ж��Ƿ����ݿ�汾�͵�ǰһ��
				if((get_db_version(indexname))==AVIINDEX_DB_VERSION)
					return 0;
			}
		}
	}	
	ret = remove(indexname);

	//���ӱ�־�ļ�
	sprintf(cmd,"touch %s/creating_db",path);
	system(cmd);
	fileindex_init_db(path);

	gtopenlog("hdutil");	
	gtloginfo("Ϊ%s�����ؽ����ݿ�\n",path);

	sqlite3_open(indexname, &db);
	ret = ftw_sort_user(path, create_index_for_file_fn, db, FTW_SORT_ALPHA, 0, fix_disk_ftw);
	sqlite3_close(db);
  
	gtopenlog("hdutil");	
	gtloginfo("Ϊ%s�����������ݿ����\n",path);
	
	//ɾ����־�ļ�
	sprintf(cmd,"rm -rf %s/creating_db",path);
	system(cmd);
	return ret;
}




int get_filetime_callback(void *time, int argc, char **value, char **name)
{
	int *timep;
	timep = (int *)time;
	
	if(time == NULL)
		return 0;
	
	*timep = atoi(value[0]);
	return 0;
}



/*************************************************************************
 * 	������:	fileindex_get_oldest_file_time()
 *	����:	��ȡ���������ϵĿ�ɾ���ļ��Ĵ���ʱ��
 *	����:	mountpath��������������,��"/hqdata/hda2"
 *	���:	
 * 	����ֵ:	�ɹ����ش���ʱ��,���򷵻ظ�ֵ
 *************************************************************************/
int fileindex_get_oldest_file_time(char *mountpath)
{
	char sqlcmd[200];
	int time=0;
       int ret;
	sqlite3 *db;
	
	if(mountpath == NULL)
		return -EINVAL;
	
	//fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }    
	sprintf(sqlcmd,"select start from %s where ing = 0 and lock =0 order by start  limit 1",INDEX_TABLE_NAME);
	exec_sqlcmd(db,sqlcmd,get_filetime_callback ,&time);
	//fileindex_close_db(mountpath,db);
	return time;
}



//��һ��.ING�ļ�ת��.AVI�ļ�
/*****************************************************************************
 *������: convert_ing_file()
 *����  : ��һ��.ING�ļ�ת��Ϊ.AVI�ļ�
 *����  : db        ��sqlite2��open���صľ��
 *        ingfile   ing�ļ���
 *���  : finalname ת������ļ���
 *����ֵ: ��ȷ����0�����󷵻ظ�ֵ
 * ***************************************************************************/
int convert_ing_file(IN sqlite3 *db, IN char *ingfile, OUT char * finalname)
{
    char aviname[200];//xx_L00.AVI
    char oldaviname[200];//xx_L00_OLD.AVI
    char oldname[200];
    char tmp[50];
    struct stat fstate;
    long freesize;
    int time;
    char *lp,*lk;
    int ret;
    struct file_info_struct info;
    
  //��������Ч��
    if((ingfile == NULL)||(finalname == NULL)||(db == NULL))
    {
        return 0;
    }
  //���ָ��ing�ļ�����Ч��
    if(access(ingfile,F_OK)!=0)//�ļ�������
    {
        gtloginfo("�����е�ing�ļ�%sʵ���Ѳ�����\n",ingfile);
        fileindex_del_file(db,ingfile);
        return 0;
    }
    printf("find ing file  name is %s\n",ingfile);
    gtloginfo("�ҵ�һ��ing�ļ�����Ϊ%s\n",ingfile);
    lp = strstr(ingfile,RECORDING_FILE_EXT);
    if(lp == NULL)
    {
        gtlogerr("[%s:%d]�����ļ���[%s]����\n",__FILE__,__LINE__,ingfile);
        return 0;
    }

    //zw-modified 2012-04-20
    ret=strlen(lp);
    if(ret>4)
    {
        // �˴�Ϊps¼���ļ�
        *lp='\0';
        sprintf(oldaviname,"%s_OLD%s",ingfile,"-ps.mpg");     //oldaviname=xxxx_OLD-ps.mpg
        sprintf(aviname,"%s%s",ingfile,"-ps.mpg");            //aviname=xxxxx-ps.mpg
        strcat(ingfile,".ING");                   //ingfile=xxxx.ING-ps.mpg
        strcat(ingfile,"-ps.mpg");
    } 
    else if(ret>0)
    {
        //�˴�Ϊһ���xxxx.ING�ļ�
        *lp='\0';
        sprintf(oldaviname,"%s%s",ingfile,OLD_FILE_EXT);  //oldaviname=xxxx_OLD.AVI
        sprintf(aviname,"%s%s",ingfile,IMG_FILE_EXT);     //aviname=xxxx.GAVI
        strcat(ingfile,RECORDING_FILE_EXT);               //ingfile=xxxx.ING
    }

    printf("[%s:%d]ingfile=%s\n",__FILE__,__LINE__,ingfile);

    //��xxx_OLD.AVI�ļ���д�����ݿ⣬�����ļ��������Ϊxxx_OLD.AVI
    fileindex_add(db,oldaviname);
    ret = rename(ingfile,oldaviname);//�Ȱ�ing�ļ�������Ϊxxx_OLD.AVI�� xxxx_OLD-ps.mpg
    if(ret!=0)
    {
        printf("��%s������Ϊ%sʧ�ܷ���%d,�޷�����\n",ingfile,oldaviname,ret);
        gtloginfo("��%s������Ϊ%sʧ�ܷ���%d,�޷�����\n",ingfile,oldaviname,ret);
        fileindex_del_file(db, oldaviname);
        return 0;
    
    }
  //ɾ�����ݿ�����Ϊxxxx.ING���ļ���¼,��Ϊ����ļ��Ѿ�����Ϊxxx_OLD.AVI,�����Ѿ����������ݿ��¼
    fileindex_del_file(db,ingfile);
    //�����жϣ����ʣ��ռ�С�ڸ��ļ��Ĵ�С���򲻽�������
    ret=stat(oldaviname,&fstate);
    if(ret<0)
    {
        printf("ȡ%s�ļ�״̬ʧ�ܷ���%d,�޷�����\n",oldaviname,ret);
        gtloginfo("ȡ%s�ļ�״̬ʧ�ܷ���%d,�޷�����\n",oldaviname,ret);
        return 0;
    }
  //��¼���ļ��������н����ļ�������    
    hdutil_filename2finfo(oldaviname, &info);
    freesize = get_disk_free(info.partition);
    if(freesize < (fstate.st_size>>20))
    {
        
        printf("%sʣ��ռ�%ldM����,�ݲ������ļ�%s\n",info.partition,freesize,oldaviname);
        gtloginfo("%sʣ��ռ�%ldM����,�ݲ������ļ�%s\n",info.partition,freesize,oldaviname);
        return 0;
    }

    lp=strstr(oldaviname,"ps");
    if(lp!=NULL)
    {
        //��ʱΪps���ļ� 
        time=22;
    }   
    else
    {
        //��ʱΪavi��׺����ͨ�ļ�
        //����ת��
        time=avi_fix_bad_file(oldaviname);  //��ԭ�����ļ������avi��ʽ���������ܱ�����������,��Ϊ��Щ�ļ��Ǵ��ڷ�����ֹͣ¼��ģ���׺ΪING
    }
    
    if(time>=0) //ת���ɹ�,ȡʱ�䳤�ȣ�������OLD.AVIΪ.AVI
    {
        lp=index(aviname,'L');
        if(lp==NULL)
        {
            printf("invalid file name\n");
        }
        else
        {
            lp++;                 //lp=00_T00.AVI
            printf("[%s:%d]lp=%s\n",__FILE__,__LINE__,lp);
            lk=index(lp,'_');     //lk=_T00.AVI
            printf("[%s:%d]lk=%s\n",__FILE__,__LINE__,lk);
            strncpy(tmp,lk,40);   //tmp=_T00.AVI
            *lp='\0';
            printf("[%s:%d]lp=%s\n",__FILE__,__LINE__,lp);
            sprintf(finalname,"%s%02d%s",aviname,time,tmp);
            printf("[%s:%d]finalname=%s\n",__FILE__,__LINE__,finalname);
            fileindex_add(db,finalname);
            if(rename(oldaviname,finalname)==0)
            {
                fileindex_del_file(db,oldaviname);
            }
            else
            {
                fileindex_del_file(db,finalname);
            }
        }
    }   
    else//ת��ʧ�ܣ�������״
    {
        sprintf(finalname,oldaviname);
        printf("����ING�ļ� %s failed ret=%d!\n",oldaviname,time);  
        gtloginfo("convert_ing_to_avi %s failed ret=%d!\n",oldaviname,time);
    }
    return 0;
}


int convert_ing_callback(void *db, int argc, char **value, char **name)
{
	char fname[100];
	char finalname[100];
	char *filename;	
	
	if(db==NULL)
		return 0;
	
	return convert_ing_file((sqlite3*) db,value[0],finalname);
}



int fileindex_convert_ing(char *mountpath)
{
	sqlite3 *db = NULL;
	int ret;
	char sqlcmd[200];
	
	if(mountpath == NULL)
		return -EINVAL;
	
	//fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            //gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
	sprintf(sqlcmd,"select * from %s where ing = '1'",INDEX_TABLE_NAME);
	
	ret =exec_sqlcmd(db,sqlcmd, convert_ing_callback, db);
	//fileindex_close_db(mountpath, db);
	return 0;
}


int queryindex_callback(void *fp, int argc, char **value, char **name)
{
	char fname[100];
	char finalname[100];
	char *filename;	
	int len;

	//printf("come to queryindex callback with %s! strlen %d\n",value[0],strlen(value[0]));
	
	if(fp==NULL)
		return 0;
	
	len = strlen(value[0]);
	strncpy(fname,value[0],len);
	fname[len]='\0';
	if(access(fname,F_OK)!=0)//������
	{
		return 0;
	}
	strncpy(finalname,fname+7,len-7);
	finalname[len-7]='\0';
	fprintf((FILE *)fp,"%s\n",finalname);
	return 0;
}

int fileindex_query_index(char *mountpath, struct query_index_process_struct *qindex)
{
	sqlite3 *db;
	int ret = 0;
	char sqlcmd[200];
	char cmd[20];
	
	if((mountpath==NULL)||(qindex==NULL))
		return -EINVAL;
	//ret = fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] ���ݿ�%s :������!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
    
	sprintf(sqlcmd,"select name from %s where  start <= %d and stop >= %d  and ing = 0 ",INDEX_TABLE_NAME,qindex->stop,qindex->start);
	if(qindex->trig_flag != -1)
	{
		sprintf(cmd," and trig = %d",qindex->trig_flag);
		strcat(sqlcmd,cmd);
	}
	
	if(qindex->ch != -1)
	{
		sprintf(cmd," and ch = %d",qindex->ch);
		strcat(sqlcmd,cmd);
	}
	exec_sqlcmd(db,sqlcmd, queryindex_callback,qindex->index_fp);
	//fileindex_close_db(mountpath,db);
	return 0;
}









