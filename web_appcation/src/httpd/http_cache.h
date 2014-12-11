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
#ifndef HTTP_CACHE_H
#define HTTP_CACHE_H
#include <ext/hash_map>
#include "sys/mmap.h"
#include "util/md5.h"
#include "sys/lock.h"
#include "util/log.h"
#include "net/timeoutable.h"
#include "sys/ref_countable.h"
MY_NAMESPACE_BEGIN

class CCacheEntity: public sys::CRefCountable, public net::CTimeoutable
{	
public:
    CCacheEntity(const unsigned char* md5, sys::mmap_t* m);
    ~CCacheEntity();
    const unsigned char* get_md5() const { return _md5; }
	sys::mmap_t* get_map() const { return _m; }

private:    
    unsigned char _md5[16];
	sys::mmap_t* _m;
};

class CHttpCache
{
	SINGLETON_DECLARE(CHttpCache)
    typedef __gnu_cxx::hash_map<const unsigned char*, CCacheEntity*, util::MD5Hasher, util::MD5Comparer> TCacheEntityTable;

public:
    CHttpCache();
    ~CHttpCache();

    CCacheEntity* get_cache_entity(const char* filename, int filename_length);
    void release_cache_entity(CCacheEntity* cache_entity);
	
private:	
    uint32_t _cache_entity_table_number;
    TCacheEntityTable* _cache_entity_table;        
	sys::CLock* _lock;
};

MY_NAMESPACE_END
#endif // HTTP_CACHE_H
