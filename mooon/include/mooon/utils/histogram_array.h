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
#ifndef MOOON_UTILS_HISTOGRAM_ARRAY_H
#define MOOON_UTILS_HISTOGRAM_ARRAY_H
#include "mooon/utils/config.h"
UTILS_NAMESPACE_BEGIN

/***
  * 直方图数组
  */
template <typename DataType>
class CHistogramArray
{
public:
    /** 数组存储的元素类型 */
    typedef DataType _DataType;
    
    /***
      * 构造一个直方图数组
      * @array_size: 直方图数组大小
      */
    CHistogramArray(uint32_t array_size)
        :_array_size(array_size)
    {
        _histogram_size_array = new uint32_t[array_size];
        _elem_array = new DataType*[array_size];

        for (uint32_t i=0; i<array_size; ++i)
        {
            _elem_array[i] = NULL;
            _histogram_size_array[i] = 0; // 初始化每个柱大小为0
        }
    }

    /** 析构直方图数组 */
    ~CHistogramArray()
    {
        for (uint32_t i=0; i<_array_size; ++i)
            delete []_elem_array[i];
        delete [](DataType**)_elem_array;
    }

    /***
      * 插入元素到指定的直方图中
      * @position: 直方图在数组中的位置
      * @elem: 需要插入的元素
      * @unique: 是否做去重检查，如果是，则不重复插入相同的元素
      * @return: 插入成功返回true，否则返回false
      */
    bool insert(uint32_t position, DataType elem, bool unique=true)
    {
        if (!histogram_exist(position))
        {
            _elem_array[position] = new DataType[1];
            _elem_array[position][0] = elem;

            ++_histogram_size_array[position];
            return true;
        }
        else
        {                    
            uint32_t histogram_size = get_histogram_size(position);
            if (unique)
            {
                for (uint32_t i=0; i<histogram_size; ++i)
                    if (_elem_array[position][i] == elem)
                        return false;
            }

            DataType* elem_array = new DataType[histogram_size+1];
            memcpy(&elem_array[0], _elem_array[position], histogram_size);
        
            delete []_elem_array[position];
            _elem_array[position] = elem_array;

            _elem_array[position][histogram_size] = elem;
            ++_histogram_size_array[position];
        
            return true;
        }
    }

    /***
      * 从直方图中删除一个元素
      * @position: 直方图在数组中的位置
      * @elem: 需要删除的元素
      * @return: 如果元素在直方图中，则返回true，否则返回false
      */
    bool remove(uint32_t position, DataType elem)
    {
        /** 直方图不存在 */
        DataType* histogram = get_histogram(position);
        if (NULL == histogram) return false;

        uint32_t histogram_size = get_histogram_size(position);
        for (uint32_t i=0; i<histogram_size; ++i)
        {
            if (_elem_array[position][i] == elem)
            {
                DataType* elem_array = NULL;
                if (histogram_size > 1)
                {
                    elem_array = new DataType[histogram_size-1];
                    memcpy(&elem_array[0], &_elem_array[position][0], i);
                    memcpy(&elem_array[i], &_elem_array[position][i+1], histogram_size-i-1);
                }                                              

                delete []_elem_array[position];
                _elem_array[position] = elem_array;
                --_histogram_size_array[position];

                return true;
            }
        }

        return false;
    }

    /** 得到可容纳的直方图个数 */
    uint32_t get_capacity() const
    {
        return _array_size;
    }

    /** 检测一个直方图是否存在 */
    bool histogram_exist(uint32_t position) const
    {        
        return get_histogram(position) != NULL;
    }

    /** 得到直方图大小 */
    uint32_t get_histogram_size(uint32_t position) const
    {
        return (position < get_capacity())? _histogram_size_array[position]: 0;
    }

    /** 得到直方图 */
    DataType* get_histogram(uint32_t position) const
    {
        return (position < get_capacity())? _elem_array[position]: NULL;
    }
    
private:
    uint32_t _array_size;            /** 直方图数组大小 */
    DataType** _elem_array;          /** 直方图元素数组 */
    uint32_t* _histogram_size_array; /** 直方图大小数组 */
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_HISTOGRAM_ARRAY_H
