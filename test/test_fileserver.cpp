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

static server::SqlConnPool* sqlPool = server::sqlConnPool::GetInstance();

void run(int sockfd);

bool _xmtoarg(char *strxmlbuffer);

int ReadT(const int sockfd, char *buffer, const int size, const int itimeout);

bool Login(SqlStatement &stmt, Connection::ptr conn, const char *buffer, const int sockfd);

bool CheckPerm(SqlStatement &stmt, Connection::ptr conn, const char *buffer, const int sockfd);

bool ExecSQL(SqlStatement &stmt, Connection::ptr conn, const char *buffer, const int sockfd);

bool getvalue(const char *buffer, const char *name, char *value, const int len);

int main(int argc, char const *argv[])
{
	server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/bin/logfile/fileserver.log"));
    g_logger->addAppender(file_appender);

	memset(&starg, 0, sizeof(struct st_arg));
	sprintf(starg.connstr, "127.0.0.1,yc,436052,IDC,3306");
	sprintf(starg.charset, "utf8");
	starg.port = 3306;

	if (g_tcpServer.InitServer(3389) == false)
	{
		SERVER_LOG_INFO(g_logger) << "tcpServer: init failed.";
		return -1;
	}
	SERVER_LOG_INFO(g_logger) << "tcpServer: init ok.";
	if (sqlPool->Init(starg.connstr, starg.charset, 20) == false)
	{
		SERVER_LOG_INFO(g_logger) << "数据库连接池初始化失败";
	}

	while (true)
	{
		// 接受客户端的连接
		if (g_tcpServer.Accept() == false) continue;
		// SERVER_LOG_INFO(g_logger) << "client IP: " << g_tcpServer.GetIP() << " connected。";
		string ip = g_tcpServer.GetIP();
		std::function<void()> run1 = std::bind(run, g_tcpServer.m_connfd);
		server::Thread::ptr thr(new server::Thread(run1, "name_" + ip));
	}

	return 0;
}

void run(int sockfd)
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

	SERVER_LOG_INFO(g_logger) << strrecvbuf;

	// 如果不是GET请求报文则不处理，线程退出
	if (strncmp(strrecvbuf, "GET", 3) != 0)
	{
		close(sockfd);
		return;
	}
	// 相应报文头部发送给客户端
	// char sendinfo[] = "just test";
	// sprintf(strsendbuf,
	// 		"HTTP/1.1 200 OK\r\n"
	// 		"Server: webserver\r\n"
	// 		"Content-Type: text/html;charset=utf-8\r\n"
	// 		"Content-Lenght: %ld\r\n\r\n", strlen(sendinfo));
	// if (Writen(sockfd, strsendbuf, strlen(strsendbuf)) == false)
	// 	return;
	// if (Writen(sockfd, sendinfo, strlen(sendinfo)) == false)
	// 	return;

	// 连接数据库
	Connection::ptr conn = sqlPool->GetConn();
	SqlStatement stmt;
	if (conn->connecttodb("127.0.0.1,yc,436052,IDC,3306", "utf8") != 0)
	{
		SERVER_LOG_INFO(g_logger) << "connect database(" << starg.connstr 
			<< ", " << conn->m_cda.message << ") failed.";
		return;
	}

	// 判断URL中用户名和密码，如果不正确，返回认证失败的相应报文，线程退出
	if (Login(stmt, conn, strrecvbuf, sockfd) == false)
	{
		close(sockfd);
		return;
	}

	// 判断用户是否有调用接口的权限，如果没有，返回没有权限的相应报文，线程退出
	if (CheckPerm(stmt, conn, strrecvbuf, sockfd) == false)
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
	if (ExecSQL(stmt, conn, strsendbuf, sockfd) == false)
	{
		close(sockfd);
	}

	close(sockfd);
}

