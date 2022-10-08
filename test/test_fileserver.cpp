/**
 * @file test_fileserver.cpp
 * @author your name (you@domain.com)
 * @brief 多线程文件传输服务端
 * @version 0.1
 * @date 2022-09-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../include/log.h"
#include "../include/_public.h"
#include "../include/_mysql.h"
#include "../include/mysql.h"
#include "../include/thread.h"
#include "../include/mutex.h"
#include "../include/sqlconnpool.h"

static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");
TcpServer g_tcpServer;

struct st_arg
{
	char connstr[101];
	char charset[51];
	int port;
} starg;

server::Mutex mutex;

// static server::SqlConnPool* sqlPool = server::sqlConnPool::GetInstance();

void run(int sockfd, server::MySQL::ptr mysql);

bool _xmtoarg(char *strxmlbuffer);

int ReadT(const int sockfd, char *buffer, const int size, const int itimeout);

bool Login(server::MySQL::ptr mysql, const char *buffer, const int sockfd);

bool CheckPerm(server::MySQL::ptr mysql, const char *buffer, const int sockfd);

bool ExecSQL(server::MySQL::ptr mysql, const char *buffer, const int sockfd);

bool getvalue(const char *buffer, const char *name, char *value, const int len);

int main(int argc, char const *argv[])
{
	// server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/bin/logfile/fileserver.log"));
    // g_logger->addAppender(file_appender);

	// memset(&starg, 0, sizeof(struct st_arg));
	// sprintf(starg.connstr, "127.0.0.1,yc,436052,IDC,3306");
	// sprintf(starg.charset, "utf8");
	// starg.port = 3306;

	std::map<std::string, std::string> params;
	params["port"] = "3306";
	params["host"] = "127.0.0.1";
	params["user"] = "yc";
	params["passwd"] = "436052";
	params["dbname"] = "IDC";

	server::MySQL::ptr mysql(new server::MySQL(params));

	if (g_tcpServer.InitServer(3389) == false)
	{
		SERVER_LOG_INFO(g_logger) << "tcpServer: init failed.";
		return -1;
	}
	SERVER_LOG_INFO(g_logger) << "tcpServer: init ok.";

	while (true)
	{
		// 接受客户端的连接
		if (g_tcpServer.Accept() == false) continue;
		// SERVER_LOG_INFO(g_logger) << "client IP: " << g_tcpServer.GetIP() << " connected。";
		string ip = g_tcpServer.GetIP();
		std::function<void()> run1 = std::bind(run, g_tcpServer.m_connfd, mysql);
		server::Thread::ptr thr(new server::Thread(run1, "name_" + ip));
	}

	return 0;
}

void run(int sockfd, server::MySQL::ptr mysql)
{
	char strrecvbuf[1024];
	char strsendbuf[1024];
	memset(strrecvbuf, 0, sizeof(strrecvbuf));

	// 读取客户端的报文，如果超时或失败，线程退出
	if (ReadT(sockfd, strrecvbuf, sizeof(strrecvbuf), 100) == -1)
	{
		SERVER_LOG_INFO(g_logger) << "读取客户端报文失败";
		return;
	}

	// 如果不是GET请求报文则不处理，线程退出
	if (strncmp(strrecvbuf, "GET", 3) != 0)
	{
		close(sockfd);
		return;
	}

	if(!mysql->connect())
	{
		std::cout << "connect fail" << std::endl;
	}

	// 判断URL中用户名和密码，如果不正确，返回认证失败的相应报文，线程退出
	if (Login(mysql, strrecvbuf, sockfd) == false)
	{
		close(sockfd);
		return;
	}

	// 判断用户是否有调用接口的权限，如果没有，返回没有权限的相应报文，线程退出
	if (CheckPerm(mysql, strrecvbuf, sockfd) == false)
	{
		close(sockfd);
		return;
	}

	// 先把响应报文的头部发送给客户端
	memset(strsendbuf, 0, sizeof(strsendbuf));
	sprintf(strsendbuf, \
		"HTTP/1.1 200 OK\r\n" \
		"Server: webserver\r\n" \
		"Content-Type: text/html;charset=utf-8\r\n\r\n");
	Writen(sockfd, strsendbuf, strlen(strsendbuf));

	// 再执行接口的sql语句，把数据返回给客户端。
	if (ExecSQL(mysql, strsendbuf, sockfd) == false)
	{
		SERVER_LOG_ERROR(g_logger) << "执行sql语句失败";
		close(sockfd);
		return;
	}

	close(sockfd);
}

bool ExecSQL(server::MySQL::ptr mysql, const char *buffer, const int sockfd)
{
	// 从请求报文中解析接口名
	char intername[30];
	memset(intername, 0, sizeof(intername));
	getvalue(buffer, "interid", intername, 30);
	std::string interid(intername);

	// 从接口参数配置表T_INTERCFG中加载接口参数

	auto stmt = std::dynamic_pointer_cast<server::MySQLStmt>(
		mysql->prepare("select selectsql,colstr,bindin from T_INTERCFG where interid=?"));
	stmt->bind(1, interid);

	auto res = std::dynamic_pointer_cast<server::MySQLStmtRes>(stmt->query());

	if(!res)
	{
		SERVER_LOG_ERROR(g_logger) << "invalid";
		return false;
	}

	if(res->getErrno()) {
		SERVER_LOG_ERROR(g_logger) << "errno=" << res->getErrno()
								   << " errstr=" << res->getErrStr() << std::endl;
		return false;
	}
	
	// while (res->next())
	// {
	int colcount = res->getColumnCount();
	std::string selectstr = res->getString(0);
	SERVER_LOG_INFO(g_logger) << colcount << " - " << selectstr;
	// }


	// 准备查询数据的SQL语句
	// stmt.prepare(selectsql);

	// CCmdStr CmdStr;
	// CmdStr.SplitToCmd(bindin, ",");

	// char invalue[CmdStr.CmdCount()][101];
	// memset(invalue, 0, sizeof(invalue));

	// for (int i = 0; i < CmdStr.CmdCount(); i++)
	// {
	// 	getvalue(buffer, CmdStr.m_vCmdStr[i].c_str(), invalue[i], 100);
	// 	stmt.bindin(i + 1, invalue[i], 100);
	// }

	// // 拆分colstr，可以得到结果集的字段数
	// CmdStr.SplitToCmd(colstr, ",");

	// // 用于存放结果集的数组
	// char colvalue[CmdStr.CmdCount()][2001];

	// // 把结果集绑定到colvalue数组
	// for (int i = 0; i < CmdStr.CmdCount(); i++)
	// {
	// 	stmt.bindout(i + 1, colvalue[i], 2000);
	// }

	// // 执行SQL语句
	// char strsendbuffer[4001];
	// memset(strsendbuffer, 0, sizeof(strsendbuffer));

	// if (stmt.execute() != 0)
	// {
	// 	sprintf(strsendbuffer, "<retcode>%d</retcode><message>%s</message>\n", stmt.m_cda.rc, stmt.m_cda.message);
	// 	Writen(sockfd, strsendbuffer, strlen(strsendbuffer));
	// 	printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
	// }
	// strcpy(strsendbuffer, "<retcode>0</retcode><message>ok</message>\n");
	// Writen(sockfd, strsendbuffer, strlen(strsendbuffer));

	// // 向客户端发送XML内容的头部标签<data>
	// Writen(sockfd, "<data>\n", strlen("<data>\n"));

	// char strtemp[2001];

	// while (true)
	// {
	// 	memset(strsendbuffer, 0, sizeof(strsendbuffer));
	// 	memset(colvalue, 0, sizeof(colvalue));

	// 	if (stmt.next() != 0) break;

	// 	for (int i = 0; i < CmdStr.CmdCount(); i++)
	// 	{
	// 		memset(strtemp, 0, sizeof(strtemp));
	// 		snprintf(strtemp, 2000, "<%s>%s</%s>", CmdStr.m_vCmdStr[i].c_str(),
	// 			colvalue[i], CmdStr.m_vCmdStr[i].c_str());
	// 		strcat(strsendbuffer, strtemp);
	// 	}

	// 	strcat(strsendbuffer, "<endl/>\n");

	// 	Writen(sockfd, strsendbuffer, strlen(strsendbuffer));
	// }
	// Writen(sockfd, "</data>\n", strlen("</data>\n"));

	// SERVER_LOG_INFO(g_logger) << "intername=" << intername
	// 	<< ", count=" << stmt.m_cda.rpc;
	
	// conn->commit();
	return true;
}

bool CheckPerm(server::MySQL::ptr mysql, const char *buffer, const int sockfd)
{
	char username[31], intername[31];

	getvalue(buffer, "username", username, 30);
	getvalue(buffer, "interid", intername, 30);
	std::string un(username);
	std::string intname(intername);
	int icount = 0;

	// stmt.connect(conn);
	auto stmt = std::dynamic_pointer_cast<server::MySQLStmt>(
		mysql->prepare("select count(*) from T_USERANDINTER where username=? and interid=? and interid in (select interid from T_INTERCFG where rsts=1)")
	);
	stmt->bind(1, un);
	stmt->bind(2, intname);
	
	server::ISQLData::ptr res = stmt->query();
	res->next();
	icount = res->getInt8(0);

	if (icount != 1)
	{
		char strbuffer[256];
		memset(strbuffer,0,sizeof(strbuffer));

		sprintf(strbuffer,\
			"HTTP/1.1 200 OK\r\n"\
			"Server: webserver\r\n"\
			"Content-Type: text/html;charset=utf-8\r\n\r\n"\
			"<retcode>-1</retcode><message>permission denied</message>");

		Writen(sockfd,strbuffer,strlen(strbuffer));
		return false;
	}

	SERVER_LOG_INFO(g_logger) << "username = " << username
			<< " interid = " << intername;

	return true;
}

bool Login(server::MySQL::ptr mysql,const char *buffer, const int sockfd)
{
	char username[31];
	char password[31];
	getvalue(buffer, "username", username, 30);		// 获取用户名
	getvalue(buffer, "passwd", password, 30);		// 获取密码
	std::string un(username);
	std::string pwd(password);
	int icount = 0;

	auto istmt = mysql->prepare("select count(*) from T_USERINFO where username=? and passwd=? and rsts=1");
	server::MySQLStmt::ptr stmt = std::dynamic_pointer_cast<server::MySQLStmt>(istmt);
	stmt->bind(1, un);
	stmt->bind(2, pwd);
	
	server::ISQLData::ptr res = stmt->query();
	res->next();
	icount = res->getInt8(0);
	if (icount == 0)
	{
		char strbuffer[256];
		memset(strbuffer, 0, sizeof(strbuffer));

		sprintf(strbuffer, \
			"HTTP/1.1 200 OK\r\n"\
			"Server: webserver\r\n"\
			"Content-Type: text/html;charset=utf-8\r\n\r\n"\
			"<retcode>-1</retcode><message>username or passwd is invailed</message>");
		Writen(sockfd, strbuffer, strlen(strbuffer));

		return false;
	}
	SERVER_LOG_INFO(g_logger) << "user : " << username << " login";

	return true;
}

/**
 * @brief 
 * GET /?username\=ty\&passwd HTTP/1.1
 * @param buffer 
 * @param name 
 * @param value 
 * @param len 
 * @return true 
 * @return false 
 */
