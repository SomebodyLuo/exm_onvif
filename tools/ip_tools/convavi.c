//#define _MAKE_WINDOWS
#ifndef _MAKE_WINDOWS
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#else
#include "stdio.h"
//#include "conio.h"
#include "string.h"
#include <sys/stat.h>
#include <stdlib.h>
#endif //_MAKE_WINDOWS

#define OK				0x00
#define ERR_OPENFILE	0x01
#define ERR_READFILE	0x02
#define ERR_WRITFILE	0x03
#define ERR_STATFILE	0x04
#define ERR_NOFILEIP	0x05
#define ERR_SIGNAL		0x06
#define ERR_OVERTIME	0x07
#define ERR_OTHER		0x99

#define	BYTE	unsigned char
#define	WORD	unsigned short
#define	DWORD	unsigned long

#define ADDRESS_HEAD1		0x00000000
#define ADDRESS_HEAD2		0x00000800

//AVI�ļ�����ͷ�ṹ,�������ݲμ�AVI�ļ���ʽ.doc
typedef struct avi_head1
{
    char FD01[4];    	//'RIFF'	
    DWORD fsize;        //�ļ���С
    char FD03[4];      //'AVI '	
    char FD04[4];        //'LIST'
    
    DWORD listlen;     	//List�ĳ���
    char FD06[4];     	//'hdrl'
    char FD07[4];     	//'avih'
    char FD08[4];     	//56��֮��Ķ���Ч�����ֽ�����
    
    DWORD FD09;   		//Microseconds per frame	
    char FD10[4];   		//MaxBytesPerSec	
    char FD11[4];   		//Reserved	
    char FD12[4];   		//�Ƿ��У��ã�index���
    
    DWORD TotalFrames; 		//TotalFrames����������
    char FD14[4]; 		//InitialFrames����ʼ����
    char FD15[4]; 		//�Ƿ�������	
    char FD16[4]; 		//SuggestedBufferSize
    		
    DWORD width;		//Width��ͼ���ȣ�	
    DWORD height;		//Height��ͼ��߶ȣ�	
    char FD19[4];		//TimeScale�����Ա�����	
    char FD20[4];		//DataRate�����Ա�����
         	
    char FD21[4];     	//StartTime�����Ա�����	
    char FD22[4];     	//DataLength�����Ա�����
    char FD23[4];     	//'LIST'	
    char FD24[4];     	//List�ĳ���
       		
    char FD25[4];   		//'strl'	
    char FD26[4];   		//'strh'	
    char FD27[4];   		//56��֮��Ķ���Ч�����ֽ�����	
    char FD28[4];   		//'vids'��type��
			
	char FD29[4];		//'���뷽ʽ'����DIVX��
	char FD30[4];		//Flags��0��
	char FD31[4];		//Reserved(0)
	char FD32[4];		//InitialFrames(0)
   	
   	char FD33[4];		//ms_per_frame��scale��	
   	char FD34[4];		//Rate	
   	char FD35[4];		//Start	
   	DWORD video_frames;		//video_frames��Length��
      		
    char FD37[4];  		//SuggestedBufferSize	
    char FD38[4];  		//Quality	
    char FD39[4];  		//SampleSize	
    char FD40[4];  		//Frame
          	
    char FD41[4];      	//Frame	
    char FD42[4];      	//'strf'	
    char FD43[4];      	//40��֮��Ķ���Ч�ֽ�����	
    char FD44[4];      	//40��Size��
    		
    char FD45[4];		//Width��ͼ���ȣ�	
    char FD46[4];		//Height��ͼ��߶ȣ�	
    char FD47[4];		//Planes	Count	
    char FD48[4];		//'���뷽ʽ'����divx��
  			
  	char FD49[4];		//SizeImage��Width* Height����	
  	char FD50[4];		//XPelsPerMeter	
  	char FD51[4];		//YPelsPerMeter	
  	char FD52[4];		//Number of colors used
    		
    char FD53[4];		//Number of colors important	
    char FD54[4];		//'LIST'	
    char FD55[4];		//Length of list	
    char FD56[4];		//'strl'
      		
    char FD57[4]; 		//'strh'	
    char FD58[4];  		//56��֮��Ķ���Ч�����ֽ�����	
    char FD59[4];  		//'auds'	
    char FD60[4];  		//'\0\0\0\0'�����뷽ʽ��
           	
    char FD61[4];       	//Flags	
    char FD62[4];       	//Reserved	
    char FD63[4];       	//InitialFrames	
    char FD64[4];       	//Sampsize��scale��
  		   	
  	char FD65[4];	   	//sampsize*AVI->a_rate������	
  	char FD66[4];	   	//Start	
  	DWORD audiobytes;  		//AVI->audio_bytes/sampsize������	
  	char FD68[4];	   	//SuggestedBufferSize
     		
    char FD69[4]; 		//Quality	
    char FD70[4]; 		//SampleSize	
    char FD71[4]; 		//Frame1	
    char FD72[4]; 		//Frame2
			
	char FD73[4];		//'strf'	
	char FD74[4];		//20��֮��Ķε���Ч�����ֽ�����	
	char FD75[2];		//Format	
	char FD76[2];		//Channel num	
	char FD77[4];		//SamplesPerSec
   			
   	char FD78[4];		//AvgBytesPerSec	
   	char FD79[2];		//BlockAlign	
   	char FD80[2];		//BitsPerSample	
   	char FD81[2];		//cbSize	
   	char FD82[2];		//wSamplesPerBlock	
   	char junk[4];		//'JUNK'
}AVI_HEAD1;
#define LEN_AVI_HEAD1 sizeof( struct avi_head1 )

