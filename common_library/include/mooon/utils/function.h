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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 *
 * 以委托方式运行线程，使用方法请参见文件尾部的示例，支持非类成员函数和类成员函数两种
 */
#ifndef MOOON_UTILS_FUNCTION_H
#define MOOON_UTILS_FUNCTION_H
#include "mooon/utils/config.h"
#include <stdint.h>
#include <stdio.h>
UTILS_NAMESPACE_BEGIN

// 使用示例，请参见文件尾
////////////////////////////////////////////////////////////////////////////////

// 基类
template <typename ReturnType>
class Function
{
public:
    virtual ~Function() {}
    virtual ReturnType operator ()() = 0;
};

////////////////////////////////////////////////////////////////////////////////
// 不带参数

template <typename ReturnType>
class FunctionWithoutParameter: public Function<ReturnType>
{
public:
    // 不使用返回值，所以不需要返回值，否则将void替换成需要的类型
    typedef ReturnType (*FunctionPtr)();

    FunctionWithoutParameter(FunctionPtr function_ptr)
        : _function_ptr(function_ptr)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)();
    }
    
private:
    FunctionPtr _function_ptr;
};

template <typename ReturnType, class ObjectType>
class MemberFunctionWithoutParameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)();
    MemberFunctionWithoutParameter(MemberFunctionPtr member_function_ptr, ObjectType* object)
        : _member_function_ptr(member_function_ptr), _object(object)
    {
    }
    
    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)();
    }
    
private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
};

////////////////////////////////////////////////////////////////////////////////
// 带1个参数

template <typename ReturnType, typename ParameterType>
class FunctionWith1Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (*FunctionPtr)(ParameterType);

    FunctionWith1Parameter(FunctionPtr function_ptr, ParameterType parameter)
        : _function_ptr(function_ptr), _parameter(parameter)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)(_parameter);
    }
    
private:
    FunctionPtr _function_ptr;
    ParameterType _parameter;
};

template <typename ReturnType, class ObjectType, typename ParameterType>
class MemberFunctionWith1Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)(ParameterType);
    MemberFunctionWith1Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter(parameter)
    {
    }
    
    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)(_parameter);
    }
    
private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    ParameterType _parameter;
};

////////////////////////////////////////////////////////////////////////////////
// 带2个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type>
class FunctionWith2Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (*FunctionPtr)(Parameter1Type, Parameter2Type);

    FunctionWith2Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)(_parameter1, _parameter2);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
};

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type>
class MemberFunctionWith2Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type);
    MemberFunctionWith2Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2)
    {
    }

    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)(_parameter1, _parameter2);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
};

////////////////////////////////////////////////////////////////////////////////
// 带3个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
class FunctionWith3Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (*FunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type);

    FunctionWith3Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)(_parameter1, _parameter2, _parameter3);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
};

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
class MemberFunctionWith3Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type);
    MemberFunctionWith3Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3)
    {
    }

    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)(_parameter1, _parameter2, _parameter3);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
};

////////////////////////////////////////////////////////////////////////////////
// 带4个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
class FunctionWith4Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (*FunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type);

    FunctionWith4Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3), _parameter4(parameter4)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)(_parameter1, _parameter2, _parameter3, _parameter4);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
    Parameter4Type _parameter4;
};

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
class MemberFunctionWith4Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type);
    MemberFunctionWith4Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3), _parameter4(parameter4)
    {
    }

    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)(_parameter1, _parameter2, _parameter3, _parameter4);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
    Parameter4Type _parameter4;
};

////////////////////////////////////////////////////////////////////////////////
// 带5个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
class FunctionWith5Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (*FunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type);

    FunctionWith5Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3), _parameter4(parameter4), _parameter5(parameter5)
    {
    }

    virtual ReturnType operator ()()
    {
        return (*_function_ptr)(_parameter1, _parameter2, _parameter3, _parameter4, _parameter5);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
    Parameter4Type _parameter4;
    Parameter5Type _parameter5;
};

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
class MemberFunctionWith5Parameter: public Function<ReturnType>
{
public:
    typedef ReturnType (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type);
    MemberFunctionWith5Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3), _parameter4(parameter4), _parameter5(parameter5)
    {
    }

    virtual ReturnType operator ()()
    {
        return (_object->*_member_function_ptr)(_parameter1, _parameter2, _parameter3, _parameter4, _parameter5);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
    Parameter4Type _parameter4;
    Parameter5Type _parameter5;
};

////////////////////////////////////////////////////////////////////////////////
template <typename ReturnType>
class Functor
{
public:
    Functor()
        : _function(NULL)
    {
    }

