#ifndef __SERVER_TCPSERVER_H__
#define __SERVER_TCPSERVER_H__


#include "log.h"
#include "address.h"
#include "socket.h"
#include "macro.h"

namespace server
{

	class TcpServer
	{
	public:
		typedef std::shared_ptr<TcpServer> ptr;

		TcpServer();

		~TcpServer();

		Socket::ptr init(const std::string& host);

		bool accept(Socket::ptr sock);

		int recv(Socket::ptr sock, void* buffer, const size_t buflen, const int timeout = 0);

		int send(Socket::ptr sock, const void* buffer, size_t buflen);

		bool close(Socket::ptr sock);

	private:
		// std::vector<Socket::ptr> m_sock;
		// std::string m_type = "tcp";
	};
}

#endif