//AVI�ļ�����ͷ�ṹ,�������ݲμ�AVI�ļ���ʽ.doc
typedef struct avi_head2
{
	char list[4];		//LIST
	DWORD listlen;		//LIST LEN
	char movie[4];		//MOVIE
}AVI_HEAD2;
#define LEN_AVI_HEAD2 sizeof( struct avi_head2 )
#define AVI_FILE_SIZE_L	0x800+LEN_AVI_HEAD2//��С�ļ���С
	
//AVI�ļ�����β���ṹ
typedef struct index_entry
{
  DWORD key;
  DWORD unknow;//???
  DWORD pos;
  DWORD len;
} INDEX_ENTRY;
#define LEN_INDEX_ENTRY sizeof( struct index_entry )

#define MAX_AVI_BLOCK	45000//====1800*25

FILE * trace;

/*
wsy add
�������:INDEX_ENTRY����,����ֵ,�ܵ�avi����
*/
static DWORD GetTotalAviLen( INDEX_ENTRY ie[] )
{
	int i = 0;
	DWORD total = 0;
	
	for( i=0;i<MAX_AVI_BLOCK;i++ )
	{
		if( ie[i].len == 0 )
			break;
		total += (ie[i].len+8);
	}
	return total;
}

/*
wsy add
����:INDEX_ENTRY����,ƫ����re,���ض�Ӧ����Ԫ�ص�pos
*/
static DWORD GetIeLength( INDEX_ENTRY ie[], int re )
{
	if( re == 0 )
		ie[re].pos = 0x00000004;
	else
		ie[re].pos = ie[re-1].pos + ie[re-1].len + 8;
	return ie[re].pos;
}

