#include "../src/socket/address.h"
#include "../src/log/log.h"

server::Logger::ptr g_logger = SERVER_LOG_ROOT();

void test()
{
	std::vector<server::Address::ptr> addrs;

	SERVER_LOG_INFO(g_logger) << "begin";
	bool v = server::Address::Lookup(addrs, "www.baidu.com:3080");
	SERVER_LOG_INFO(g_logger) << "end";
	if(!v)
	{
		SERVER_LOG_INFO(g_logger) << "lookup fail";
		return;
	}

	for (size_t i = 0; i < addrs.size(); ++i)
	{
		SERVER_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
	}

	auto addr = server::Address::LookupAny("www.google.com:4080");
	if(addr)
	{
		SERVER_LOG_INFO(g_logger) << addr->toString();
	}
	else
	{
		SERVER_LOG_INFO(g_logger) << "error";
	}
}

int main(int argc, char const *argv[])
{
	test();
	return 0;
}