    Functor(const Functor& other)
    {
        _function = other._function;
        
        Functor& mutable_other = const_cast<Functor&>(other);
        mutable_other._function = NULL;
    }

    ~Functor()
    {
        delete _function;
    }

    Functor& operator =(const Functor& other)
    {
        _function = other._function;
        
        Functor& mutable_other = const_cast<Functor&>(other);
        mutable_other._function = NULL;

        return *this;
    }
    
    ReturnType operator ()()
    {
        return (*_function)();
    }

    // 不带参数
    Functor(typename FunctionWithoutParameter<ReturnType>::FunctionPtr function_ptr)
    {
        _function = new FunctionWithoutParameter<ReturnType>(function_ptr);
    }

    template <class ObjectType>
    Functor(typename MemberFunctionWithoutParameter<ReturnType, ObjectType>::MemberFunctionPtr member_function_ptr, ObjectType* object)
    {
        _function = new MemberFunctionWithoutParameter<ReturnType, ObjectType>(member_function_ptr, object);
    }
   
    // 带1个参数
    template < typename ParameterType>
    Functor(typename FunctionWith1Parameter<ReturnType, ParameterType>::FunctionPtr function_ptr, ParameterType parameter)
    {
        _function = new FunctionWith1Parameter<ReturnType, ParameterType>(function_ptr, parameter);
    }

    template <class ObjectType, typename ParameterType>
    Functor(typename MemberFunctionWith1Parameter<ReturnType, ObjectType, ParameterType>::MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
    {
        _function = new MemberFunctionWith1Parameter<ReturnType, ObjectType, ParameterType>(member_function_ptr, object, parameter);
    }

    // 带2个参数
    template <typename Parameter1Type, typename Parameter2Type>
    Functor(typename FunctionWith2Parameter<ReturnType, Parameter1Type, Parameter2Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
    {
        _function = new FunctionWith2Parameter<ReturnType, Parameter1Type, Parameter2Type>(function_ptr, parameter1, parameter2);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type>
    Functor(typename MemberFunctionWith2Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
    {
        _function = new MemberFunctionWith2Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type>(member_function_ptr, object, parameter1, parameter2);
    }

    // 带3个参数
    template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
    Functor(typename FunctionWith3Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
    {
        _function = new FunctionWith3Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type>(function_ptr, parameter1, parameter2, parameter3);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
    Functor(typename MemberFunctionWith3Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
    {
        _function = new MemberFunctionWith3Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>(member_function_ptr, object, parameter1, parameter2, parameter3);
    }

    // 带4个参数
    template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
    Functor(typename FunctionWith4Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
    {
        _function = new FunctionWith4Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>(function_ptr, parameter1, parameter2, parameter3, parameter4);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
    Functor(typename MemberFunctionWith4Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
    {
        _function = new MemberFunctionWith4Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>(member_function_ptr, object, parameter1, parameter2, parameter3, parameter4);
    }

    // 带5个参数
    template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
    Functor(typename FunctionWith5Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
    {
        _function = new FunctionWith5Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>(function_ptr, parameter1, parameter2, parameter3, parameter4, parameter5);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
    Functor(typename MemberFunctionWith5Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
    {
        _function = new MemberFunctionWith5Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>(member_function_ptr, object, parameter1, parameter2, parameter3, parameter4, parameter5);
    }

public:
    Function<ReturnType>* _function;
};

UTILS_NAMESPACE_END

/*
 * 使用示例：
#include "mooon/utils/bind.h"

int foo() { printf("foo\n"); return 0; }
void woo() { printf("woo\n"); }
void xoo(int m) { printf("xoo: %d\n", m); }

class X
{
private:
    int _m;

public:
    X(int m) { _m = m; }
    void xoo(int n) { printf("X::xoo:%d:%d\n", _m, n); }
    int zoo(int n) { printf("X::zoo:%d:%d\n", _m, n); return 88; }
};

int main()
{
    mooon::utils::bind<int>(&foo)();
    mooon::utils::bind<void>(&woo)();

    mooon::utils::Functor<int> f1 = mooon::utils::bind<int>(&foo);
    f1();

    mooon::utils::Functor<void> f2 = mooon::utils::bind<void>(&xoo, 3);
    f2();

    X x1(7);
    mooon::utils::Functor<void> f3 = mooon::utils::bind<void>(&X::xoo, &x1, 9);
    f3();

    X x2(5);
    mooon::utils::Functor<int> f4 = mooon::utils::bind<int>(&X::zoo, &x2, 1);
    int m = f4();
    printf("m=%d\n", m);

    return 0;
}
*/
#endif // MOOON_UTILS_FUNCTION_H
