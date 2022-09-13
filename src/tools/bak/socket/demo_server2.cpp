/**
 * @file demo_server.cpp
 * @author your name (you@domain.com)
 * @brief 用于演示银行APP的多进程socket通讯服务端
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

bool bsession = false;

bool _main(const char *recvbuffer, char *sendbuffer);

bool srv001(const char *recvbuffer, char *sendbuffer);
bool srv002(const char *recvbuffer, char *sendbuffer);

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
        char recvbuffer[1024], sendbuffer[1024];
        // memset(buffer, 0, sizeof(buffer));

        while (true) {
            memset(recvbuffer, 0, sizeof(recvbuffer));
            memset(sendbuffer, 0, sizeof(sendbuffer));
            // 接收客户端的请求报文
            if ( tcpServer.Read(recvbuffer) == false) return -1;
            SERVER_LOG_INFO(logger) << "接收信息：" << recvbuffer;

            // 处理业务的主函数
            if (_main(recvbuffer, sendbuffer) == false) break;

            // 向客户端发送响应结果
            if (tcpServer.Write(sendbuffer) == false) return -1;
            SERVER_LOG_INFO(logger) << "发送信息：" << sendbuffer;
        }
        // 子进程退出
        ChildEXIT(0);
    }

    return 0;
}

bool _main(const char *recvbuffer, char *sendbuffer)
{
    // 解析recvbuffer，获取服务代码
    int isrvcode = -1;
    GetXMLBuffer(recvbuffer, "srvcode", &isrvcode);

    if ( (isrvcode != 1) && (bsession == false))
    {
        strcpy(sendbuffer, "<retcode>-1</retcode><message>用户未登录。</message>"); return true;
    }

    // 处理每种业务
    switch (isrvcode)
    {
        case 1:     // 登录
            srv001(recvbuffer, sendbuffer); break;
        case 2:     // 查询余额
            srv002(recvbuffer, sendbuffer); break;
        default:
            SERVER_LOG_INFO(logger) << "业务代码不合法：" << recvbuffer;
            return false;
    }
    return true;
}

bool srv001(const char *recvbuffer, char *sendbuffer)
{
    // 解析参数
    char tel[21], password[31];
    GetXMLBuffer(recvbuffer, "tel", tel, 20);
    GetXMLBuffer(recvbuffer, "password", password, 30);

    // 处理业务
    if ( (strcmp(tel, "1392220000") == 0) && (strcmp(password, "123456") == 0) )
    {
        bsession = true;
        strcpy(sendbuffer, "<retcode>0</retcode><message>成功。</message>");
    }
    else
        strcpy(sendbuffer, "<retcode>-1</retcode><message>失败。</message>");
    
    return true;
}

bool srv002(const char *recvbuffer, char *sendbuffer)
{
    // 解析参数
    char cardid[31];
    GetXMLBuffer(recvbuffer, "cardid", cardid, 30);

    // 处理业务
    if ( strcmp(cardid, "139124432220000") == 0 )
        strcpy(sendbuffer, "<retcode>0</retcode><message>成功。</message><balance>100000.58</balance>");
    else
        strcpy(sendbuffer, "<retcode>-1</retcode><message>失败。</message>");
    
    return true;
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