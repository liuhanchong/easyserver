/*
 * configmanage.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef CONFIG_CONFIGMANAGE_H_
#define CONFIG_CONFIGMANAGE_H_

#include "config.h"
#include "../log/logmanage.h"

typedef struct ServerConfig
{
	/*线程池配置*/
	int nAddThreadTaskNumber;/*当任务队列超过多少 自动增加线程*/
	int nMaxThreadNumber;
	int nCoreThreadNumber;
	int nAccThreadOverTime;/*线程未使用时间超时*/
	int nAccThreadLoopSpace;/*线程未使用的判断间隔*/
	int nAddThreadNumber;/*增加线程时候增加的个数*/
	int nAddThreadLoopSpace;/*增加线程时候的判断间隔*/
	int nExeThreadOverTime;/*执行线程的时间超时*/
	int nExeThreadLoopSpace;/*执行线程的判断间隔*/

	/*日志配置*/
	int nMaxLogListLength;
	int nProLogLoopSpace;

	/*数据库连接池*/
	int nMaxConnNumber;
	int nCoreConnNumber;
	int nAccConnOverTime;/*连接未使用时间超时*/
	int nAccConnLoopSpace;/*连接未使用的判断间隔*/
	int nAddConnNumber;/*每次增加的链接数*/

	/*数据库配置*/
	char *pHost;
	char *pUser;
	char *pPasswd;
	char *pDB;
	char *pUnixSocket;
	unsigned long lClientFlag;
	unsigned int nDBPort;

	/*socket 配置*/
	int nAccSocketOutTime;
	int nAccSocketThreadLoopSpace;
	int nMaxAcceptSocketNumber;
	int nAccOutTimeSocketLoopSpace;

	/*aio配置*/
	int nMaxAioQueueLength;
	int nAioLoopSpace;
	int nMaxAioBufferLength;

	/*data处理配置*/
	int nMaxDataRecvListLen;
	int nMaxDataSendListLen;
	int nProRecvDataLoopSpace;
	int nProSendDataLoopSpace;

	/*服务器管理配置*/
	char *pServerIp;
	int nServerPort;
	int nServerListenNumber;
} ServerConfig;

ServerConfig serverConfig;

int InitServerConfig(int nMaxRowLength);
int ReleaseServerConfig();

#endif /* CONFIG_CONFIGMANAGE_H_ */
