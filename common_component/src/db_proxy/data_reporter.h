// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_DATA_REPORTER_H
#define MOOON_DB_PROXY_DATA_REPORTER_H
#include "mooon/observer/observer_manager.h"
#include <mooon/sys/log.h>
namespace mooon { namespace db_proxy {

class CDataReporter: public observer::IDataReporter
{
public:
    CDataReporter(mooon::sys::ILogger* report_logger);

private:
    virtual void report(const char* format, ...);

private:
    mooon::sys::ILogger* _report_logger;
};

} // namespace db_proxy
} // namespace mooon
#endif // MOOON_DB_PROXY_DATA_REPORTER_H
