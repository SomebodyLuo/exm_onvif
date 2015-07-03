#ifndef WATCH_PROCESS_H
#define WATCH_PROCESS_H
#include <file_def.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
 #include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define	VERSION		"2.49"

//2.49  ���multichannelѡ��Ĳ�ͬ���������׳��򣬲�ͬ��watch_target
//2.48  �����playback����ļ��
// 2.47 �������߳�����ȡ��bug
//2.46  �������hdmodule��diskman�ļ��
// 2.45 ����ip1004�豸��ȡ��ĳЩ���̼�أ����߳�������
// 2.44 videoenc���߳�������6��������videoenc
// 2.43 diskman���̼߳���� 4->3
// 2.42 ͳ�������ĺ����ŵ���commonlib��
// 2.41 hdmodule��3->4
// 2.40 ֧�ֶ��ͺ�,ȥ����/log/vsftpd.txt�ļ��
// 2.12 ������Z�Ľ���Ҳ����Ϣ����/log/debug
// 2.11 �����vsftpd����־�ļ���С�ļ���
// 2.10 changed vsmain thread num 16->15
// 2.09 �Ľ����ж��߳����ķ�����
// 2.08 �����߳�����ʱ��sleep 1��Ȼ���ٶ�ȡһ��
// 2.07 ����֤mac��ַ�Ϸ��ԵĹ����ŵ�patch_conf�У���Ӧ�ó�������Ϊwatch_proc
// 2.06 ���뵱�߳���������ʱ����ʱ�������¼��־/log/debug
// 2.04 added warning info about USE_V1 when compile
// 2.03 ��������������ʱ������ signal(SIGCHLD,SIG_IGN);���µ��޷�����system������
// 2.02 �ı��˳�������˳��
// 2.02 close_all_res �����fix_disk; USEV1�궨��ɼ��v1����� wsy
// 2.01 ����ͨ���ж��߳��������жϽ����Ƿ��������еĹ���
// 2.00 �����ṹ����ض���ĳ�����
// 0.40 fix problem when other program not exist
// 0.39 ������vsmain�ĳ���Ҳ������һ������
// 0.38 fix close_all_res() from lib
// 0.37��������ʱ�Լ��ʱ������ļ�飬���С��1����1
//0.36 ��������ʱ������������ļ�
// 0.35 added diskman support
#define WATCH_PARA_FILE 	"/conf/ip1004.ini"
#define WATCH_LOCK_FILE	"/lock/ipserver/watch_process"



#endif
