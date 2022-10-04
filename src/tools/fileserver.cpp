/**
 * @file fileserver.cpp
 * @author your name (you@domain.com)
 * @brief 文件传输的服务端
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "_public.h"
#include "log/log.h"

// 程序运行的参数结构体
struct st_arg
{
    int  clienttype;            // 客户端类型，1-上传文件；2-下载文件
    char ip[31];                // 服务器的IP地址
    int  port;                  // 服务器的端口
    int  ptype;                 // 文件上传后的处理方式；1-删除文件；2-备份文件
    char clientpath[301];       // 文件存放的根目录
    char clientpathbak[301];    // 文件成功上传后，本地文件备份的根目录
    bool andchild;              // 是否上传本地目录中各级子目录的文件
    char matchname[301];        // 待上传文件名的匹配规则
    char srvpath[301];          // 服务端文件存放的根目录
    int  timeval;               // 扫描本地目录文件的时间间隔，单位：秒。
    int  timeout;               // 进程心跳的超时时间
    char pname[51];             // 进程名，建议使用"tcpputfiles_后缀"的格式
} starg;

char recvbuffer[1024];
char sendbuffer[1024];

server::Logger::ptr logger(new server::Logger);

TcpServer tcpServer;

// XML解析函数
bool _xmltoarg(const char *strxmlbuffer);

// 父进程退出函数
void FathEXIT(int sig);
// 子进程退出函数
void ChildEXIT(int sig);

// 业务处理主函数
// bool _main(const char *recvbuffer, char *sendbuffer);

bool Login();

bool recvfiles();
bool recvFileText(const char *filepath);
// bool srv001(const char *recvbuffer, char *sendbuffer);

bool sendfiles();
bool sendFileText(const char* filepath, const char* filename);

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("Using:./fileserver port logfile\n");
        printf("Example:./fileserver 5005 /logfile/fileserver.log\n");
        printf("         /home/yc/project/tools/bin/procctl 10 /home/yc/project/tools/bin/fileserver 5005 /home/yc/project/logfile/fileserver.log\n\n\n");
        return -1;
    }
    server::FileLogAppender::ptr file_appender(new server::FileLogAppender(argv[2]));
    logger->addAppender(file_appender);

    // 忽略信号，避免子进程成为僵尸进程
    // CloseIOAndSignal(); signal(SIGINT, FathEXIT); signal(SIGTERM, FathEXIT); 

	if (tcpServer.InitServer(atoi(argv[1])) == false)
    {
        SERVER_LOG_INFO(logger) << "tcpServer: " << tcpServer.GetIP() << " init failed.";
    }
    SERVER_LOG_INFO(logger) << "tcpServer: " << tcpServer.GetIP() << " init ok.";

    while (true)
    {
        // 第4步：接受客户端的连接
        if (tcpServer.Accept() == false) return -1;

        SERVER_LOG_INFO(logger) << "client IP: " << tcpServer.GetIP() << " connected。";

        // if (fork() > 0) 
        // {
        //     // 父进程不需要socket连接句柄，因此关闭
        //     tcpServer.CloseClientSocket(); 
        //     continue; 
        // }

        // signal(SIGINT, ChildEXIT); signal(SIGTERM, ChildEXIT);

        // 子进程只负责通讯，不需要监听，因此关闭监听
        // tcpServer.CloseListen();

        if (Login() == false)
        {
            SERVER_LOG_INFO(logger) << "client IP: " << tcpServer.GetIP() << " login failed。";
            return false;
        }
            // ChildEXIT(-1);
        // 子进程退出
        // ChildEXIT(0);
        if (starg.clienttype == 1)
        {
            if (recvfiles() == false)
                return false;
        }

        if (starg.clienttype == 2)
        {
            if (sendfiles() == false)
                return false;

        }
    }

    return 0;
}

bool sendfiles()
{
    CDir Dir;
    // 先读取服务器文件目录
    if (Dir.OpenDir(starg.srvpath, starg.matchname) == false)
    {
        SERVER_LOG_INFO(logger) << "open dir (" << starg.srvpath << ") failed.";
        return false;
    }

    // SERVER_LOG_INFO(logger) << "open dir (" << starg.srvpath << ") ok.";
    // fstream file;
    // 循环下载每个文件
    while (true)
    {

        memset(sendbuffer, 0, sizeof(sendbuffer));
        memset(recvbuffer, 0, sizeof(recvbuffer));

        if (Dir.ReadDir() == false)
        {
            // SERVER_LOG_INFO(logger) << "read file: " << Dir.m_FileName << " failed.";
            break;
        }

        // 先发送文件名、修改时间和文件大小
        SNPRINTF(sendbuffer, sizeof(sendbuffer), 1000, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>", Dir.m_FileName, Dir.m_ModifyTime, Dir.m_FileSize);

        // SERVER_LOG_INFO(logger) << "prepare to send message: " << sendbuffer;

        if (tcpServer.Write(sendbuffer) == false)
        {
            SERVER_LOG_INFO(logger) << "send message failed.";
            return false;
        }
        // 接收确认报文
        if (tcpServer.Read(recvbuffer) == false) return false;
        int msg = 0;
        GetXMLBuffer(recvbuffer, "msg", &msg);

        if (msg == 1)
        {
            // 发送文件内容,先读取文件，然后逐行发送
            if (sendFileText(Dir.m_FullFileName, Dir.m_FileName) == false)
            {
                SERVER_LOG_INFO(logger) << "send file text failed.";
                return false;
            }
            // SERVER_LOG_INFO(logger) << "send file (" << Dir.m_FullFileName << ") succesed";
        }
    }

    SNPRINTF(sendbuffer, sizeof(sendbuffer), 1000, "<finish>1</finish>");
    if (tcpServer.Write(sendbuffer) == false)
    {
        SERVER_LOG_INFO(logger) << "send finish flag failed.";
        return false;
    }

    return true;
}

// 文件传送完成后会错误发送一个.bak
bool sendFileText(const char* filepath, const char* filename)
{
    memset(sendbuffer, 0, sizeof(sendbuffer));
    ifstream infile(filepath);
    sleep(5);
    if (!infile.is_open())
    {
        SERVER_LOG_INFO(logger) << "open file failed.";
        return false;
    }

    while (!infile.eof())
    {
        infile.getline(sendbuffer, 1000);
        if (tcpServer.Write(sendbuffer) == false)
        {
            SERVER_LOG_INFO(logger) << "send file text: " << sendbuffer;
            return false;
        }
    }
    infile.close();
    SNPRINTF(sendbuffer, sizeof(sendbuffer), 1000, "<endflag>1</endflag>");
    if (tcpServer.Write(sendbuffer) == false)
    {
        SERVER_LOG_INFO(logger) << "send file end flag failed.";
        return false;
    }

    return true;
}

bool recvfiles()
{
    memset(recvbuffer, 0, sizeof(recvbuffer));
    memset(sendbuffer, 0, sizeof(sendbuffer));

    while(true)
    {
        if (tcpServer.Read(recvbuffer, 20) == false) 
        {
            SERVER_LOG_INFO(logger) << "recv message failed.";
            return false;
        }
        // SERVER_LOG_INFO(logger) << "recv message: " << recvbuffer;
        // 读取客户端发送的文件信息
        char clientfilename[301];
        memset(clientfilename, 0, sizeof(clientfilename));
        char mtime[31];
        memset(mtime, 0, sizeof(mtime));
        int msize = 0;

        GetXMLBuffer(recvbuffer, "filename", clientfilename);
        GetXMLBuffer(recvbuffer, "mtime", mtime);
        GetXMLBuffer(recvbuffer, "size", &msize);

        char rsvfilename[1024];
        SNPRINTF(rsvfilename, sizeof(rsvfilename), 1000, "%s/%s", starg.srvpath, clientfilename);

        // 回复确认信息
        SNPRINTF(sendbuffer, sizeof(sendbuffer), 1000, "<msg>1</msg>");
        if (tcpServer.Write(sendbuffer) == false) return false;
        // SERVER_LOG_INFO(logger) << "write file: " << rsvfilename;
        if (recvFileText(rsvfilename) == false) return false;

        SERVER_LOG_INFO(logger) << "recv file (" << rsvfilename << ") successed";

    }
    return true;
}

bool recvFileText(const char *filepath)
{
    char bakname[301];
    SNPRINTF(bakname, sizeof(bakname), 300, "%s.bak", filepath);
    ofstream outfile(bakname, std::ios::app);
    int flag = 0;
    if (!outfile.is_open())
    {
        SERVER_LOG_INFO(logger) << "open file failed.";
        return false;
    }

    while(true)
    {
        // char linebuffer[1024];
        memset(recvbuffer, 0, sizeof(recvbuffer));
        if (tcpServer.Read(recvbuffer, 20) == false)
        {
            // SERVER_LOG_INFO(logger) << "recv file text failed.";
            return false;
        }
        // SERVER_LOG_INFO(logger) << "recv file text: " << recvbuffer;

        GetXMLBuffer(recvbuffer, "endflag", &flag);
        if(flag == 1) break;
        else outfile << recvbuffer << std::endl;
    }
    outfile.close();

    // 文件传输完成，进行更名
    if (RENAME(bakname, filepath) == false) 
    {
        // SERVER_LOG_INFO(logger) << "src: " << bakname << "dst: " << filepath;
        return false;
    }
    return true;
}

bool Login()
{
    memset(recvbuffer, 0, sizeof(recvbuffer));
    memset(sendbuffer, 0, sizeof(sendbuffer));

    if (tcpServer.Read(recvbuffer, 20) == false)
    { 
        SERVER_LOG_INFO(logger) << "recv login message failed" ;
        return false;
    }

    //
    _xmltoarg(recvbuffer);
    if ((starg.clienttype != 1) && (starg.clienttype != 2)) strcpy(sendbuffer, "failed");
    else strcpy(sendbuffer, "ok");

    SERVER_LOG_INFO(logger) << "client IP: " << tcpServer.GetIP() << " type is " << starg.clienttype;

    if (tcpServer.Write(sendbuffer) == false)
    {
        SERVER_LOG_INFO(logger) << "tcpServer send failed.";
        return  false;
    }

    SERVER_LOG_INFO(logger) << tcpServer.GetIP() << " login " << sendbuffer;

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


bool _xmltoarg(const char *strxmlbuffer)
{
    memset(&starg, 0, sizeof(struct st_arg));

    GetXMLBuffer(strxmlbuffer, "ip", starg.ip, 30);

    GetXMLBuffer(strxmlbuffer, "port", &starg.port);

    GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);

    GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath, 300);

    GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak, 300);

    GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);

    GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 300);

    GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath, 300);

    GetXMLBuffer(strxmlbuffer, "timeval", &starg.timeval);
    starg.timeval = min(starg.timeval, 30);

    GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
    starg.timeout = min(starg.timeout, 50);

    GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50);
    strcat(starg.pname, "_srv");

    GetXMLBuffer(strxmlbuffer, "clienttype", &starg.clienttype);
    return true;
}