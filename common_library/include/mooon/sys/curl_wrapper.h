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

    // response_body 输出参数，可以为NULL，表示不关心响应的HTTP包体
    // enable_insecure为true时，相当于curl命令的“-k”或“--insecure”参数
    void get(std::string* response_body, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);

    // response_header 输出参数，可以为NULL，表示不关心响应的HTTP头
    // response_body 输出参数，可以为NULL，表示不关心响应的HTTP包体
    void get(std::string* response_header, std::string* response_body, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);

public:
    // 取得响应的状态码，如：200、403、500等
    int get_response_code() const throw (utils::CException);

private:
    void* _curl;
    int _timeout_seconds;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_CURL_WRAPPER_H
