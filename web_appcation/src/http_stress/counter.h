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
#ifndef COUNTER_H
#define COUNTER_H
#include <sstream>
#include <vector>
#include <sys/event.h>
#include <sys/atomic.h>
MOOON_NAMESPACE_BEGIN

class CCounter
{
	SINGLETON_DECLARE(CCounter);

public:
	CCounter();
	const std::string& get_http_req() const
	{
		return _http_req;
	}

	void wait_finish();
        void inc_num_sender_finished();
	void set_total_num_sender(int32_t total_num_sender)
	{
		_total_num_sender = total_num_sender;
	}


private:
	bool is_finished() const;

private:
	sys::CLock _lock;
	sys::CEvent _event;
	atomic_t _num_sender_finished;
	int32_t _total_num_sender;
	std::string _http_req;
};

MOOON_NAMESPACE_END
#endif // COUNTER_H
