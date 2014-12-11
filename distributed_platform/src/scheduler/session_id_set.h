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
#ifndef MOOON_SCHEDULER_SESSION_ID_SET_H
#define MOOON_SCHEDULER_SESSION_ID_SET_H
#include "kernel_service.h"
#include <sys/lock.h>
#include <util/array_queue.h>
#include <util/bit_util.h>
SCHED_NAMESPACE_BEGIN

/***
  * SessionId集合，用来管理可用的SessionId
  */
template <class Lock>
class CSessionIdSet
{
public:
	CSessionIdSet(uint32_t max_ID);
	~CSessionIdSet();

	/***
	  * 从集合中借出一个SessionId
	  * @return 如果容器中有可借的，则返回一个有效的未被使用的SessionId，
	  *         否则返回INVALID_SESSION_ID
	  */
	uint32_t borrow();

	/***
	  * 归还一个SessionId
	  * @session_id 需要归还的SessionId，如果是一个无效的SessionId，则忽略
	  */
	void pay_back(uint32_t session_id);

private:
	bool session_id_exist(uint32_t session_id) const;

private:
	Lock _lock;
	char* _bits_table; // 用来指示session_id是否在集合中
	util::CArrayQueue<uint32_t> _session_id_queue;
};

template <class Lock>
CSessionIdSet<Lock>::CSessionIdSet(uint32_t max_ID)
 :_session_id_queue(max_ID + 1)
{
	_bits_table = new char[max_ID/8 + max_ID%8];
	for (uint32_t session_id=1; session_id<=max_ID + 1; ++session_id)
	{
		_session_id_queue.push_back(session_id);
		util::CBitUtil::set_bit(_bits_table, session_id, true);
	}
}

template <class Lock>
CSessionIdSet<Lock>::~CSessionIdSet()
{
	delete []_bits_table;
}

template <class Lock>
uint32_t CSessionIdSet<Lock>::borrow()
{
	uint32_t session_id = INVALID_SESSION_ID;
	sys::LockHelper<Lock> lh(_lock);

	if (!_session_id_queue.is_empty())
	{
		session_id = _session_id_queue.pop_front();
		util::CBitUtil::set_bit(_bits_table, session_id, false);
	}

	return session_id;
}

template <class Lock>
void CSessionIdSet<Lock>::pay_back(uint32_t session_id)
{
	sys::LockHelper<Lock> lh(_lock);
	if (!_session_id_queue.is_full() && !session_id_exist(session_id))
	{
		_session_id_queue.push_back(session_id);
		util::CBitUtil::set_bit(_bits_table, session_id, true);
	}
}

template <class Lock>
bool CSessionIdSet<Lock>::session_id_exist(uint32_t session_id) const
{
	return util::CBitUtil::test(_bits_table, session_id);
}

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_SESSION_ID_SET_H
