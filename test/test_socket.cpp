#include "../include/socket.h"
#include "../include/log.h"
#include "../include/mysql.h"
#include "../include/server.h"


static server::Logger::ptr g_logger = SERVER_LOG_ROOT();

void test_tcpserver()
{
    server::TcpServer::ptr tcpServer;

    auto sock = tcpServer->init("0.0.0.0:3389");
    if (!sock)
    {
        SERVER_LOG_INFO(g_logger) << "init failed";
        return;
    }

    std::string buffer;
    buffer.resize(1024);
    while (true)
    {
        if (!tcpServer->accept(sock))
            continue;
        if (tcpServer->recv(sock, &buffer[0], buffer.size(), 100))
        {
            SERVER_LOG_INFO(g_logger) << buffer;
            tcpServer->send(sock, "message recved", strlen("message recved"));
        }
    }   
}

void test_socket2()
{
    server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
    if(addr) {
        SERVER_LOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        SERVER_LOG_ERROR(g_logger) << "get address fail";
        return;
    }
    
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();

    if(!sock->bind(addr)) {
        SERVER_LOG_ERROR(g_logger) << "bind fail errno="
            << errno << " errstr=" << strerror(errno)
            << " addr=[" << addr->toString() << "]";
        return;
    }
    if(!sock->listen()) {
        SERVER_LOG_ERROR(g_logger) << "listen fail errno="
            << errno << " errstr=" << strerror(errno)
            << " addr=[" << addr->toString() << "]";
        return;
    }
    SERVER_LOG_INFO(g_logger) << "listening...";

    std::string buffer;
    buffer.resize(1024);
    while (true)
    {
        auto client = sock->accept();
        client->setRecvTimeout(100);
        if (!client)
            continue;
        SERVER_LOG_INFO(g_logger) << "new client: " << client->toString();
        while (true)
        {
            if (client->recv(&buffer[0], buffer.size()))
            {
                SERVER_LOG_INFO(g_logger) << buffer;
                client->send("message recved", strlen("message recved"));
            }
        }
    }
}

void test_socket1() {
    //std::vector<SERVER::Address::ptr> addrs;
    //SERVER::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //SERVER::IPAddress::ptr addr;
    //for(auto& i : addrs) {
    //    SERVER_LOG_INFO(g_looger) << i->toString();
    //    addr = std::dynamic_pointer_cast<SERVER::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SERVER_LOG_INFO(g_logger) << "get address: " << addr->toString();
    } else {
        SERVER_LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    server::Socket::ptr sock = server::Socket::CreateTCP(addr);
    addr->setPort(3389);
    SERVER_LOG_INFO(g_logger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        SERVER_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SERVER_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        SERVER_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        SERVER_LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SERVER_LOG_INFO(g_logger) << buffs;
}

// void test2() {
//     server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("www.baidu.com:80");
//     if(addr) {
//         SERVER_LOG_INFO(g_looger) << "get address: " << addr->toString();
//     } else {
//         SERVER_LOG_ERROR(g_looger) << "get address fail";
//         return;
//     }

//     server::Socket::ptr sock = server::Socket::CreateTCP(addr);
//     if(!sock->connect(addr)) {
//         SERVER_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
//         return;
//     } else {
//         SERVER_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
//     }

//     uint64_t ts = server::GetCurrentUS();
//     for(size_t i = 0; i < 10000000000ul; ++i) {
//         if(int err = sock->getError()) {
//             SERVER_LOG_INFO(g_looger) << "err=" << err << " errstr=" << strerror(err);
//             break;
//         }

//         //struct tcp_info tcp_info;
//         //if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
//         //    SERVER_LOG_INFO(g_looger) << "err";
//         //    break;
//         //}
//         //if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
//         //    SERVER_LOG_INFO(g_looger)
//         //            << " state=" << (int)tcp_info.tcpi_state;
//         //    break;
//         //}
//         static int batch = 10000000;
//         if(i && (i % batch) == 0) {
//             uint64_t ts2 = server::GetCurrentUS();
//             SERVER_LOG_INFO(g_looger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
//             ts = ts2;
//         }
//     }
// }

int main(int argc, char** argv) {
    test_tcpserver();
    return 0;
}
