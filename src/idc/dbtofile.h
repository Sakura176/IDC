#ifndef _DBTOFILE_H_
#define _DBTOFILE_H_

#include "../public/_public.h"
#include "../config/config.h"
#include "../mysql/_mysql.h"

struct SelectCfg
{
	string connstr;			// 数据库连接参数
	string charset;			// 数据库字符集
	string selectsql;		// 从数据库抽取数据的SQL语句
	string fieldstr;		// 抽取数据的SQL语句输出结果集字段名，字段名之间用逗号分隔
	string fieldlen;		// 抽取数据的SQL语句输出结果集字段的长度，用逗号分割
	string bfilename;		// 输出xml文件的前缀
	string efilename;		// 输出xml文件的后缀
	string outpath;			// 文件存放的目录

	bool operator==(const SelectCfg& oth) const
	{
		return connstr == oth.connstr 
			&& charset == oth.charset 
			&& selectsql == oth.selectsql 
			&& fieldstr == oth.fieldstr 
			&& fieldlen == oth.fieldlen 
			&& bfilename == oth.bfilename 
			&& efilename == oth.efilename 
			&& outpath == oth.outpath;
	}

	bool operator<(const SelectCfg& oth) const
	{
		return connstr < oth.connstr;
	}

	void toString()
	{

	}
};

class DBToFile
{
public:
	typedef std::shared_ptr<DBToFile> ptr;

	DBToFile(string cfg);
	~DBToFile();

	bool selInDB();

	bool run();

	bool writeToFile(SqlStatement &stmt);
private:
	string getFileName();

private:
	string m_cfg;					// 配置参数文件路径
	Connection m_conn;
	char m_fieldstr[500][31];
	int m_fieldlen[500];
	char m_fieldname[500][1000];
	int m_fieldcount;
	int m_flag;
	// string m_item;
	SelectCfg m_selcfg;
};

#endif