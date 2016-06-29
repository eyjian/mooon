// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
namespace cpp mooon.db_proxy

typedef list<string> DBRow
typedef list<DBRow> DBTable

// 对应的配置文件为sql.json
service DbProxyService
{
    // 查询DB
    // sign 签名，用来防止非法调用，sign在sql.json中配置
    // seq 方便用来和服务端对比日志
    // database_index 目标数据库
    // query_index 对应的查询语句
    // limit 最多返回的记录数
    // limit_start 从哪条记录开始，如果值小于或等于0则忽略
    DBTable query(1: string sign, 2: i32 seq, 3: i32 query_index, 4: list<string> tokens, 5: i32 limit, 6: i32 limit_start)
    
    // DB更新和插入
    // 返回更新或插入的记录数
    i32 update(1: string sign, 2: i32 seq, 3: i32 update_index, 4: list<string> tokens)
    
    // update()的异步版本
    oneway void async_update(1: string sign, 2: i32 seq, 3: i32 update_index, 4: list<string> tokens)
}
