#include "util.h"
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/syscall.h>

namespace server
{
	long GetThreadId()
	{
		return syscall(SYS_gettid);
	}
}