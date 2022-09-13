/*
 * @brief: 本程序用于生成全国气象站点观测的分钟数据
 * @Autor: yc
 * @Date: 2022-07-08 00:01:14
 * @LastEditors: yc
 * @LastEditTime: 2022-07-13 10:10:44
 * @FilePath: /project/idc/src/crtsurfdata.cpp
 */

// 增加生成历史数据文件的功能，为压缩文件和清理文件准备历史数据文件。
// 增加信号处理函数，处理2和15的信号
// 解决调用exit函数退出时局部对象没有调用析构函数的问题。
// 把心跳函数写入共享内存

#include "_public.h"

CLogFile logfile(2);

CFile File;

CPActive Pactive;

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
	char ddatetime[21]; // 当前时间
	int t;
	int p;
	int u;
	int wd;
	int wf;
	int r;
	int vis;
};

vector<struct st_surfdata> vsurfdata;

char strddatetime[21];		// 时间变量

void CrtSurfData();

bool LoadSTCode(const char *inifile);

bool CrtSurfFile(const char *outpath, const char *datafmt);

// 程序退出和信号2、15的处理函数
void EXIT(int sig)
{
	logfile.Write("程序退出, sig=%d\n\n", sig);
	exit(0);
}

int main(int argc, char const *argv[])
{
	// inifile outpath logfile
	if ( (argc != 5) && (argc != 6))
	{
		printf("Using: ./crtsurfdata inifile outpath logfile datafmt [datetime]\n");
		printf("Example:/project/idc/bin/crtsurfdata /project/idc/ini/stcode.ini /project/tmp/surfdata /project/logfile/idc/crtsurfdata.log xml,json,csv 2021071025\n\n");
				
		printf("inifile 全国气象站点参数文件名。\n");
		printf("outpath 全国气象站点数据文件存放的目录。\n");
		printf("logfile 本程序运行的日志文件名。\n");
		printf("datafmt 生成数据文件的格式, 支持xml、json和csv三种格式, 中间用逗号分隔。\n");
		printf("datetime 这是一个可选参数，表示生成指定时间的数据和文件。\n\n");

		return -1;
	}

	// 
	CloseIOAndSignal(true); signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

	// 打开程序的日志文件
	if (logfile.Open(argv[3], "a+", true) == false) {
        printf("logfile.Open(%s) failed.\n", argv[3]); 
        return -1;
    } 

	logfile.Write("crtsurfdata 开始运行。\n");

	Pactive.AddPInfo(20, "crtsurfdata");

    // 在这里插入处理业务的代码
	if (LoadSTCode(argv[1]) == false) return -1;

	// 获取当前时间，当成观测时间
	memset(strddatetime, 0, sizeof(strddatetime));
	if (argc == 5)
		LocalTime(strddatetime, "yyyymmddhh24miss");
	else
		STRCPY(strddatetime, sizeof(strddatetime), argv[5]);
	
	// 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中。
	CrtSurfData();

	// 将数据写入文件，分三种格式
	if (strstr(argv[4], "xml")!= 0) CrtSurfFile(argv[2], "xml");
	if (strstr(argv[4], "json")!= 0) CrtSurfFile(argv[2], "json");
	if (strstr(argv[4], "csv")!= 0) CrtSurfFile(argv[2], "csv");

	logfile.Write("crtsurfdata 运行结束。\n");
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

		// TODO 把站点参数的每个数据项保存到站点参数结构体中。
		CmdStr.GetValue(0, stcode.provname, 30);
		CmdStr.GetValue(1, stcode.obtid, 10);
		CmdStr.GetValue(2, stcode.obtname, 30);
		CmdStr.GetValue(3, &stcode.lat);
		CmdStr.GetValue(4, &stcode.lon);
		CmdStr.GetValue(5, &stcode.height);

		// 把站点参数结构体放入站点参数容器中
		vstcode.push_back(stcode);
	}

	// for (int ii = 0; ii < vstcode.size(); ii++)
	// 	logfile.Write("provname=%s, obtid=%s, obtname=%s, lat=%.2f, lon=%.2f, height=%.2f\n",
	// 					vstcode[ii].provname, vstcode[ii].obtid, vstcode[ii].obtname, vstcode[ii].lat,
	// 					vstcode[ii].lon, vstcode[ii].height);

	return true;
}

