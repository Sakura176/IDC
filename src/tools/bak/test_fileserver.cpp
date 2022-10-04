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
#include "../src/log/log.h"
#include "../src/public/_public.h"
#include "../src/thread/thread.h"

server::Logger::ptr g_logger = SERVER_LOG_ROOT();
TcpServer g_tcpServer;

void run(TcpServer tcpServer);

int main(int argc, char const *argv[])
{
	server::FileLogAppender::ptr file_appender(new server::FileLogAppender("/home/yc/IDC/logfile/fileserver.log"));
    g_logger->addAppender(file_appender);

	if (g_tcpServer.InitServer(5006) == false)
	{
		SERVER_LOG_INFO(g_logger) << "tcpServer: " << g_tcpServer.GetIP() << " init failed.";
	}
	SERVER_LOG_INFO(g_logger) << "tcpServer: " << g_tcpServer.GetIP() << " init ok.";

	while (true)
	{
		// 接受客户端的连接
		if (g_tcpServer.Accept() == false) continue;
		SERVER_LOG_INFO(g_logger) << "client IP: " << g_tcpServer.GetIP() << " connected。";
		string ip = g_tcpServer.GetIP();
		std::function<void()> run1 = std::bind(run, std::ref(g_tcpServer));
		server::Thread::ptr thr(new server::Thread(run1, "name_" + ip));
	}

	return 0;
}

void run(TcpServer tcpServer)
{
	char buffer[1024];
	while(true)
	{
		memset(buffer, 0, sizeof(buffer));
		// 接收客户端的请求报文
		if ( tcpServer.Read(buffer, 5) == false) break;
		SERVER_LOG_INFO(g_logger) << "接收信息：" << buffer;

		strcpy(buffer, "收到了");
		// 向客户端发送响应结果
		if (tcpServer.Write(buffer) == false) break;
		SERVER_LOG_INFO(g_logger) << "发送信息：" << buffer;
	}
}