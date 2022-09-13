/*
 * @brief: 
 * @Autor: yc
 * @Date: 2022-07-14 00:12:56
 * @LastEditors: yc
 * @LastEditTime: 2022-07-14 15:55:46
 * @FilePath: /project/tools/src/procctl.cpp
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Using:./procctl timetvl program argv ...\n");
		printf("Example:/project/tools/bin/procctl 5 /usr/bin/tar zxvf /project/tmp/AIR_POLLU.tar\n\n");

		printf("本程序是服务程序的调度程序, 周期性启动服务或shell脚本。\n");
		printf("timetvl 运行周期， 单位: 秒。被调用的程序结束后, 在timetvl秒后会被重新启动\n");
		printf("program 被调用的程序名, 必须使用绝对路径\n");
		printf("argvs   被调用程序的参数\n");
		printf("注意, 本程序不会被kill杀死, 可以使用kill -9强行杀死\n");

		return -1;
	}

	// 关闭信号和IO， 本程序不希望被打扰
	for (int i = 0; i < 64; i++)
	{
		signal(i, SIG_IGN); close(i);
	}

	// 生成子进程，父进程退出，让程序运行在后台，由系统1号进行托管
	if (fork() != 0) exit(0);

	// 启用SIGCHLD信号，让父进程可以wait子进程退出的状态
	signal(SIGCHLD, SIG_DFL);

	char *pargv[argc];
	for (int ii = 2; ii < argc; ii++)
		pargv[ii-2] = argv[ii];
	pargv[argc-2] = NULL;

	while (true)
	{
		if (fork() == 0)
		{
			// execl(argv[2], argv[2], argv[3], argv[4], (char*)0);
			execv(argv[2], pargv);
			// 若execv调用失败，执行函数退出程序
			exit(0);
		}
		else
		{
			int status;
			wait(&status);
			sleep(atoi(argv[1]));
		}
	}
	
	return 0;
}
