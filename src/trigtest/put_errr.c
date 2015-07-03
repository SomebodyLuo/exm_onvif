#include <stdio.h>
#include "put_errr.h"

/**********************************************************************************************
* ������   :get_err_str()
* ����  :       ���ش������ַ���
* ����  :      errr	������					
*						
* ���  :       void        
* ����ֵ:   ����������ַ���
**********************************************************************************************/
char *get_err_str(int errr)
{
	if(errr<0)
		errr=-errr;

	switch(errr)
	{
		case ERR_NO:
			return STR_ERR_NO;

		case ERR_TEST:
			return STR_ERR_TEST;
			
		default:
			return STR_ERR_UNKNOW;
	}

	return NULL;
}

