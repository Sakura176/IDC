#ifndef __IDC_CRTDATA_H__
#define __IDC_CRTDATA_H__

#include "_public.h"
// #include "../log/log.h"

struct st_stcode
{
	char provname[31];	// 省
	char obtid[11];		// 站号
	char obtname[31];	// 站名
	double lat;			// 纬度
	double lon;			// 经度
	double height;		// 海拔高度
};

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

class CrtData
{
public:
	CrtData(const string file, const string datapath);
	
	~CrtData() {}

	bool loadStcode();

	bool run();

	bool writeToFile(const string datafmt);

private:
	string m_stcodeFile;
	string m_datapath;
	string m_time;
	vector<struct st_stcode> m_stcode;
	vector<struct st_surfdata> m_stdata;
};


#endif