#include "obttodb.h"

static server::Logger::ptr logger = SERVER_LOG_NAME("system");
static server::FileLogAppender::ptr file_appender(
	new server::FileLogAppender("/home/yc/IDC/logfile/obttodb.log"));

ObtToDB::ObtToDB(string inifile, string datapath)
	: m_inifile(inifile),
		m_datapath(datapath) 
{ 
	logger->addAppender(file_appender);
	m_connstr = "127.0.0.1,yc,436052,IDC,3306";
	m_charset = "utf8";
}

ObtToDB::~ObtToDB()
{

}

void ObtToDB::setConnstr(string connstr)
{
	m_connstr = connstr;
}

void ObtToDB::setCharset(string charset)
{
	m_charset = charset;
}

bool ObtToDB::codeToDB()
{
	// 连接数据库
	if (m_conn.connecttodb(m_connstr.c_str(), m_charset.c_str()) != 0)
	{
		SERVER_LOG_INFO(logger) << "connect database(" << m_connstr << ", " << m_conn.m_cda.message << ") failed.";
		return -1;
	}
	SERVER_LOG_INFO(logger) << "connect database(" << m_charset << ") ok.";

	// 准备插入表的SQL语句
	SqlStatement stmtins(&m_conn);
	stmtins.prepare("insert into T_ZHOBTCODE(obtid,cityname,provname,lat,lon,height,upttime) \
					values(:1,:2,:3,:4,:5,:6,now())");
	stmtins.bindin(1, m_stcode.obtid, 10);
	stmtins.bindin(2, m_stcode.cityname, 30);
	stmtins.bindin(3, m_stcode.provname, 30);
	stmtins.bindin(4, m_stcode.lat, 10);
	stmtins.bindin(5, m_stcode.lon, 10);
	stmtins.bindin(6, m_stcode.height, 10);

	// 准备更新表的SQL语句
	SqlStatement stmtupt(&m_conn);
	stmtupt.prepare("update T_ZHOBTCODE set cityname=:1,provname=:2,lat=:3,lon=:4,height=:5,upttime=now() where id=:6");
	stmtupt.bindin(1, m_stcode.cityname, 30);
	stmtupt.bindin(2, m_stcode.provname, 30);
	stmtupt.bindin(3, m_stcode.lat, 10);
	stmtupt.bindin(4, m_stcode.lon, 10);
	stmtupt.bindin(5, m_stcode.height, 10);
	stmtupt.bindin(6, m_stcode.obtid, 10);

	int inscount = 0, uptcount = 0, totalcount = 0;
	CTimer Timer;

	ifstream infile(m_inifile);

	if (!infile.is_open())
	{
		SERVER_LOG_DEBUG(logger) << "open file (" << m_inifile <<") failed.";
		return false;
	}

	CCmdStr CmdStr;
	string line;
	while (getline(infile, line))
	{
		// 把读取到的一行拆分
		CmdStr.SplitToCmd(line, ",", true);

		if ((CmdStr.CmdCount() != 6)) continue;
		CmdStr.GetValue(0, m_stcode.provname, 30);
		CmdStr.GetValue(1, m_stcode.obtid, 10);
		CmdStr.GetValue(2, m_stcode.cityname, 30);
		CmdStr.GetValue(3, m_stcode.lat, 5);
		CmdStr.GetValue(4, m_stcode.lon, 5);
		CmdStr.GetValue(5, m_stcode.height, 5);

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
		totalcount++;
	}

	// 把总记录数、插入记录数、更新记录数、消耗时长记录日志
	SERVER_LOG_INFO(logger) << "总记录数=" << totalcount << ", 插入=" << inscount
			<< ", 更新=" << uptcount << ", 耗时=" << Timer.Elapsed();
	
	// 提交事务
	m_conn.commit();
	return true;
}

bool ObtToDB::dataToDB()
{
	// 连接数据库
	if (m_conn.connecttodb(m_connstr.c_str(), m_charset.c_str()) != 0)
	{
		SERVER_LOG_INFO(logger) << "connect database(" << m_connstr << ", " << m_conn.m_cda.message << ") failed.";
		return false;
	}
	SERVER_LOG_INFO(logger) << "connect database(" << m_connstr << ") ok.";

	struct st_surfdata stsurfdata;
	SqlStatement stmt(&m_conn);
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

	int totalcnt, insertcnt;
	CTimer Timer;
	CDir Dir;
	CFile File;

	if (Dir.OpenDir(m_datapath.c_str(), "*.xml") == false)
	{
		SERVER_LOG_INFO(logger) << "open datapath dir (" << m_datapath << ") failed.";
		return false;
	}

	while (true)
	{
		totalcnt = 0, insertcnt = 0;
		if (Dir.ReadDir() == false)
			break;

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
			char tmp[11];
			GetXMLBuffer(strBuffer, "t", tmp, 10);
			if (strlen(tmp) > 0) snprintf(stsurfdata.t, 10, "%d", (int)(atof(tmp) * 10));
			GetXMLBuffer(strBuffer, "p", tmp, 10);
			if (strlen(tmp) > 0) snprintf(stsurfdata.p, 10, "%d", (int)(atof(tmp) * 10));
			GetXMLBuffer(strBuffer, "u", stsurfdata.u, 10);
			GetXMLBuffer(strBuffer, "wd", stsurfdata.wd, 10);

			GetXMLBuffer(strBuffer, "wf", tmp, 10);
			if (strlen(tmp) > 0) snprintf(stsurfdata.wf, 10, "%d", (int)(atof(tmp) * 10));
			GetXMLBuffer(strBuffer, "r", tmp, 10);
			if (strlen(tmp) > 0) snprintf(stsurfdata.r, 10, "%d", (int)(atof(tmp) * 10));
			GetXMLBuffer(strBuffer, "vis", tmp, 10);
			if (strlen(tmp) > 0) snprintf(stsurfdata.vis, 10, "%d", (int)(atof(tmp) * 10));
			
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
			else
				insertcnt++;
			totalcnt++;
		}
		SERVER_LOG_INFO(logger) << "insert files (" << Dir.m_FileName << "), total count=" << totalcnt 
			<< ", insert count=" << insertcnt << ", time consume=" << Timer.Elapsed();

		File.Close();
		m_conn.commit();
	}
	return true;
}