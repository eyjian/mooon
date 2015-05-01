/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: jian yi, eyjian@qq.com
 * 
 * NetInt是一个专用于网络上传输的整数类型，
 * 使用它的好处是屏蔽了字节序问题，可以象基本类型一样使用它，如：
 * struct Message
 * {
 *     NInt16 a;
 *     NInt16 b;
 *     NInt32 x;
 *     NInt32 y;
 *     NInt64 z;
 * };
 * 可以直接调用send将Message通过网络发送出去，而不用做网络字节序的转换；
 * 而且收到后，也不需要做转换，可以直接使用，它会自动隐式的转换回主机字节序。
 *
 * 如果定义了宏BIG_NET_BYTES_ORDER，则网络字节序使用大字节序；
 * 否则如果没有定义宏BIG_NET_BYTES_ORDER，则网络字节序使用小字节序。
 * 默认没有定义宏BIG_NET_BYTES_ORDER，也就是默认网络字节序为小字节序
 */
#ifndef MOOON_NET_INTTYPES_H
#define MOOON_NET_INTTYPES_H
#include <mooon/net/utils.h>
#include <mooon/utils/bit_utils.h>
NET_NAMESPACE_BEGIN

template <typename RawInt>
struct NetInt
{
public:
    NetInt()
     :_m(0)
    {
    }
    
    NetInt(RawInt m)
    {
        _m = transform(m);
    }
    
    NetInt& operator =(RawInt m)
    {
        _m = transform(m);
        return *this;
    }
    
    operator RawInt() const
    {
        return transform(_m);
    }
    
    RawInt to_int() const
    {
        return transform(_m);
    }
    
private:
    static RawInt transform(RawInt old_int)
    {
        RawInt new_int = 0;
        
        if (CUtils::is_little_endian())
        {
#ifndef BIG_NET_BYTES_ORDER
            new_int = old_int;
#else
            new_int = CUtils::reverse_bytes(old_int);
#endif
        }
        else
        {
#ifdef BIG_NET_BYTES_ORDER
            new_int = old_int;
#else
            new_int = CUtils::reverse_bytes(old_int);
#endif
        }
        
        return new_int;
    }
    
private:    
    NetInt(const NetInt&);

private:
    RawInt _m;
};

/***
  * 所以类型以N打头，N是Net的缩写
  */
typedef int8_t NInt8;
typedef uint8_t NUInt8;
typedef NetInt<int16_t> NInt16;
typedef NetInt<int32_t> NInt32;
typedef NetInt<int64_t> NInt64;
typedef NetInt<uint16_t> NUInt16;
typedef NetInt<uint32_t> NUInt32;
typedef NetInt<uint64_t> NUInt64;

typedef NInt8 nint8_t;
typedef NInt16 nint16_t;
typedef NInt32 nint32_t;
typedef NInt64 nint64_t;
typedef NUInt8 nuint8_t;
typedef NUInt16 nuint16_t;
typedef NUInt32 nuint32_t;
typedef NUInt64 nuint64_t;

/***
  * 通用的消息头结构
  */
#pragma pack(4)
typedef struct TCommonMessageHeader
{
    nuint32_t size;     // 消息体字节数，不包括头本身
    nuint32_t command;  // 消息命令字
}common_message_header;
#pragma pack()

NET_NAMESPACE_END
/***
  * 引用以全局名字空间，以简化使用，
  * 命名以N打头，就是为减少在全局名字空间内的名字冲突
  */
using mooon::net::NInt8;
using mooon::net::NInt16;
using mooon::net::NInt32;
using mooon::net::NInt64;
using mooon::net::NUInt8;
using mooon::net::NUInt16;
using mooon::net::NUInt32;
using mooon::net::NUInt64;

using mooon::net::nint8_t;
using mooon::net::nint16_t;
using mooon::net::nint32_t;
using mooon::net::nint64_t;
using mooon::net::nuint8_t;
using mooon::net::nuint16_t;
using mooon::net::nuint32_t;
using mooon::net::nuint64_t;
#endif // MOOON_NET_INTTYPES_H
