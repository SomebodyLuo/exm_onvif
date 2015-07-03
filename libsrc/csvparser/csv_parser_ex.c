/* csvparser�����չ��

Ŀ�����ṩһ�׽ṹ�ͽӿڣ��Ա㽫csvfile�����������ڴ��У�
���ڴ��н��в�ѯ���޸ĵȲ����������һ����д���ļ���
�����ܽϴ���ȵ����Ч�ʣ�

����ڴ�ʹ��������Ӧ�����ڴ�ӳ���ļ���

*/
#include "csv_parser_ex.h"
#include "csvparser.h"
#include "stdlib.h"



/**
  @brief    ����csv�ļ������ݽṹ������ס�ļ�
  @param    csvfile:Ҫ�򿪵�csv�ļ���
  	    	wait:����������ļ��Ѿ�������������Ƿ���еȴ� 1��ʾ�ȴ� 0��ʾֱ���˳�
   @return   csv_dict�ṹ ,NULL��ʾ���� 
**/
csv_dict * csvparser_load_lockfile(char * csvfile,int wait)
{
	//fixme later
	csv_dict *csvdict =NULL;
	csvdict = (csv_dict *)malloc(sizeof(csv_dict));	
	strncpy(csvdict->filename, csvfile,strlen(csvfile));
	csvdict->filename[strlen(csvfile)]='\0';
	return csvdict; 
	
}



/**
  @brief    ��csvdict���ݽṹ���µ������ļ���������
  @param    filename:Ҫ�洢�������ļ���
  	    	csvdict:�Ѿ����õ����ݽṹ
  	     
  @return   0��ʾ�ɹ���ֵ��ʾ����
**/
int csvparser_savefile (char *filename,csv_dict *csvdict)
{
	if(csvdict != NULL)
		free(csvdict);
	return 0;
}


/************************************************************************
*������	:csvparser_get_total_records
*����	:��ȡһ��csv_dict���ݽṹ�е�csv��¼����
*����	:  	 IN csv_dict * csvdict ;	//csv_dict���ݽṹ
*����ֵ	:�Ǹ�ֵ��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************/
int csvparser_get_total_records(csv_dict * csvdict)
{
	return csvfile_get_total_records(csvdict->filename);

}

/************************************************************************
*������	:csvparser_get_record
*����	:��csv_dict���ݽṹ�л�ȡָ����¼�ŵĽṹ��Ϣ
*����	:
	 IN csv_dict * csvdict;	//csv_dict���ݽṹ
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,-1��ʾ������һ��)
*��� 	:OUT CSV_T *csv;		//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ�� 
*************************************************************************/
int csvparser_get_record(csv_dict * csvdict,int record_no, CSV_T *csv)
{
	return csvfile_get_record(csvdict->filename,record_no, csv);
}

/************************************************************************
*������	:csvparser_set_record
*����	:��csv�ṹд��csv_dict���ݽṹ��
*����	:  
	 IN csv_dict * csvdict;	//csv_dict���ݽṹ
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 				//����ʱ����������������ʾ���ļ�β������
					//����ʱ���������Χ���ʾ���ײ�����
* 	 IN CSV_T *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************/
int csvparser_set_record(csv_dict * csvdict, int record_no, IN CSV_T *csv)
{
	return csvfile_set_record(csvdict->filename,record_no, csv);

}

/************************************************************************
*������	:csvparser_insert_record
*����	:��csv�ṹ����csv_dict���ݽṹ��
*����	:  
	 	IN csv_dict * csvdict;	//csv_dict���ݽṹ
	 IN int record_no;			//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 						//����ʱ����������������ʾ���ļ�β������
							//����ʱ���������Χ���ʾ���ײ�����
* 	 IN CSV_T *csv;			//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************/
int csvparser_insert_record(csv_dict * csvdict, int record_no, IN CSV_T *csv)
{
	return csvfile_insert_record(csvdict->filename, record_no, csv);
}

/************************************************************************
*������	:csvparser_rm_record
*����	:��filename�ļ��еĵ�record_no����Ϣɾ����
*����	:  
		IN csv_dict * csvdict;	//csv_dict���ݽṹ
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,��ֵ��ʾ���������)
	 				//����ʱ����������������ʾ���ļ�β��ɾ��
					//����ʱ���������Χ���ʾ���ײ�ɾ��
*����ֵ	:0��ʾ�ɹ�����ֵ��ʾʧ��
*�޸���־:
*************************************************************************/
int csvparser_rm_record(csv_dict * csvdict, int record_no)
{
	return csvfile_rm_record(csvdict->filename,record_no);
}


