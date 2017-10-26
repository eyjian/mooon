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
#include "utils/object.h"
UTILS_NAMESPACE_BEGIN

void CObject::set_type_name(const std::string& type_name)
{
    _type_name = type_name;
}

////////////////////////////////////////////////////////////////////////////////
SINGLETON_IMPLEMENT(CObjectFacotry)

CObjectFacotry::~CObjectFacotry()
{
    for (ObjectCreatorTable::iterator iter=_object_creator_table.begin(); iter!=_object_creator_table.end(); ++iter)
    {
        CObjectCreator* object_creator = iter->second;
        delete object_creator;
    }

    _object_creator_table.clear();
}

void CObjectFacotry::register_object_creater(const std::string& type_name, CObjectCreator* object_creator)
{
    std::pair<ObjectCreatorTable::iterator, bool> ret =
        _object_creator_table.insert(std::make_pair(type_name, object_creator));
    MOOON_ASSERT(ret.second); // 在调试阶段即可发现问题
}

CObjectCreator* CObjectFacotry::get_object_creator(const std::string& type_name) const
{
    CObjectCreator* object_creator = NULL;
    ObjectCreatorTable::const_iterator iter = _object_creator_table.find(type_name);

    if (iter != _object_creator_table.end())
        object_creator = iter->second;
    return object_creator;
}

CObject* CObjectFacotry::create_object(const std::string& type_name) const
{
    if (type_name.empty())
    {
        return NULL;
    }
    else
    {
        CObject* object = NULL;
        CObjectCreator* object_creator = get_object_creator(type_name);

        if (object_creator != NULL)
            object = object_creator->create_object();
        return object;
    }
}

bool CObjectFacotry::object_type_exists(const std::string& type_name) const
{
    return _object_creator_table.count(type_name) > 0;
}

UTILS_NAMESPACE_END
