// Writed by yijian, eyjian@qq.com or eyjian@gmail.com
#include "sys/curl_wrapper.h"

#if HAVE_CURL==1
#include <curl/curl.h>

SYS_NAMESPACE_BEGIN

static CURL* get_curl(void* curl)
{
    return static_cast<CURL*>(curl);
}

static size_t on_write(void* buffer, size_t size, size_t nmemb, void* stream)
{
    std::string* result = reinterpret_cast<std::string*>(stream);
    result->append(reinterpret_cast<char*>(buffer), size*nmemb);
    return size * nmemb;
}

CCurlWrapper::CCurlWrapper(int timeout_seconds) throw (utils::CException)
    : _timeout_seconds(timeout_seconds)
{
    _curl = curl_easy_init();
    if (NULL == _curl)
        THROW_EXCEPTION("curl_easy_init failed", -1);

    try
    {
        reset();
    }
    catch (...)
    {
        curl_easy_cleanup(get_curl(_curl));
        throw;
    }
}

CCurlWrapper::~CCurlWrapper() throw ()
{
    curl_easy_cleanup(get_curl(_curl));
    _curl = NULL;
}

void CCurlWrapper::reset() throw (utils::CException)
{
    CURLcode errcode;
    errcode = curl_easy_setopt(get_curl(_curl), CURLOPT_TIMEOUT, _timeout_seconds);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(get_curl(_curl), CURLOPT_READFUNCTION, NULL);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(get_curl(_curl), CURLOPT_WRITEFUNCTION, on_write);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    curl_easy_reset(get_curl(_curl));
}

void CCurlWrapper::get(std::string* result, const std::string& url) throw (utils::CException)
{
    CURLcode errcode;
    errcode = curl_easy_setopt(get_curl(_curl), CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(get_curl(_curl), CURLOPT_WRITEDATA, result);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_perform(get_curl(_curl));
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
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
        std::string result;
        curl_wrapper.get(&result, argv[1]);
        printf("result =>\n%s\n", result.c_str());

        result.clear();
        curl_wrapper.get(&result, argv[1]);
        printf("result =>\n%s\n", result.c_str());
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
