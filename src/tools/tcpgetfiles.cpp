/**
 * @file tcpgetfiles.cpp
 * @author your name (you@domain.com)
 * @brief 采用tcp协议，实现从服务器下载文件
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
    char clientpath[301];       // 本地文件存放的根目录
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

// CPActive PActive;

TcpClient tcpClient;

// 帮助文档
void _help();

// 程序退出和信号2、15的处理函数
void EXIT(int sig);

// XML解析函数
bool _xmltoarg(const char *strxmlbuffer);

// 心跳
bool ActiveTest();

bool Login(const char *agrv);  // 登录业务

// 文件上传的主函数，执行一次文件上次的任务
bool _tcpgetfiles();
// 文件内容上传
bool recvFileText(const char *filepath);

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        _help();
        return -1;
    }

    // CloseIOAndSignal();
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);
    
    server::FileLogAppender::ptr file_appender(new server::FileLogAppender(argv[1]));
    logger->addAppender(file_appender);

    // 解析xml，得到运行参数
    if (_xmltoarg(argv[2]) == false)
        return -1;

	// 向服务端发起连接请求
	if (tcpClient.ConnectToServer(starg.ip, starg.port) == false)
	{
		SERVER_LOG_INFO(logger) << "tcpClient connect IP:" << starg.ip << ", port:" << starg.port << " failed.";
		return -1;
	}
	SERVER_LOG_INFO(logger) << "tcpClient connect IP:" << starg.ip << ", port:" << starg.port << " ok.";

    // 登录
    if (Login(argv[2]) == false)
    {
        SERVER_LOG_INFO(logger) << "Login failed.";
        EXIT(-1);
    }

    bool flag = false;
    while(true)
    {
        if (_tcpgetfiles() == false)
        {
            SERVER_LOG_INFO(logger) << "recv files failed.";
            return false;
        }
        GetXMLBuffer(recvbuffer, "finish", &flag);
        if (flag) 
        {
            SERVER_LOG_INFO(logger) << "all file is recved";
            break;
        }
    }

    return 0;
}

bool _tcpgetfiles()
{
    memset(recvbuffer, 0, sizeof(recvbuffer));
    memset(sendbuffer, 0, sizeof(sendbuffer));

    while(true)
    {
        if (tcpClient.Read(recvbuffer, 20) == false) 
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
        SNPRINTF(rsvfilename, sizeof(rsvfilename), 1000, "%s/%s", starg.clientpath, clientfilename);

        // 回复确认信息
        SNPRINTF(sendbuffer, sizeof(sendbuffer), 1000, "<msg>1</msg>");
        if (tcpClient.Write(sendbuffer) == false) return false;
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
        if (tcpClient.Read(recvbuffer, 20) == false)
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

bool ActiveTest()
{
    char buffer[1024];

    SPRINTF(buffer, sizeof(buffer), "<srvcode>0</srvcode>");
    printf("send: %s\n", buffer);
    // 向服务端发送请求报文
    if ( tcpClient.Write(buffer, strlen(buffer)) == false) return false;

    memset(buffer, 0, sizeof buffer);
    // 接收服务端的回应报文
    if (tcpClient.Read(buffer) == false) return false;
    printf("recv: %s\n", buffer);

    return true;
}

bool Login(const char *argv)
{
    memset(recvbuffer, 0, sizeof(recvbuffer));
    memset(sendbuffer, 0, sizeof(sendbuffer));
    SPRINTF(sendbuffer, sizeof(sendbuffer), "%s<clienttype>1</clienttype>", argv);
    SERVER_LOG_INFO(logger) << "send: " << sendbuffer;

    // 向服务端发送请求报文
    if ( tcpClient.Write(sendbuffer, strlen(sendbuffer)) == false) return false;

    // 接收服务端的回应报文
    if (tcpClient.Read(recvbuffer, 20) == false) return false;

    SERVER_LOG_INFO(logger) << "recv: " << recvbuffer;
    SERVER_LOG_INFO(logger) << "login (" << starg.ip << ":" <<starg.port << ") successed。";

    return true;
}

bool _xmltoarg(const char *strxmlbuffer)
{
    memset(&starg, 0, sizeof(struct st_arg));

    GetXMLBuffer(strxmlbuffer, "ip", starg.ip, 30);
    if (strlen(starg.ip) == 0)
    { SERVER_LOG_INFO(logger) << "IP地址:" << starg.ip << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "port", &starg.port);
    if (starg.port == 0)
    { SERVER_LOG_INFO(logger) << "端口:" << starg.port << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);
    if ( starg.ptype != 1 && starg.ptype != 2)
    { SERVER_LOG_INFO(logger) << "ptype:" << starg.ptype << " 无效！"; return false;}

    GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath, 300);
    if (strlen(starg.clientpath) == 0)
    { SERVER_LOG_INFO(logger) << "客户端文件路径:" << starg.clientpath << " 错误！"; return false;}

    // GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak, 300);
    // if (strlen(starg.clientpathbak) == 0)
    // { SERVER_LOG_INFO(logger) << "客户端备份文件夹路径:" << starg.clientpathbak << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);

    GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 300);
    if (strlen(starg.matchname) == 0)
    { SERVER_LOG_INFO(logger) << "文件夹匹配规则:" << starg.matchname << " 为空！"; return false;}

    GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath, 300);
    if (strlen(starg.srvpath) == 0)
    { SERVER_LOG_INFO(logger) << "服务器文件路径:" << starg.srvpath << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "timeval", &starg.timeval);
    if (starg.timeval == 0)
    { SERVER_LOG_INFO(logger) << "扫描时间间隔 timeval:" << starg.timeval << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
    if (starg.timeout == 0)
    { SERVER_LOG_INFO(logger) << "扫描时间间隔 timeout:" << starg.timeout << " 错误！"; return false;}

    GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50);
    if (strlen(starg.pname) == 0)
    { SERVER_LOG_INFO(logger) << "进程名为空"; return false;}

    return true;
}

void EXIT(int sig)
{
    SERVER_LOG_INFO(logger) << "程序退出, sig=" << sig;

    exit(0);
}

void _help()
{
    printf("\n");
    printf("Using :/home/yc/project/tools/bin/tcpgetfiles logfilename xmlbuffer\n\n");

    printf("Sample :/home/yc/project/tools/bin/procctl 20 /home/yc/project/tools/bin/tcppgetfiles /home/yc/project/logfile/tcpgetfiles_surfdata.log "    \
            "\"<ip>192.168.174.133</ip>"                                                                        \
            "<port>5005</port>"                                                                                 \
            "<clienttype>2<clienttype>"                                                                         \
            "<ptype>1</ptype>"                                                                                  \
            "<clientpath>/home/yc/project/tmp/local/surfdata</clientpath>"                                      \
            "<andchild>true</andchild>"                                                                         \
            "<matchname>*.XML,*.CSV,*.JSON</matchname>"                                                         \
            "<srvpath>/home/yc/project/tmp/idc/surfdata</srvpath>"                                              \
            "<timeval>10</timeval>"                                                                             \
            "<timeout>50</timeout>"                                                                             \
            "<pname>tcpputfiles_surfdata</pname>\"\n");
printf("Sample :/home/yc/project/tools/bin/procctl 20 /home/yc/project/tools/bin/tcpgetfiles /home/yc/project/logfile/tcpgetfiles_surfdata.log "    \
            "\"<ip>192.168.174.133</ip>"                                                                        \
            "<port>5005</port>"                                                                                 \
            "<ptype>2</ptype>"                                                                                  \
            "<clientpath>/home/yc/project/tmp/local/surfdata</clientpath>"                                      \
            "<clientpathbak>/home/yc/project/tmp/bak/surfdata</clientpathbak>"                                  \
            "<andchild>true</andchild>"                                                                         \
            "<matchname>*.XML,*.CSV,*.JSON</matchname>"                                                         \
            "<srvpath>/home/yc/project/tmp/idc/surfdata</srvpath>"                                              \
            "<timeval>10</timeval>"                                                                             \
            "<timeout>50</timeout>"                                                                             \
            "<pname>tcpputfiles_surfdata</pname>\"\n");

    printf("本程序是数据中心的公共功能模块，采用tcp协议下载服务端中的文件。\n");
    printf("logfilename   本程序运行的日志文件。\n");
    printf("xmlbuffer     本程序运行的参数，如下：\n");
    printf("ip            服务端的IP地址。\n");
    printf("port          服务端的端口。\n");
    printf("ptype         文件上传成功后的处理方式：1-删除文件；2-移动到备份目录。\n");
    printf("clientpath    本地文件存放的根目录。\n");
    printf("clientpathbak 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。\n");
    printf("andchild      是否上传clientpath目录下各级子目录的文件，true-是；false-否，缺省为false。\n");
    printf("matchname     待上传文件名的匹配规则，如\"*.TXT,*.XML\"\n");
    printf("srvpath       服务端文件存放的根目录。\n");
    printf("timetvl       扫描本地目录文件的时间间隔，单位：秒，取值在1-30之间。\n");
    printf("timeout       本程序的超时时间，单位：秒，视文件大小和网络带宽而定，建议设置50以上。\n");
    printf("pname         进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");
}
