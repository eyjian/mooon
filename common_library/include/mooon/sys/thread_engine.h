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
#ifndef MOOON_SYS_THREAD_ENGINE_H
#define MOOON_SYS_THREAD_ENGINE_H
#include "mooon/sys/syscall_exception.h"
#include <stdint.h> // pthread_t在32位上是4字节，在64位上是8字节
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

// 基类
class Function
{
public:
    virtual ~Function() {}
    virtual void operator ()() = 0;
};

////////////////////////////////////////////////////////////////////////////////
// 不带参数

class FunctionWithoutParameter: public Function
{
public:
    // 不使用返回值，所以不需要返回值，否则将void替换成需要的类型
    typedef void (*FunctionPtr)();

    FunctionWithoutParameter(FunctionPtr function_ptr)
        : _function_ptr(function_ptr)
    {
    }

    virtual void operator ()()
    {
        (*_function_ptr)();
    }
    
private:
    FunctionPtr _function_ptr;
};

template <class ObjectType>
class MemberFunctionWithoutParameter: public Function
{
public:
    typedef void (ObjectType::*MemberFunctionPtr)();
    MemberFunctionWithoutParameter(MemberFunctionPtr member_function_ptr, ObjectType* object)
        : _member_function_ptr(member_function_ptr), _object(object)
    {
    }
    
    virtual void operator ()()
    {
        (_object->*_member_function_ptr)();
    }
    
private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
};

////////////////////////////////////////////////////////////////////////////////
// 带1个参数

template <typename ParameterType>
class FunctionWith1Parameter: public Function
{
public:
    typedef void (*FunctionPtr)(ParameterType);

    FunctionWith1Parameter(FunctionPtr function_ptr, ParameterType parameter)
        : _function_ptr(function_ptr), _parameter(parameter)
    {
    }

    virtual void operator ()()
    {
        (*_function_ptr)(_parameter);
    }
    
private:
    FunctionPtr _function_ptr;
    ParameterType _parameter;
};

template <class ObjectType, typename ParameterType>
class MemberFunctionWith1Parameter: public Function
{
public:
    typedef void (ObjectType::*MemberFunctionPtr)(ParameterType);
    MemberFunctionWith1Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter(parameter)
    {
    }
    
    virtual void operator ()()
    {
        (_object->*_member_function_ptr)(_parameter);
    }
    
private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    ParameterType _parameter;
};

////////////////////////////////////////////////////////////////////////////////
// 带2个参数

template <typename Parameter1Type, typename Parameter2Type>
class FunctionWith2Parameter: public Function
{
public:
    typedef void (*FunctionPtr)(Parameter1Type, Parameter2Type);

    FunctionWith2Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2)
    {
    }

    virtual void operator ()()
    {
        (*_function_ptr)(_parameter1, _parameter2);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
};

template <class ObjectType, typename Parameter1Type, typename Parameter2Type>
class MemberFunctionWith2Parameter: public Function
{
public:
    typedef void (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type);
    MemberFunctionWith2Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2)
    {
    }

    virtual void operator ()()
    {
        (_object->*_member_function_ptr)(_parameter1, _parameter2);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
};

////////////////////////////////////////////////////////////////////////////////
// 带3个参数

template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
class FunctionWith3Parameter: public Function
{
public:
    typedef void (*FunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type);

    FunctionWith3Parameter(FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
        : _function_ptr(function_ptr), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3)
    {
    }

    virtual void operator ()()
    {
        (*_function_ptr)(_parameter1, _parameter2, _parameter3);
    }

private:
    FunctionPtr _function_ptr;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
};

template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
class MemberFunctionWith3Parameter: public Function
{
public:
    typedef void (ObjectType::*MemberFunctionPtr)(Parameter1Type, Parameter2Type, Parameter3Type);
    MemberFunctionWith3Parameter(MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
        : _member_function_ptr(member_function_ptr), _object(object), _parameter1(parameter1), _parameter2(parameter2), _parameter3(parameter3)
    {
    }

    virtual void operator ()()
    {
        (_object->*_member_function_ptr)(_parameter1, _parameter2, _parameter3);
    }

private:
    MemberFunctionPtr _member_function_ptr;
    ObjectType* _object;
    Parameter1Type _parameter1;
    Parameter2Type _parameter2;
    Parameter3Type _parameter3;
};

////////////////////////////////////////////////////////////////////////////////
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
    
    void operator ()()
    {
        (*_function)();
    }
    
    // 不带参数
    Functor(FunctionWithoutParameter::FunctionPtr function_ptr)
    {
        _function = new FunctionWithoutParameter(function_ptr);
    }

    template <class ObjectType>
    Functor(typename MemberFunctionWithoutParameter<ObjectType>::MemberFunctionPtr member_function_ptr, ObjectType* object)
    {
        _function = new MemberFunctionWithoutParameter<ObjectType>(member_function_ptr, object);
    }
   
    // 带1个参数
    template <typename ParameterType>
    Functor(typename FunctionWith1Parameter<ParameterType>::FunctionPtr function_ptr, ParameterType parameter)
    {
        _function = new FunctionWith1Parameter<ParameterType>(function_ptr, parameter);
    }

