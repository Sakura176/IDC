# 开发框架头文件路径
PUBINCL = -I$(realpath ../)

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库是为了方便调试。
PUBCPP = $(realpath ../_public.cpp ../log/log.cpp)

BIN = $(realpath ./bin/)

# 编译参数
CFLAGS = -g

all: demo_client demo_server

demo_client:demo_client.cpp
		g++ $(CFLAGS) -o $(BIN)/demo_client demo_client.cpp $(PUBINCL) $(PUBCPP) -lm -lc

demo_server:demo_server.cpp
		g++ $(CFLAGS) -o $(BIN)/demo_server demo_server.cpp $(PUBINCL) $(PUBCPP) -lm -lc
clean:
	rm $(BIN)/demo_server $(BIN)/demo_client