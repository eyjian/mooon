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
 * Author: eyjian@qq.com or eyjian@gmail.com
 *
 * 作用: close助手类，用于自动调用close，省去每个分支显示调用close的繁琐，使代码变得简洁美观
 *
 */
#ifndef MOOON_SYS_CLOSE_HELPER_H
#define MOOON_SYS_CLOSE_HELPER_H
#include "mooon/sys/utils.h"
SYS_NAMESPACE_BEGIN

/***
  * 类类型close助手函数，要求该类有公有的close方法
  */
template <class ClassType>
class CloseHelper
{
public:
    /***
      * 构造一个CloseHelper对象
      * @obj: 需要CloseHelper自动调用其公有close方法的对象(非指针)
      */
    CloseHelper(ClassType* obj)
        :_obj(obj)
    {
    }

    /** 析构函数，用于自动调用对象的close方法 */
    ~CloseHelper()
    {
        if (_obj != NULL)
            _obj->close();
    }

    ClassType* operator ->()
    {
        return _obj;
    }

    ClassType* release()
    {
        ClassType* obj = _obj;
        _obj = NULL;
        return obj;
    }

private:
    ClassType* _obj;
};
 
/***
  * 针对整数类型文件句柄
  */
template <>
class CloseHelper<int>
{
public:
    CloseHelper<int>(int fd)
        :_fd(fd)
    {
    }
    
    /** 析构函数，自动调用::close */
    ~CloseHelper<int>()
    {
        if (_fd != -1)
            ::close(_fd);
    }

    operator int() const
    {
        return _fd;
    }

    int get() const
    {
        return _fd;
    }

    int release()
    {
        int fd = _fd;
        _fd = -1;
        return fd;
    }

private:
    int _fd;
};
 
/***
  * 针对标准I/O
  */
template <>
class CloseHelper<FILE*>
{
public:
    CloseHelper<FILE*>(FILE* fp)
        :_fp(fp)
    {
    }
    
    /** 析构函数，自动调用fclose */
    ~CloseHelper<FILE*>()
    {
        if (_fp != NULL)
            fclose(_fp);
    }

    operator FILE*() const
    {
        return _fp;
    }

    FILE* release()
    {
        FILE* fp = _fp;
        _fp = NULL;
        return fp;
    }
 
private:
    FILE* _fp;
};

/***
  * 自动调用release方法
  */
template <class ClassType>
class ReleaseHelper
{
public:
    /***
      * 构造一个ReleaseHelper对象
      * @obj: 需要ReleaseHelper自动调用其公有close方法的对象(非指针)
      */
    ReleaseHelper(ClassType* obj)
        :_obj(obj)
    {
    }

    /** 析构函数，用于自动调用对象的close方法 */
    ~ReleaseHelper()
    {
        _obj->release();
    }

    ClassType* operator ->()
    {
        return _obj;
    }

private:
    ClassType* _obj;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_CLOSE_HELPER_H
