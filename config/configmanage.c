/*
 * configmanage.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "configmanage.h"

int InitServerConfig(int nMaxRowLength)
{
	Ini ini;
	if (!InitIni(&ini, "../file/ini/server.ini", nMaxRowLength))
	{
		return 0;
	}

	/*线程池配置*/
	serverConfig.nAddThreadTaskNumber = GetInt(&ini, "POOLNUMBER", "AddThreadTaskNumber", 4);
	serverConfig.nMaxThreadNumber = GetInt(&ini, "POOLNUMBER", "MaxThreadNumber", 99);
	serverConfig.nCoreThreadNumber = GetInt(&ini, "POOLNUMBER", "CoreThreadNumber", 59);
	serverConfig.nAccThreadOverTime = GetInt(&ini, "ACCOVERTIME", "AccOverTime", 1799);
	serverConfig.nAccThreadLoopSpace = GetInt(&ini, "ACCOVERTIME", "AccThreadLoopSpace", 899);
	serverConfig.nAddThreadNumber = GetInt(&ini, "ADDTHREAD", "AddThreadNumber", 4);
	serverConfig.nAddThreadLoopSpace = GetInt(&ini, "ADDTHREAD", "AddThreadLoopSpace", 19);
	serverConfig.nExeThreadOverTime = GetInt(&ini, "EXEOVERTIME", "ExeThreadOverTime", 4);
	serverConfig.nExeThreadLoopSpace = GetInt(&ini, "EXEOVERTIME", "ExeThreadLoopSpace", 9);

	/*日志配置*/
	serverConfig.nMaxLogListLength = GetInt(&ini, "LOG", "MaxLogListLength", 9999);
	serverConfig.nProLogLoopSpace = GetInt(&ini, "LOG", "ProLogLoopSpace", 0);

	/*数据库连接池*/
	serverConfig.nMaxConnNumber = GetInt(&ini, "CONNPOOLNUMBER", "MaxConnNumber", 99);
	serverConfig.nCoreConnNumber = GetInt(&ini, "CONNPOOLNUMBER", "CoreConnNumber", 29);
	serverConfig.nAccConnOverTime = GetInt(&ini, "CONNACCOVERTIME", "AccOverTime", 1700);
	serverConfig.nAccConnLoopSpace = GetInt(&ini, "CONNACCOVERTIME", "AccConnLoopSpace", 500);
	serverConfig.nAddConnNumber = GetInt(&ini, "CONNADDTHREAD", "AddConnNumber", 4);

	/*数据库配置*/
	serverConfig.pHost = GetString(&ini, "MYSQLDB", "Host", "127.0.0.1");
	serverConfig.pUser = GetString(&ini, "MYSQLDB", "User", "root");
	serverConfig.pPasswd = GetString(&ini, "MYSQLDB", "Passwd", "");
	serverConfig.pDB = GetString(&ini, "MYSQLDB", "DB", "test");
	serverConfig.pUnixSocket = GetString(&ini, "MYSQLDB", "UnixSocket", "");
	serverConfig.lClientFlag = GetInt(&ini, "MYSQLDB", "ClientFlag", 0);
	serverConfig.nDBPort = GetInt(&ini, "MYSQLDB", "Port", 3306);

	/*socket 配置*/
	serverConfig.nAccSocketOutTime = GetInt(&ini, "ACCSOCKETOVERTIME", "AccOutTime", 1799);
	serverConfig.nAccOutTimeSocketLoopSpace = GetInt(&ini, "ACCSOCKETOVERTIME", "AccOutTimeThreadLoopSpace", 799);
	serverConfig.nMaxAcceptSocketNumber = GetInt(&ini, "SOCKETNUMBER", "MaxAcceptSocketNumber", 9999);
	serverConfig.nAccSocketThreadLoopSpace = GetInt(&ini, "ACCSOCKET", "AccThreadLoopSpace", 0);

	/*aio配置*/
	serverConfig.nMaxAioQueueLength = GetInt(&ini, "AIO", "MaxAioQueueLength", 299);
	serverConfig.nAioLoopSpace = GetInt(&ini, "AIO", "AioLoopSpace", 0);
	serverConfig.nMaxAioBufferLength = GetInt(&ini, "BUFFER", "MaxBufferLength", 1023);

	/*data 配置*/
	serverConfig.nMaxDataRecvListLen = GetInt(&ini, "DATANUMBER", "MaxRecvListLen", 9999);
	serverConfig.nMaxDataSendListLen = GetInt(&ini, "DATANUMBER", "MaxSendListLen", 9999);
	serverConfig.nProRecvDataLoopSpace = GetInt(&ini, "DATANUMBER", "ProRecvDataLoopSpace", 0);
	serverConfig.nProSendDataLoopSpace = GetInt(&ini, "DATANUMBER", "ProSendDataLoopSpace", 0);

	/*服务器设置*/
	serverConfig.pServerIp = GetString(&ini, "SERVER", "IP", "127.0.0.1");
	serverConfig.nServerPort = GetInt(&ini, "SERVER", "PORT", 2112);
	serverConfig.nServerListenNumber = GetInt(&ini, "SERVER", "LISNUMBER", 10);

	if (!ReleaseIni(&ini))
	{
		return 0;
	}

	return 1;
}

int ReleaseServerConfig()
{
	if (serverConfig.pHost)
	{
		free(serverConfig.pHost);
		serverConfig.pHost = NULL;
	}

	if (serverConfig.pUser)
	{
		free(serverConfig.pUser);
		serverConfig.pUser = NULL;
	}

	if (serverConfig.pPasswd)
	{
		free(serverConfig.pPasswd);
		serverConfig.pPasswd = NULL;
	}

	if (serverConfig.pDB)
	{
		free(serverConfig.pDB);
		serverConfig.pDB = NULL;
	}

	if (serverConfig.pUnixSocket)
	{
		free(serverConfig.pUnixSocket);
		serverConfig.pUnixSocket = NULL;
	}

	if (serverConfig.pServerIp)
	{
		free(serverConfig.pServerIp);
		serverConfig.pServerIp = NULL;
	}
	return 1;
}
