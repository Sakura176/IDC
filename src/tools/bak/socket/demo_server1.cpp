/**
 * @file demo_server.cpp
 * @author your name (you@domain.com)
 * @brief 用于演示TcpServer类的多进程socket通讯服务端
 * 1) 在多进程的服务程序中，如果杀掉一个子进程，和这个子进程通讯的客户端会断开，但是，不会影响其它的子进程和客户端，也不会影响父进程
 * 2) 如果杀掉父进程，不会影响正在通讯中的子进程，但是，新的客户端无法建立连接
 * 3) 如果killall+程序名，可以杀掉父进程和全部的子进程
 * 
 * 多进程网络服务端程序退出的三种情况：
 * 1) 如果是子进程收到退出信号，该子进程断开与客户端连接的socket，然后退出。
 * 2) 如果是父进程收到退出信号，父进程先关闭监听的socket，然后向全部的子进程发出退出信号
 * 3) 如果父子进程都收到退出信号，本质上 与第2种情况相同。
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../_public.h"
#include "../log/log.h"

// static server::Logger::ptr g_logger = SERVER_LOG_ROOT();
TcpServer tcpServer;

server::Logger::ptr logger(new server::Logger);

// 父进程退出函数
void FathEXIT(int sig);
// 子进程退出函数
void ChildEXIT(int sig);

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("Using:./demo_server port\nExample:./demo_server 5005 /project/logfile/tcp_server.log\n\n"); 
        return -1;
    }
    server::FileLogAppender::ptr file_appender(new server::FileLogAppender(argv[2]));
    logger->addAppender(file_appender);

    // 忽略信号，避免子进程成为僵尸进程
    CloseIOAndSignal(); signal(SIGINT, FathEXIT); signal(SIGTERM, FathEXIT); 

	if (tcpServer.InitServer(atoi(argv[1])) == false) FathEXIT(-1);

    while (true)
    {
        // 第4步：接受客户端的连接
        if (tcpServer.Accept() == false) return -1;
        SERVER_LOG_INFO(logger) << "客户端IP:" << tcpServer.GetIP() << " 已连接。";

        if (fork() > 0) 
        {
            // 父进程不需要socket连接句柄，因此关闭
            tcpServer.CloseClientSocket(); 
            continue; 
        }

        signal(SIGINT, ChildEXIT); signal(SIGTERM, ChildEXIT);

        // 子进程只负责通讯，不需要监听，因此关闭监听
        tcpServer.CloseListen();

        // 第5步：与客户端通讯，接收客户端发过来的报文后，回复"收到啦"
        char buffer[102400];
        memset(buffer, 0, sizeof(buffer));

        while (true) {
            memset(buffer, 0, sizeof buffer);
            // 接收客户端的请求报文
            if ( tcpServer.Read(buffer) == false) return -1;
            SERVER_LOG_INFO(logger) << "接收信息：" << buffer;

            strcpy(buffer, "收到啦");
            // 向客户端发送响应结果
            if (tcpServer.Write(buffer) == false) return -1;
            SERVER_LOG_INFO(logger) << "发送信息：" << buffer;
        }
        // 子进程退出
        ChildEXIT(0);
    }

    return 0;
}

void FathEXIT(int sig)
{
    // 防止信号处理函数在执行的过程中被信号中断
    signal(SIGINT, SIG_IGN); signal(SIGTERM, SIG_IGN);

    SERVER_LOG_INFO(logger) << "父进程退出。sig=" << sig;
    // 关闭监听的socket
    tcpServer.CloseListen();
    
    // 通知全部的子进程退出
    kill(0, 15);

    exit(0);
}

void ChildEXIT(int sig)
{
    signal(SIGINT, SIG_IGN); signal(SIGTERM, SIG_IGN);

    SERVER_LOG_INFO(logger) << "子进程退出。sig=" << sig;
    // 关闭监听的socket
    tcpServer.CloseClientSocket();

    exit(0);
}