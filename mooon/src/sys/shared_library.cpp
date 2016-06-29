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
#include "sys/shared_library.h"
SYS_NAMESPACE_BEGIN

CSharedLibrary::CSharedLibrary()
	:_handle(NULL)
{
}

CSharedLibrary::~CSharedLibrary()
{
	unload();
}

bool CSharedLibrary::load(const char *filename, int flag)
{
	_handle = dlopen(filename, flag);
	if (NULL == _handle)
	{
		char* error = dlerror();
		_error_message = (NULL == error)? "Unknown error": error;
	}

	return (_handle != NULL);
}

void CSharedLibrary::unload()
{
	if (_handle != NULL)
	{
		dlclose(_handle);
		_handle = NULL;
	}
}

void* CSharedLibrary::get_symbol(const char *symbol_name)
{
	void* symbol = dlsym(_handle, symbol_name);
	if (NULL == symbol)
	{
		char* error = dlerror();
		_error_message = (NULL == error)? "Unknown error": error;
	}

	return symbol;
}

SYS_NAMESPACE_END
