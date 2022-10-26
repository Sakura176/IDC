#include "../include/server.h"
#include "../include/log.h"

static server::Logger::ptr g_logger = SERVER_LOG_ROOT();

int main(int argc, char const *argv[])
{
	std::map<std::string, std::string> params;
	params["host"] = "127.0.0.1";
	params["user"] = "yc";
	params["passwd"] = "436052";
	params["dbname"] = "IDC";

	server::TcpServer::ptr http_server(new server::TcpServer("127.0.0.1:3306", 500, params, 5, 3));
	http_server->start();
	return 0;
}
