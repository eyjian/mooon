// Writed by yijian, eyjian@qq.com or eyjian@gmail.com
#include "sys/curl_wrapper.h"
#include "utils/string_utils.h"

#if MOOON_HAVE_CURL==1
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
        // TODO warning: dereferencing type-punned pointer will break strict-aliasing rules
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_COPYCONTENTS, contents.c_str(), CURLFORM_END);
    }
    else
    {
        // TODO warning: dereferencing type-punned pointer will break strict-aliasing rules
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
        // TODO warning: dereferencing type-punned pointer will break strict-aliasing rules
        errcode = curl_formadd((struct curl_httppost**)&_post, (struct curl_httppost**)&_last,
            CURLFORM_COPYNAME, name.c_str(),
            CURLFORM_FILE, filepath.c_str(), CURLFORM_END);
    }
    else
    {
        // TODO warning: dereferencing type-punned pointer will break strict-aliasing rules
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

CCurlWrapper::CCurlWrapper(int data_timeout_seconds, int connect_timeout_seconds, bool nosignal) throw (utils::CException)
    : _data_timeout_seconds(data_timeout_seconds), _connect_timeout_seconds(connect_timeout_seconds), _nosignal(nosignal)
{
    _curl_version_info = curl_version_info(CURLVERSION_NOW);

    CURL* curl = curl_easy_init();
    if (NULL == curl)
        THROW_EXCEPTION("curl_easy_init failed", -1);
    _head_list = NULL;

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

    curl_slist* head_list = static_cast<curl_slist*>(_head_list);
    if (_head_list != NULL)
    {
        curl_slist_free_all(head_list);
        _head_list = NULL;
    }
}

void CCurlWrapper::reset() throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;
    curl_slist* head_list = static_cast<curl_slist*>(_head_list);
    
    // 重置
    curl_easy_reset(curl);
    if (_head_list != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);
        curl_slist_free_all(head_list);
        _head_list = NULL;
    }

    // CURLOPT_NOSIGNAL
    // If onoff is 1, libcurl will not use any functions that install signal handlers or any functions that cause signals to be sent to the process.
    if (_nosignal)
    {
        // DNS解析阶段timeout的实现机制是通过SIGALRM+sigsetjmp/siglongjmp来实现的
        // 解析前通过alarm设定超时时间，并设置跳转的标记
        // 在等到超时后，进入alarmfunc函数实现跳转
        // 如果设置了CURLOPT_NOSIGNAL，则DNS解析不设置超时时间
        //
        // 为解决DNS解析不超时问题，编译时指定c-ares（一个异步DNS解析库），configure参数：
        // --enable-ares[=PATH]  Enable c-ares for DNS lookups
        const long onoff = _nosignal? 1: 0;
        errcode = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, onoff);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    // CURLOPT_TIMEOUT
    // In unix-like systems, this might cause signals to be used unless CURLOPT_NOSIGNAL is set.
    errcode = curl_easy_setopt(curl, CURLOPT_TIMEOUT, _data_timeout_seconds);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_CONNECTTIMEOUT
    // In unix-like systems, this might cause signals to be used unless CURLOPT_NOSIGNAL is set.
    errcode = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _connect_timeout_seconds);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_READFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_HEADERFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, on_write_response_header);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_WRITEFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_response_body);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
}

bool CCurlWrapper::add_request_header(const std::string& name_value_pair)
{
    curl_slist* head_list = static_cast<curl_slist*>(_head_list);

    _head_list = curl_slist_append(head_list, name_value_pair.c_str());
    if (NULL == _head_list)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void CCurlWrapper::http_get(std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    const curl_version_info_data* curl_version_info = (curl_version_info_data*)_curl_version_info;
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    // CURLOPT_URL
    errcode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_HTTPHEADER
    if (_head_list != NULL)
    {
        curl_slist* head_list = static_cast<curl_slist*>(_head_list);
        errcode = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head_list);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    // CURLOPT_HEADERFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_WRITEDATA
    errcode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_SSL_VERIFYPEER
    // 相当于curl命令的“-k”或“--insecure”参数
    const int ssl_verifypeer = enable_insecure? 0: 1;
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_SSL_VERIFYHOST
    // When the verify value is 1,
    // curl_easy_setopt will return an error and the option value will not be changed.
    // It was previously (in 7.28.0 and earlier) a debug option of some sorts,
    // but it is no longer supported due to frequently leading to programmer mistakes.
    // Future versions will stop returning an error for 1 and just treat 1 and 2 the same.
    //
    // When the verify value is 0, the connection succeeds regardless of the names in the certificate.
    // Use that ability with caution!
    //
    // 7.28.1开始默认值为2
    int ssl_verifyhost = 0;
    if (enable_insecure)
    {
        ssl_verifyhost = 0;
    }
    else
    {
        if (curl_version_info->version_num > 0x071C00) // 7.28.0
        {
            ssl_verifyhost = 2;
        }
        else
        {
            ssl_verifyhost = 1;
        }
    }
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, ssl_verifyhost);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_COOKIE
    // 设置cookie
    if (cookie != NULL)
    {
        errcode = curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    // CURLOPT_HTTPGET
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

    // CURLOPT_PROXY
    errcode = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_PROXYPORT
    errcode = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy_port);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    http_get(response_header, response_body, url, enable_insecure, cookie);
}

