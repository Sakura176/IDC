#include "../include/timer.h"
#include "../include/log.h"

static server::Logger::ptr g_logger = SERVER_LOG_ROOT();

static int timeout = 1000;
static server::Timer::ptr s_timer;

void timer_callback() {
    SERVER_LOG_INFO(g_logger) << "timer callback, timeout = " << timeout;
    timeout += 1000;
    if(timeout < 5000) {
        s_timer->reset(timeout, true);
    } else {
        s_timer->cancel();
    }
}

void test_timer() {
	server::TimerManager tm;
	// 循环定时器
	s_timer = tm.addTimer(1000, timer_callback, true);
    
    // 单次定时器
    tm.addTimer(500, []{
        SERVER_LOG_INFO(g_logger) << "500ms timeout";
    });
    tm.addTimer(5000, []{
        SERVER_LOG_INFO(g_logger) << "5000ms timeout";
    });
}

int main(int argc, char *argv[]) 
{

    test_timer();

    SERVER_LOG_INFO(g_logger) << "end";

    return 0;
}