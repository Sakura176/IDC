#include "../include/bytearray.h"
#include "../include/endian.h"
#include "../include/log.h"

#include <fstream>
#include <sstream>
#include <string.h>
#include <iomanip>

namespace server
{
	static server::Logger::ptr g_logger = SERVER_LOG_NAME("system");

	ByteArray::Node::Node(size_t s)
		: ptr(new char[s])
		, next(nullptr)
		, size(s) {  }

	ByteArray::Node::Node()
		: ptr(nullptr)
		, next(nullptr)
		, size(0) { }
}