///net配置类
/********************************************************************
	created:	2015/12/02
	created:	2:12:2015   21:01
	filename: 	e:\myproject\shyloo\netsvr\netconfig.h
	file path:	e:\myproject\shyloo\netsvr
	file base:	netconfig
	file ext:	h
	author:		ddc
	
	purpose:	net配置类
*********************************************************************/

#ifndef _NET_CONFIG_H_
#define _NET_CONFIG_H_

#include "netdef.h"
namespace sl
{
	class CNetConfig
	{
	public:
		CNetConfig(): ConfigName(FILE_CONFIG){}

		//共享内存
		CSizeString<SL_PATH_MAX>		MgrShmKey;
		
		//ShmQueue的配置
		CSizeString<SL_PATH_MAX>		FrontEndShmKey;		///< 前端共享内存的Key
		int								FrontEndShmSize;	///< 前端共享内存的大小
		CSizeString<SL_PATH_MAX>		FrontEndSocket;		///< 前端Net的UnixSocketFile
		CSizeString<SL_PATH_MAX>		BackEndSocket;		///< 后端的UnixSocketFile

		//缓冲区
		int								RecvBufferSize;		///< SOCKET缓冲区大小
		int								SendBufferSize;		///< SOCKET缓冲区大小
		int								BufferCount;		///< SOCKET缓冲区个数，Recv和Send的缓冲区都是BufferCount个

		//连接数
		int								SocketMaxCount;		///< 最大连接数

		//监听端口信息
		CArray<CNetListenInfo, NET_MAX_LISTEN> ListenArray;

		///流量控制配置
		int			MaxConnect;								///< 最大连接数
		int			NewConnCheckInterval;					///< 新连接创建数量检查时间间隔（微秒）	
		int			NewConnMax;								///< 在NewConnCheckInterval时间内所能接收的最大连接数
		int			SendUpCheckInterval;					///< 向上层发送的数据包数量检查时间间隔（微秒）
		int			SendUpMax;								///< 在SendUpCheckInterval时间内向上层所能发送数据包的最大数量
		int			StopAcceptInterval;						///< 停止接收新连接的时间间隔（秒）

		CSizeString<SL_PATH_MAX>		ConfigName;			///< 配置文件名
	public:
		int		LoadConfig();

		int		ReloadConfig();

		//读取日志配置
		int		LoadLogConfig();

	}; //class CNetConfig
}

//全局配置
#define CONF (CSingleton<CNetConfig, 0>::Instance())
#endif