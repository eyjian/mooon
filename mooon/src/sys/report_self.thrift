// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
namespace cpp mooon.sys

service ReportSelfService
{
    // sql ReportSelf客户端的提交的sql语句
    void report(1: list<string> tokens);
}
