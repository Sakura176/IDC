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
#include "../include/_public.h"
#include "../include/socket.h"
#include "../include/log.h"
#include "../include/macro.h"
#include "sys/epoll.h"

static server::Logger::ptr g_logger = SERVER_LOG_ROOT();

int main(int argc, char const *argv[])
{
    int ret;

    auto addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
    SERVER_ASSERT(addr);

    auto socket = server::Socket::CreateTCPSocket();
    SERVER_ASSERT(socket);

    ret = socket->bind(addr);
    SERVER_ASSERT(ret);
    
    SERVER_LOG_INFO(g_logger) << "bind success";

    ret = socket->listen();
    SERVER_ASSERT(ret);

    int epollfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socket->getSocket();
    epoll_ctl(epollfd, EPOLL_CTL_ADD, socket->getSocket(), &ev);
    struct epoll_event evs[10];

    SERVER_LOG_INFO(g_logger) << socket->toString() ;
    SERVER_LOG_INFO(g_logger) << "listening...";
    std::string buffer;
    buffer.resize(1024);
    while (1)
    {
        int infds = epoll_wait(epollfd, evs, 10, -1);
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
            if (evs[i].data.fd == socket->getSocket())
            {
                auto client = socket->accept();
                ev.data.fd = client->getSocket();
                ev.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, client->getSocket(), &ev);
            }
            else
            {
                std::string buffer;
                buffer.resize(1024);
            }
        }

        auto client = socket->accept();
        SERVER_ASSERT(client);
        SERVER_LOG_INFO(g_logger) << "new client: " << client->toString();
        while(true)
        {
            int len = client->recv(&buffer[0], buffer.size());

            if(len > 0)
            {
                SERVER_LOG_INFO(g_logger) << buffer;
                client->send("hello world", strlen("hello world"));
            }
        }
        // client->close();
    }
    return 0;
}