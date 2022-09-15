/**
 * @file obtcodetodb.cpp
 * @author your name (you@domain.com)
 * @brief 本程序用于将全国站点气候数据保存到数据库T_ZHOBTCODE表中。
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../public/_public.h"
#include "../mysql/_mysql.h"
#include "../log/log.h"

static server::Logger::ptr logger = SERVER_LOG_NAME("system");
static server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/logfile/obtdatatodb.log"));

static Connection conn;

// struct st_stcode
// {
// 	char provname[31];		// 省
// 	char obtid[11];			// 站号
// 	char cityname[31];		// 站名
// 	char lat[11];			// 纬度
// 	char lon[11];			// 经度
// 	char height[11];		// 海拔高度
// };

// 全国气象站点分钟观测数据结构
struct st_surfdata
{
	char obtid[11];			// 站点代码
	char ddatetime[21]; 	// 当前时间
	char t[11];				// 温度
	char p[11];				// 气压
	char u[11];				// 相对湿度
	char wd[11];			// 风向
	char wf[11];			// 风俗
	char r[11];				// 降雨量
	char vis[11];			// 可见度
};

// static vector<struct st_stcode> vstcode;

void _help1();

bool obtDataToDB(int argc, const char *argv[]);

// 把站点参数文件中加载到vstcode容器中
// bool LoadSTCode(const char *inifile);


void _help1()
{
	printf("\n");
	printf("Using:./obtcodetodb inifile connstr charset logfile\n");

	printf("Example:/home/yc/IDC/src/tools/procctl 120 /home/yc/IDC/bin/test_datatodb /home/yc/data/surfdata 127.0.0.1,root,mysqlpwd,mysql,3306 utf8\n\n");

	printf("本程序用于把全国站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新。\n");
	printf("inifile 站点参数文件名(全路径).\n");
	printf("connstr 数据库连接参数: ip,username,password,dbname,port\n");
	printf("charset 数据库的字符集。\n");
	printf("logfile 本程序运行的日志文件名。\n");
	printf("程序每120秒运行一次,由procctl调度。\n\n\n");
}

bool obtDataToDB(int argc, const char *argv[])
{
	// 加入文件日志
	logger->addAppender(file_appender);

	if (argc != 4)
	{
		_help1();
		return false;
	}

	// CloseIOAndSignal();
	// signal(SIGINT, EXIT());
	// signal(SIGTERM, EXIT());

	struct st_surfdata stsurfdata;

	// 连接数据库
	if (conn.connecttodb(argv[2], argv[3]) != 0)
	{
		SERVER_LOG_INFO(logger) << "connect database(" << argv[2] << ", " << conn.m_cda.message << ") failed.";
		return false;
	}
	SERVER_LOG_INFO(logger) << "connect database(" << argv[2] << ") ok.";

	SqlStatement stmt(&conn);
	stmt.prepare("insert into T_ZHOBTMIND(obtid,ddatetime,t,p,u,wd,wf,r,vis,upttime) \
			values(:1,str_to_date(:2,'%%Y%%m%%d%%H%%i%%s'),:3,:4,:5,:6,:7,:8,:9,now())");
	stmt.bindin(1, stsurfdata.obtid, 10);
	stmt.bindin(2, stsurfdata.ddatetime, 20);
	stmt.bindin(3, stsurfdata.t, 10);
	stmt.bindin(4, stsurfdata.p, 10);
	stmt.bindin(5, stsurfdata.u, 10);
	stmt.bindin(6, stsurfdata.wd, 10);
	stmt.bindin(7, stsurfdata.wf, 10);
	stmt.bindin(8, stsurfdata.r, 10);
	stmt.bindin(9, stsurfdata.vis, 10);

	CDir Dir;
	if (Dir.OpenDir(argv[1], "*.xml") == false)
	{
		SERVER_LOG_INFO(logger) << "open dir (" << argv[1] << ") failed.";
		return false;
	}

	// SERVER_LOG_INFO(logger) << "open dir (" << argv[1] << ") ok.";
	CFile File;

	while (true)
	{
		
		if (Dir.ReadDir() == false)
			break;
		SERVER_LOG_INFO(logger) << "filename=" << Dir.m_FullFileName;

		if (File.Open(Dir.m_FullFileName, "r") == false)
		{
			SERVER_LOG_INFO(logger) << "Dir.Open(" << Dir.m_FullFileName << ") failed.";
			return false;
		}

		char strBuffer[1001];

		while(true)
		{
			if (File.FFGETS(strBuffer, 1000, "<endl>") == false)
				break;
			
			// 处理文件中的每一行
			memset(&stsurfdata, 0, sizeof(struct st_surfdata));
			GetXMLBuffer(strBuffer, "obtid", stsurfdata.obtid, 10);
			GetXMLBuffer(strBuffer, "ddatetime", stsurfdata.ddatetime, 14);
			GetXMLBuffer(strBuffer, "t", stsurfdata.t, 10);
			GetXMLBuffer(strBuffer, "p", stsurfdata.p, 10);
			GetXMLBuffer(strBuffer, "u", stsurfdata.u, 10);
			GetXMLBuffer(strBuffer, "wd", stsurfdata.wd, 10);
			GetXMLBuffer(strBuffer, "wf", stsurfdata.wf, 10);
			GetXMLBuffer(strBuffer, "r", stsurfdata.r, 10);
			GetXMLBuffer(strBuffer, "vis", stsurfdata.vis, 10);

			// SERVER_LOG_INFO(logger) << "obtid=" << stsurfdata.obtid << ", ddatetime=" << stsurfdata.ddatetime
			// 						<< ", t=" << stsurfdata.t << ", p=" << stsurfdata.p << ", u=" << stsurfdata.u
			// 						<< ", wd=" << stsurfdata.wd << ", wf=" << stsurfdata.wf << ", r=" << stsurfdata.r
			// 						<< ", vis=" << stsurfdata.vis;
			
			// 把结构体中的数据插入表中
			if (stmt.execute() != 0)
			{
				if (stmt.m_cda.rc != 1062)
				{
					SERVER_LOG_INFO(logger) << "Buffer=" << strBuffer;
					SERVER_LOG_INFO(logger) << "stmt.execute() failed.\n"
											<< "sql: \n" << stmt.m_sql
											<< "message: " << stmt.m_cda.message;
				}
			}
		}
		File.Close();
		conn.commit();
	}
	return true;
}

// bool LoadSTCode(const char *inifile)
// {
// 	CFile File;
// 	// 打开站点参数文件
// 	if (File.Open(inifile, "r") == false)
// 	{
// 		SERVER_LOG_INFO(logger) << "File.Open(" << inifile << ") failed."; 
// 		return false;
// 	}

// 	char strBuffer[301];

// 	CCmdStr CmdStr;

// 	struct st_stcode stcode;

// 	while (true)
// 	{
// 		// 从站点参数文件中读取一行，如果已读完，跳出循环。
// 		if (File.Fgets(strBuffer, 300, true) == false) break;

// 		// logfile.Write("=%s=\n", strBuffer);
// 		// 把读取到的一行拆分
// 		CmdStr.SplitToCmd(strBuffer, ",", true);

// 		if ((CmdStr.CmdCount() != 6)) continue;

// 		// TODO 把站点参数的每个数据项保存到站点参数结构体中。
// 		CmdStr.GetValue(0, stcode.provname, 30);
// 		CmdStr.GetValue(1, stcode.obtid, 10);
// 		CmdStr.GetValue(2, stcode.cityname, 30);
// 		CmdStr.GetValue(3, stcode.lat, 10);
// 		CmdStr.GetValue(4, stcode.lon, 10);
// 		CmdStr.GetValue(5, stcode.height, 10);

// 		// 把站点参数结构体放入站点参数容器中
// 		vstcode.push_back(stcode);
// 	}

// 	// for (int ii = 0; ii < vstcode.size(); ii++)
// 	// 	logfile.Write("provname=%s, obtid=%s, obtname=%s, lat=%.2f, lon=%.2f, height=%.2f\n",
// 	// 					vstcode[ii].provname, vstcode[ii].obtid, vstcode[ii].obtname, vstcode[ii].lat,
// 	// 					vstcode[ii].lon, vstcode[ii].height);

// 	return true;
// }
