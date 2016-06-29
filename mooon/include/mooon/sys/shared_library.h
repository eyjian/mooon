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
#ifndef MOOON_SYS_SHARED_LIBRARY_H
#define MOOON_SYS_SHARED_LIBRARY_H
#include "mooon/sys/utils.h"
#include <dlfcn.h>
SYS_NAMESPACE_BEGIN

/***
  * 共享库加载工具类
  * 非线程安全类，不要跨线程使用
  */
class CSharedLibrary
{
public:
    /** 构造一个共享库加载器 */
	CSharedLibrary();
	~CSharedLibrary();

	/**
      * 根据文件名加载共享库
      * @filename: 共享库文件名
      * @flag: 加载标志，可取值RTLD_NOW或RTLD_LAZY
      * @return: 如果加载成功返回true，否则返回false，如果返回false，
      *          可调用get_error_message获取失败原因
	  */
	bool load(const char *filename, int flag = RTLD_NOW);

    /** 卸载已经加载的共享库 */
	void unload();

    /**
      * 得到出错信息，load或get_symbol失败时，
      * 应当调用它来得到出错信息
      */
	const std::string& get_error_message() const { return _error_message; }

    /***
      * 根据符号名得到符号的地址
      * @symbol_name: 符号名，通常为函数名
      * @return: 返回符号对应的地址，通常为函数地址，如果失败则返回NULL
      */
	void* get_symbol(const char *symbol_name);

private:
	void *_handle;
	std::string _error_message;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SHARED_LIBRARY_H
