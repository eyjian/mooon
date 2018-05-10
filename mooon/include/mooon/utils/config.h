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
 */
#ifndef MOOON_UTILS_CONFIG_H
#define MOOON_UTILS_CONFIG_H
#include <limits> // std::numeric_limits<>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <stddef.h> // offsetof

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS // PRId64
#endif // __STDC_FORMAT_MACROS
#include <inttypes.h>
// __STDC_FORMAT_MACROS

/* 定义名字空间宏 */
#define MOOON_NAMESPACE_BEGIN namespace mooon {
#define MOOON_NAMESPACE_END                   }
#define MOOON_NAMESPACE_USE using namespace mooon;

#define UTILS_NAMESPACE_BEGIN namespace mooon { namespace utils {
#define UTILS_NAMESPACE_END                   }}
#define UTILS_NAMESPACE_USE using namespace mooon::utils;

MOOON_NAMESPACE_BEGIN
enum
{
    SIZE_32   = 32, // 32 bytes
    SIZE_64   = 64,
    SIZE_128  = 128,
    SIZE_256  = 256,
    SIZE_512  = 512,
    SIZE_1K = 1024,
    SIZE_2K = 2048,
    SIZE_4K = 4096,
    SIZE_8K = 8192
};
MOOON_NAMESPACE_END

// 单例
// 为规避多线程问题，请在所有线程创建之前先调用一次ClassName::get_singleton()，
// 可以通过delete来删除单例对象
#define SINGLETON_DECLARE(ClassName) \
    public: \
        static ClassName* get_singleton(); \
        static void destroy()

#define SINGLETON_IMPLEMENT(ClassName) \
    static ClassName* __sg_singleton_##ClassName = NULL; \
    ClassName* ClassName::get_singleton() \
    { \
        if (NULL == __sg_singleton_##ClassName) \
        __sg_singleton_##ClassName = new ClassName; \
        return __sg_singleton_##ClassName; \
    } \
    void ClassName::destroy() \
    { \
        delete __sg_singleton_##ClassName; \
        __sg_singleton_##ClassName = NULL; \
    }

/** 回调接口 */
#define CALLBACK_INTERFACE

/** 断言宏 */
#define MOOON_ASSERT assert

/** 定义域名的最大长度 */
#define DOMAIN_NAME_MAX 60
/** 定义IP的最大长度 */
#define IP_ADDRESS_MAX sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx")

/** 文件全名最大字节数 */
#ifdef FILENAME_MAX
#undef FILENAME_MAX
#endif
#define FILENAME_MAX 1024

/** 目录最大字节数 */
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX 1024

#ifndef	LINE_MAX
#define	LINE_MAX 1024
#endif

/** IO操作通用缓冲区大小 */
#ifdef IO_BUFFER_MAX
#undef IO_BUFFER_MAX
#endif
#define IO_BUFFER_MAX 4096

/***
  * 请不要使用INT_MIN和INT_MAX等宏，而应当选择使用stl库中的std::numeric_limits替代，
  * 头文件为#include <limits>
  */

/***
  * 通过成员，得到结构体首地址
  * @struct_type: 结构体类型名
  * @member_address: 成员地址
  * @member_name: 成员名称
  */
#define get_struct_head_address(struct_type, member_name, member_address) \
        ((struct_type *)((char *)(member_address) - offsetof(struct_type, member_name)))

UTILS_NAMESPACE_BEGIN

/***
  * delete帮助类，用来自动释放new分配的内存
  */
template <class ObjectType>
class DeleteHelper
{
public:
    /***
      * 构造一个delete_helper对象
      * @obj: 需要自动删除的对象指针
      * @is_array: 是否为new出来的数组
      */
    DeleteHelper(ObjectType*& obj, bool is_array=false)
        :_obj(obj)
        ,_is_array(is_array)
    {
    }

    /** 析构中，用于自动调用delete或delete []，调用后，指针将被置为NULL */
    ~DeleteHelper()
    {
        if (_is_array)
            delete []_obj;
        else
            delete _obj;

        _obj = NULL; // 由于_obj是obj的引用，所以obj也会受影响
    }

private:
    ObjectType*& _obj;
    bool _is_array;
};

/***
  * malloc帮助类，用来自动释放new分配的内存
  */
template <typename ObjectType>
class FreeHelper
{
public:
    /***
      * 构造一个free_helper对象
      * @obj: 需要自动删除的对象指针
      */
    FreeHelper(ObjectType*& obj)
        :_obj(obj)
    {
    }

    /** 析构中，用于自动调用free，调用后，指针将被置为NULL */
    ~FreeHelper()
    {
        if (_obj != NULL)
        {
            free(_obj);
            _obj = NULL; // 由于_obj是obj的引用，所以obj也会受影响
        }
    }

private:
    ObjectType*& _obj;
};

/***
  * va_list帮助类，用于自动调用va_end
  */
class VaListHelper
{
public:
    VaListHelper(va_list& args)
        :_args(args)
    {
    }

    /** 析构函数，自动调用va_end */
    ~VaListHelper()
    {
        va_end(_args);
    }

private:
    va_list& _args;
};

/***
  * 计数帮助类，用于自动对计数器进行增一和减一操作
  */
template <typename DataType>
class CountHelper
{
public:
    /** 构造函数，对计数器m进行增一操作 */
    CountHelper(DataType& m)
        :_m(m)
    {
        ++m;
    }

    /** 析构函数，对计数器m进行减一操作 */
    ~CountHelper()
    {
        --_m;
    }

private:
    DataType& _m;
};

/***
  * 处理结果类型
  */
typedef enum
{
    handle_error     = 1,   /** 处理出错 */
    handle_finish    = 2,   /** 处理成功完成 */
    handle_continue  = 3,   /** 处理未完成，需要继续 */
    handle_release   = 4,   /** 交出控制权 */
    handle_close     = 5    /** 可以关闭了 */
}handle_result_t;

// 复制容器
// 将容器C1中的元素复制到容器C2中
// 容器C1可以为set/list/map/vector等，容器C2只可为vector/list等
// 返回int是为便于C2为vector时的遍历
template <class C1, class C2>
int copy_container(const C1& c1, C2* c2)
{
    for (typename C1::const_iterator iter=c1.begin(); iter!=c1.end(); ++iter)
        c2->push_back(*iter);

    return static_cast<int>(c2->size());
}

// 删除序列容器中的元素，并清空序列容器
template <class ContainerClass>
void clear_sequence_container(ContainerClass* container)
{
    for (typename ContainerClass::iterator iter=container->begin(); iter != container->end(); ++iter)
    {
        delete *iter;
    }

    container->clear();
}

// 删除关联容器中的元素，并清空关联容器
template <class ContainerClass>
void clear_associative_container(ContainerClass* container)
{
    for (typename ContainerClass::iterator iter=container->begin(); iter != container->end(); ++iter)
    {
        delete iter->second;
    }

    container->clear();
}

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_CONFIG_H
