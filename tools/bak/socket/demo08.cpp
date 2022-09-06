/**
 * @file demo08.cpp
 * @author your name (you@domain.com)
 * @brief 用于演示TcpServer类的socket通讯的服务端
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../_public.h"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Using:./demo08 port\nExample:./demo08 5005\n\n"); 
        return -1;
    }

    TcpServer tcpServer;

	if (tcpServer.InitServer(atoi(argv[1])) == false) return -1;

	// 第4步：接受客户端的连接
	if (tcpServer.Accept() == false) return -1;
	printf("客户端IP:%s 已连接。\n", tcpServer.GetIP());
    
    // 第5步：与客户端通讯，接收客户端发过来的报文后，回复"收到啦"
    char buffer[102400];
	memset(buffer, 0, sizeof(buffer));

    while (true) {
        memset(buffer, 0, sizeof buffer);
        // 接收客户端的请求报文
    	if ( tcpServer.Read(buffer) == false) return -1;
        printf("接收信息：%s\n", buffer);

        strcpy(buffer, "收到啦");
        // 向客户端发送响应结果
        if (tcpServer.Write(buffer) == false) return -1;

        printf("发送信息：%s\n", buffer);
    }

    return 0;
}
