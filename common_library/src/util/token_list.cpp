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
#include "util/token_list.h"
UTIL_NAMESPACE_BEGIN

void CTokenList::parse(TTokenList& token_list, const std::string& source, const std::string& sep)
{
	std::string::size_type pos = 0;
	std::string::size_type old_pos = 0;

	/*
	while (true)
	{
		pos = source.find(sep, old_pos);
		if (std::string::npos == pos)
		{
			if (0 == old_pos)
				token_list.push_back(source);
			else
				token_list.push_back(source.substr(old_pos+sep.length()-1));

			break;
		}

		// 如果两个sep是连接着的，那么pos和old_pos值相差为1，空的token过滤掉
		if (pos >= old_pos+sep.length())
			token_list.push_back(source.substr(old_pos+sep.length()-1, pos-old_pos-sep.length()+1));

		old_pos = pos + 1;
	}
	*/

	//解决当sep大于2个字符的时候的bug.
	do
	{
		pos = source.find(sep, old_pos);

		//处理当sep出现在最前面的特殊情况
		if (pos == old_pos)
		{
			old_pos = pos + 1;
		}
		else if (pos == std::string::npos)
		{
			token_list.push_back( source.substr(old_pos) );
			break;
		}
		else
		{
			token_list.push_back( source.substr(old_pos, pos - old_pos) );
			old_pos = pos + 1;
		}
		old_pos = source.find_first_not_of(sep, old_pos);
	} while (pos != std::string::npos && old_pos != std::string::npos);
}

UTIL_NAMESPACE_END