bool ExecSQL(SqlStatement &stmt, Connection::ptr conn, const char *buffer, const int sockfd)
{
	// 从请求报文中解析接口名
	char intername[30];
	memset(intername, 0, sizeof(intername));
	getvalue(buffer, "intername", intername, 30);

	// 从接口参数配置表T_INTERCFG中加载接口参数
	char selectsql[1001], colstr[301], bindin[301];
	memset(selectsql, 0, sizeof(selectsql));
	memset(colstr, 0, sizeof(colstr));
	memset(bindin, 0, sizeof(bindin));

	stmt.connect(conn);
	stmt.prepare("select selectsql,colstr,bindin from T_INTERCFG where intername=:1");
	stmt.bindin(1, intername, 30);
	stmt.bindout(1, selectsql, 1000);
	stmt.bindout(2, colstr, 300);
	stmt.bindout(3, bindin, 300);
	stmt.execute();
	stmt.next();

	// 准备查询数据的SQL语句
	stmt.prepare(selectsql);

	CCmdStr CmdStr;
	CmdStr.SplitToCmd(bindin, ",");

	char invalue[CmdStr.CmdCount()][101];
	memset(invalue, 0, sizeof(invalue));

	for (int i = 0; i < CmdStr.CmdCount(); i++)
	{
		getvalue(buffer, CmdStr.m_vCmdStr[i].c_str(), invalue[i], 100);
		stmt.bindin(i + 1, invalue[i], 100);
	}

	// 拆分colstr，可以得到结果集的字段数
	CmdStr.SplitToCmd(colstr, ",");

	// 用于存放结果集的数组
	char colvalue[CmdStr.CmdCount()][2001];

	// 把结果集绑定到colvalue数组
	for (int i = 0; i < CmdStr.CmdCount(); i++)
	{
		stmt.bindout(i + 1, colvalue[i], 2000);
	}

	// 执行SQL语句
	char strsendbuffer[4001];
	memset(strsendbuffer, 0, sizeof(strsendbuffer));

	if (stmt.execute() != 0)
	{
		sprintf(strsendbuffer, "<retcode>%d</retcode><message>%s</message>\n", stmt.m_cda.rc, stmt.m_cda.message);
		Writen(sockfd, strsendbuffer, strlen(strsendbuffer));
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
	}
	strcpy(strsendbuffer, "<retcode>0</retcode><message>ok</message>\n");
	Writen(sockfd, strsendbuffer, strlen(strsendbuffer));

	// 向客户端发送XML内容的头部标签<data>
	Writen(sockfd, "<data>\n", strlen("<data>\n"));

	char strtemp[2001];

	while (true)
	{
		memset(strsendbuffer, 0, sizeof(strsendbuffer));
		memset(colvalue, 0, sizeof(colvalue));

		if (stmt.next() != 0) break;

		for (int i = 0; i < CmdStr.CmdCount(); i++)
		{
			memset(strtemp, 0, sizeof(strtemp));
			snprintf(strtemp, 2000, "<%s>%s</%s>", CmdStr.m_vCmdStr[i].c_str(),
				colvalue[i], CmdStr.m_vCmdStr[i].c_str());
			strcat(strsendbuffer, strtemp);
		}

		strcat(strsendbuffer, "<endl/>\n");

		Writen(sockfd, strsendbuffer, strlen(strsendbuffer));
	}
	Writen(sockfd, "</data>\n", strlen("</data>\n"));

	SERVER_LOG_INFO(g_logger) << "intername=" << intername
		<< ", count=" << stmt.m_cda.rpc;
	
	conn->commit();
	return true;
}

bool CheckPerm(SqlStatement &stmt, Connection::ptr conn, const char *buffer, const int sockfd)
{
	char username[31], intername[31];

	getvalue(buffer, "username", username, 30);
	getvalue(buffer, "intername", intername, 30);

	// stmt.connect(conn);
	stmt.prepare("select count(*) from T_USERANDINTER where username=:1 and intername=:2 and intername in (select intername from T_INTERCFG where rsts=1)");
	stmt.bindin(1, username, 30);
	stmt.bindin(2, intername, 30);
	int icount = 0;
	stmt.bindout(1, &icount);
	if (stmt.execute() != 0)
	{
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
		return false;
	}
	stmt.next();

	if (icount!=1)
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
	conn->commit();
	// stmt.disconnect();
	return true;
}

bool Login(SqlStatement &stmt, Connection::ptr conn,const char *buffer, const int sockfd)
{
	char username[31];
	char password[31];
	getvalue(buffer, "username", username, 30);		// 获取用户名
	getvalue(buffer, "passwd", password, 30);		// 获取密码

	// 查询T_USERINFO表，判断用户名和密码是否存在
	// stmt.connect(conn);
	stmt.prepare("select count(*) from T_USERINFO where username=:1 and passwd=:2 and rsts=1");
	stmt.bindin(1, username, 30);
	stmt.bindin(2, password, 30);
	int icount = 0;
	stmt.bindout(1, &icount);
	// SERVER_LOG_INFO(g_logger) << "icount -->" << icount;
	if (stmt.execute() != 0)
	{
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql,stmt.m_cda.message);
		return false;
	}
	stmt.next();

	if (icount==0)
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

	conn->commit();
	// stmt.disconnect();
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