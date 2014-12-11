#include <sys/time.h>

#include "statistics_print_thread.h"
#include "statistics.h"

PP_NAMESPACE_BEGIN

void CStatisticsPrintThread::run()
{
    uint32_t millisecond;

    PP_LOG_DEBUG("[CStatisticsPrintThread::run] begin\n");

    millisecond = 1000;

    while(! is_stop())
    {
        //if (_time_event.timed_wait(_time_event_lock, millisecond))
        _time_event.timed_wait(_time_event_lock, millisecond);
        {
            PP_LOG_INFO("[CStatisticsPrintThread::run] %lld recv msg count %llu\n", 
                now_to_microseconds(), CStatistics::get_singleton()->pp_msg_count());
        }
    }

    PP_LOG_DEBUG("[CStatisticsPrintThread::run] end\n");
}

int64_t CStatisticsPrintThread::now_to_microseconds(void)
{
    struct timeval tv;

    gettimeofday(&tv, 0);

    return tv.tv_sec * (int64_t)1000000 + tv.tv_usec;
}

PP_NAMESPACE_END


