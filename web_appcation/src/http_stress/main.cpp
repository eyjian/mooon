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
 *
 * HTTP协议压力测试工具，可用它取代apache benchmark
 */
#include <iostream>
#include <vector>

#include <sys/event.h>
#include <sys/logger.h>
#include <sys/main_template.h>
#include <sys/util.h>

#include <dispatcher/dispatcher.h>
#include <util/args_parser.h>

#include "counter.h"
#include "http_reply_handler.h"

/** 每个用户发起的请求个数  */
INTEGER_ARG_DEFINE(false, uint32_t, nr, 1000, 1, 100000000, "the number of request to a user";)
/** 并发的用户个数  */
INTEGER_ARG_DEFINE(false, uint16_t, nu, 1, 1, 10000, "the number of users to request");
/** 用于发送的线程个数  */
INTEGER_ARG_DEFINE(false, uint16_t, nt, 0, 0, 1000, "the number of threads to send");
/** 待连接的端口号  */
INTEGER_ARG_DEFINE(false, uint16_t, port, 80, 1, 65535, "the port to connect");
/** 连接超时秒数 */
INTEGER_ARG_DEFINE(true, uint16_t, tm, 5, 1, 65535, "the timeout of connect");
/** 待连接的IP地址  */
STRING_ARG_DEFINE(false, ip, "", "the IP to connect");
/** 待连接的域名  */
STRING_ARG_DEFINE(true, dn, "", "the domain to connect");
/** 请求的页面  */
STRING_ARG_DEFINE(false, pg, "/", "the page to request");
STRING_ARG_DEFINE(true, ka, "true", "enable keep-alive if true");

MOOON_NAMESPACE_BEGIN
sys::CLock g_lock;
sys::CEvent g_event;
atomic_t g_sender_finished;

class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper()
     :_logger(NULL)
     ,_dispatcher(NULL)
     ,_sender_table(NULL)
    {
    }

private:
	virtual bool init(int argc, char* argv[])
	{
		if (!parse_args(argc, argv))
		{
			return false;
		}
		else
		{
            _logger = new sys::CLogger;
            _logger->enable_screen(true);
            _logger->set_log_level(sys::LOG_LEVEL_DEBUG);
			_logger->create(".", "hs.log", 10000);

			dispatcher::logger = _logger;
		}

		uint16_t thread_count;
		if (0 == ArgsParser::nt->get_value())
		{
			thread_count = sys::CUtil::get_cpu_number();
		}
		else
		{
			thread_count = ArgsParser::nt->get_value();
		}
		if (thread_count > ArgsParser::nu->get_value())
		{
			// 线程数不必高于并发数
			thread_count = ArgsParser::nu->get_value();
		}

		_dispatcher = dispatcher::create(thread_count, ArgsParser::tm->get_value());
		if (NULL == _dispatcher)
		{
			std::cerr << "failed to create dispatcher." << std::endl;
			return false;
		}
		if (!create_senders())
		{
			return false;
		}

		// 等待所有请求完成
		CCounter::get_singleton()->set_total_num_sender(_sender_array.size());
		CCounter::get_singleton()->wait_finish();
		stat();

        return true;
	}

	virtual void fini()
	{
		dispatcher::destroy(_dispatcher);
        if (_logger != NULL)
        {
		    _logger->destroy();
            _logger = NULL;
        }
	}

	virtual sys::ILogger* get_logger() const
	{
		return _logger;
	}

private:
	/***
	  * 对命令行参数进行解析，并检查参数的有效性
	  */
	bool parse_args(int argc, char* argv[])
	{
		if (!ArgsParser::parse(argc, argv))
		{
			std::cerr << ArgsParser::g_error_message.c_str() << std::endl;
			return false;
		}
		if (ArgsParser::ip->get_value().empty())
		{
			std::cerr << "--ip can not be empty." << std::endl;
			return false;
		}
		if (ArgsParser::pg->get_value().empty())
		{
			std::cerr << "--pg can not be empty." << std::endl;
			return false;
		}
		if (ArgsParser::pg->get_value()[0] != '/')
		{
			std::cerr << "--pg should begin with '/'." << std::endl;
			return false;
		}
		if (ArgsParser::ka->get_value().empty())
		{
			std::cerr << "--ka should be true or false: " << ArgsParser::ka->get_value() << "." << std::endl;
			return false;
		}
		else if (ArgsParser::ka->get_value().compare("true")
			  && ArgsParser::ka->get_value().compare("false"))
		{
			std::cerr << "--ka should be true or false." << std::endl;
			return false;
		}

		return true;
	}

	bool create_senders()
	{
		_sender_table = _dispatcher->get_managed_sender_table();
		for (uint16_t i=0; i<ArgsParser::nu->get_value(); ++i)
		{
			dispatcher::SenderInfo sender_info;
			sender_info.key = i;
			sender_info.ip_node.port = ArgsParser::port->get_value();
			sender_info.ip_node.ip = ArgsParser::ip->get_value().c_str();
			sender_info.queue_size = 2;
			sender_info.resend_times = 0;
			sender_info.reconnect_times = -1;
			sender_info.reply_handler = new CHttpReplyHandler;

			dispatcher::ISender* sender = _sender_table->open_sender(sender_info);
			if (sender != NULL)
			{
				_sender_array.push_back(sender);
			}
			else
			{
				std::cerr << "failed to open sender." << std::endl;
				return false;
			}
		}

		return true;
	}

	/***
	  * 输出统计数据
	  */
	void stat()
	{
		uint32_t num_success = 0;
		uint32_t num_failure = 0;
		uint64_t bytes_recv = 0;
		uint64_t bytes_send = 0;

		// 统计
		for (std::vector<dispatcher::ISender*>::size_type i=0; i<_sender_array.size(); ++i)
		{
			dispatcher::ISender* sender = _sender_array[i];
			CHttpReplyHandler* reply_handler = static_cast<CHttpReplyHandler*>(sender->get_sender_info().reply_handler);

			num_success += reply_handler->get_num_success(); // 成功的请求个数
			num_failure += reply_handler->get_num_failure(); // 失败的请求个数
			bytes_recv += reply_handler->get_bytes_recv(); // 接收到的字节数
			bytes_send += reply_handler->get_bytes_send(); // 发送出的字节数
		}

		// 输出
		std::cout << "summary ==>" << std::endl
                  << "num_success: " << num_success << std::endl
				  << "num_failure: " << num_failure << std::endl
				  << "bytes_recv: " << bytes_recv << std::endl
				  << "bytes_send: " << bytes_send << std::endl;
	}

private:
	sys::CLogger* _logger;
	dispatcher::IDispatcher* _dispatcher;
	dispatcher::IManagedSenderTable* _sender_table;
	std::vector<dispatcher::ISender*> _sender_array;
};

extern "C" int main(int argc, char* argv[])
{
	CMainHelper main_helper;
	return main_template(&main_helper, argc, argv);
}

MOOON_NAMESPACE_END
