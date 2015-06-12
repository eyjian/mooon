// Writed by yijian (eyjian@qq.com or eyjian@gmail.com)
#include <mooon/sys/config.h>
#include <mooon/utils/exception.h>
#include <string>
#ifndef MOOON_SYS_CURL_WRAPPER_H
#define MOOON_SYS_CURL_WRAPPER_H
SYS_NAMESPACE_BEGIN

class CCurlWrapper
{
public:
    static void global_init();
    static void global_cleanup();

public:
    CCurlWrapper(int timeout_seconds) throw (utils::CException);
    ~CCurlWrapper() throw ();

    void reset() throw (utils::CException);

    // enable_insecure为true时，相当于curl命令的“-k”或“--insecure”参数
    void get(std::string* result, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);

private:
    void* _curl;
    int _timeout_seconds;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_CURL_WRAPPER_H
