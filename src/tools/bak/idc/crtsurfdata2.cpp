/*
 * @brief: 本程序用于生成全国气象站点观测的分钟数据
 * @Autor: yc
 * @Date: 2022-07-08 00:01:14
 * @LastEditors: yc
 * @LastEditTime: 2022-07-11 15:17:23
 * @FilePath: /project/idc/src/crtsurfdata2.cpp
 */

#include "_public.h"

CLogFile logfile(2);

struct st_stcode
{
	char provname[31];	// 省
	char obtid[11];		// 站号
	char obtname[31];	// 站名
	double lat;			// 纬度
	double lon;			// 经度
	double height;		// 海拔高度
};

vector<struct st_stcode> vstcode;

bool LoadSTCode(const char *inifile);

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
	if (LoadSTCode(argv[1]) == false) return -1;

	logfile.WriteEx("crtsurfdata1 运行结束。\n");
	return 0;
}

bool LoadSTCode(const char *inifile)
{
	CFile File;
	// 打开站点参数文件
	if (File.Open(inifile, "r") == false)
	{
		logfile.Write("File.Open(%s) failed.\n", inifile); return false;
	}

	char strBuffer[301];

	CCmdStr CmdStr;

	struct st_stcode stcode;

	while (true)
	{
		// 从站点参数文件中读取一行，如果已读完，跳出循环。
		if (File.Fgets(strBuffer, 300, true) == false) break;

		// logfile.Write("=%s=\n", strBuffer);
		// 把读取到的一行拆分
		CmdStr.SplitToCmd(strBuffer, ",", true);

		if ((CmdStr.CmdCount() != 6)) continue;

		CmdStr.GetValue(0, stcode.provname, 30);
		CmdStr.GetValue(1, stcode.obtid, 10);
		CmdStr.GetValue(2, stcode.obtname, 30);
		CmdStr.GetValue(3, &stcode.lat);
		CmdStr.GetValue(4, &stcode.lon);
		CmdStr.GetValue(5, &stcode.height);

		// 把站点参数的每个数据项保存到站点参数结构体中。
		vstcode.push_back(stcode);

		// 把站点参数结构体放入站点参数容器中
	}

	for (int ii = 0; ii < vstcode.size(); ii++)
		logfile.Write("provname=%s, obtid=%s, obtname=%s, lat=%.2f, lon=%.2f, height=%.2f\n",
						vstcode[ii].provname, vstcode[ii].obtid, vstcode[ii].obtname, vstcode[ii].lat,
						vstcode[ii].lon, vstcode[ii].height);

	return true;
}
