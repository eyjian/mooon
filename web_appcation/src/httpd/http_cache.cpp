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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "http_cache.h"
#include "util/integer_util.h"
MY_NAMESPACE_BEGIN

CCacheEntity::CCacheEntity(const unsigned char* md5, sys::mmap_t* m)
	:_m(m)
{
    memcpy(_md5, md5, 16);
}

CCacheEntity::~CCacheEntity()
{
	sys::CMMap::unmap(_m);
}

//////////////////////////////////////////////////////////////////////////
// CHttpCache

SINGLETON_IMPLEMENT(CHttpCache)

CHttpCache::CHttpCache()
{
    _cache_entity_table_number = 10000;
    while (!util::CIntergerUtil::is_prime_number(_cache_entity_table_number))
        ++_cache_entity_table_number;

    _cache_entity_table = new TCacheEntityTable[_cache_entity_table_number];
    _lock = new sys::CLock[_cache_entity_table_number];
}

CHttpCache::~CHttpCache()
{
    delete []_cache_entity_table;
    delete []_lock;
}

CCacheEntity* CHttpCache::get_cache_entity(const char* filename, int filename_length)
{
    // 得到文件名的MD5码
    struct MD5Context ctx;
    unsigned char md5[16];

    MD5Init(&ctx);
    MD5Update(&ctx, (const unsigned char *)filename, filename_length);
    MD5Final(md5, &ctx);

    TCacheEntityTable::iterator iter;
    uint32_t index = *(uint64_t *)md5 % _cache_entity_table_number;

    {    
        sys::CLockHelper<sys::CLock> lock(_lock[index]);      
        iter = _cache_entity_table[index].find(md5);
        if (iter != _cache_entity_table[index].end())
        {
            iter->second->inc_refcount();
            return iter->second;
        }
    }
    
    int fd = open(filename, O_RDONLY);
    if (-1 == fd)
    {
        MYLOG_DEBUG("Can not open %s for %s.\n", filename, strerror(errno));
        return NULL;
    }

    try
    {
        sys::mmap_t* m = sys::CMMap::map(fd, getpagesize());
        CCacheEntity* cache_entity = new CCacheEntity(md5, m);
        cache_entity->inc_refcount();
        
        sys::CLockHelper<sys::CLock> lock(_lock[index]);
        std::pair<TCacheEntityTable::iterator, bool> retval = _cache_entity_table[index].insert(std::make_pair(md5, cache_entity));
        if (!retval.second) // 已经存在
        {
            delete cache_entity;            
            retval.first->second->inc_refcount();
            return retval.first->second;
        }

        cache_entity->inc_refcount();
        return cache_entity;
    }    
    catch (sys::CSyscallException& ex)
    {
        MYLOG_DEBUG("Map %s error: %s.\n", filename, strerror(ex.get_errcode()));
        return NULL;
    }
}

void CHttpCache::release_cache_entity(CCacheEntity* cache_entity)
{
    const unsigned char* md5 = cache_entity->get_md5();
    uint32_t index = *(uint64_t *)md5 % _cache_entity_table_number;

    sys::CLockHelper<sys::CLock> lock(_lock[index]);
    if (cache_entity->dec_refcount())
        _cache_entity_table[index].erase(md5);
}

MY_NAMESPACE_END
