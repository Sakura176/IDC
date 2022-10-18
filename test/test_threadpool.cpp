#include "../include/threadpool.h"
#include "../include/log.h"

server::Logger::ptr g_logger = SERVER_LOG_ROOT();

server::RWMutex g_mutex;

void func1(int num)
{
	sleep(1);
	server::RWMutex::ReadLock rdlock(g_mutex);
	SERVER_LOG_INFO(g_logger) << "test threadpool func1 : num " << num;
}

int main(int argc, char const *argv[])
{
	SERVER_LOG_INFO(g_logger) << "begin threadpool test";

	server::ThreadPool::ptr tp(new server::ThreadPool(5));
	SERVER_LOG_INFO(g_logger) << "begin add task";
	int num = 0;
	while (true)
	{
		tp->addTask(std::bind(&func1, std::ref(num)));
		num++;
	}
	SERVER_LOG_INFO(g_logger) << "end threadpool test";
	return 0;
}
