/**
 * @file demo07.cpp
 * @author your name (you@domain.com)
 * @brief 用于演示网银APP软件的客户端
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../_public.h"

TcpClient tcpClient;

bool srv001();  // 登录业务
bool srv002();  // 我的账户（查询余额）

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("Using:./demo07 ip port\nExample:./demo07 127.0.0.1 5005\n\n"); 
        return -1;
    }
    
	// 向服务端发起连接请求
	if (tcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false)
	{
		printf("tcpClient connect IP:%s, port:%s failed.\n", argv[1], argv[2]);
		return -1;
	}
    
    // if (srv001() == false)
    // {
    //     printf("srv001() failed.\n"); return -1;
    // }

    if (srv002() == false)
    {
        printf("srv002() failed.\n"); return -1;
    }

    return 0;
}

bool srv001()
{
    char buffer[1024];

    SPRINTF(buffer, sizeof(buffer), "<srvcode>1</srvcode><tel>1392220000</tel><password>123456</password>");
    // 向服务端发送请求报文
    if ( tcpClient.Write(buffer, strlen(buffer)) == false) return false;

    memset(buffer, 0, sizeof buffer);
    // 接收服务端的回应报文
    if (tcpClient.Read(buffer) == false) return false;

    // 解析服务端返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode);
    if (iretcode != 0) { printf("登录失败。\n"); return false; }

    printf("登录成功。\n");

    return true;
}

bool srv002()
{
    char buffer[1024];

    SPRINTF(buffer, sizeof(buffer), "<srvcode>2</srvcode><cardid>139124432220000</cardid>");
    // 向服务端发送请求报文
    if ( tcpClient.Write(buffer, strlen(buffer)) == false) return false;

    memset(buffer, 0, sizeof buffer);
    // 接收服务端的回应报文
    if (tcpClient.Read(buffer) == false) return false;

    // 解析服务端返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode);
    if (iretcode != 0) { printf("查询余额失败。\n"); return false; }

    double balance = 0;
    GetXMLBuffer(buffer, "balance", &balance);

    printf("查询余额成功(%.2f)。\n", balance);

    return true;
}