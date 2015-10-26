// Writed by yijian (eyjian@qq.com or eyjian@gmail.com)
#include <mooon/sys/config.h>
#include <mooon/utils/exception.h>
#include <string>
#include <vector>
#ifndef MOOON_SYS_CURL_WRAPPER_H
#define MOOON_SYS_CURL_WRAPPER_H
SYS_NAMESPACE_BEGIN

// HTTP的POST数据类
class CHttpPostData
{
public:
    CHttpPostData();
    ~CHttpPostData();

    void reset();
    const void* get_post() const { return _post; }

    void add_content(const std::string& name, const std::string& contents, const std::string& content_type=std::string("")) throw (utils::CException);

    // filepath 被上传的文件
    void add_file(const std::string& name, const std::string& filepath, const std::string& content_type=std::string("")) throw (utils::CException);

private:
    void* _last;
    void* _post;
};

// libcurl包装类
class CCurlWrapper
{
public:
    static void global_init();
    static void global_cleanup();

public:
    CCurlWrapper(int timeout_seconds) throw (utils::CException);
    ~CCurlWrapper() throw ();

    void reset() throw (utils::CException);

    // HTTP GET请求
    // response_header 输出参数，存放响应的HTTP头
    // response_body 输出参数，存放响应的HTTP包体
    void http_get(std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);
    void proxy_http_get(std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);

    // HTTP POST请求
    void http_post(const std::string& data, std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);
    void http_post(const CHttpPostData* http_post, std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);
    void proxy_http_post(const CHttpPostData* http_post, std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure=false, const char* cookie=NULL) throw (utils::CException);

    std::string escape(const std::string& source);
    std::string unescape(const std::string& source_encoded);

public:
    // 取得响应的状态码，如：200、403、500等
    int get_response_code() const throw (utils::CException);

private:
    void* _curl;
    int _timeout_seconds;
};

/*
 * 普通POST使用示例：
 *
#include <mooon/sys/curl_wrapper.h>
#include <iostream>

extern "C" int main(int argc, char* argv[])
{
    try
    {
        mooon::sys::CCurlWrapper curl_wrapper(2);
        std::string response_header;
        std::string response_body;

        std::string url = "http://127.0.0.1/cgi-bin/hello.cgi";
        curl_wrapper.http_post("helloX=world", response_header, response_body, url);

        std::cout << response_body << std::endl;
    }
    catch (mooon::utils::CException& ex)
    {
        std::cout << ex.str() << std::endl;
    }

    return 0;
}
*/

SYS_NAMESPACE_END
#endif // MOOON_SYS_CURL_WRAPPER_H
