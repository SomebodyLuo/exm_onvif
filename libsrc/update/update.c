/*

	�������ļ� by wsy
*/
///// lsk 2009-5-18
#ifdef  __cplusplus
extern "C" {
#endif


#include "update.h"
#include <commonlib.h>
#include <gt_errlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <file_def.h>
#include <devinfo.h>
#include <xmlparser.h>
#include <gtlog.h>


#define PROG_PATH 		"/conf"
#define VAR_TMP_PATH 	"/var/tmp"
#define HD_UPDATE_PATH  "/ip1004"

#define RAMDISK_DEV 	"/dev/mtd/4"
#define KERNEL_DEV 		"/dev/mtd/3"
#define UPFILE 			"up"   //�����ű��ļ�

/*������ĸ����������ָ����ʽ�����������ַ���*/
int generate_updatemsg(IN char * username, IN char * pswd, IN char *ftpip, IN int port,
      IN char *path, OUT char *updatemsg)
{
	if(updatemsg==NULL)
		return -1;
	sprintf(updatemsg, "wget -c ftp://%s:%s@%s:%d%s",username,pswd,ftpip,port,path);	
	return 0;
}

/*
	�������������xml�ļ���Device�ֶΣ��жϱ��豸�Ƿ���Ҫ����
	����ֵ:1:��Ҫ���� 0:����Ҫ���� -1:����
*/
int is_update_devtype(char * xmlfile)
{
	IXML_Document *document=NULL;
	IXML_NodeList *nodelist=NULL;
	IXML_Node 	  *node=NULL;
	IXML_Node	 *childnode=NULL;
//	IXML_Node	*child=NULL;
	DOMString	  value;
	char		   *devtype;
	
	DWORD bufdoc[20000];
	
	int number=0; //xml�ļ��к���Device�ֶεĸ���
	int i;
	int rc=0; //����ֵ
	
	if(xmlfile==NULL)
		return -1;
	
	devtype=get_devtype_str();
	if(devtype==NULL) //�豸û���ͺ��ַ���,����������˵
		return 1;
	//gtloginfo("test!!!���豸Ϊ %s\n",devtype); 

	document=(IXML_Document *)bufdoc;

	document=ixmlLoadDocument(xmlfile);
	if(document==NULL)
	{
		gtloginfo("xml�����������ļ�%sʧ��\n",xmlfile);
		return -1;
	}

	nodelist=ixmlDocument_getElementsByTagName(document,"Device");
	if(nodelist==NULL)
	{
		gtloginfo("xml������ȡdevice�ֶ�����ʧ��\n");
		return -1;
	}
	number=ixmlNodeList_length(nodelist);
	//gtloginfo("number is %d\n",number);
	if(number==0)
		return 0;
	
	for(i=0;i<number;i++)
	{
		node=ixmlNodeList_item(nodelist,i);
		if(node==NULL)
		{
			continue;
		}
		childnode=ixmlNode_getFirstChild(node);
		if(!childnode)
			continue;

	    value=ixmlNode_getNodeValue(childnode);
		if(value==NULL)
			{
				continue;
			}
		if(i==0)
			gtloginfo("������֧�ֵ��豸�б�: %d. %s\n",i+1,value);
		else
			gtloginfo("                      %d. %s\n",i+1,value);
		
		if(strncasecmp(value,devtype,100)==0) //����Ҫ��
			{
				rc=1;
				//break;        ///<remed by shixin Ϊ���ܹ������е��ͺż�¼��־
			}
	}

	return rc;
	
}


/*Զ���������
  �������� updatefilesize ��������С�����ֽ�Ϊ��λ
  �������� updatemsg  ������Ϣ,��ʽ "wget -c ftp://usr:pswd@192.168.1.160:8080/path/xxx.tar.gz"
  ��������ֵ 0Ϊ�ɹ�������Ϊ������(����),������������ֵ(��0- 0x1010)

	ERR_DVC_INVALID_NAME	0x1010  //�����ļ����ָ�ʽ����
	ERR_DVC_LOGIN_FTP    	0x1011  //�޷���¼ftp������
	ERR_DVC_NO_FILE      	0x1012  //ftp����������ָ�����ļ����û������޶�Ȩ��
	ERR_DVC_UNTAR        	0x1013  //��ѹ�ļ�ʧ��
	ERR_NO_SPACE         	0x1014  //�豸�洢�ռ䲻�����޷�����
	ERR_DVC_PKT_NO_MATCH	0x1015	//���������豸�ͺŲ�ƥ��
	ERR_EVC_NOT_SUPPORT		0x1006  //�豸�յ�һ����֧�ֵ�����
 
*/



/*  ������ check_update_space()
	����: ����ɵ���������Ŀ¼,�����̿ռ��Ƿ��㹻,�����update����Ŀ¼�ַ���
	����: updatefilesize(��λ:byte)
	���: updatedir,�����Ĺ���Ŀ¼,��"/hqdata/update"
	����ֵ: -ERR_NO_SPACE: �ռ䲻��
			RESULT_SUCCESS: �ɹ�
*/
int check_update_space(IN int updatefilesize, OUT char * updatedir)
{
	//lc do ������������  ����ʹ��/hqdata/update��ֱ�Ӳ鿴/ip1004/�´�С���������������ø�Ŀ¼����ѹ����update�ű���ִ�иýű���������������/conf/Ŀ¼��
	//system("rm -rf /hqdata/update/*");
	//system("rm -rf /var/tmp/update/*");
	//mkdir ("/hqdata/update",0755);
	//if((get_disk_free(HDSAVE_PATH)<5*(updatefilesize>>20))||(access(HDSAVE_PATH,R_OK)!=0)||(access(HDSAVE_PATH,W_OK)!=0))
	if((get_disk_free(HD_UPDATE_PATH)<3*(updatefilesize>>20))||(access(HD_UPDATE_PATH,R_OK)!=0)||(access(HD_UPDATE_PATH,W_OK)!=0))
	
	{
			//if(get_disk_free(VAR_TMP_PATH)<5*(updatefilesize>>20)) //changed by shixin from 10
			printf("������%ld�ֽ�,��Ҫ%dM,updateĿ¼%dM\n",updatefilesize,3*(updatefilesize>>20),get_disk_free(HD_UPDATE_PATH));
			gtloginfo("������%ld�ֽ�,��Ҫ%dM,updateĿ¼%dM\n",updatefilesize,3*(updatefilesize>>20),get_disk_free(HD_UPDATE_PATH));
			/*
			if(get_disk_free(VAR_TMP_PATH)<3*(updatefilesize>>20)) //changed by shixin from 10
			{
				printf("no enough space, cannot update\n");
				gtloginfo("���̿ռ䲻��,�޷�����\n");
	    		gtloginfo("������%ld�ֽ�,��Ҫ%dM,ʵ���ڴ�%dM��updateĿ¼%dM\n",updatefilesize,(updatefilesize>>20)*3,get_disk_free(VAR_TMP_PATH),get_disk_free(HD_UPDATE_PATH));
	    		return -ERR_NO_SPACE;
			}
			else 
			{	
				sprintf(updatedir,"%s/%s",VAR_TMP_PATH,"update");
				printf("work in %s instead\n",VAR_TMP_PATH);
			}
			*/
			return -ERR_NO_SPACE;
	}
	else 
		sprintf(updatedir,"%s",HD_UPDATE_PATH);

	printf("���̿ռ����,��%sĿ¼��������\n",updatedir);
	gtloginfo("���̿ռ����,��%sĿ¼��������\n",updatedir);
	
	
	return RESULT_SUCCESS;

}

void kill_apps4update(void)
{
    gtloginfo("Ϊ������������Ӧ�ó���,��ͣͼ���¼�����...\n");
    printf("Ϊ������������Ӧ�ó���,��ͣͼ���¼�����...\n");
 
	system("killall -15 watch_proc");
	system("killall -15 rtimage");
	system("killall -15 encbox");
	system("killall -15 diskman");
	system("killall -15 hdmodule");
	system("ipcrm -M 0x30000");
	system("ipcrm -M 0x30001");
	system("ipcrm -M 0x30002");
	system("ipcrm -M 0x30003");
	system("ipcrm -M 0x30004");
	system("ipcrm -M 0x30005");
    
}

int update_software(IN int updatefilesize,IN char *updatemsg,int interval)
{
	int errortimes=0;	
	int errortype=0;
	char *lp,*lk;
	int i;
	char update[50]; // /ip1004/ ����Ŀ¼
	char temp[100]; // //var/tmp/update/temp ,��ѹĿ¼
	char filename[100]; //ѹ�����ļ���
	char contentname[100];//update1.58-xx-xxx.txt,��ѹ��Ϣ��¼�ļ�
	char xmlname[100];
	char makedir[100];
	char untar[100];
	char mv[100];
	char rmupdate[100];
	char up[100];
	char upname[100];
	char buf[2000];
	char tempcmd[100];
	FILE *fp;
	char xmlpath[200];
	struct stat filebuf;
	int is_devtype=0;//�Ƿ���xml��ָ����devtype


	//0.�������Ŀ¼
	memset(tempcmd,0,100);
	sprintf(tempcmd,"mkdir -p %s","/ip1004/");
	i=system(tempcmd);   
	sprintf(tempcmd,"mkdir -p %s/temp","/ip1004");  
	i=system(tempcmd);
	i=chdir("/ip1004/");
	sprintf(tempcmd,"cp /ip1004/hardreboot /tmp/ -frv");
	system(tempcmd);
	sprintf(tempcmd,"cp /ip1004/hwrbt /tmp/ -frv");
	system(tempcmd);
	sprintf(tempcmd,"rm -rf %s/*","/ip1004");
	system(tempcmd);
	//1.���μ����̿ռ��Ƿ��㹻�����㹻��ֱ�ӷ��ش�����,������tempΪ����Ŀ¼
	i = check_update_space(updatefilesize, update);
	if(i != RESULT_SUCCESS)
		return i;
	
	//2.�����ַ���������Ƿ�wget���(����Ƿ�.tar.gz�ļ�)
	//lp=strstr(updatemsg,".tar.gz");
	lk=strstr(updatemsg,"wget ");
	//if((lp==NULL)||((*(lp+7))!='\0')||(lk==NULL)||(lk!=updatemsg))  //wsy,�������չ��
	if((lk==NULL)||(lk!=updatemsg))
			{
			printf("invalid update msg name\n");
			gtloginfo("������updatemsg���ϸ�ʽ��ȡ������");
	    	return -ERR_DVC_INVALID_NAME;
		}

	//3.errortimes��0,��ʼѭ������
	while(errortimes<5)
	{
		//3.1 ͨ��ϵͳ������һ���ɾ���/ip1004/,����/ip1004/�¹���
		memset(tempcmd,0,100);
		sprintf(tempcmd,"cp /ip1004/hardreboot /tmp/ -frv");
		system(tempcmd);
		sprintf(tempcmd,"cp /ip1004/hwrbt /tmp/ -frv");
		system(tempcmd);
		sprintf(rmupdate,"rm -rf %s/*",update);  
		i=system(rmupdate);
		sprintf(makedir,"mkdir -p %s",update);
		i=system(makedir);   
		sprintf(makedir,"mkdir -p %s/temp",update);  
		i=system(makedir);
		i=chdir(update);

		//3.2 ��updatemsg��ȡ����Ӧ��filename,�����contentname,xmlname
		lp=rindex(updatemsg,'/');
		lp++;
		strcpy(filename,lp);
		sprintf(xmlname,"%s",lp);
//		lk=strstr(xmlname,".tar.gz");
//		*lk='\0';
		sprintf(contentname,"%s.txt",xmlname);
		sprintf(upname,"%s.%s",xmlname,UPFILE);
		strcat(xmlname,".xml");
		
		//3.3 ����tar��,����continue
#ifdef SHOW_WORK_INFO
	printf("updatemsg received: %s \n",updatemsg);		
#endif
		i=system(updatemsg);
		if(i!=0)
		{
			if(i==2)
					errortype=ERR_DVC_LOGIN_FTP;
			else
					errortype=ERR_DVC_NO_FILE;
			errortimes++;
			gtloginfo("��%d�����ز��ɹ�\n",errortimes);
			continue;
		}
		
		//3.4 ������ص��ļ�size
		stat(filename,&filebuf);
		if((int)filebuf.st_size!=updatefilesize)
			{
				printf("wrong size!!update %d, st_size %d\n",updatefilesize,(int)filebuf.st_size);
				gtloginfo("�����ļ���С����wrong size!!update %d, st_size %d\n",updatefilesize,(int)filebuf.st_size);
				errortimes++;
				errortype=ERR_DVC_WRONG_SIZE;
				continue;
			}

		//lc 2014-2-21 �ӽ����ļ���ʼ��ʱ��ֹͣ����������5���ӱ�Ϊ���
		//ʹ����������
		if(interval > 0)
			update_set_com_mode(1,interval);

        kill_apps4update();
        
		//3.5 ��ɹ����ص�.tar.gz�ļ���ִ�У�����ѹ
		chmod(filename,777);
		sprintf(temp,"%s/temp",update);
              //changed by shixin ,���������������ض���Ļ����п��ܳ��ֽ�ѹ��ʧ�ܵ����(��watch_proc ����vsmainʱ)
		sprintf(untar,"tar zxvf %s -C %s/temp 0</dev/null 1> %s 2>>%s",filename,update,contentname,contentname);			
		printf("ready to untar upfile...%s\n",untar);
		i=system(untar);
		if(i!=0) //untarʧ��
			{
				printf("untar failed i=%d\n",i);
				gtloginfo("��ѹ��%sʧ��,ret=%d\n",filename,i);
				errortimes++;
				errortype=ERR_DVC_UNTAR;
				continue;
			}
		else
		{
			errortype=0;
			break;
		}
	}		

	if(errortype!=0)//�����ѭ��ʧ��,�������˳�
		{
			gtlogerr("������ؽ�ѹ��ʧ��,����Ϊ%s\n",get_gt_errname(errortype));	
			return -errortype;
		}

	//4 ��ѹ�ɹ������Ƿ���xml
#ifdef SHOW_WORK_INFO
		printf("content %s, xml %s",contentname,xmlname);
#endif
		fp=fopen(contentname,"r+");
		fread(buf,1,2000,fp);
		fclose(fp);
		if(strstr(buf,"xml")==NULL)
			{
				gtlogerr("��ѹ��δ����xml�ļ�\n");
				
				return -ERR_EVC_NOT_SUPPORT;
			}

	//5 ��ѹ�ɹ������Ƿ���UPFILE�ļ�,�����Ƶ�/log/update/��
		chdir(temp);			 
		if(access(UPFILE,F_OK)!=0)
			{	
				gtlogerr("��ѹ��δ���������ű�\n");
				
				return -ERR_EVC_NOT_SUPPORT;
			}
		chmod(UPFILE,777);
		if(access("/log/update",F_OK)!=0);
			mkdir("/log/update",0755);
		
		sprintf(mv,"cp -f %s /log/update/%s",UPFILE,upname);
		system(mv);

	//6 �������ű��ļ����Ƹ�����/log/update/�£�������־

		sprintf(mv,"mv -f ../%s /log/update/",contentname);
		system(mv);
		sprintf(mv,"mv -f updatedesp.xml /log/update/%s",xmlname);
		system(mv);

		gtloginfo("�յ����������%s,�����ļ���/log/update/%s\n",filename,xmlname);


//7 ����Ƿ�ø����豸���� wsy add in Nov 2006
		
		
		sprintf(xmlpath,"/log/update/%s",xmlname);
		is_devtype=is_update_devtype(xmlpath);
		//gtloginfo("test,is_devtype�����%d\n",is_devtype);
		if(is_devtype!=1)
			{
				gtlogerr("�����豸�ͺŲ�����,���豸��%s\n",get_devtype_str());
				return -ERR_DVC_PKT_NO_MATCH;
			}
		else
			{
				gtloginfo("�����豸�ͺŷ���,��������\n");
			}
		
	//8 ִ������
	    system("chmod +x ./up");//shixin added 
		sprintf(up,"/bin/sh %s 1>>/log/update/%s 2>>/log/update/%s",UPFILE,contentname,contentname);
		i=system(up);
		if(i!=0)
		{	
			gtlogwarn("ִ�������ű�����,����%d\n",i);
			return -ERR_EVC_NOT_SUPPORT;
		}
		i=system(rmupdate);
		return 0;
		
	
}

/*
	����: �������tar.gz�ļ���ѹ������ִ������
	����ֵ: ��������

*/
int direct_update_software(IN char* gzfilename, IN char *updatedir )
{
	char temp[100];
	char cmd[200];
	char contentname[100];
	char xmlname[100];
	char upname[100];
	BYTE buf[2000];
	char xmlpath[100];
	char *lk;
	FILE *fp;
	int result=0;
	
	chmod(gzfilename,777);

	sprintf(cmd,"rm -rf %s/temp",updatedir);
	system(cmd);

	sprintf(temp,"%s/temp",updatedir);
	sprintf(cmd, "mkdir -p %s",temp);
	system(cmd);

	//lc do �ڳ��ڴ�ռ�
	printf("Ϊ/ip1004/�ڳ��ռ� begin!\n");
	system("killall -15 watch_proc");
	system("killall -15 rtimage");
	system("killall -15 encbox");
	system("killall -15 diskman");
	system("killall -15 hdmodule");
	system("ipcrm -M 0x30000");
	system("ipcrm -M 0x30001");
	system("ipcrm -M 0x30002");
	system("ipcrm -M 0x30003");
	system("ipcrm -M 0x30004");
	system("ipcrm -M 0x30005");
	printf("Ϊ/ip1004/�ڳ��ռ� end!\n");

	sleep(5);

	chdir(temp);
	//���xmlname, contentname, upname
	/* /ip1004/up_direct_%04d%02d%02d.tar.gz */
	sprintf(xmlname,"%s",rindex(gzfilename,'/')+1);
	lk=strstr(xmlname,".tar.gz");
	*lk='\0';                           /*xmlname="up_direct_xxx"*/
	sprintf(contentname,"%s.txt",xmlname);    /*contentname="up_direct_xxx.txt"*/
	sprintf(upname,"%s.%s",xmlname,"up");   /*upname="up_direct_xxx.up"*/
	strcat(xmlname,".xml");             /*xmlname="up_direct_xxx.xml"*/
	
	
	
   	sprintf(cmd,"tar zxvf %s -C %s 0</dev/null 1> %s 2>>%s",gzfilename,temp,contentname,contentname);			
	printf("ready to untar upfile...%s\n",cmd);
	//sprintf(cmd,"tar zxvf %s -C %s %s 0</dev/null 1> %s 2>>%s",gzfilename,temp,,contentname,contentname);			
	//printf("ready to untar upxmlfile...%s\n",cmd);
	result=system(cmd);

	
	if(result!=0) //untar ʧ��
	{
		printf("untar failed ret=%d\n",result);
		gtlogerr("��ѹ��%sʧ��,ret=%d\n",gzfilename,result);
		return ERR_DVC_UNTAR;
	}
	
#ifdef SHOW_WORK_INFO
	printf("content %s, xml %s",contentname,xmlname);
#endif
	fp=fopen(contentname,"r+");
	fread(buf,1,2000,fp);
	fclose(fp);
	if(strstr((const char*)buf,"xml")==NULL)	// lsk 2009 -5-18
	{
		gtlogerr("��ѹ��δ����xml�ļ�\n");
		return ERR_EVC_NOT_SUPPORT;
	}

//��ѹ�ɹ������Ƿ���UPFILE�ļ�,�����Ƶ�/log/update/��
	chdir(temp);
	system("pwd");
	if(access(UPFILE,F_OK)!=0)
	{	
		gtlogerr("��ѹ��δ���������ű�\n");
		return ERR_EVC_NOT_SUPPORT;
	}
	chmod(UPFILE,777);
	if(access("/log/update",F_OK)!=0);
	mkdir("/log/update",0755);

	sprintf(cmd,"cp -f %s /log/update/%s",UPFILE,upname);
	system(cmd);

// �������ű��ļ����Ƹ�����/log/update/�£�������־

	sprintf(cmd,"mv -f ./%s /log/update/",contentname);
	system(cmd);
	sprintf(cmd,"mv -f updatedesp.xml /log/update/%s",xmlname);
	system(cmd);

	gtloginfo("���������ļ���/log/update/%s\n",xmlname);


	// ����Ƿ�ø����豸���� 
		
		
	sprintf(xmlpath,"/log/update/%s",xmlname);

	//gtloginfo("test,is_devtype�����%d\n",is_devtype);
	if(is_update_devtype(xmlpath)!=1)
	{
		gtlogerr("�����豸�ͺŲ�����,���豸��%s\n",get_devtype_str());
		return ERR_DVC_PKT_NO_MATCH;
	}
	else
	{
		gtloginfo("�����豸�ͺŷ���,��������\n");
	}
		
	// ִ������
	system("pwd");
	chdir(temp);
    system("chmod 777 up");//lc added 
    //system("cd ./ip1004/");
	sprintf(cmd,"/bin/sh %s 1>>/log/update/%s 2>>/log/update/%s",UPFILE,contentname,contentname);
	result=system(cmd);
	if(result!=0)
	{	
		gtlogwarn("ִ�������ű�����,����%d\n",result);
		return ERR_EVC_NOT_SUPPORT;
	}
		
	return 0;
}
#ifdef  __cplusplus
}
#endif

