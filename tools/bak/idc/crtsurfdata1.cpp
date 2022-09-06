/*
 * @brief: 本程序用于生成全国气象站点观测的分钟数据
 * @Autor: yc
 * @Date: 2022-07-08 00:01:14
 * @LastEditors: yc
 * @LastEditTime: 2022-07-11 09:58:53
 * @FilePath: /project/idc/src/crtsurfdata1.cpp
 */

#include "_public.h"

CLogFile logfile(2);

int main(int argc, char const *argv[])
{
	// inifile outpath logfile
	if (argc != 4)
	{
		printf("Using: ./crtsurfdata1 inifile outpath logfile\n");
		printf("Example:/project/idc/bin/crtsurfdata1 /project/idc/ini/stdcode.ini /project/tmp/surfdata /home/yc/project/logfile/surfdata.log\n");
		printf("inifile 全国气象站点参数文件名。\n");
		printf("outpath 全国气象站点数据文件存放的目录。\n");
		printf("logfile 本程序运行的日志文件名。\n\n");

		return -1;
	}

	// 打开程序的日志文件
	if (logfile.Open(argv[3], "a+", true) == false) {
        printf("logfile.Open(%s) failed.\n", argv[3]); 
        return -1;
    } 

	logfile.Write("crtsurfdata1 开始运行。\n");

    // 在这里插入处理业务的代码
	for(int i = 0; i < 100000000; i++)
	{
		logfile.Write("测试代码测试代码测试代码.\n");
	}

	logfile.WriteEx("crtsurfdata1 运行结束。\n");
	return 0;
}