bool getvalue(const char *buffer, const char *name, char *value, const int len)
{
	value[0] = 0;

	char *start, *end;
	start = end = 0;

	start = strstr((char *)buffer, (char *)name);
	if (start == 0) return false;

	end = strstr(start, "&");
	if (end == 0) end = strstr(start, " ");

	if (end == 0) return false;

	int ilen = end - (start + strlen(name) + 1);
	if (ilen > len) ilen = len;

	strncpy(value, start + strlen(name) + 1, ilen);
	value[ilen] = 0;

	return true;
}

bool _xmltoarg(char *strxmlbuffer)
{
	memset(&starg, 0, sizeof(struct st_arg));
	GetXMLBuffer(strxmlbuffer, "connstr", starg.connstr, 100);
	if (strlen(starg.connstr) == 0) 
	{
		SERVER_LOG_INFO(g_logger) << "connstr is null";
		return false;
	}

	GetXMLBuffer(strxmlbuffer, "charset", starg.charset, 50);
	if (strlen(starg.charset) == 0) 
	{
		SERVER_LOG_INFO(g_logger) << "charset is null";
		return false;
	}

	GetXMLBuffer(strxmlbuffer, "port", &starg.port);
	if (starg.port == 0)
	{
		SERVER_LOG_INFO(g_logger) << "port is null";
		return false;
	}
	return true;
}

int ReadT(const int sockfd, char *buffer, const int size, const int itimeout)
{
	if (itimeout > 0)
	{
		struct pollfd fds;
		fds.fd = sockfd;
		fds.events = POLLIN;
		int iret;
		if ( (iret = poll(&fds, 1, itimeout*1000)) <= 0)
			return iret;
	}
	return recv(sockfd, buffer, size, 0);
}