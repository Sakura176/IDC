#include "../include/tcpserver.h"
#include "../include/log.h"
#include <sys/epoll.h>
#include <poll.h>

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	TcpServer::TcpServer()
	{
	}

	Socket::ptr TcpServer::init(const std::string& host)
	{
		server::IPAddress::ptr addr = server::Address::LookupAnyIPAddress("0.0.0.0:3389");
		if(addr) {
			SERVER_LOG_INFO(g_logger) << "get address: " << addr->toString();
		} else {
			SERVER_LOG_ERROR(g_logger) << "get address fail";
			return nullptr;
		}
		auto sock = Socket::CreateTCP(addr);

		if(!sock->bind(addr)) {
			SERVER_LOG_ERROR(g_logger) << "bind fail errno="
				<< errno << " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() << "]";
			return nullptr;
		}
		if(!sock->listen()) {
			SERVER_LOG_ERROR(g_logger) << "listen fail errno="
				<< errno << " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() << "]";
			return nullptr;
		}

		return sock;
	}

	bool TcpServer::accept(Socket::ptr sock)
	{
		if(sock->accept())
		{
			return true;
		}
		return false;
	}

	int TcpServer::recv(Socket::ptr sock, void* buffer, const size_t buflen, const int timeout)
	{
		if (timeout > 0)
		{
			struct pollfd fds;
			fds.fd = sock->getSocket();
			fds.events = POLLIN;
			int iret;
			if ( (iret = poll(&fds, 1, timeout*1000)) <= 0)
				return iret;
		}
		return sock->recv(buffer, buflen);
	}

	int TcpServer::send(Socket::ptr sock, const void* buffer, size_t buflen)
	{
		if (!sock->isConnected())
			return -1;
		return sock->send(buffer, buflen);
	}

	bool TcpServer::close(Socket::ptr sock)
	{
		return sock->close();
	}

	TcpServer::~TcpServer()
	{
	}
}