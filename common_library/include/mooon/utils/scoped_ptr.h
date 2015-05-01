// Provide scoped_ptr and scoped_array.
//
// Copied and modified from Google's Protocol Buffers library.
// google/protobuf/stub/common.h
//
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 原始来源：
// https://code.google.com/p/protobuf-expectation/source/browse/trunk/src/google/protobuf-expectation/stubs/scoped_ptr.h
// 相关的：
// https://github.com/google/protobuf/blob/master/src/google/protobuf/stubs/shared_ptr.h
#ifndef MOOON_UTILS_SCOPED_PTR_H
#define MOOON_UTILS_SCOPED_PTR_H
#include "mooon/utils/config.h"
#include <assert.h>
#include <cstddef>
UTILS_NAMESPACE_BEGIN

/**
 * 本文件实现了以下两个智能指针类
 */
template <class T> class ScopedPtr;
template <class T> class ScopedArray;

/***
 * 面向非对象数组和非数组变量的智能指针，使用示例:
#include "utils/scoped_ptr.h"
#include <stdio.h>
#include <string.h>

class X
{
public:
    void hello()
    {
        printf("%s\n", "hello");
    }
};

extern "C" int main()
{
    ScopedPtr<X> x(new X);
    x->hello();

    ScopedArray<char> str(new char[1024]);
    strncpy(str, "abc");
    printf("%s\n", str.get());

    return 0;
}
 */
template <class T>
class ScopedPtr
{
public:
    typedef T element_type;

    explicit ScopedPtr(T* p = NULL)
        : _ptr(p)
    {
    }

    ~ScopedPtr()
    {
        enum { type_must_be_complete = sizeof(T) };

        delete _ptr;
        _ptr = reinterpret_cast<T*>(-1);
    }

    bool operator !() const
    {
        return NULL == _ptr;
    }

    void reset(T* p = NULL)
    {
        if (p != _ptr)
        {
            enum { type_must_be_complete = sizeof(T) };

            delete _ptr;
            _ptr = p;
        }
    }

    T& operator *() const
    {
        assert(_ptr != NULL);
        return *_ptr;
    }

    T* operator ->() const
    {
        assert(_ptr != NULL);
        return _ptr;
    }

    T* get() const
    {
        return _ptr;
    }

    bool operator ==(T* p) const
    {
        return _ptr == p;
    }

    bool operator !=(T* p) const
    {
        return _ptr != p;
    }

    void swap(ScopedPtr& p2)
    {
        T* tmp = _ptr;
        _ptr = p2._ptr;
        p2._ptr = tmp;
    }

    T* release()
    {
        T* retVal = _ptr;
        _ptr = NULL;

        return retVal;
    }

private:
    ScopedPtr(const ScopedPtr&);
    ScopedPtr& operator =(const ScopedPtr&);
    template <class T2> bool operator ==(ScopedPtr<T2> const& p2) const;
    template <class T2> bool operator !=(ScopedPtr<T2> const& p2) const;

private:
    T* _ptr;
};

/***
 * 面向对象数组或数组变量的智能指针
 */
template <class T>
class ScopedArray
{
public:
    typedef T element_type;

    explicit ScopedArray(T* p = NULL)
        : _array(p)
    {
    }

    ~ScopedArray()
    {
        enum { type_must_be_complete = sizeof(T) };

        delete[] _array;
        _array = reinterpret_cast<T*>(-1);
    }

    bool operator !() const
    {
        return NULL == _array;
    }

    void reset(T* p = NULL)
    {
        if (p != _array)
        {
            enum { type_must_be_complete = sizeof(T) };

            delete[] _array;
            _array = p;
        }
    }

    T& operator [](std::ptrdiff_t i) const
    {
        assert(i >= 0);
        assert(_array != NULL);

        return _array[i];
    }

    T* get() const
    {
        return _array;
    }

    bool operator==(T* p) const { return _array == p; }
    bool operator!=(T* p) const { return _array != p; }

    void swap(ScopedArray& p2)
    {
        T* tmp = _array;
        _array = p2._array;
        p2._array = tmp;
    }

    T* release()
    {
        T* retVal = _array;
        _array = NULL;

        return retVal;
    }

private:
    ScopedArray(const ScopedArray&);
    ScopedArray& operator=(const ScopedArray&);
    template <class T2> bool operator ==(ScopedArray<T2> const& p2) const;
    template <class T2> bool operator !=(ScopedArray<T2> const& p2) const;

private:
    T* _array;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_SCOPED_PTR_H
