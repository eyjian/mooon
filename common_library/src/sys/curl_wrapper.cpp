// Writed by yijian, eyjian@qq.com or eyjian@gmail.com
#include "sys/curl_wrapper.h"
#include "utils/string_utils.h"

#if HAVE_CURL==1
#include <curl/curl.h>

SYS_NAMESPACE_BEGIN

void CCurlWrapper::global_init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void CCurlWrapper::global_cleanup()
{
    curl_global_cleanup();
}

static size_t on_write_response_body(void* buffer, size_t size, size_t nmemb, void* stream)
{
    std::string* result = reinterpret_cast<std::string*>(stream);
    result->append(reinterpret_cast<char*>(buffer), size*nmemb);
    return size * nmemb;
}

static size_t on_write_response_header(void* buffer, size_t size, size_t nmemb, void* stream)
{
    std::string* result = reinterpret_cast<std::string*>(stream);
    result->append(reinterpret_cast<char*>(buffer), size*nmemb);
    return size * nmemb;
}

CCurlWrapper::CCurlWrapper(int timeout_seconds) throw (utils::CException)
    : _timeout_seconds(timeout_seconds)
{
    CURL* curl = curl_easy_init();
    if (NULL == curl)
        THROW_EXCEPTION("curl_easy_init failed", -1);

    try
    {
        _curl = (void*)curl; // 须放在reset()之前，因为reset()有使用_curl
        reset(); 
    }
    catch (...)
    {
        curl_easy_cleanup(curl);
        _curl = NULL;
        throw;
    }
}

CCurlWrapper::~CCurlWrapper() throw ()
{
    CURL* curl = (CURL*)_curl;
    curl_easy_cleanup(curl);
    _curl = NULL;
}

void CCurlWrapper::reset() throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;
    
    // 重置
    curl_easy_reset(curl);

    errcode = curl_easy_setopt(curl, CURLOPT_TIMEOUT, _timeout_seconds);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, on_write_response_header);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_response_body);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
}

void CCurlWrapper::http_get(std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    errcode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_HEADERFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_WRITEDATA
    errcode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // 相当于curl命令的“-k”或“--insecure”参数
    int ssl_verifypeer = enable_insecure? 0: 1;
    //int ssl_verifyhost = enable_insecure? 0: 1;
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    //errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, ssl_verifyhost);
    //if (errcode != CURLE_OK)
    //    THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // 设置cookie
    if (cookie != NULL)
    {
        errcode = curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    errcode = curl_easy_perform(curl);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
}

void CCurlWrapper::proxy_http_get(std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    errcode = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    errcode = curl_easy_setopt(curl, CURLOPT_PROXYPORT, mooon::utils::CStringUtils::int_tostring(proxy_port).c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    http_get(response_header, response_body, url, enable_insecure, cookie);
}

int CCurlWrapper::get_response_code() const throw (utils::CException)
{
    long response_code = 0;
    CURLcode errcode = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    return static_cast<int>(response_code);
}

#if 0
extern "C" int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: curl_get url\n");
        exit(1);
    }
    
    try
    {
        CCurlWrapper curl_wrapper(2);
        std::string response_body;
        curl_wrapper.get(NULL, &response_body, argv[1]);
        printf("result =>\n%s\n", response_body.c_str());

        response_body.clear();
        curl_wrapper.get(&response_body, argv[1]);
        printf("result =>\n%s\n", response_body.c_str());
    }
    catch (utils::CException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
    
    return 0;
}
#endif

SYS_NAMESPACE_END
#endif // HAVE_CURL
