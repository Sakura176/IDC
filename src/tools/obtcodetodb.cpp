/**
 * @file obtcodetodb.cpp
 * @author your name (you@domain.com)
 * @brief 本程序用于将全国站点参数数据保存到数据库T_ZHOBTCODE表中。
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
static server::FileLogAppender::ptr file_appender(new server::FileLogAppender("../logfile/obtcodetodb.log"));

static Connection conn;

struct st_stcode
{
	char provname[31];		// 省
	char obtid[11];			// 站号
	char cityname[31];		// 站名
	char lat[11];			// 纬度
	char lon[11];			// 经度
	char height[11];		// 海拔高度
};

static vector<struct st_stcode> vstcode;

// 把站点参数文件中加载到vstcode容器中
bool LoadSTCode(const char *inifile);

void _help();

void _help()
{
	printf("\n");
	printf("Using:./obtcodetodb inifile connstr charset logfile\n");

	printf("Example:/home/yc/IDC/src/tools/procctl 120 /home/yc/IDC/bin/test_datatodb /home/yc/data/ini/stcode.ini 127.0.0.1,root,mysqlpwd,mysql,3306 utf8\n\n");

	printf("本程序用于把全国站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新。\n");
	printf("inifile 站点参数文件名(全路径).\n");
	printf("connstr 数据库连接参数: ip,username,password,dbname,port\n");
	printf("charset 数据库的字符集。\n");
	printf("logfile 本程序运行的日志文件名。\n");
	printf("程序每120秒运行一次,由procctl调度。\n\n\n");
}

bool obtCodeToDB(int argc, const char *argv[])
{
	// 加入文件日志
	logger->addAppender(file_appender);

	if (argc != 4)
	{
		_help();
		return false;
	}

	// CloseIOAndSignal();
	// signal(SIGINT, EXIT());
	// signal(SIGTERM, EXIT());

	// 把全国站点参数文件加载到vstcode容器中
	if (LoadSTCode(argv[1]) == false)
	{
		SERVER_LOG_INFO(logger) << "加载全国站点参数文件失败。";
		return false;
	}
	SERVER_LOG_INFO(logger) << "加载参数文件(" << argv[1] << ")成功，站点数(" << vstcode.size() << ")。";

	// 连接数据库
	if (conn.connecttodb(argv[2], argv[3]) != 0)
	{
		SERVER_LOG_INFO(logger) << "connect database(" << argv[2] << ", " << conn.m_cda.message << ") failed.";
		return -1;
	}
	SERVER_LOG_INFO(logger) << "connect database(" << argv[2] << ") ok.";

	struct st_stcode stcode;

	// 准备插入表的SQL语句
	SqlStatement stmtins(&conn);
	stmtins.prepare("insert into T_ZHOBTCODE(obtid,cityname,provname,lat,lon,height,upttime) \
					values(:1,:2,:3,:4,:5,:6,now())");
	stmtins.bindin(1, stcode.obtid, 10);
	stmtins.bindin(2, stcode.cityname, 30);
	stmtins.bindin(3, stcode.provname, 30);
	stmtins.bindin(4, stcode.lat, 10);
	stmtins.bindin(5, stcode.lon, 10);
	stmtins.bindin(6, stcode.height, 10);

	// 准备更新表的SQL语句
	SqlStatement stmtupt(&conn);
	stmtupt.prepare("update T_ZHOBTCODE set cityname=:1,provname=:2,lat=:3,lon=:4,height=:5,upttime=now() where id=:6");
	stmtupt.bindin(1, stcode.cityname, 30);
	stmtupt.bindin(2, stcode.provname, 30);
	stmtupt.bindin(3, stcode.lat, 10);
	stmtupt.bindin(4, stcode.lon, 10);
	stmtupt.bindin(5, stcode.height, 10);
	stmtupt.bindin(6, stcode.obtid, 10);

	int inscount = 0, uptcount = 0;
	CTimer Timer;

	// 遍历vstcode容器
	for (size_t i = 0; i < vstcode.size(); i++)
	{
		// 读取数据到结构体
		memcpy(&stcode, &vstcode[i], sizeof(struct st_stcode));

		// 执行插入的语句
		if (stmtins.execute() != 0)
		{
			if (stmtins.m_cda.rc == 1062)
			{
				// 如果记录已存在，执行更新语句
				if (stmtupt.execute() != 0)
				{
					SERVER_LOG_INFO(logger) << "stmtupt.execute() failed.\n" 
						<< "sql ("<< stmtins.m_sql << ")\nmessage: " << stmtins.m_cda.message;
					return false;
				}
				else
					uptcount++;
			}
			else
			{
				SERVER_LOG_INFO(logger) << "stmtins.execute() failed.\n" 
					<< "sql ("<< stmtins.m_sql << ")\nmessage: " << stmtins.m_cda.message;
				return false;
			}
		}
		else
			inscount++;
	}

	// 把总记录数、插入记录数、更新记录数、消耗时长记录日志
	SERVER_LOG_INFO(logger) << "总记录数=" << vstcode.size() << ", 插入=" << inscount
			<< ", 更新=" << uptcount << ", 耗时=" << Timer.Elapsed();
	
	// 提交事务
	conn.commit();
	return true;
}

bool LoadSTCode(const char *inifile)
{
	CFile File;
	// 打开站点参数文件
	if (File.Open(inifile, "r") == false)
	{
		SERVER_LOG_INFO(logger) << "File.Open(" << inifile << ") failed."; 
		return false;
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
		CmdStr.GetValue(2, stcode.cityname, 30);
		CmdStr.GetValue(3, stcode.lat, 10);
		CmdStr.GetValue(4, stcode.lon, 10);
		CmdStr.GetValue(5, stcode.height, 10);

		// 把站点参数结构体放入站点参数容器中
		vstcode.push_back(stcode);
	}

	// for (int ii = 0; ii < vstcode.size(); ii++)
	// 	logfile.Write("provname=%s, obtid=%s, obtname=%s, lat=%.2f, lon=%.2f, height=%.2f\n",
	// 					vstcode[ii].provname, vstcode[ii].obtid, vstcode[ii].obtname, vstcode[ii].lat,
	// 					vstcode[ii].lon, vstcode[ii].height);

	return true;
}
