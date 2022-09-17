#ifndef _OBTTODB_H_
#define _OBTTODB_H_

#include "../public/_public.h"
#include "../mysql/_mysql.h"
#include "../log/log.h"

struct st_stcode
{
	char provname[31];		// 省
	char obtid[11];			// 站号
	char cityname[31];		// 站名
	char lat[6];			// 纬度
	char lon[6];			// 经度
	char height[6];		// 海拔高度
};

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

class ObtToDB
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * @param _inifile 站点参数文件路径
	 * @param _datapath 观测数据路径
	 */
	ObtToDB(string inifile, string datapath);

	/**
	 * @brief 析构函数
	 * 
	 */
	~ObtToDB();

	/**
	 * @brief 设置数据库连接参数
	 * 
	 * @param connstr 数据库连接参数
	 */
	void setConnstr(string connstr);

	/**
	 * @brief 设置数据库字符集
	 * 
	 * @param connstr 数据库字符集
	 */
	void setCharset(string charset);

	/**
	 * @brief 上传站点参数到数据库
	 * 
	 * @return
	 */
	bool codeToDB();

	/**
	 * @brief 上传观测数据到数据库
	 * 
	 * @return true 
	 */
	bool dataToDB();

private:


private:
	string m_inifile;						// 站点参数文件
	string m_datapath;						// 观测数据文件路径
	Connection m_conn;						// 数据库连接对象
	string m_connstr;						// 数据库连接参数
	string m_charset;						// 数据库字符集
	st_stcode m_stcode;		// 站点参数存放容器
};

#endif