/*
ת��avi�ļ�
�������:���ļ���ʵ�ʴ�Сlen
		 ���ļ���fp
		 ����ļ���fp
		 avi_time.txt��fp
����ֵ: ������
*/
static int ProcessAviFile( DWORD len, FILE * f_in, FILE * f_out,FILE * f_time )
{
	AVI_HEAD1 AH1;
	AVI_HEAD2 AH2;
	INDEX_ENTRY IE[MAX_AVI_BLOCK];
	BYTE temp[2048];
	DWORD file_len = len;
	DWORD repair_file_img_len = 0;
	DWORD index_len = 0;
	WORD repair_blocks = 0;//����Ԫ���±�
	int avi_time = 0;
	int i = 0;
	int ret = 0;
	char * avi_ptr = NULL;   //��ǰͼ�����Ƶ����
	static const char AVI_IMAGE_VIDEO_FLAG[] = "\x30\x30\x64\x63";
	static const char AVI_IMG_END_FLAG[] = "\x69\x64\x78\x31";
	
	printf( "Converting the avi file,be patient!!!\n" );
	//AVI�ļ�С����С����,�򲻿���װ��
	if( file_len <= AVI_FILE_SIZE_L)
	{
		printf( "File len is too small!\n" );
		fprintf( trace, "File len is too small!\n" );
		fflush( trace );
		return -1;
	}		
	//����avi�ļ�ͷ1 ���������Ϣ��ӡ��/avi.txt�� *wsyadd
	ret = fread( (void *)&AH1, LEN_AVI_HEAD1, 1, f_in );	
	if(ret <= 0 )
	{
		printf( "Read the conving avi file error!\n" );
		fprintf( trace, "Read the conving avi file error!\n" );
		fflush( trace );
		return ERR_READFILE;	
	}
	else
	{
		fprintf( trace,"The avi len is %04x %04x\n", (WORD)(AH1.fsize>>16),(WORD)(AH1.fsize));	
		fprintf( trace,"The avi frames is %ld\n", AH1.TotalFrames);	
		fprintf( trace,"The avi video_frames is %ld\n", AH1.video_frames);	
		fprintf( trace,"The avi audiobytes is %ld\n", AH1.audiobytes);	
		memcpy(temp,AH1.junk,4);temp[4]=0;
		fprintf( trace,"The avi junk is %s\n",temp );	
		fflush( trace );
	}
	
	//�����ļ�ͷ2���������Ϣ��ӡ��/avi.txt�� *wsyadd
	fseek( f_in, (long)ADDRESS_HEAD2, SEEK_SET );	
	ret = fread( (void *)&AH2, LEN_AVI_HEAD2, 1, f_in );
	if(ret <= 0 )
	{
		printf( "Read the conving avi file error!\n" );
		fprintf( trace, "Read the conving avi file error!\n" );
		fflush( trace );
		return ERR_READFILE;	
	}
	else
	{
		memcpy(temp,AH2.list,4);temp[4]=0;
		fprintf( trace,"\nThe avi list is %s\n", temp);	
		fprintf( trace,"The avi listlen is %04x %04x\n", (WORD)(AH2.listlen>>16),(WORD)(AH2.listlen&0xffff));	
		memcpy(temp,AH2.movie,4);temp[4]=0;
		fprintf( trace,"The avi movie is %s\n\n",temp );	
		fflush( trace );
	}

	//
	memset( IE, 0x00, MAX_AVI_BLOCK * LEN_INDEX_ENTRY );	
	//��һ����Ч�����ݿ�д������ļ�
	fseek( f_out, (long)AVI_FILE_SIZE_L, SEEK_SET );	
	while ( 1 )
	{
		ret = fread( (void *)temp, 6, 1, f_in );
		if(ret <= 0 )
		{
			break;
		}
		else		
		{
			if(!memcmp(temp,AVI_IMG_END_FLAG,1))
			{
				fprintf( trace,"Meet the img tail!!!\n" );	
				fflush( trace );
				break;			
			}
			else if( !memcmp(temp,AVI_IMAGE_VIDEO_FLAG,4) )
			{
				IE[repair_blocks].len = *(WORD *)&temp[4];
				IE[repair_blocks].pos = GetIeLength( IE,repair_blocks);
				repair_file_img_len = GetTotalAviLen( IE );	
//				fprintf( trace, "[%02x %02x %02x %02x %02x %02x]", temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);
				fflush( trace );
			}
			else
			{
				fprintf( trace,"Unknow image block!!!\n" );	
				fflush( trace );
				break;			
			}
		}
		
		if( file_len < AVI_FILE_SIZE_L + repair_file_img_len )
		{	
			repair_file_img_len -= (IE[repair_blocks].len+8);
			fprintf( trace,"Avi block[%d] is uncorrect\n",repair_blocks );	
			fflush( trace );
			break;
		}

		//IE[?].lenΪʵ�ʿ鳤��+����2
		avi_ptr = (char *)malloc( IE[repair_blocks].len + 8 );
		memcpy( avi_ptr, temp, 6 );
		fread( (void *)(avi_ptr+6), IE[repair_blocks].len+2, 1, f_in );
		
		//Write the block of image to the fp_out
		fwrite( (void *)avi_ptr, IE[repair_blocks].len+8, 1, f_out );
				
		repair_blocks++;
		if( repair_blocks == MAX_AVI_BLOCK )
		{
			printf( "Too big avi block we can repair,exit!!!" );
			fprintf( trace, "Too big avi block we can repair,exit!!!" );
			free( avi_ptr );
			return ERR_OTHER;
		}
	}	
/*
	//If there is no avi block to repair,then we exit!!!
	if(repair_blocks < 8) 
	{
		printf( "The avi block we can repair is less than 8,exit!!!" );
		fprintf( trace, "The avi block we can repair is less than 8,exit!!!" );
		free( avi_ptr );
		return ERR_OTHER;
	}
	else
	{
		repair_blocks -= repair_blocks % 8;
		IE[repair_blocks].len = 0;
		repair_file_img_len = GetTotalAviLen( IE );	
	}
*/
	fprintf( trace,"The repair avi img length %04x %04x,there are [%d] blocks\n", 
		(WORD)(repair_file_img_len>>16),(WORD)(repair_file_img_len&0xffff),repair_blocks );	
	fflush( trace );

	//����ļ�ͷ1
	AH1.fsize = AVI_FILE_SIZE_L + repair_file_img_len + 8 + repair_blocks*LEN_INDEX_ENTRY;
	AH1.fsize -= 8; //-HEAD - LENGTH
	AH1.TotalFrames = repair_blocks;
	AH1.video_frames = repair_blocks;
	AH1.audiobytes = 0;//???
	memcpy(AH1.junk,"JUNK",4);
	fprintf( trace,"Repair HEAD1, FSZIE is %04x %04x\n",(WORD)(AH1.fsize>>16),
		(WORD)(AH1.fsize&0xffff) );	
	fprintf( trace,"Repair HEAD1, TotalFrames is %04x %04x\n",(WORD)(AH1.TotalFrames>>16),
		(WORD)(AH1.TotalFrames&0xffff) );	
	fprintf( trace,"Repair HEAD1, video_frames is %04x %04x\n",(WORD)(AH1.video_frames>>16),
		(WORD)(AH1.video_frames&0xffff) );	
	fprintf( trace,"Repair HEAD1, audiobytes is %04x %04x\n",(WORD)(AH1.audiobytes>>16),
		(WORD)(AH1.audiobytes&0xffff) );	
	fprintf( trace,"Repair HEAD1, junk is JUNK\n" );	
	fflush( trace );

	//����ļ�ͷ2
	memcpy(AH2.list,"LIST",4);
	AH2.listlen = repair_file_img_len + 4;
	memcpy(AH2.movie,"movi",4);

	
	fseek( f_out, (long)0, SEEK_SET );	
	memset( temp, 0x00, 2048 );
	fwrite( (void *)temp, 2048, 1, f_out );
	//д�ļ�ͷ1
	fseek( f_out, (long)0, SEEK_SET );	
	fwrite( (void *)&AH1, LEN_AVI_HEAD1, 1, f_out );
	fwrite( (void *)"\xbc\x06", 2, 1, f_out );
	fseek( f_out, (long)ADDRESS_HEAD2, SEEK_SET );	
	//д�ļ�ͷ2
	fwrite( (void *)&AH2, LEN_AVI_HEAD2, 1, f_out );
	fseek( f_out, (long)AVI_FILE_SIZE_L+repair_file_img_len, SEEK_SET );	
	//д������־
	fwrite( (void *)AVI_IMG_END_FLAG, 4, 1, f_out );
	index_len = repair_blocks*LEN_INDEX_ENTRY;
	fwrite( (void *)&index_len, 4, 1, f_out );
	//��һд������
	for( i=0; i<repair_blocks; i++ )
	{
		if( !(i % 8) )
			memcpy( (char *)&IE[i], "\x30\x30\x64\x63\x01\x00\x00\x00", 8);
		else
			memcpy( (char *)&IE[i], "\x30\x30\x64\x63\x00\x00\x00\x00", 8);
		fwrite( (void *)&IE[i], LEN_INDEX_ENTRY, 1, f_out );	
	}
		
	//��ʱ��д��avi_time.txt
	avi_time = (int)(AH1.FD09 * AH1.TotalFrames / 1000000 );
	if( avi_time == 0 )
		avi_time = 1;
	fprintf( f_time,"%d\n", avi_time );

	fclose( f_in );
	fclose( f_out );
	fclose( f_time);

	free( avi_ptr );

	return OK;	
}

