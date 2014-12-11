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
 * Author: jian yi, eyjian@qq.com
 */
#include <signal.h>
#include <stdexcept>
#include "gtf/gtf.h"
#include "util/log.h"
#include "http_factory.h"
#include "sys/sys_util.h"
#include "util/string_util.h"
#include "httpd/http_config.h"
#include "sys/main_template.h"
#include "plugin/plugin_log4cxx/plugin_log4cxx.h"
#include "plugin/plugin_tinyxml/plugin_tinyxml.h"

util::IConfigFile* util::g_config = NULL;

my::IGtf* gtf = NULL;

bool httpd_initialize (int argc, char* argv[], const std::string& home_dir)
{
	// 配置文件目录基于主目录写死，形成规范，以简化运营
	std::string conf_path = home_dir + "/conf";
	
	for (;;)
	{	
		try
		{
			util::g_logger = plugin::create_logger(std::string(conf_path + "/log.conf").c_str());		
			MYLOG_INFO("Created logger success.\n");
		}
		catch (std::invalid_argument& ex)
		{
			fprintf(stderr, "%s:%d %s.\n", __FILE__, __LINE__, ex.what());
			break;
		}
		
		util::g_config = plugin::create_config_file();
		if (NULL == util::g_config)
		{
			MYLOG_FATAL("Can not create config file.\n");
			break;
		}	

		// 加载配置文件
		std::string conf_file = conf_path + "/jhttpd.conf";
		if (!util::g_config->open(conf_file))
		{
			MYLOG_FATAL("Load config %s failed and exited.\n", conf_file.c_str());
			break;
		}

		my::CHttpConfig* config = new my::CHttpConfig(util::g_config);
		my::CHttpFactory* factory = new my::CHttpFactory;

		if (!config->load())
		{
			break;
		}

		gtf = my::create_gtf();
		if (NULL == gtf)
		{
			break;
		}    
		if (!gtf->create(config, factory))
		{
			break;
		}

		return true;
	}

	return false;
}

void httpd_uninitialize()
{
	gtf->destroy();
    my::destroy_gtf(gtf);
}

//
// home_dir
//    |
//    -----conf_dir
//    |
//    -----bin_dir
//    |
//    -----lib_dir
//    |
//    -----log_dir
//
int main(int argc, char* argv[])
{    
    sys::my_initialize = httpd_initialize;
    sys::my_uninitialize = httpd_uninitialize;
	return sys::main_template(argc, argv);
}
