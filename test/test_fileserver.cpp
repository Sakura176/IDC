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
#include "../include/address.h"
#include "../include/socket.h"
#include "../include/mysql.h"
#include "../include/thread.h"
#include "../include/threadpool.h"
#include "../include/mutex.h"
#include "../include/epoll.h"

static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

server::Mutex g_mutex;

// static server::SqlConnPool* sqlPool = server::sqlConnPool::GetInstance();

void run(server::Socket::ptr sock, server::MySQL::ptr mysql);

void main_thread(server::Socket::ptr sock, server::Epoll::ptr ep, 
		std::map<int, server::Socket::ptr>& sock_list);

// void test(server::Socket::ptr sock, server::MySQL::ptr mysql);

bool _xmtoarg(char *strxmlbuffer);

int ReadT(const int sockfd, char *buffer, const int size, const int itimeout);

bool Login(server::MySQL::ptr mysql, const std::string msg, server::Socket::ptr sock);

bool CheckPerm(server::MySQL::ptr mysql, const std::string msg, server::Socket::ptr sock);

bool ExecSQL(server::MySQL::ptr mysql, const std::string msg, server::Socket::ptr sock);

bool getvalue(const char *buffer, const char *name, char *value, const int len);
std::string getvalue(const std::string buffer, const std::string name);

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

	server::ThreadPool::ptr tp(new server::ThreadPool(5));

	server::Epoll::ptr ep(new server::Epoll(1024));
	ep->addFd(sock->getSocket(), EPOLLIN);
    std::map<int, server::Socket::ptr> sock_list;
    sock_list[sock->getSocket()] = sock;

	while (true)
	{
		int infds = ep->wait();
		if (infds < 0)
        {
            break;
        }

        if (infds == 0)
        {
            continue;
        }

		for (int i = 0; i < infds; i++)
        {
            if (ep->getEventFd(i) == sock->getSocket())
            {
                auto client = sock->accept();
				ep->addFd(client->getSocket(), EPOLLIN);
				sock_list[client->getSocket()] = client;
				// tp->addTask(
				// 	std::bind(main_thread, sock, ep, sock_list));
			}
            else
            {
				server::MySQL::ptr mysql(new server::MySQL(params));
				tp->addTask(std::bind(run, sock_list[ep->getEventFd(i)], mysql));
			}
		}	
	}

	return 0;
}

void main_thread(server::Socket::ptr sock, server::Epoll::ptr ep, 
		std::map<int, server::Socket::ptr>& sock_list)
{
	// server::Mutex::Lock lock(g_mutex);
	while(true)
	{
		auto client = sock->accept();
		if(client)
		{
			ep->addFd(client->getSocket(), EPOLLIN);
			sock_list[client->getSocket()] = client;
			break;
		}
	}
}

void run(server::Socket::ptr sock, server::MySQL::ptr mysql)
{
	std::string msg;
	msg.resize(1024);

	if (sock->recv(&msg[0], msg.size()) <= 0)
	{
		sock->close();
		return;
	}

	// SERVER_LOG_INFO(g_logger) << msg;
	// 如果不是GET请求报文则不处理，线程退出
	if (strncmp(msg.c_str(), "GET", 3) != 0)
	{
		sock->close();
		return;
	}

	{
		// server::Mutex::Lock lock(g_mutex);
		if(!mysql->connect())
		{
			std::cout << "connect fail" << std::endl;
		}
	}

	// 判断URL中用户名和密码，如果不正确，返回认证失败的相应报文，线程退出
	if (Login(mysql, msg, sock) == false)
	{
		sock->close();
		return;
	}

	// 判断用户是否有调用接口的权限，如果没有，返回没有权限的相应报文，线程退出
	if (CheckPerm(mysql, msg, sock) == false)
	{
		sock->close();
		return;
	}

	// 先把响应报文的头部发送给客户端
	std::string sendmsg = "HTTP/1.1 200 OK\r\n" \
		"Server: webserver\r\n" \
		"Content-Type: text/html;charset=utf-8\r\n\r\n";

	if (sock->send(&sendmsg[0], sendmsg.size()) <= 0)
	{
		SERVER_LOG_ERROR(g_logger) << "发送数据失败";
		return;
	}
	// 再执行接口的sql语句，把数据返回给客户端。
	if (ExecSQL(mysql, msg, sock) == false)
	{
		SERVER_LOG_ERROR(g_logger) << "执行sql语句失败";
		sock->close();
		return;
	}

	// 查询成功，将记录插入表中
	// if (UpdateStmt(mysql, msg, sock))

	sock->close();
}

