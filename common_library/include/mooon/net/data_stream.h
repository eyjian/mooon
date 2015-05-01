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
 */
#ifndef MOOON_NET_DATA_STREAM_H
#define MOOON_NET_DATA_STREAM_H
#include "mooon/net/utils.h"
NET_NAMESPACE_BEGIN

/***
  * 从数据流中读取数据类
  */
class CStreamReader
{
public:
    /***
      * 析构释放内存，
      * 但如果调用了detach，则什么也不做
      */
    ~CStreamReader()
    {
        delete []_buffer;
    }

    /***
      * 构造一个数据流读取操作对象，数据流空间构造时创建
      * @size: 需要创建的流空间大小
      * @reverse_bytes: 是否反转字节
      */
    CStreamReader(uint32_t size, bool reverse_bytes=false)
        :_size(size)
        ,_reverse_bytes(reverse_bytes)
    {
        _buffer = new char[_size];
    }

    /***
      * 构造一个数据流读取操作对象，数据流空间需要调用者创建
      * @buffer: 用于数据流的空间，必须是new char[]出来的
      * @size: 数据流空间大小
      * @reverse_bytes: 是否反转字节
      */
    CStreamReader(char* buffer, uint32_t size, bool reverse_bytes=false)
        :_buffer(buffer)
        ,_size(size)
        ,_reverse_bytes(reverse_bytes)
    {        
    }

    /***
      * 得到指向数据流空间起始位置的指针
      */
    char* get_buffer()
    {
        return _buffer;
    }

    /***
      * 得到总的数据流空间大小
      */
    uint32_t get_size() const
    {
        return _size;
    }

    /***
      * 得到已经读取到的数据大小
      */
    uint32_t get_offset() const
    {
        return _offset;
    }

    /***
      * 脱离数据流，并返回指向流空间起始位置的地址，
      * 调用后，数据流空间的内存需要调用者调用delete[]去释放
      */
    char* detach()
    {
        char* ptr = _buffer;
        _buffer = NULL;
        return ptr;
    }

    /***
      * 从数据流中读取数据，DataType只能为对象类型，不能为指针类型
      * @m: 用于存储从数据流中读取到的数据
      * @return: 如果读取会越界则返回false，否则返回true
      */
    template <typename DataType>
    bool read(DataType& m)
    {
        if (_offset + sizeof(m) > _size) return false;

        DataType n = *((DataType *)(_buffer + _offset));
        _offset += sizeof(m);

        if (!_reverse_bytes)
            m = n;
        else
            CUtils::reverse_bytes<DataType>(&n, &m);
        
        return true;
    }

    /***
      * 从数据流中读取数据
      * @buffer: 用于存储读取到的数据
      * @size: 需要读取的数据大小
      * @return: 如果读取会越界则返回false，否则返回true
      */
    bool read(char* buffer, uint32_t size)
    {
        if (_offset + size > _size) return false;

        memcpy(buffer, _buffer+_offset, size);
        _offset += size;

        return true;
    }
    
private:
    char* _buffer;
    uint32_t _size;
    uint32_t _offset;
    bool _reverse_bytes; /** 是否反转字节 */
};

/***
  * 往数据流写数据类
  */
class CStreamWriter
{
public:
    /***
      * 析构释放内存，
      * 但如果调用了detach，则什么也不做
      */
    ~CStreamWriter()
    {
        delete []_buffer;
    }

    /***
      * 构造数据流写对象，并创建流空间
      * @size: 需要创建的数据流空间大小
      * @reverse_bytes: 是否反转字节
      */
    CStreamWriter(uint32_t size, bool reverse_bytes=false)
        :_size(size)
        ,_reverse_bytes(reverse_bytes)
    {
        _buffer = new char[_size];
    }

    /***
      * 构造数据流写对象，不创建数据流空间
      * @buffer: 用于数据流的空间，必须是new char[]出来的
      * @size: 数据流空间的大小
      * @reverse_bytes: 是否反转字节
      */
    CStreamWriter(char* buffer, uint32_t size, bool reverse_bytes=false)
        :_buffer(buffer)
        ,_size(size)
        ,_reverse_bytes(reverse_bytes)
    {        
    }

    /***
      * 得到指向数据流空间起始位置的地址
      */
    char* get_buffer()
    {
        return _buffer;
    }

    /***
      * 得到总的数据流空间大小
      */
    uint32_t get_size() const
    {
        return _size;
    }

    /***
      * 得到已经写入的数据大小
      */
    uint32_t get_offset() const
    {
        return _offset;
    }
    
    /***
      * 脱离数据流，并返回指向流空间起始位置的地址，
      * 调用后，流空间的内存需要调用者调用delete[]去释放
      */
    char* detach()
    {
        char* ptr = _buffer;
        _buffer = NULL;
        return ptr;
    }

    /***
      * 往数据流里写数据
      * @m: 需要写入数据流中的数据
      * @return: 如果数据流空间不够写下m则返回false，否则返回true
      */
    template <typename DataType>
    bool write(const DataType& m)
    {
        if (_offset + sizeof(m) > _size) return false;

        DataType n;
        if (!_reverse_bytes)
            n = m;
        else
            CUtils::reverse_bytes<DataType>(&m, &n);
        
        memcpy(_buffer, &n, sizeof(m));
        _offset += sizeof(m);

        return true;
    }

    /***
      * 往数据流里写入数据
      * @buffer: 需要写入数据流中的数据
      * @size: 需要写的大小
      */
    bool write(const char* buffer, uint32_t size)
    {
        if (_offset + size > _size) return false;
        
        memcpy(buffer, _buffer+_offset, size);
        _offset += size;

        return true;
    }
    
private:
    char* _buffer;
    uint32_t _size;
    uint32_t _offset;
    bool _reverse_bytes; /** 是否反转字节 */
};

NET_NAMESPACE_END
#endif // MOOON_NET_DATA_STREAM_H
