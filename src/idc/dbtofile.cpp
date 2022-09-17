#include "dbtofile.h"

static server::Logger::ptr logger = SERVER_LOG_NAME("system");
static server::FileLogAppender::ptr file_appender(
	new server::FileLogAppender("/home/yc/IDC/logfile/dbtofile.log"));

namespace server
{
	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::vector<T>)
	 */
	template<>
	class LexicalCast<std::string, SelectCfg >
	{
	public:
		SelectCfg operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			SelectCfg ret;
			ret.connstr = node["connstr"].as<std::string>();
			ret.charset = node["charset"].as<std::string>();
			ret.selectsql = node["selectsql"].as<std::string>();
			ret.fieldstr = node["fieldstr"].as<std::string>();
			ret.fieldlen = node["fieldlen"].as<std::string>();
			ret.bfilename = node["bfilename"].as<std::string>();
			ret.efilename = node["efilename"].as<std::string>();
			ret.outpath = node["outpath"].as<std::string>();
			return ret;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::vector<T> 转换成 YAML String)
	 */
	template<>
	class LexicalCast<SelectCfg, std::string >
	{
	public:
		std::string operator() (const SelectCfg& v)
		{
			YAML::Node node;
			node["connstr"] = v.connstr;
			node["charset"] = v.charset;
			node["selectsql"] = v.selectsql;
			node["fieldstr"] = v.fieldstr;
			node["fieldlen"] = v.fieldlen;
			node["bfilename"] = v.bfilename;
			node["efilename"] = v.efilename;
			node["outpath"] = v.outpath;

			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};
}

DBToFile::DBToFile(string cfg)
{
	logger->addAppender(file_appender);

	server::ConfigVar<SelectCfg>::ptr g_sql_defines =
		server::Config::Lookup("sqlconfig", SelectCfg(), "sql config");
	YAML::Node root = YAML::LoadFile(cfg);
	server::Config::LoadFromYaml(root);
	m_selcfg = g_sql_defines->getValue();

	m_fieldcount = 0;
	m_flag = 0;
	memset(m_fieldlen, 0, sizeof(m_fieldlen));
	memset(m_fieldstr, 0, sizeof(m_fieldstr));

	SERVER_LOG_INFO(logger) << root;
}

bool DBToFile::selInDB()
{
	CCmdStr CmdStr;

	// 解析fieldlen字段
	CmdStr.SplitToCmd(m_selcfg.fieldlen, ",");

	if (CmdStr.CmdCount() > 1000)
	{
		SERVER_LOG_INFO(logger) << "字段数太多，超出了限制";
		return false;
	}

	m_fieldcount = 0;
	for (int i = 0; i < CmdStr.CmdCount(); i++)
	{
		CmdStr.GetValue(i, &m_fieldlen[i]);
		// SERVER_LOG_INFO(logger) << m_fieldlen[i];
	}
	m_fieldcount = CmdStr.CmdCount();

	// 解析fieldstr字段
	memset(&m_fieldstr, 0, sizeof(m_fieldstr));
	CmdStr.SplitToCmd(m_selcfg.fieldstr, ",");

	if (CmdStr.CmdCount() > 1000)
	{
		SERVER_LOG_INFO(logger) << "字段数太多，超出了限制";
		return false;
	}

	if (m_fieldcount != CmdStr.CmdCount())
	{
		SERVER_LOG_INFO(logger) << "字段数与字段数长度数量不匹配";
	}

	for (int i = 0; i < CmdStr.CmdCount(); i++)
	{
		CmdStr.GetValue(i, m_fieldstr[i], 30);
		// SERVER_LOG_INFO(logger) << m_fieldstr[i];
	}

	return true;
}

bool DBToFile::run()
{
	selInDB();

	// 连接数据库
	if (m_conn.connecttodb(m_selcfg.connstr.c_str(), m_selcfg.charset.c_str()) != 0)
	{
		SERVER_LOG_INFO(logger) << "connect database(" << m_selcfg.connstr << ", " << m_conn.m_cda.message << ") failed.";
		return -1;
	}
	SERVER_LOG_INFO(logger) << "connect database(" << m_selcfg.connstr << ", " << m_selcfg.charset << ") ok.";

	// 准备查询的SQL语句
	SqlStatement stmt(&m_conn);
	stmt.prepare(m_selcfg.selectsql.c_str());
	// char fieldname[m_fieldcount][1000+1];
	for (int i = 1; i < m_fieldcount + 1; i++)
	{
		stmt.bindout(i, m_fieldname[i-1], m_fieldlen[i-1]);
		// SERVER_LOG_INFO(logger) << fieldname[i-1];
	}

	if (stmt.execute() != 0)
	{
		if (stmt.m_cda.rc != 1062)
		{
			SERVER_LOG_INFO(logger) << "stmt.execute() failed.\n"
									<< "sql: \n" << stmt.m_sql
									<< "message: " << stmt.m_cda.message;
			return false;
		}
	}

	while (true)
	{
		writeToFile(stmt);
		if (m_flag > -1)
			continue;
		else if (m_flag == -1)
			break;
	}

	return true;
}

string DBToFile::getFileName()
{
	time_t timep;
	struct tm tm;
	time(&timep);
	localtime_r(&timep, &tm);
	char strtime[64];
	strftime(strtime, sizeof(strtime), "%Y%m%d%H%M%S", &tm);

	char buf[5];
	sprintf(buf, "%d", m_flag);
	string filename = m_selcfg.outpath + "/" + m_selcfg.bfilename + "_" + strtime + "_" + m_selcfg.efilename + ".xml";
	if (m_flag > -1)
		filename = m_selcfg.outpath + "/" + m_selcfg.bfilename + "_" + strtime + "_" + m_selcfg.efilename + "_" + buf + ".xml";

	return filename;
}

bool DBToFile::writeToFile(SqlStatement &stmt)
{
	string filename = getFileName();
	ofstream ofs(filename);
	if (!ofs.is_open())
	{
		SERVER_LOG_INFO(logger) << "open file(" << filename << ") failed.";
		return false;
	}

	ofs << "<data>\n";

	int count = 0;
	while (true)
	{
		memset(m_fieldname, 0, sizeof(m_fieldname));
		if (stmt.next() != 0)
			break;

		stringstream ss;
		for(int i = 1; i <= m_fieldcount; i++)
		{
			ss << "<" << m_fieldstr[i-1] << ">" << m_fieldname[i-1] << "</" << m_fieldstr[i-1] << ">";
		}
		ss << "\n";
		ofs << ss.str();
		count++;
		
		if (count >= 1000)
		{
			ofs << "</data>";
			ofs.close();
			SERVER_LOG_INFO(logger) << "write " << count << " items to (" << filename << ").";
			m_flag++;
			return true;
		}
	}

	ofs << "</data>";
	ofs.close();

	SERVER_LOG_INFO(logger) << "write " << count << " items to (" << filename << ").";
	m_flag = -1;
	return 0;
}

DBToFile::~DBToFile()
{

}