bool ExecSQL(server::MySQL::ptr mysql, const std::string msg, server::Socket::ptr sock)
{
	// 从请求报文中解析接口名
	std::string username = getvalue(msg, "username");
	std::string interid = getvalue(msg, "interid");
	server::MySQLStmtRes::ptr res;

	// 从接口参数配置表T_INTERCFG中加载接口参数
	{
		// server::Mutex::Lock lock(g_mutex);
		res = std::dynamic_pointer_cast<server::MySQLStmtRes>(mysql->queryStmt(
			"select selectsql,colstr,bindin from T_INTERCFG where interid=?", interid
		));
	}

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

	// 准备查询数据的SQL语句
	{
		// server::Mutex::Lock lock(g_mutex);
		res = std::dynamic_pointer_cast<server::MySQLStmtRes>(mysql->queryStmt(
			selectsql.c_str()));
	}

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
	int linecount = 0;
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
		linecount++;
	}
	sock->send("</data>\n", strlen("</data>\n"));
	// std::vector<std::string> invalue;
	res = std::dynamic_pointer_cast<server::MySQLStmtRes>(mysql->queryStmt(
		"select typeid from T_INTERCFG where interid=?", interid));
	res->next();
	std::string type_id = res->getString(0);
	auto stmt = std::dynamic_pointer_cast<server::MySQLStmt>(
		mysql->prepare(
			"insert into T_USERLOG(username,interid,upttime,ip,rpc) value(?,?,?,?,?)"));
	stmt->bind(1, username);
	stmt->bind(2, interid);
	stmt->bindTime(3, time(0));
	stmt->bind(4, sock->getRemoteAddress()->toString());
	stmt->bind(5, linecount);

	if (stmt->execute() != 0)
	{
		SERVER_LOG_INFO(g_logger) << "insert failed errno=" << stmt->getErrno()
			<< " errstr=" << stmt->getErrStr();
	}

	return true;
}

bool CheckPerm(server::MySQL::ptr mysql, const std::string msg, server::Socket::ptr sock)
{
	std::string username = getvalue(msg, "username");
	std::string interid = getvalue(msg, "interid");
	int icount = 0;
	// SERVER_LOG_INFO(g_logger) << "username = " << username
			// << " interid = " << interid;
	// stmt.connect(conn);
	server::MySQLStmt::ptr stmt;
	{
		// server::Mutex::Lock lock(g_mutex);
		stmt = std::dynamic_pointer_cast<server::MySQLStmt>(
			mysql->prepare("select count(*) from T_USERANDINTER where username=? and interid=? and interid in (select interid from T_INTERCFG where rsts=1)")
		);
		stmt->bind(1, username);
		stmt->bind(2, interid);
		
		server::ISQLData::ptr res = stmt->query();
		res->next();
		icount = res->getInt8(0);
	// SERVER_LOG_INFO(g_logger) << "icount --> " << icount;
	}
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
	return true;
}

bool Login(server::MySQL::ptr mysql,const std::string msg, server::Socket::ptr sock)
{
	std::string username = getvalue(msg, "username");		// 获取用户名
	std::string passwd = getvalue(msg, "passwd");		// 获取密码
	int icount = 0;

	// SERVER_LOG_INFO(g_logger) << username << " - " << passwd;
	server::MySQLStmt::ptr stmt;
	{
		// server::Mutex::Lock lock(g_mutex);
		stmt = std::dynamic_pointer_cast<server::MySQLStmt>(
			mysql->prepare("select count(*) from T_USERINFO where username=? and passwd=? and rsts=1")
		);
		stmt->bind(1, username);
		stmt->bind(2, passwd);
		
		server::ISQLData::ptr res = stmt->query();
		res->next();
		icount = res->getInt8(0);
	}

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
	// SERVER_LOG_INFO(g_logger) << "user : " << username << " login";

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
std::string getvalue(const std::string buffer, const std::string name)
{
	std::string str(buffer);
	size_t start = str.find(name) + name.size() + 1;
	if (start == std::string::npos)
		return nullptr;
	// str = str.substr(start);

	size_t end = str.find("&", start);
	if (end == std::string::npos)
		end = str.find(" ", start);
	
	if (end == std::string::npos)
		return nullptr;
	// SERVER_LOG_INFO(g_logger) << name << " start: " << start << " end: " << end << " size: " << str.size();
	std::string ret = str.substr(start, end - start);

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