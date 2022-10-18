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
#include "../include/address.h"
#include "../include/socket.h"
#include "../include/mysql.h"
#include "../include/thread.h"
#include "../include/threadpool.h"
#include "../include/mutex.h"
#include "../include/sqlconnpool.h"

static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");


// static server::SqlConnPool* sqlPool = server::sqlConnPool::GetInstance();

void run(server::Socket::ptr sock, server::MySQL::ptr mysql);

// void test(server::Socket::ptr sock, server::MySQL::ptr mysql);

bool _xmtoarg(char *strxmlbuffer);

int ReadT(const int sockfd, char *buffer, const int size, const int itimeout);

bool Login(server::MySQL::ptr mysql, const char *buffer, server::Socket::ptr sock);

bool CheckPerm(server::MySQL::ptr mysql, const char *buffer, server::Socket::ptr sock);

bool ExecSQL(server::MySQL::ptr mysql, const char *buffer, server::Socket::ptr sock);

bool getvalue(const char *buffer, const char *name, char *value, const int len);
std::string getvalue(const char *buffer, const std::string name);

int main(int argc, char const *argv[])
{
    server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
    if(addr) {
        SERVER_LOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        SERVER_LOG_ERROR(g_logger) << "get address fail";
        return -1;
    }
    server::Socket::ptr sock = server::Socket::CreateTCP(addr);
    
    if(!sock->bind(addr)) {
        SERVER_LOG_ERROR(g_logger) << "bind fail errno="
            << errno << " errstr=" << strerror(errno)
            << " addr=[" << addr->toString() << "]";
        return -1;
    }
    if(!sock->listen()) {
        SERVER_LOG_ERROR(g_logger) << "listen fail errno="
            << errno << " errstr=" << strerror(errno)
            << " addr=[" << addr->toString() << "]";
        return -1;
    }

	std::map<std::string, std::string> params;
	params["port"] = "3306";
	params["host"] = "127.0.0.1";
	params["user"] = "yc";
	params["passwd"] = "436052";
	params["dbname"] = "IDC";
	server::MySQL::ptr mysql(new server::MySQL(params));
	std::queue<server::Socket::ptr> client_pool;
	server::Socket::ptr cli;

	while (true)
	{
		// server::ThreadPool::ptr tp(new server::ThreadPool(5));
		cli = sock->accept();
		SERVER_LOG_INFO(g_logger) << cli->toString();
		if (cli)
		{
			client_pool.push(cli);
		}
		// 接受客户端的连接
		if (client_pool.empty())
			continue;
		
		server::Socket::ptr client = client_pool.front();
		client->setRecvTimeout(100);
		server::Address::ptr ip = client->getRemoteAddress();
		SERVER_LOG_INFO(g_logger) << ip->toString();
		std::function<void()> run1 = std::bind(run, client, mysql);
		server::Thread::ptr thr(new server::Thread(run1, "name_" + ip->toString()));
			// std::function<void()> run1 = std::bind(run, client, mysql);
			// tp->AddTask(run1);
		client_pool.pop();
	}

	return 0;
}

void run(server::Socket::ptr sock, server::MySQL::ptr mysql)
{
	char strrecvbuf[1024];
	char strsendbuf[1024];
	memset(strrecvbuf, 0, sizeof(strrecvbuf));

	SERVER_LOG_INFO(g_logger) << "子进程开始接收数据...";

	// 读取客户端的报文，如果超时或失败，线程退出
	if (sock->recv(strrecvbuf, strlen(strrecvbuf)) <= 0)
	{
		SERVER_LOG_INFO(g_logger) << "读取客户端报文失败";
		return;
	}
	SERVER_LOG_INFO(g_logger) << strrecvbuf;
	// 如果不是GET请求报文则不处理，线程退出
	if (strncmp(strrecvbuf, "GET", 3) != 0)
	{
		sock->close();
		return;
	}

	if(!mysql->connect())
	{
		std::cout << "connect fail" << std::endl;
	}

	// 判断URL中用户名和密码，如果不正确，返回认证失败的相应报文，线程退出
	if (Login(mysql, strrecvbuf, sock) == false)
	{
		sock->close();
		return;
	}

	// 判断用户是否有调用接口的权限，如果没有，返回没有权限的相应报文，线程退出
	if (CheckPerm(mysql, strrecvbuf, sock) == false)
	{
		sock->close();
		return;
	}

	// 先把响应报文的头部发送给客户端
	memset(strsendbuf, 0, sizeof(strsendbuf));
	sprintf(strsendbuf, \
		"HTTP/1.1 200 OK\r\n" \
		"Server: webserver\r\n" \
		"Content-Type: text/html;charset=utf-8\r\n\r\n");
	if(sock->send(strsendbuf, strlen(strsendbuf)) <= 0)
	{
		SERVER_LOG_ERROR(g_logger) << "发送数据失败";
	}

	// 再执行接口的sql语句，把数据返回给客户端。
	if (ExecSQL(mysql, strrecvbuf, sock) == false)
	{
		SERVER_LOG_ERROR(g_logger) << "执行sql语句失败";
		sock->close();
		return;
	}

	sock->close();
}

