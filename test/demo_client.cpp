#include "../include/_public.h"
#include "../include/socket.h"
#include "../include/log.h"
#include "../include/macro.h"

static server::Logger::ptr g_logger = SERVER_LOG_ROOT();


int main(int argc, char* argv[]) {
    int ret;

    auto socket = server::Socket::CreateTCPSocket();
    SERVER_ASSERT(socket);

    auto addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
    SERVER_ASSERT(addr);

    ret = socket->connect(addr);
    SERVER_ASSERT(ret);

    SERVER_LOG_INFO(g_logger) << "connect success, peer address: " << socket->getRemoteAddress()->toString();

    std::string buffer;
    buffer.resize(1024);
	std::string recvbuffer;
	recvbuffer.resize(1024);
	for (int i = 0; i < 500; i++)
	{
		buffer = "第" + std::to_string(i) + "个数据";
		socket->send(&buffer[0], buffer.size());
		SERVER_LOG_INFO(g_logger) << "send: " << buffer;

		socket->recv(&recvbuffer[0], recvbuffer.size());
		SERVER_LOG_INFO(g_logger) << "recv: " << recvbuffer;
	}
	socket->close();
	return 0;
}