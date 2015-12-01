//封装Socket一些常用操作
#ifndef _SL_SOCKET_UTILS_H_
#define _SL_SOCKET_UTILS_H_
#include "../slconfig.h"
#ifdef SL_OS_WINDOWS
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
	
	//WIN32下没有PF_LOCAL，为了能编译，用PF_INET代替
	#ifndef PF_LOCAL
		#define PF_LOCAL	PF_INET
	#endif
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
#endif
#include "../slsingleton.h"
#define		SL_ESOCKETEXIT	29999			///< 自己定义的表示要求socket退出
namespace sl
{
	//Socket常用函数类
	class CSocketUtils
	{
	public:
		static int NonblockSocket(SOCKET s)
		{
#ifndef SL_OS_WINDOWS
			int flag = 1;
			if(ioctl(s, FIONBIO, &flag) && 
				((flag = fcntl(s, F_GETEL, 0))< 0 || fcntl(s, F_SETEL, flag|O_NONBLOCK) < 0))
			{
				return errno;
			}
#else
			unsigned long ul = 1;
			if(ioctlsocket(s, FIONBIO, &ul))
			{
				return SL_WSA_ERRNO;
			}
#endif
			return 0;
		}

		//超时的单位是毫秒
		static int Select(SOCKET sock, bool rd, int iTimeOut)
		{
			struct timeval tv = {0};
			struct timeval tv2 = {0};
			struct timeval* ptv = NULL;

			int iLoop = 0;
			const int iSelectTime = 100; //100毫秒

			if(iTimeOut < 0) //阻塞
			{
				ptv = NULL;
				iLoop = 0x7FFFFFFF;
			}
			else
			{
				ptv = &tv2;
				if(iTimeOut >= iSelectTime)
				{
					iLoop = SL_CEIL(iTimeOut, iSelectTime);
					tv.tv_sec = iSelectTime / 1000;
					tv.tv_usec = (iSelectTime % 1000) * 1000;
				}
				else
				{
					iLoop = 1;
					tv.tv_sec  =  iTimeOut / 1000;
					tv.tv_usec = (iTimeOut % 1000) * 1000;
				}
			}

			fd_set fds;
			int iRet = 0;
			fd_set* prd = (rd ? &fds : NULL);
			fd_set* pwr = (rd ? NULL : &fds);
			int nfds = static_cast<int>(sock) + 1;
			for(; iLoop > 0; --iLoop)
			{
				if(ptv != NULL)
				{
					tv2.tv_sec = tv.tv_sec;
					tv2.tv_usec = tv.tv_usec;
				}

				FD_ZERO(&fds);
				FD_SET(sock, &fds);
				iRet = select(nfds, prd, pwr, NULL, ptv);
				if(iRet > 0)
				{
					return iRet;
				}
				else if(iRet == 0)
				{
					continue;
				}
				else
				{
					if(SL_WSA_ERRNO == EINTR)
					{
						continue;
					}
					return SL_WSA_ERRNO;
				}
			}
			return SL_ETIME;

		}

	}; // class CSocketUtils

}// namespace sl
#endif