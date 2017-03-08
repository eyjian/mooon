// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
namespace cpp mooon.db_proxy
namespace go mooon.db_proxy
namespace java mooon.db_proxy
namespace php mooon.db_proxy
namespace py mooon.db_proxy

typedef list<string> DBRow
typedef list<DBRow> DBTable

// Where语句结点：left op right（如果is_string为true，则为：left op "right"）
struct Condition
{
    1: string op,             // SQL比较操作符
    2: string left,           // 要求为字段名
    3: string right,          // 可为字段名或字符串值或数字值
    4: bool is_string = false // 如果为true，则会对right加引号括起来，即以字符串方式比较，否则以数字方式比较
}

// 对应的配置文件为sql.json
service DbProxyService
{
    // 查询DB
    // sign 签名，用来防止非法调用，sign在sql.json中配置
    // seq 方便用来和服务端对比日志
    // database_index 目标数据库
    // query_index 对应的查询语句
    // limit 最多返回的记录数，值不能小于0否则为sql语法错误
    // limit_start 从哪条记录开始，如果值小于0则表示不使用limit_start
    //
    // limit - limit_start的值不能超过1000，如果limit_start的值小于0则limit的值不能超过1000，
    // 1000的限制是为了保护db_proxy自身和后端的mysql，以防止一次请求返回过大的数据将两者撑死。
    DBTable query(1: string sign, 2: i32 seq, 3: i32 query_index, 4: list<string> tokens, 5: i32 limit, 6: i32 limit_start)

    // DB更新和插入
    // 返回更新或插入的记录数
    i32 update(1: string sign, 2: i32 seq, 3: i32 update_index, 4: list<string> tokens)

    // update()的异步版本
    oneway void async_update(1: string sign, 2: i32 seq, 3: i32 update_index, 4: list<string> tokens)

    // 根据参数自动拼接sql，不需要模版
    // conditions Where条件结点，只有AND关系，如：(conditions[0].left op conditions[0].right) AND (conditions[1].left op conditions[1].right)
    // UPDATE tablename (tokens[0].first,tokens[1].first) VALUES (tokens[0].second,tokens[1].second) WHERE (condtion[0].left op condtion[0].right) 
    i32 update2(1: i32 seq, 2: i32 database_index, 3: string tablename, 4: map<string, string> tokens, 5: list<Condition> conditions)

    // 根据参数自动拼接sql，不需要模版
    // INSERT INTO tablename (tokens[0].first,tokens[1].first) VALUES (tokens[0].second,tokens[1].second)
    i32 insert2(1: i32 seq, 2: i32 database_index, 3: string tablename, 4: map<string, string> tokens)

    // 根据参数自动拼接sql，不需要模版
    // conditions Where条件结点，只有AND关系，如：(conditions[0].left op conditions[0].right) AND (conditions[1].left op conditions[1].right)
    // groupby和orderby可以为空，表示不分组不排序，也可以只其中一个为空
    DBTable query2(1: i32 seq, 2: i32 database_index,  3: string tablename, 4: list<string> fields, 5: list<Condition> conditions, 6: string groupby, 7: string orderby, 8: i32 limit, 9: i32 limit_start)
}
