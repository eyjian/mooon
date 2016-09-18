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
 */
#ifndef MOOON_UTILS_BIND_H
#define MOOON_UTILS_BIND_H
#include "mooon/utils/function.h"
#include <stdint.h>
#include <stdio.h>
UTILS_NAMESPACE_BEGIN

// 使用示例，请参见文件尾
////////////////////////////////////////////////////////////////////////////////
// 不带参数

template <typename ReturnType>
inline Functor<ReturnType> bind(typename FunctionWithoutParameter<ReturnType>::FunctionPtr function_ptr)
{
    return Functor<ReturnType>(function_ptr);
}

template <typename ReturnType, class ObjectType>
inline Functor<ReturnType> bind(typename MemberFunctionWithoutParameter<ReturnType, ObjectType>::MemberFunctionPtr member_function_ptr, ObjectType* object)
{
    return Functor<ReturnType>(member_function_ptr, object);
}

////////////////////////////////////////////////////////////////////////////////
// 带1个参数

template <typename ReturnType, typename ParameterType>
inline Functor<ReturnType> bind(typename FunctionWith1Parameter<ReturnType, ParameterType>::FunctionPtr function_ptr, ParameterType parameter)
{
    return Functor<ReturnType>(function_ptr, parameter);
}

template <typename ReturnType, class ObjectType, typename ParameterType>
inline Functor<ReturnType> bind(typename MemberFunctionWith1Parameter<ReturnType, ObjectType, ParameterType>::MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
{
    return Functor<ReturnType>(member_function_ptr, object, parameter);
}

////////////////////////////////////////////////////////////////////////////////
// 带2个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type>
inline Functor<ReturnType> bind(typename FunctionWith2Parameter<ReturnType, Parameter1Type, Parameter2Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
{
    return Functor<ReturnType>(function_ptr, parameter1, parameter2);
}

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type>
inline Functor<ReturnType> bind(typename MemberFunctionWith2Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
{
    return Functor<ReturnType>(member_function_ptr, object, parameter1, parameter2);
}

////////////////////////////////////////////////////////////////////////////////
// 带3个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
inline Functor<ReturnType> bind(typename FunctionWith3Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
{
    return Functor<ReturnType>(function_ptr, parameter1, parameter2, parameter3);
}

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
inline Functor<ReturnType> bind(typename MemberFunctionWith3Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
{
    return Functor<ReturnType>(member_function_ptr, object, parameter1, parameter2, parameter3);
}

////////////////////////////////////////////////////////////////////////////////
// 带4个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
inline Functor<ReturnType> bind(typename FunctionWith4Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
{
    return Functor<ReturnType>(function_ptr, parameter1, parameter2, parameter3, parameter4);
}

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type>
inline Functor<ReturnType> bind(typename MemberFunctionWith4Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4)
{
    return Functor<ReturnType>(member_function_ptr, object, parameter1, parameter2, parameter3, parameter4);
}
////////////////////////////////////////////////////////////////////////////////
// 带5个参数

template <typename ReturnType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
inline Functor<ReturnType> bind(typename FunctionWith5Parameter<ReturnType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
{
    return Functor<ReturnType>(function_ptr, parameter1, parameter2, parameter3, parameter4, parameter5);
}

template <typename ReturnType, class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type, typename Parameter4Type, typename Parameter5Type>
inline Functor<ReturnType> bind(typename MemberFunctionWith5Parameter<ReturnType, ObjectType, Parameter1Type, Parameter2Type, Parameter3Type, Parameter4Type, Parameter5Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3, Parameter4Type parameter4, Parameter5Type parameter5)
{
    return Functor<ReturnType>(member_function_ptr, object, parameter1, parameter2, parameter3, parameter4, parameter5);
}

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
#endif // MOOON_UTILS_BIND_H
