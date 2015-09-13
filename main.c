/*
 * main.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "server/servermanage.h"

void ProcessMessage()
{
	while (1);
}

int main(int argc, char *argv[])
{
	if (argc < 1 || argc > 2)
	{
		printf("%s-参数个数不符!\n", "main");
		return 0;
	}

	//启动服务器
	if (argc == 1 || strcmp(argv[1], "start") == 0)
	{
		if (CreateShareMemory() != 1)
		{
			printf("%s-创建共享内存失败!\n", "main");
			return 1;
		}

		if (server.pShareMemory->nRun == 1)
		{
			printf("%s-服务器已启动!\n", "main");
			return 1;
		}

		if (StartServer() != 1)
		{
			return 1;
		}
	}
	//重启服务器
	else if (strcmp(argv[1], "restart") == 0)
	{
		if (CreateShareMemory() != 1)
		{
			return 1;
		}

		if (server.pShareMemory->nRun == 1)
		{
			if (!RestartServer())
			{
				return 1;
			}
		}
		else
		{
			if (!StartServer())
			{
				return 1;
			}
		}
	}
	//停止服务器
	else if (strcmp(argv[1], "stop") == 0)
	{
		if (CreateShareMemory() != 1)
		{
			return 1;
		}

		if (server.pShareMemory->nRun != 1)
		{
			printf("%s-服务器未启动!\n", "main");
			return 1;
		}

		kill(server.pShareMemory->proId, SIGTERM);//关闭服务器
		if (ReleaseShareMemory() != 1)
		{
			printf("%s-释放共享内存失败!\n", "main");
			return 1;
		}

		return 0;
	}

	ProcessMessage();

	return 0;
}
