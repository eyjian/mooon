// Writed by yijian on 2015/6/17
// 秒表用于计时
#ifndef MOOON_SYS_STOP_WATCH_H
#define MOOON_SYS_STOP_WATCH_H
#include "mooon/sys/config.h"
#include <sys/time.h>
SYS_NAMESPACE_BEGIN

class CStopWatch
{
public:
    CStopWatch()
    {
        restart();
        _stop_time.tv_sec = 0;
        _stop_time.tv_usec = 0;
    }

    void restart()
    {
        (void)gettimeofday(&_start_time, NULL);
    }

    // 返回纳秒级的时间
    // restart 调用之后是否重计时
    unsigned int get_elapsed_time(bool restart)
    {
        (void)gettimeofday(&_stop_time, NULL);
        unsigned int elapsed_time = static_cast<unsigned int>((_stop_time.tv_sec - _start_time.tv_sec) * 1000000 + (_stop_time.tv_usec - _start_time.tv_usec));

        // 重计时
        if (restart)
        {
            _start_time.tv_sec = _stop_time.tv_sec;
            _start_time.tv_usec = _stop_time.tv_usec;
        }

        return elapsed_time;
    }

private:
    struct timeval _start_time;
    struct timeval _stop_time;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_STOP_WATCH_H
