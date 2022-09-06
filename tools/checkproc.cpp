/*
 * @brief: 
 * @Autor: yc
 * @Date: 2022-07-19 20:44:04
 * @LastEditors: yc
 * @LastEditTime: 2022-07-21 17:10:17
 * @FilePath: /project/tools/src/checkproc.cpp
 */
#include "../src/public/_public.h"

// 程序运行的日志
CLogFile logfile;

int main(int argc, char const *argv[])
{
	// 程序的帮助
	if (argc != 2)
	{
		printf("\n");
		printf("Using:./checkproc logfilename\n");
		printf("Example:/project/tools/bin/procctl 10 /project/tools/bin/checkproc /project/tmp/checkproc.log\n\n");

		printf("本程序用于检测后台服务程序是否超时,若超时,则终止\n");
		printf("注意: \n");
		printf(" 1) 本程序由procctl启动,运行周期建议为10秒。\n");
		printf(" 2) 为了避免被普通用户误杀,本程序应该用root用户启动。\n\n");

		return 0;
	}

	// 忽略全部信号和IO
	CloseIOAndSignal(true);

	// 打开日志文件
	if (logfile.Open(argv[1], "a+") == false)
	{ printf("logfile.Open(%s) failes.\n", argv[1]); return -1; }

	// 创建/获取共享内存
	int shmid = 0;
	if ( (shmid=shmget(SHMKEYP_, MAXNUMP_*sizeof(struct st_pinfo), 0640|IPC_CREAT)) == -1)
	{
		logfile.Write("创建/获取共享内存(%x)失败。\n", SHMKEYP_);

		return false;
	}

	// 将共享内存连接到当前进程的地址空间
	struct st_pinfo *shm = (struct st_pinfo *)shmat(shmid, 0, 0);

	// 遍历共享内存中全部的记录
	for (int ii = 0; ii < MAXNUMP_; ii++)
	{
		// 如果记录的pid==0， 表示空记录，continue
		if (shm[ii].pid == 0) continue;

		// 如果记录的pid != 0，表示为服务程序的心跳记录。
		// logfile.Write("ii=%d, pid=%d, pname=%s, timeout=%d, atime=%d\n",
		// 		 ii, shm[ii].pid, shm[ii].pname, shm[ii].timeout, shm[ii].atime);

		// 向进程发送信号0，判断是否存在，如果不存在，从共享内存中删除该记录，continue
		int iret = kill(shm[ii].pid, 0);
		if (iret == -1)
		{
			logfile.Write("进程pid=%d(%s)不存在。\n", (shm+ii)->pid, (shm+ii)->pname);
			memset(shm+ii, 0, sizeof(struct st_pinfo));
			continue;
		}
		
		time_t now = time(0);		// 取当前时间

		// 如果进程未超时，continue
		if (now - shm[ii].atime < shm[ii].timeout) continue;

		// 如果已超时
		logfile.Write("进程pid=%d(%s)已经超时。\n", (shm+ii)->pid, (shm+ii)->pname);

		// 发送信号15，尝试正常终止进程
		kill(shm[ii].pid, 15);

		// 每隔1秒判断一次进程是否存在，累计5秒
		for (int jj=0; jj < 5; jj++)
		{
			sleep(1);
			iret = kill(shm[ii].pid, 0);
			if (iret == -1) break;
		}

		// 如果进程仍存在，就发送信号9，强制终止
		if (iret == -1)
			logfile.Write("进程pid=%d(%s)已经正常终止。\n", (shm+ii)->pid, (shm+ii)->pname);
		else
		{
			kill(shm[ii].pid, 9);
			logfile.Write("进程pid=%d(%s)已经强制终止。\n", (shm+ii)->pid, (shm+ii)->pname);
		}

		// 从共享内存中删除已超时进程的心跳记录。
		memset(shm+ii, 0, sizeof(struct st_pinfo));
	}

	// 把共享内存从当前进程中分离
	shmdt(shm);

	return 0;
}