void CrtSurfData()
{
	// 定义随机数种子
	srand(time(0));

	// // 获取当前时间，当成观测时间
	// memset(strddatetime, 0, sizeof(strddatetime));
	// LocalTime(strddatetime, "yyyymmddhh24miss");

	// cout << string(strddatetime) << endl;
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
}

// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
bool CrtSurfFile(const char *outpath, const char *datafmt)
{	
	// CFile File;
	// 拼接生成数据的文件名，例如：SURF_ZH_202207121010_2254.CSV
	char strFileName[301];
	memset(strFileName, 0, sizeof(strFileName));

	sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, strddatetime, getpid(), datafmt);

	// 打开文件
	if (File.OpenForRename(strFileName, "w") == false)
	{
		logfile.Write("File.Open(%s) failed.", strFileName);
		return false;
	}

	// 写入第一行标题
	if (strcmp(datafmt, "csv") == 0)
	{
		File.Fprintf("%s\n", \
					"站点代码, 数据时间, 气温, 气压, 相对湿度, 风向, 风速, 降雨量, 能见度");
	}

	if (strcmp(datafmt, "xml") == 0)
	{
		File.Fprintf("<data>\n");
	}

	if (strcmp(datafmt, "json") == 0)
	{
		File.Fprintf("{\"data\":[\n");
	}
	// 遍历存放观测数据的vsurfdata容器
	for (int ii = 0; ii < vsurfdata.size(); ii++)
	{
		// 写入一条记录
		// cout << vsurfdata[ii].ddatetime << endl;
		if (strcmp(datafmt, "csv") == 0)
			File.Fprintf("%s, %s, %.1f, %.1f, %d, %d, %.1f, %.1f, %.1f\n", \
						vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t/10.0, vsurfdata[ii].p/10.0,\
						vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf/10.0, vsurfdata[ii].r/10.0, vsurfdata[ii].vis/10.0);
		
		if (strcmp(datafmt, "xml") == 0)
			File.Fprintf("<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</t><p>%.1f</p><u>%d</u>" \
							"<wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl>\n", \
						vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t/10.0, vsurfdata[ii].p/10.0,\
						vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf/10.0, vsurfdata[ii].r/10.0, vsurfdata[ii].vis/10.0);
		
		if (strcmp(datafmt, "json") == 0)
		{
			File.Fprintf("{\"obtid\":\"%s\", \"ddatetime\":\"%s\",\"t\":\"%.1f\", \"p\":\"%.1f\", \"u\":\"%d\"," \
						" \"wd\":\"%d\", \"wf\":\"%.1f\", \"r\":\"%.1f\", \"vis\":\"%.1f\"}", \
						vsurfdata[ii].obtid, vsurfdata[ii].ddatetime, vsurfdata[ii].t/10.0, vsurfdata[ii].p/10.0,\
						vsurfdata[ii].u, vsurfdata[ii].wd, vsurfdata[ii].wf/10.0, vsurfdata[ii].r/10.0, vsurfdata[ii].vis/10.0);
			if (ii < vsurfdata.size()-1) File.Fprintf(",\n");
			else File.Fprintf("\n");
		}
	}
	if (strcmp(datafmt, "xml") == 0) File.Fprintf("</data>\n");
	if (strcmp(datafmt, "json") == 0) File.Fprintf("]}\n");

	// sleep(10);
	// 关闭文件
	File.CloseAndRename();

	UTime(strFileName, strddatetime); // 修改文件的时间属性。

	logfile.Write("生成数据文件%s成功, 数据时间%s, 记录数%d。\n", strFileName, strddatetime, vsurfdata.size());
	return true;
}

