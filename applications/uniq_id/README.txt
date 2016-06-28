MOOON-UniqId是一个每秒可产生千万级唯一ID的服务，基于租约的思想，架构和实现均十分简单。
UniqAgent和UniqMaster均为单进程无线程结构，UniqAgent提供ID服务，UniqMaster维护Label，
通过Label来唯一区别机器，Label为1字节数，最大支持255台机器同时提供服务，值为0的Label内部使用。

MOOON-UniqId弱主从架构，具备高可用性，包含master在内的任意节点挂掉，均不影响服务。
单个UniqAgent提供超过20万/秒（CPU）取8字节无符号整数ID的能力，
100台机器可以提供2000多万/秒的服务，只占单个CPU，无缓存低内存需求（单个UDP包大小为28字节，加上IP和UDP头为56字节）。

应用场景：
1）8字节整数的唯一ID，可用于订单号等
2）各类包含日期和业务类型等的订单号、流水号、交易号等，如

如何保证ID的唯一性？
1）为每台机器分配唯一的Label（标签），uniq_id的实现支持Lable取值1~255，也就是最多255台机器
2）每台机器自维护一个4字节无符号的循环递增Sequence（序号）
3）唯一ID加上日期时间（年、月、日、小时等）
4）以上3部分组成即可保证唯一性。

使用租约管理Label，以简化Label的维护，租期可定制，但建议以天为单位。
MOOON-UniqId各组成均为单进程无线程，成员间通讯采用UDP协议，以致实现轻巧但又高效。

日志控制：
可通过设置环境变量MOOON_LOG_LEVEL和MOOON_LOG_SCREEN来控制日志级别和是否在屏幕上输出日志
1) MOOON_LOG_LEVEL可以取值debug,info,error,warn,fatal
2) MOOON_LOG_SCREEN取值为1表示在屏幕输出日志，其它则表示不输出

工具说明：
1) uniq_cli可用于测试uniq_agent，并可作为性能测试工具
2) master_cli可用于测试master，对于master来说不需要考虑性能

参数设置建议：
1) Label过期参数expire的值建议以天为单位进行设置，比如至少1天，建议为1周或更大，以便机器故障时，快乐休假
2) 请注意保持master和agent上的Label过期参数expire的值相同！！！
3) agent的参数interval值，建议至少为1分钟，建议为10分钟或更大一点，要和expire的值保持合理关系，
     由于UDP不可靠，所以在一个expire周期内，最好保持10次以上的interval，也就是expire的值最好是interval的10倍或更大
4) master的timeout值要小于agent的interval值，以便及时的将值更新到DB中，比如可以为interval值的一半，或6/10等
5) agent的steps值最好不小于10000，设置为10万会更佳，每重启一次agent进程，最多会浪费steps两倍的sequence，因此太大也不好。

当前实现还有两个性能可优化点：
1) 启用recvmmsg
2) 支持一次调用取多个
