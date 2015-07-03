/*
 * gt_netsdk ��һЩ��չ����ṹ����
 *
 */
#ifndef GT_NETSDK_EX_DEFINE
#define GT_NETSDK_EX_DEFINE

typedef struct {///�豸��ǰ�ı���״̬��Ϣ
	int	trig[32];			///���ӱ���״̬			0:���� 1:�б���
	int motion[32];			///�ƶ���ⱨ��״̬		0:����	1:�б���
}dev_alarm_stat_t;


typedef struct{///�豸Ӳ����Ϣ
	WORD    diskno;                 // Ӳ�̱��,0��1,2�����ȣ���Ӳ��ʱ��
    BYTE    model[16];              //Ӳ���ͺţ��ַ���    
    BYTE    serialno[16];           //Ӳ�����кţ��ַ���
    BYTE    firmware[8];            //�̼��汾�ţ��ַ���
    WORD    volume;                 //����(GΪ��λ����250G��320G)
    WORD    temprature;             //��ǰ�¶�(����)
	WORD    maxtemprature;			//��ʷ����¶�(����),ֵ��100������Ч
    DWORD   age;                    //����Сʱ��
    DWORD   relocate;               //�ط���������
    DWORD   pending;                //��ǰ����������
    DWORD   error_no;               //������־��
	int		shot_test_res;			///�̲��Խ�� 0ͨ����1ʧ��,2��������δ���Թ�,3������
	int		shortstatus;			/////�̲������ڽ����У���ɵİٷֱȣ�0-100������   
	int		long_test_res;			///�����Խ�� 0ͨ����1ʧ��,2��������δ���Թ�,3������
	int		longstatus;				//���������ڽ����У���ɵİٷֱȣ�0-100������   
}dev_hd_info_t;

typedef struct{///�豸��ǰ����״̬��Ϣ
	int		trig[32];			///<�������ӵ�״̬ 0:����״̬ 1:����״̬
	int		motion[16];			///<�ƶ�����0:���� 1:����״̬
}dev_trig_info_t;

typedef struct{///�豸��ע����Ϣ
		BYTE	dev_guid[8];			// 8�ֽڵ�guid
		BYTE	dev_guid_str[128];		// guid���ַ�����ʽ

        DWORD vendor;                   //�豸�����̱�ʶ(4) +�豸�ͺű�ʶ(4)
        DWORD device_type;              //�豸�ͺ�      
        BYTE site_name[40];             //��װ�ص�����  

        WORD video_num;					//�豸��Ƶ����ͨ������
        WORD com_num;                   // ������
        WORD storage_type;              //�豸�Ĵ洢�������� 0��û�� 1��cf�� 2��Ӳ��
        DWORD storage_room;				//�豸�Ĵ洢�������� ��λMbyte
        WORD compress_num;			    //ѹ��ͨ����    ĿǰΪ1��2��5
        WORD compress_type;				//ѹ���������ͣ�(ѹ��оƬ��ѹ����ʽ)
        WORD audio_compress;			//��������ѹ������
        WORD audio_in;                  //��������ͨ������ĿǰΪ1
        WORD switch_in_num;             //����������ͨ����
        WORD switch_out_num;			//���������ͨ����
       
		DWORD    cmd_port;              //�������˿�
        DWORD    image_port;            //ͼ�����˿�  
        DWORD    audio_port;            //��Ƶ����˿�
        BYTE    firmware[20];           //�̼��汾�ţ���ʱ����
        BYTE    dev_info[40];           //�豸��һЩ�����Ϣ
        BYTE    ex_info[160];           //���dvs(����еĻ�)�������Ϣ������Ʒ�ƣ��˿ڣ��û���������

		BYTE    video_names[64][30];    //���64·ͨ������

}dev_regist_info_t;





#endif
