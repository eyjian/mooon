// Writed by yijian, eyjian@qq.com or eyjian@gmail.com
#include "sys/curl_wrapper.h"
#include "utils/string_utils.h"

#if HAVE_CURL==1
#include <curl/curl.h>
SYS_NAMESPACE_BEGIN

CHttpPostData::CHttpPostData()
    : _last(NULL), _post(NULL)
{
}

CHttpPostData::~CHttpPostData()
{
    reset();
}

void CHttpPostData::reset()
{
    if (_post != NULL)
    {
        struct curl_httppost* post = (struct curl_httppost*)_post;
        curl_formfree(post);
        _post = NULL;
    }
}

void CHttpPostData::add_content(const std::string& name, const std::string& contents, const std::string& content_type) throw (utils::CException)
{
    CURLFORMcode errcode = CURL_FORMADD_OK;

    if (content_type.empty())
    {
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_COPYCONTENTS, contents.c_str(), CURLFORM_END);
    }
    else
    {
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_COPYCONTENTS, contents.c_str(),
            CURLFORM_CONTENTTYPE, content_type.c_str(), CURLFORM_END);
    }

    if (errcode != CURL_FORMADD_OK)
        THROW_EXCEPTION(utils::CStringUtils::format_string("add content[%s] error", name.c_str()), errcode);
}

void CHttpPostData::add_file(const std::string& name, const std::string& filepath, const std::string& content_type) throw (utils::CException)
{
    CURLFORMcode errcode = CURL_FORMADD_OK;

    if (content_type.empty())
    {
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_FILE, filepath.c_str(), CURLFORM_END);
    }
    else
    {
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_FILE, filepath.c_str(),
            CURLFORM_CONTENTTYPE, content_type.c_str(), CURLFORM_END);
    }

    if (errcode != CURL_FORMADD_OK)
        THROW_EXCEPTION(utils::CStringUtils::format_string("add file[%s] error", name.c_str()), errcode);
}

////////////////////////////////////////////////////////////////////////////////
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

    // 之前如何调用了非GET如POST，这个是必须的
    errcode = curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

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
    errcode = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy_port);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    http_get(response_header, response_body, url, enable_insecure, cookie);
}

void CCurlWrapper::http_post(const std::string& data, std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    errcode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_setopt(curl, CURLOPT_POST, 1L);
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

void CCurlWrapper::http_post(const CHttpPostData* http_post_data, std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
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

    errcode = curl_easy_setopt(curl, CURLOPT_HTTPPOST, http_post_data->get_post());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    errcode = curl_easy_perform(curl);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
}

void CCurlWrapper::proxy_http_post(const CHttpPostData* http_post_data, std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    errcode = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    errcode = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy_port);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    http_post(http_post_data, response_header, response_body, url, enable_insecure, cookie);
}


std::string CCurlWrapper::escape(const std::string& source)
{
    std::string result;
    CURL* curl = (CURL*)_curl;

    char *output = curl_easy_escape(curl, source.c_str(), static_cast<int>(source.length()));
    if (output != NULL)
    {
        result = output;
        curl_free(output);
    }

    return result;
}

std::string CCurlWrapper::unescape(const std::string& source_encoded)
{
    std::string result;
    CURL* curl = (CURL*)_curl;
    int outlength;

    char *output = curl_easy_unescape(curl, source_encoded.c_str(), static_cast<int>(source_encoded.length()), &outlength);
    if (output != NULL)
    {
        result = output;
        curl_free(output);
    }

    return result;
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
        const int timeout_seconds = 2;
        CCurlWrapper curl_wrapper(timeout_seconds);
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