    template <class ObjectType, typename ParameterType>
    Functor(typename MemberFunctionWith1Parameter<ObjectType, ParameterType>::MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
    {
        _function = new MemberFunctionWith1Parameter<ObjectType, ParameterType>(member_function_ptr, object, parameter);
    }

    // 带2个参数
    template <typename Parameter1Type, typename Parameter2Type>
    Functor(typename FunctionWith2Parameter<Parameter1Type, Parameter2Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
    {
        _function = new FunctionWith2Parameter<Parameter1Type, Parameter2Type>(function_ptr, parameter1, parameter2);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type>
    Functor(typename MemberFunctionWith2Parameter<ObjectType, Parameter1Type, Parameter2Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
    {
        _function = new MemberFunctionWith2Parameter<ObjectType, Parameter1Type, Parameter2Type>(member_function_ptr, object, parameter1, parameter2);
    }

    // 带3个参数
    template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
    Functor(typename FunctionWith3Parameter<Parameter1Type, Parameter2Type, Parameter3Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
    {
        _function = new FunctionWith3Parameter<Parameter1Type, Parameter2Type, Parameter3Type>(function_ptr, parameter1, parameter2, parameter3);
    }

    template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
    Functor(typename MemberFunctionWith3Parameter<ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
    {
        _function = new MemberFunctionWith3Parameter<ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>(member_function_ptr, object, parameter1, parameter2, parameter3);
    }

public:
    Function* _function;
};

////////////////////////////////////////////////////////////////////////////////
// 不带参数

Functor bind(FunctionWithoutParameter::FunctionPtr function_ptr)
{
    return Functor(function_ptr);    
}

template <class ObjectType>
Functor bind(typename MemberFunctionWithoutParameter<ObjectType>::MemberFunctionPtr member_function_ptr, ObjectType* object)
{
    return Functor(member_function_ptr, object);
}

////////////////////////////////////////////////////////////////////////////////
// 带1个参数

template <typename ParameterType>
Functor bind(typename FunctionWith1Parameter<ParameterType>::FunctionPtr function_ptr, ParameterType parameter)
{
    return Functor(function_ptr, parameter);    
}

template <class ObjectType, typename ParameterType>
Functor bind(typename MemberFunctionWith1Parameter<ObjectType, ParameterType>::MemberFunctionPtr member_function_ptr, ObjectType* object, ParameterType parameter)
{
    return Functor(member_function_ptr, object, parameter);
}

////////////////////////////////////////////////////////////////////////////////
// 带2个参数

template <typename Parameter1Type, typename Parameter2Type>
Functor bind(typename FunctionWith2Parameter<Parameter1Type, Parameter2Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2)
{
    return Functor(function_ptr, parameter1, parameter2);
}

template <class ObjectType, typename Parameter1Type, typename Parameter2Type>
Functor bind(typename MemberFunctionWith2Parameter<ObjectType, Parameter1Type, Parameter2Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2)
{
    return Functor(member_function_ptr, object, parameter1, parameter2);
}

////////////////////////////////////////////////////////////////////////////////
// 带3个参数

template <typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
Functor bind(typename FunctionWith3Parameter<Parameter1Type, Parameter2Type, Parameter3Type>::FunctionPtr function_ptr, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
{
    return Functor(function_ptr, parameter1, parameter2, parameter3);
}

template <class ObjectType, typename Parameter1Type, typename Parameter2Type, typename Parameter3Type>
Functor bind(typename MemberFunctionWith3Parameter<ObjectType, Parameter1Type, Parameter2Type, Parameter3Type>::MemberFunctionPtr member_function_ptr, ObjectType* object, Parameter1Type parameter1, Parameter2Type parameter2, Parameter3Type parameter3)
{
    return Functor(member_function_ptr, object, parameter1, parameter2, parameter3);
}

////////////////////////////////////////////////////////////////////////////////
static void* thread_proc(void* parameter)
{
    Functor* functor = (Functor*)parameter;
    (*functor)();

    delete functor; // 记得这里需要delete
    return NULL;
}

class CThreadEngine
{
public:
    static uint64_t get_current_thread_id()
    {
        return static_cast<uint64_t>(pthread_self());
    }

public:
    CThreadEngine(const Functor& functor) throw (CSyscallException)
        : _thread(0)
    {
        // bind()返回的是一个临时对象，
        // 故需要new一个，以便在thread_proc()过程中有效
        Functor* new_functor = new Functor(functor);

        int errcode = pthread_create(&_thread, NULL, thread_proc, new_functor);
        if (errcode != 0)
        {
            delete new_functor;
            THROW_SYSCALL_EXCEPTION(strerror(errcode), errcode, "pthread_create");
        }
    }
    
    ~CThreadEngine()
    {
        join();
    }
    
    void join()
    {
        if (_thread > 0)
        {
            pthread_join(_thread, NULL);
            _thread = 0;
        }
    }

    pthread_t thread_id() { return _thread; }
    void cancel_thread()
    {
        pthread_cancel(_thread);
    }

    bool is_alive() const
    {
//#ifdef __USE_GNU
//        int ret = pthread_tryjoin_np(_thread, NULL);
//        return EBUSY == ret;
//#endif // __USE_GNU
        int policy = -1;
        struct sched_param param;
        int ret = pthread_getschedparam(_thread, &policy, &param);
        return ret != ESRCH; // 3 (No such process)
    }

    void detach() throw (CSyscallException)
    {
        int ret = pthread_detach(_thread);
        if (ret != 0)
        {
            THROW_SYSCALL_EXCEPTION(strerror(errno), errno, "pthread_detach");
        }
    }

private:
    CThreadEngine();
    CThreadEngine(const CThreadEngine&);
    CThreadEngine& operator =(const CThreadEngine&);

private:
    pthread_t _thread;
};

////////////////////////////////////////////////////////////////////////////////
// 以下为测试代码
#if defined(TEST_THREAD_ENGINE)

////////////////////////////////////////////////////////////////////////////////
// 非类成员函数

static void hello()
{
    printf("[%u] hello\n", pthread_self());
}

static void world()
{
    printf("[%u] world\n", pthread_self());
}

static void hello(int m)
{
    printf("[%u] hello: %d\n", pthread_self(), m);
}

static void world(int m)
{
    printf("[%u] world: %d\n", pthread_self(), m);
}

static void hello(int m, int n)
{
    printf("[%u] hello: %d/%d\n", pthread_self(), m, n);
}

static void world(int m, int n)
{
    printf("[%u] world: %d/%d\n", pthread_self(), m, n);
}

static void hello(int m, int n, int l)
{
    printf("[%u] hello: %d/%d/%d\n", pthread_self(), m, n, l);
}

static void world(int m, int n, int l)
{
    printf("[%u] world: %d/%d/%d\n", pthread_self(), m, n, l);
}

////////////////////////////////////////////////////////////////////////////////
// 类成员函数

class X
{
public:
    //
    // 不带参数
    //

    void hello()
    {
        printf("[%u] X::hello\n", pthread_self());
    }
    
    void world()
    {
        printf("[%u] X::world\n", pthread_self());
    }
    
    //
    // 带1个参数
    //

    void hello(int m)
    {
        printf("[%u] X::hello: %d\n", pthread_self(), m);
    }
    
    void world(int m)
    {
        printf("[%u] X::world: %d\n", pthread_self(), m);
    }

    //
    // 带2个参数
    //

    void hello(int m, int n)
    {
        printf("[%u] X::hello: %d/%d\n", pthread_self(), m, n);
    }

    void world(int m, int n)
    {
        printf("[%u] X::world: %d/%d\n", pthread_self(), m, n);
    }

    //
    // 带3个参数
    //

    void hello(int m, int n, int l)
    {
        printf("[%u] X::hello: %d/%d/%d\n", pthread_self(), m, n, l);
    }

    void world(int m, int n, int l)
    {
        printf("[%u] X::world: %d/%d/%d\n", pthread_self(), m, n, l);
    }
};

////////////////////////////////////////////////////////////////////////////////
int main()
{
    // 不带参数
    CThreadEngine engine1(bind(&hello));
    CThreadEngine engine2(bind(&world));
    
    X* x1 = new X;
    X* x2 = new X;
    CThreadEngine engine3(bind(&X::hello, x1));
    CThreadEngine engine4(bind(&X::world, x2));

    // 带1个参数
    CThreadEngine engine5(bind(&hello, 2015));
    CThreadEngine engine6(bind(&world, 2016));
    
    CThreadEngine engine7(bind(&X::hello, x1, 2015));
    CThreadEngine engine8(bind(&X::world, x2, 2016));    

    // 带2个参数
    CThreadEngine engine9(bind(&hello, 2015, 5));
    CThreadEngine engine10(bind(&world, 2016, 5));

    CThreadEngine engine11(bind(&X::hello, x1, 2015, 5));
    CThreadEngine engine12(bind(&X::world, x2, 2016, 5));

    // 带3个参数
    CThreadEngine engine13(bind(&hello, 2015, 5, 3));
    CThreadEngine engine14(bind(&world, 2016, 5, 3));

    CThreadEngine engine15(bind(&X::hello, x1, 2015, 5, 3));
    CThreadEngine engine16(bind(&X::world, x2, 2016, 5, 3));

    // 按任意键继续继续
    printf("Press ENTER to exit ...\n");
    getchar();

    engine1.join();
    engine2.join();
    engine3.join();
    engine4.join();
    engine5.join();
    engine6.join();
    engine7.join();
    engine8.join();
    
    engine9.join();
    engine10.join();
    engine11.join();
    engine12.join();
    engine13.join();
    engine14.join();
    engine15.join();
    engine16.join();

    delete x1;
    delete x2;
    
    // 按任意键继续退出
    printf("Press ENTER to exit ...\n");
    getchar();

    return 0;
}
#endif // TEST_THREAD_ENGINE

SYS_NAMESPACE_END
#endif // MOOON_SYS_THREAD_ENGINE_H