/***
 ������˵��:
 ת��AVI�ļ�,�û�����CONVAVI ./AAA.AVI ./BBB.AVI
 ����ת���𻵵�AVI�ļ�AAA.AVIΪ�ɹ���BBB.AVI�ļ�
 ***/
int main(int argc, char * argv[])
{ 
	struct stat sbuf;
	FILE * fp_avi_in;
	FILE * fp_avi_out;
	FILE * fp_avi_time;
#ifdef _MAKE_WINDOWS
	char input[128];
	char output[128];
#endif //_MAKE_WINDOWS
	int ret;
	char * file_avi_in = NULL;
	char * file_avi_out = NULL;

	const char Version[] = "[convavi(ver:1.01)]";

	//��ʾ�汾��  *wsyadd
	printf( "%s\n", Version );

	//����/tmp/aviĿ¼����/avi.txt , /avi_time.txt�ļ� *wsyadd
#ifndef _MAKE_WINDOWS
	system("mkdir /tmp/avi");
	trace = fopen("/tmp/avi/avi.txt", "w");
	fp_avi_time = fopen( "/tmp/avi/avi_time.txt", "w" );
#else
	trace = fopen("./avi.txt", "w");
	fp_avi_time = fopen( "./avi_time.txt", "w" );
#endif //_MAKE_WINDOWS
	if( fp_avi_time == NULL )
	{
		printf( "Cannot open avi_time.txt!!!\n" );
		fprintf(trace,"Cannot open avi_time file!!!\n");
 		return ERR_OPENFILE;
	}
	if(trace==NULL)
	{
		printf("Cannot opentrace file!!!\n");	
	}	

	//���������в���,����ȷ�򱨴��˳� *wsyadd
	if(argc>=3)
	{
      	file_avi_in = argv[1];
      	file_avi_out = argv[2];
	}
	else
	{
#ifdef _MAKE_WINDOWS
	  	strcpy(input,"./1.AVI");
	  	strcpy(output,"./repair.AVI");	  	
		file_avi_in = input;
     	file_avi_out = output;	  	
#else
  		fprintf(stderr,"Usage:Pls input file like [conavi source.avi dest.avi]!\n");
		fprintf(trace,"Usage:!Pls input like [convavi source.avi dest.avi],retcode=[%d]!\n",ERR_NOFILEIP);
	 	return ERR_NOFILEIP;
#endif //_MAKE_WINDOWS
	}

	//�ֱ������������avi�ļ� *wsyadd
	fp_avi_in=fopen(file_avi_in,"rb");
	if(fp_avi_in == NULL)
	{
		printf("Cannot open [%s] file!!!\n",file_avi_in);
		fprintf(trace,"Cannot open avi_in file!!!\n");
 		return ERR_OPENFILE;
	}    
	fp_avi_out=fopen(file_avi_out,"wb");
	if(fp_avi_out == NULL)
	{
		printf("Cannot open [%s] file!!!\n",file_avi_out);
		fprintf(trace,"Cannot open avi_out file!!!\n");
 		return ERR_OPENFILE;
	}    

	//��ȡ����/avi.txt��д�������ļ����ƣ����ȵ���Ϣ  *wsyadd
	fprintf( trace,"Stat file %s\n",file_avi_in);
    if ( fstat( fileno(fp_avi_in), &sbuf ) < 0 )
    {
		printf("Stat the file error,retcode=[%d]!\n",ERR_STATFILE);
		fprintf(trace, "Stat the file error,retcode=[%d]!\n",ERR_STATFILE);
	 	return ERR_STATFILE;
    }
	fprintf( trace, "The conving avi file's size is %ld\n", sbuf.st_size );

	//����ת�������ؽ�� *wsyadd
	ret = ProcessAviFile( sbuf.st_size, fp_avi_in, fp_avi_out, fp_avi_time );
	if( OK == ret )
		printf("Conv succeed!\n");
	else
		printf("Conv failed!\n");
	
	return (ret);
}	