void CCurlWrapper::http_post(const std::string& data, std::string& response_header, std::string& response_body, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    const curl_version_info_data* curl_version_info = (curl_version_info_data*)_curl_version_info;
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    // CURLOPT_URL
    errcode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_HTTPHEADER
    if (_head_list != NULL)
    {
        curl_slist* head_list = static_cast<curl_slist*>(_head_list);
        errcode = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head_list);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    // CURLOPT_POSTFIELDS
    errcode = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_POST
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

    // CURLOPT_SSL_VERIFYPEER
    // 相当于curl命令的“-k”或“--insecure”参数
    const int ssl_verifypeer = enable_insecure? 0: 1;
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_SSL_VERIFYHOST
    // When the verify value is 1,
    // curl_easy_setopt will return an error and the option value will not be changed.
    // It was previously (in 7.28.0 and earlier) a debug option of some sorts,
    // but it is no longer supported due to frequently leading to programmer mistakes.
    // Future versions will stop returning an error for 1 and just treat 1 and 2 the same.
    //
    // When the verify value is 0, the connection succeeds regardless of the names in the certificate.
    // Use that ability with caution!
    //
    // 7.28.1开始默认值为2
    int ssl_verifyhost = 0;
    if (enable_insecure)
    {
        ssl_verifyhost = 0;
    }
    else
    {
        if (curl_version_info->version_num > 0x071C00) // 7.28.0
        {
            ssl_verifyhost = 2;
        }
        else
        {
            ssl_verifyhost = 1;
        }
    }
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, ssl_verifyhost);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_COOKIE
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
    const curl_version_info_data* curl_version_info = (curl_version_info_data*)_curl_version_info;
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    // CURLOPT_URL
    errcode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_HTTPHEADER
    if (_head_list != NULL)
    {
        curl_slist* head_list = static_cast<curl_slist*>(_head_list);
        errcode = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head_list);
        if (errcode != CURLE_OK)
            THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);
    }

    // CURLOPT_HEADERFUNCTION
    errcode = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_WRITEDATA
    errcode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_SSL_VERIFYPEER
    // 相当于curl命令的“-k”或“--insecure”参数
    const int ssl_verifypeer = enable_insecure? 0: 1;
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, ssl_verifypeer);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_SSL_VERIFYHOST
    // When the verify value is 1,
    // curl_easy_setopt will return an error and the option value will not be changed.
    // It was previously (in 7.28.0 and earlier) a debug option of some sorts,
    // but it is no longer supported due to frequently leading to programmer mistakes.
    // Future versions will stop returning an error for 1 and just treat 1 and 2 the same.
    //
    // When the verify value is 0, the connection succeeds regardless of the names in the certificate.
    // Use that ability with caution!
    //
    // 7.28.1开始默认值为2
    int ssl_verifyhost = 0;
    if (enable_insecure)
    {
        ssl_verifyhost = 0;
    }
    else
    {
        if (curl_version_info->version_num > 0x071C00) // 7.28.0
        {
            ssl_verifyhost = 2;
        }
        else
        {
            ssl_verifyhost = 1;
        }
    }
    errcode = curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, ssl_verifyhost);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_COOKIE
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

void CCurlWrapper::proxy_http_post(const std::string& data, std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    // CURLOPT_PROXY
    errcode = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_PROXYPORT
    errcode = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy_port);
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    http_post(data, response_header, response_body, url, enable_insecure, cookie);
}

void CCurlWrapper::proxy_http_post(const CHttpPostData* http_post_data, std::string& response_header, std::string& response_body, const std::string& proxy_host, uint16_t proxy_port, const std::string& url, bool enable_insecure, const char* cookie) throw (utils::CException)
{
    CURLcode errcode;
    CURL* curl = (CURL*)_curl;

    // CURLOPT_PROXY
    errcode = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_host.c_str());
    if (errcode != CURLE_OK)
        THROW_EXCEPTION(curl_easy_strerror(errcode), errcode);

    // CURLOPT_PROXYPORT
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
#endif // MOOON_HAVE_CURL
