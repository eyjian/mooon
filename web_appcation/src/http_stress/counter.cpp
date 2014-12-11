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
#include "counter.h"

#include <util/args_parser.h>
#include <util/string_util.h>
#include <dispatcher/dispatcher.h>

STRING_ARG_DECLARE(ip);
STRING_ARG_DECLARE(dn);
STRING_ARG_DECLARE(pg);
STRING_ARG_DECLARE(ka);

MOOON_NAMESPACE_BEGIN
SINGLETON_IMPLEMENT(CCounter);

CCounter::CCounter()
 :_num_sender_finished(0)
 ,_total_num_sender(0)
{
	std::string host;
	std::string keep_alive;

	if (ArgsParser::dn->get_value().empty())
	{
		host = ArgsParser::ip->get_value();
	}
	else
	{
		host = ArgsParser::dn->get_value();
	}
	if (ArgsParser::ka->get_value().compare("true"))
	{
		keep_alive = "Connection: Keep-Alive";
	}
	else
	{
		keep_alive = "Connection: Close";
	}

        std::stringstream hr;
	hr << "GET http://" << host << ArgsParser::pg->get_value()
			  << "\r\n"
			  << "host: " << host
			  << "\r\n"
			  << keep_alive
			  << "\r\n"
			  << "\r\n";
        _http_req = hr.str();

	atomic_set(&_num_sender_finished, 0);
}

void CCounter::inc_num_sender_finished()
{
	sys::LockHelper<sys::CLock> lh(_lock);
	atomic_inc(&_num_sender_finished);

	if (is_finished())
	{
		_event.signal();
	}
}

void CCounter::wait_finish()
{
	sys::LockHelper<sys::CLock> lh(_lock);
	while (!is_finished())
	{
		_event.wait(_lock);
	}
}

bool CCounter::is_finished() const
{
	return atomic_read(&_num_sender_finished) == _total_num_sender;
}

MOOON_NAMESPACE_END
