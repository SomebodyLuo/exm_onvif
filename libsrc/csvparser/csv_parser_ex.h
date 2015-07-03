#ifndef CSVPARSER_EX_H
#define CSVPARSER_EX_H

#include "csvparser.h"

typedef  struct {
char filename[100]; 
}csv_dict ;      //fixme later

/**
  @brief    ����csv�ļ������ݽṹ������ס�ļ�
  @param    csvfile:Ҫ�򿪵�csv�ļ���
  	    	wait:����������ļ��Ѿ�������������Ƿ���еȴ� 1��ʾ�ȴ� 0��ʾֱ���˳�
  @return   csv_dict�ṹ ,NULL��ʾ���� 
**/
csv_dict * csvparser_load_lockfile(char * csvfile,int wait);



/**
  @brief    ��csvdict���ݽṹ���µ������ļ���������
  @param    filename:Ҫ�洢�������ļ���
  	    	csvdict:�Ѿ����õ����ݽṹ
  	     
  @return   0��ʾ�ɹ���ֵ��ʾ����
**/
int csvparser_savefile (char *filename,csv_dict *csvdict);


/************************************************************************
*������	:csvparser_get_total_records
*����	:��ȡһ��csv_dict���ݽṹ�е�csv��¼����
*����	:  	 IN csv_dict * csvdict ;	//csv_dict���ݽṹ
*����ֵ	:�Ǹ�ֵ��ʾ�ɹ���ֵ��ʾʧ��
*�޸���־:
*************************************************************************/
int csvparser_get_total_records(csv_dict * csvdict);

/************************************************************************
*������	:csvparser_get_record
*����	:��csv_dict���ݽṹ�л�ȡָ����¼�ŵĽṹ��Ϣ
*����	:
	 IN csv_dict * csvdict;	//csv_dict���ݽṹ
	 IN int record_no;		//csv�ṹ�еļ�¼��(��ֵ��ʾ���������,-1��ʾ������һ��)
*��� 	:OUT CSV_T *csv;		//ָ����¼�ŵļ�¼��Ϣ
*����ֵ	:0��ʾ�ɹ���ֵ��ʾʧ�� 
*************************************************************************/
int csvparser_get_record(csv_dict * csvdict,int record_no, CSV_T *csv);

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
int csvparser_set_record(csv_dict * csvdict,int record_no, IN CSV_T *csv);

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
int csvparser_insert_record(csv_dict * csvdict,int record_no,IN CSV_T *csv);

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
int csvparser_rm_record(csv_dict * csvdict,int record_no);



#endif