bool ExecSQL(server::MySQL::ptr mysql, const char *buffer, server::Socket::ptr sock)
{
	// 从请求报文中解析接口名
	std::string interid = getvalue(buffer, "interid");

	// 从接口参数配置表T_INTERCFG中加载接口参数
	auto res = std::dynamic_pointer_cast<server::MySQLStmtRes>(mysql->queryStmt(
		"select selectsql,colstr,bindin from T_INTERCFG where interid=?", interid
	));

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
	
	res->next();
	std::string selectsql = res->getString(0);
	std::string colstr = res->getString(1);
	std::string bindin = res->getString(2);
	SERVER_LOG_INFO(g_logger) << selectsql;
	SERVER_LOG_INFO(g_logger) << colstr;
	SERVER_LOG_INFO(g_logger) << bindin;


	// 准备查询数据的SQL语句
	res = std::dynamic_pointer_cast<server::MySQLStmtRes>(mysql->queryStmt(
		selectsql.c_str()));

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

	// 向客户端发送XML内容的头部标签<data>
	sock->send("<data>\n", strlen("<data>\n"));

	CCmdStr CmdStr;
	CmdStr.SplitToCmd(colstr, ",");
	char strsendbuffer[4001];
	memset(strsendbuffer, 0, sizeof(strsendbuffer));
	std::string line;
	while (res->next())
	{
		for (int i = 0; i < CmdStr.CmdCount(); i++)
		{
			switch (res->getColumnType(i))
			{
			case 254 :
				line += "<" + CmdStr.m_vCmdStr[i] + ">" + res->getString(i).c_str() + "</" + CmdStr.m_vCmdStr[i] + ">";
				break;
			case 3 :
				line += "<" + CmdStr.m_vCmdStr[i] + ">" + std::to_string(res->getInt32(i)) + "</" + CmdStr.m_vCmdStr[i] + ">";
				break;
			default:
				line += "<" + CmdStr.m_vCmdStr[i] + ">" + res->getString(i).c_str() + "</" + CmdStr.m_vCmdStr[i] + ">";
				break;
			}
		}
		line += "<endl>\n";
		// SERVER_LOG_INFO(g_logger) << line;
		memcpy(strsendbuffer, line.c_str(), line.size() + 1);
		sock->send(strsendbuffer, strlen(strsendbuffer));
		line.clear();
	}
	sock->send("</data>\n", strlen("</data>\n"));

	std::vector<std::string> invalue;

	return true;
}

bool CheckPerm(server::MySQL::ptr mysql, const char *buffer, server::Socket::ptr sock)
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

		sock->send(strbuffer,strlen(strbuffer));
		return false;
	}

	SERVER_LOG_INFO(g_logger) << "username = " << username
			<< " interid = " << intername;

	return true;
}

bool Login(server::MySQL::ptr mysql,const char *buffer, server::Socket::ptr sock)
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
		sock->send(strbuffer, strlen(strbuffer));

		return false;
	}
	SERVER_LOG_INFO(g_logger) << "user : " << username << " login";

	return true;
}

/**
 * @brief 
 * GET /?username=ty&passwd HTTP/1.1
 * @param buffer 
 * @param name 
 * @param value 
 * @param len 
 * @return true 
 * @return false 
 */
std::string getvalue(const char *buffer, const std::string name)
{
	std::string str(buffer);
	size_t start = str.find(name) + name.size() + 1;
	str = str.substr(start);

	size_t end = str.find("HTTP");
	std::string ret = str.substr(0, end - 1);

	return ret;
}

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

// bool _xmltoarg(char *strxmlbuffer)
// {
// 	memset(&starg, 0, sizeof(struct st_arg));
// 	GetXMLBuffer(strxmlbuffer, "connstr", starg.connstr, 100);
// 	if (strlen(starg.connstr) == 0) 
// 	{
// 		SERVER_LOG_INFO(g_logger) << "connstr is null";
// 		return false;
// 	}

// 	GetXMLBuffer(strxmlbuffer, "charset", starg.charset, 50);
// 	if (strlen(starg.charset) == 0) 
// 	{
// 		SERVER_LOG_INFO(g_logger) << "charset is null";
// 		return false;
// 	}

// 	GetXMLBuffer(strxmlbuffer, "port", &starg.port);
// 	if (starg.port == 0)
// 	{
// 		SERVER_LOG_INFO(g_logger) << "port is null";
// 		return false;
// 	}
// 	return true;
// }

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