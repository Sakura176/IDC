#include "_public.h"

void EXIT(int sig);

int main(int argc, char const *argv[])
{
	// 程序的帮助
	if (argc != 4)
	{
		printf("\n");
		printf("Using:/project/tools/bin/deletefiles pathname matchstr timeout\n\n");
		// 存在问题，视频讲解中说特殊符号需要双引号\"*.xml,*.json\", 但程序会报错，去掉双引号程序反而正常运行
		// 在控制台输出时需要加双引号，实际程序中不需要
		printf("Example:/project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02\n");
		printf("        /project/tools/bin/deletefiles /project/tmp/surfdata \"*.xml,*.json\" 0.01\n");
		printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02\n");
		printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/tmp/surfdata \"*.xml,*.json\" 0.01\n\n");

		printf("这是一个工具程序, 用于删除历史的数据文件或日志文件。\n");
		printf("本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部删除, timeout可以是小数。\n");
		printf("本程序不写日志文件, 也不会在控制台输出任何信息。\n\n\n");

		return -1;
	}
	// 关闭全部的信号和输入输出
	// CloseIOAndSignal(true); 
	signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

	// 获取文件超时的时间点
	char strTimeOut[21];
	LocalTime(strTimeOut, "yyyy-mm-dd hh24:mi:ss", 0-(int)(atof(argv[3])*24*60*60));

	CDir Dir;
	// 打开目录
	if (Dir.OpenDir(argv[1], argv[2], 10000, true)== false)
	{
		printf("Dir.OpenDir(%s) failed.\n", argv[1]); return -1;
	}
	
	char strCmd[1024];

	// 遍历目录中的文件名
	while (true)
	{
		// 得到一个文件的信息，
		if (Dir.ReadDir() == false) break;
		// printf("DirName=%s, FileName=%s, FullFileName=%s, FileSize=%d, ModifyTime=%s, CreateTime=%s, AccessTime=%s\n",\
		// 		Dir.m_DirName, Dir.m_FileName, Dir.m_FullFileName, Dir.m_FileSize, Dir.m_ModifyTime,\
		// 		Dir.m_CreateTime, Dir.m_AccessTime);

		// 与超时的时间点比较，如果更早，就需要压缩
		if ( strcmp(Dir.m_ModifyTime, strTimeOut) < 0 )
		{
			// 压缩文件，调用操作系统的gzip命令
			if (REMOVE(Dir.m_FullFileName) == 0)
				printf("REMOVE %s ok.\n", Dir.m_FullFileName);
			else
				printf("REMOVE %s ok.\n", Dir.m_FullFileName);
		}
	}
	
	return 0;
}

void EXIT(int sig)
{
	printf("程序退出, sig=%d\n\n", sig);

	exit(0);
}
