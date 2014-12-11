#ifndef STATISTICS_PRINT_THREAD_H
#define STATISTICS_PRINT_THREAD_H

#include <sys/thread.h>
#include "config.h"

PP_NAMESPACE_BEGIN

class CStatisticsPrintThread : public sys::CThread
{
private:
    virtual void run();
    int64_t now_to_microseconds(void);

private:
    sys::CEvent _time_event;
    sys::CLock _time_event_lock;
};

PP_NAMESPACE_END

#endif


