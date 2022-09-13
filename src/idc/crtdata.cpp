#include "crtdata.h"
#include "../log/log.h"

static server::Logger::ptr logger = SERVER_LOG_NAME("system");
static server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/logfile/crtdata.log"));

CrtData::CrtData(const string file, const string datapath)
{
	logger->addAppender(file_appender);
	m_stcodeFile = file;
	m_datapath = datapath;
	if (loadStcode() == true);
		SERVER_LOG_INFO(logger) << "站点信息读取完成！";
}

bool CrtData::loadStcode()
{
	ifstream infile(m_stcodeFile);

	if (!infile.is_open())
	{
		SERVER_LOG_DEBUG(logger) << "open file (" << m_stcodeFile <<") failed.";
		return false;
	}

	CCmdStr CmdStr;
	string s;
	struct st_stcode stcode;
	while (getline(infile, s))
	{
		// 把读取到的一行拆分
		CmdStr.SplitToCmd(s, ",", true);

		if ((CmdStr.CmdCount() != 6)) continue;

		// TODO 把站点参数的每个数据项保存到站点参数结构体中。
		CmdStr.GetValue(0, stcode.provname, 30);
		CmdStr.GetValue(1, stcode.obtid, 10);
		CmdStr.GetValue(2, stcode.obtname, 30);
		CmdStr.GetValue(3, &stcode.lat);
		CmdStr.GetValue(4, &stcode.lon);
		CmdStr.GetValue(5, &stcode.height);

		m_stcode.push_back(stcode);
	}
	infile.close();
	return true;
}

bool CrtData::run()
{
	// 定义随机数种子
	srand(time(0));

	// // 获取当前时间，当成观测时间
	char strddatetime[21];
	memset(strddatetime, 0, sizeof(strddatetime));
	LocalTime(strddatetime, "yyyymmddhh24miss");
	m_time = strddatetime;
	// cout << string(strddatetime) << endl;
	struct st_surfdata stsurfdata;
	
	// 遍历气象站点参数的vscode容器
	for (size_t i=0; i < m_stcode.size(); i++)
	{
		memset(&stsurfdata, 0, sizeof(struct st_surfdata));
		// 用随机数填充分钟观测数据的结构体
		memcpy(stsurfdata.obtid, m_stcode[i].obtid, 10);	// 站点代码
		memcpy(stsurfdata.ddatetime, strddatetime, 14);		// 数据时间：格式yyyymmddhh24misss
		stsurfdata.t = rand()%351;							// 气温：单位。0.1摄氏度
		stsurfdata.p = rand()%256+10000;					// 气压：0.1百帕
		stsurfdata.u = rand()%100+1;						// 相对湿度，0-100之间的值
		stsurfdata.wd = rand()%360;							// 风向，0-360之间的值
		stsurfdata.wf = rand()%150;							// 风速：单位0.1m/s
		stsurfdata.r = rand()%16;							// 降雨量：0.1mm
		stsurfdata.vis = rand()%5001+100000;				// 能见度：0.1米

		// 将观测数据放入容器
		m_stdata.push_back(stsurfdata);
	}
	SERVER_LOG_INFO(logger) << "站点随机气候数据生成成功！";
	return true;
}

bool CrtData::writeToFile(const string datafmt)
{
	stringstream ss;
	ss << m_datapath << "/SURF_ZH_" << m_time << "_" << getpid() << "." << datafmt;
	string fileName = ss.str();

	ss << ".bak"; string fileNameBak = ss.str();

	ofstream outfile(ss.str(), std::ios::app);
	if (!outfile.is_open())
	{
		SERVER_LOG_INFO(logger) << "open file (" << ss.str() <<") failed.";
		return false;
	}

	// 写入第一行标题
	if (datafmt == "csv")
	{
		outfile << "站点代码, 数据时间, 气温, 气压, 相对湿度, 风向, 风速, 降雨量, 能见度\n";

		for (auto it = m_stdata.begin(); it != m_stdata.end(); it++)
		{
			outfile << it->obtid << "," << it->ddatetime << "," << it->t / 10.0 << ","
					<< it->p / 10.0 << "," << it->u << "," << it->wd << "," 
					<< it->wf / 10.0 << "," << it->r / 10.0 << "," <<it->vis / 10.0 << "\n";
		}
	}
	else if (datafmt == "xml")
	{
		outfile << "<data>";

		for (auto it = m_stdata.begin(); it != m_stdata.end(); it++)
		{
			outfile << "<obtid>" << it->obtid << "</obtid><ddatetime>" << it->ddatetime
					<< "</ddatetime><t>" << it->t / 10.0 << "</t><p>" << it->p / 10.0
					<< "</p><u>" << it->u << "</u><wd>" << it->wd << "</wd><wf>"
					<< it->wf / 10.0 << "</wf><r>" << it->r / 10.0 << "</r><vis>"
					<< it->vis / 10.0 << "</vis><endl>\n";
		}

		outfile << "</data>";
	}
	else if (datafmt == "json")
	{
		outfile << "{\"data\":[\n";

		for (size_t i = 0; i < m_stdata.size(); i++)
		{
			outfile << "{\"obtid\":\"" << m_stdata[i].obtid << "\", \"ddatetime\":\"" << m_stdata[i].ddatetime
					<< "\",\"t\":\"" << m_stdata[i].t / 10.0 << "\", \"p\":\"" << m_stdata[i].p / 10.0
					<< "\", \"u\":\"" << m_stdata[i].u << "\", \"wd\":\"" << m_stdata[i].wd 
					<< "\", \"wf\":\"" << m_stdata[i].wf / 10.0 << "\", \"r\":\"" << m_stdata[i].r / 10.0 
					<< "\", \"vis\":\""<< m_stdata[i].vis / 10.0 << "\"}";
			if (i == m_stdata.size() - 1)
				outfile << "\n";
			else
				outfile << ",\n";
		}

		outfile << "]}\n";
	}

	outfile.close();

	RENAME(fileNameBak.c_str(), fileName.c_str());
	SERVER_LOG_INFO(logger) << "生成数据文件(" << fileName << ")成功，数据时间(" << m_time << ")，记录数：" << m_stdata.size();
	return true;
}