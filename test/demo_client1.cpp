/**
 * @file demo07.cpp
 * @author your name (you@domain.com)
 * @brief 用于演示采用TcpClient类实现的socket通讯客户端
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../src/public/_public.h"

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        printf("Using:./demo07 ip port\nExample:./demo07 127.0.0.1 5005\n\n"); 
        return -1;
    }

    TcpClient tcpClient;

	// 向服务端发起连接请求
	if (tcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false)
	{
		printf("tcpClient connect IP:%s, port:%s failed.\n", argv[1], argv[2]);
		return -1;
	}
    
    // 第3步：与服务端通讯，发送一个报文后等待回复，然后再发下一个报文
    char buffer[102400];
    for (int i = 0; i < 1000; i++) {
        SPRINTF(buffer, sizeof(buffer), "这是第%d个数据, 编号为%03d", i + 1, i + 1);
        // 向服务端发送请求报文
        if ( tcpClient.Write(buffer, strlen(buffer)) == false) break;
        printf("发送信息：%s\n", buffer);

        memset(buffer, 0, sizeof buffer);
        // 接收服务端的回应报文
        if (tcpClient.Read(buffer) == false) break;
		printf("接收信息: %s\n", buffer);

        sleep(1); // 每隔一秒后再次发送报文
    }

    return 0;
}