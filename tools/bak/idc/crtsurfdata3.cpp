/*
 * @brief: 本程序用于生成全国气象站点观测的分钟数据
 * @Autor: yc
 * @Date: 2022-07-08 00:01:14
 * @LastEditors: yc
 * @LastEditTime: 2022-07-12 09:31:41
 * @FilePath: /project/idc/src/crtsurfdata3.cpp
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

// 全国气象站点分钟观测数据结构
struct st_surfdata
{
	char obtid[11];		// 站点代码
	char ddatetime[21];
	int t;
	int p;
	int u;
	int wd;
	int wf;
	int r;
	int vis;
};

vector<struct st_surfdata> vsurfdata;

void CrtSurfData();

bool LoadSTCode(const char *inifile);

int main(int argc, char const *argv[])
{
	// inifile outpath logfile
	if (argc != 4)
	{
		printf("Using: ./crtsurfdata1 inifile outpath logfile\n");
		printf("Example:/project/idc/bin/crtsurfdata1 /home/yc/project/idc/ini/stcode.ini /project/tmp/surfdata /home/yc/project/logfile/idc/crtsurfdata3.log\n");
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

	CrtSurfData();

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

void CrtSurfData()
{
	// 定义随机数种子
	srand(time(0));

	// 获取当前时间，当成观测时间
	char strddatetime[21];
	memset(strddatetime, 0, sizeof(strddatetime));
	LocalTime(strddatetime, "yyyymmdd24miss");

	struct st_surfdata stsurfdata;
	// 遍历气象站点参数的vscode容器
	for (int ii=0; ii < vstcode.size(); ii++)
	{
		memset(&stsurfdata, 0, sizeof(struct st_surfdata));
		// 用随机数填充分钟观测数据的结构体
		strncpy(stsurfdata.obtid, vstcode[ii].obtid, 10);	// 站点代码
		strncpy(stsurfdata.ddatetime, strddatetime, 14);	// 数据时间：格式yyyymmddhh24misss
		stsurfdata.t = rand()%351;							// 气温：单位。0.1摄氏度
		stsurfdata.p = rand()%256+10000;					// 气压：0.1百帕
		stsurfdata.u = rand()%100+1;						// 相对湿度，0-100之间的值
		stsurfdata.wd = rand()%360;							// 风向，0-360之间的值
		stsurfdata.wf = rand()%150;							// 风速：单位0.1m/s
		stsurfdata.r = rand()%16;							// 降雨量：0.1mm
		stsurfdata.vis = rand()%5001+100000;				// 能见度：0.1米

		// 将观测数据放入容器
		vsurfdata.push_back(stsurfdata);
	}

	printf("aaa\n");
}
