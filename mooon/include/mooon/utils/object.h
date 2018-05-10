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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_UTILS_OBJECT_H
#define MOOON_UTILS_OBJECT_H
#include "mooon/utils/config.h"
#include <map>
UTILS_NAMESPACE_BEGIN

// 用来注册CObjectCreator的宏
// 使用了匿名名字空间，原因是不需要对外可见和使用
//
// 注意必须保证同一进程中object_type_name唯一，
// object_type_name为std::string类型，ObjectClass为CObject的子类类型
//
// 使用示例：
// class CMyClass: public mooon::utils::CObject
// {
// };
//
// REGISTER_OBJECT_CREATOR("myclass", CMyClass);
//
// 在其它文件中创建CMyClass类型的对象，
// 如果未调用REGISTER_OBJECT_CREATOR注册，则返回NULL，
// 如果存在同名的，则debug模式会触发断言错误，非debug模式将导致内存泄露：
// CMyClass* myclass = (CMyClass*)mooon::utils::CObjectFacotry::get_singleton()->create_object("myclass");
#define REGISTER_OBJECT_CREATOR(object_type_name, ObjectClass) \
    namespace { \
        class ObjectClass##Creator: public ::mooon::utils::CObjectCreator \
        { \
        public: \
            ObjectClass##Creator() \
            { \
                ::mooon::utils::CObjectFacotry* object_factory = ::mooon::utils::CObjectFacotry::get_singleton(); \
                object_factory->register_object_creater(object_type_name, this); \
            }\
        private: \
            virtual ::mooon::utils::CObject* create_object() \
            { \
                ObjectClass* object = new ObjectClass; \
                object->set_type_name(object_type_name); \
                return object; \
            } \
        }; \
        static ObjectClass##Creator _g_##ObjectClass; \
    }

// 需要通过对象工厂创建的所有对象基类
class CObject
{
public:
    virtual ~CObject() {}
    void set_type_name(const std::string& type_name);
    const std::string& get_type_name() const { return _type_name; }

private:
    std::string _type_name; // 对象类型名，每类对象的类型名要求唯一

protected: // CObject本身的实例无意义，所以禁止
    CObject() {}

private:
    CObject(const CObject&);
    CObject& operator =(const CObject&);
};

////////////////////////////////////////////////////////////////////////////////
// 对象创建者抽象接口
class CObjectCreator
{
public:
    virtual ~CObjectCreator() {}
    virtual CObject* create_object() = 0;
};

// 对象工厂
// 使用时建议在main()函数中，
// 所有线程创建之前先调用一次::mooon::utils::CObjectFacotry::get_singleton()，
// 目的是为规避单例的多线程问题。
class CObjectFacotry
{
    SINGLETON_DECLARE(CObjectFacotry);

public:
    typedef std::map<std::string, CObjectCreator*> ObjectCreatorTable; // key为类型名
    const ObjectCreatorTable& get_object_creator_table() const { return _object_creator_table; }

public:
    ~CObjectFacotry();

    // type_name 对象类型名，每类对象的类型名要求唯一，如果类型名不唯一，则debug模式编译后运行会触发abort()
    // object_creator 实现了create_object()，用来创建对象
    void register_object_creater(const std::string& type_name, CObjectCreator* object_creator);

    // 根据类型名type_name得到它的CObjectCreator
    // 返回NULL表示type_name对应的CObjectCreator不存在或未被注册
    CObjectCreator* get_object_creator(const std::string& type_name) const;

    // 根据类型创建一个对象
    CObject* create_object(const std::string& type_name) const;

    // 指定的对象类型是否存在
    bool object_type_exists(const std::string& type_name) const;

private:
    ObjectCreatorTable _object_creator_table;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_OBJECT_H
