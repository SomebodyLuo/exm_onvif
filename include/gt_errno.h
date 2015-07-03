#ifndef GT_ERRNO_H
#define GT_ERRNO_H
#include <stdio.h>
#include <errno.h>


#define	GT_ERRNO_BASE		1000		///<��ͨ�Զ��������Ļ���
#define E_ACCOUNT		(GT_ERRNO_BASE +  0)	///�˺Ż��������
#define E_NOINIT		(GT_ERRNO_BASE +  1)	///û�г�ʼ��
#define E_CHANNEL		(GT_ERRNO_BASE +  2)	///ͨ���Ŵ���
#define E_MAXUSER		(GT_ERRNO_BASE +  3)	///�û����ﵽ���
#define E_MACHVER		(GT_ERRNO_BASE +  4)	///�汾��ƥ��
#define E_MACHTYPE		(GT_ERRNO_BASE +  5)	///���͡��ͺŲ�ƥ��
#define E_READ			(GT_ERRNO_BASE +  6)	///��ȡʧ��
#define E_WRITE			(GT_ERRNO_BASE +  7)	///д��ʧ��
#define E_DATA			(GT_ERRNO_BASE +  8)	///���ݴ���
#define E_ORDER			(GT_ERRNO_BASE +  9)	///���ô������
#define E_STATE			(GT_ERRNO_BASE + 10)	///״̬����
#define E_DISK			(GT_ERRNO_BASE + 11)	///���̴���
#define E_OPERATE		(GT_ERRNO_BASE + 12) 	///����ʧ��
#define E_NORESOURCE		(GT_ERRNO_BASE + 13) 	///��Դ����
#define E_OPEN			(GT_ERRNO_BASE + 14)	///����Դ���ļ�ʧ��
#define E_FORMAT		(GT_ERRNO_BASE + 15)	///��ʽ����
#define E_UPDATE		(GT_ERRNO_BASE + 16)	///����ʧ��
#define E_MACHIP		(GT_ERRNO_BASE + 17)	///IP��ַ��ƥ��
#define E_MACHMAC		(GT_ERRNO_BASE + 18)	///MAC��ַ��ƥ��
#define E_UPDATEFILE		(GT_ERRNO_BASE + 19)	///�����ļ���ƥ��
#define E_UNKNOW		(GT_ERRNO_BASE + 20)	///δ֪����
				












#ifdef _WIN32
   // error code mapping for windows
#undef  EINTR
#define EINTR 					WSAEINTR			/* Interrupted system call */

#define ETIME                   ERROR_SEM_TIMEOUT	/* Timer expired */
#define EWOULDBLOCK             WSAEWOULDBLOCK		/* Operation would block */
#define EINPROGRESS             WSAEINPROGRESS		/* Operation now in progress */
#define EALREADY                WSAEALREADY			/* Operation already in progress */
#define ENOTSOCK                WSAENOTSOCK			/* Socket operation on non-socket */
#define EDESTADDRREQ            WSAEDESTADDRREQ		/* Destination address required */
#define EMSGSIZE                WSAEMSGSIZE			/* Message too long */
#define EPROTOTYPE              WSAEPROTOTYPE		/* Protocol wrong type for socket */
#define ENOPROTOOPT             WSAENOPROTOOPT		/* Protocol not available */
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT	/* Protocol not supported */
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT	/* Socket type not supported */
#define EOPNOTSUPP              WSAEOPNOTSUPP		/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT            WSAEPFNOSUPPORT		/* Protocol family not supported */
#define EAFNOSUPPORT            WSAEAFNOSUPPORT		/* Address family not supported by protocol */
#define EADDRINUSE              WSAEADDRINUSE		/* Address already in use */
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL	/* Cannot assign requested address */
#define ENETDOWN                WSAENETDOWN			/* Network is down */
#define ENETUNREACH             WSAENETUNREACH		/* Network is unreachable */
#define ENETRESET               WSAENETRESET		/* Network dropped connection because of reset */
#define ECONNABORTED            WSAECONNABORTED		/* Software caused connection abort */
#define ECONNRESET              WSAECONNRESET		/* Connection reset by peer */
#define ENOBUFS                 WSAENOBUFS			/* No buffer space available */
#define EISCONN                 WSAEISCONN			/* Transport endpoint is already connected */
#define ENOTCONN                WSAENOTCONN			/* Transport endpoint is not connected */
#define ESHUTDOWN               WSAESHUTDOWN		/* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS            WSAETOOMANYREFS		/* Too many references: cannot splice */
#ifndef ETIMEDOUT
#define ETIMEDOUT               WSAETIMEDOUT		/* Connection timed out */
#endif
#define ECONNREFUSED            WSAECONNREFUSED		/* Connection refused */
#define ELOOP                   WSAELOOP			/* Too many symbolic links encountered */
#define EHOSTDOWN               WSAEHOSTDOWN		/* Host is down */
#define EHOSTUNREACH            WSAEHOSTUNREACH		/* No route to host */
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS			/* Too many users */
#define EDQUOT                  WSAEDQUOT			/* Quota exceeded */
#define ESTALE                  WSAESTALE			/* Quota exceeded */
#define EREMOTE                 WSAEREMOTE			/* Object is remote */
 // #define ENAMETOOLONG            WSAENAMETOOLONG	/* File name too long */


#endif

///���ش������Ӧ�������ַ���
///��Ҫʹ��gt_errno��
//
//char *gt_strerror(int err_no);
#endif
