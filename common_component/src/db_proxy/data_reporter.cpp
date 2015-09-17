#include "data_reporter.h"
#include <mooon/sys/log.h>
#include <mooon/utils/string_utils.h>
namespace mooon { namespace db_proxy {

CDataReporter::CDataReporter(mooon::sys::ILogger* report_logger)
    : _report_logger(report_logger)
{
}

void CDataReporter::report(const char* format, ...)
{
    va_list ap;
    char line[1024];

    va_start(ap, format);
    utils::CStringUtils::fix_vsnprintf(line, sizeof(line), format, ap);
    _report_logger->log_raw("%s", line);
    va_end(ap);
}

} // namespace db_proxy
} // namespace mooon
