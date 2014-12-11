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
 * df命令的实现
 */
#include "sys/fs_util.h"
using namespace sys;

int main()
{
	CFSTable fs_table(true, "/");
	CFSTable::fs_entry_t fs_entry;

        printf("%-18s  %12s  %12s  %12s  %5s  %s\n"
	      ,"文件系统"
              ,"1K-块"
              ,"已用"
              ,"可用"
              ,"已用%"
              ,"挂载点"
        );
    
	while (fs_table.get_entry(fs_entry) != NULL)	
	{
		try
		{
			CFSUtil::fs_stat_t stat_buf;
			CFSUtil::stat_fs(fs_entry.dir_path.c_str(), stat_buf);

			unsigned long total = stat_buf.total_block_nubmer * (stat_buf.block_bytes/1024);
			unsigned long used = (stat_buf.total_block_nubmer-stat_buf.free_block_nubmer) * (stat_buf.block_bytes/1024);
			unsigned long avail = stat_buf.avail_block_nubmer*(stat_buf.block_bytes/1024);

			printf("%-15s%12Ld%12Ld%12Ld%5d    %s\n"
			      ,fs_entry.fs_name.c_str()
			      ,total
			      ,used
			      ,avail
			      ,((total-avail) * 100) / total
			      ,fs_entry.dir_path.c_str()
			);
		}
		catch (CSyscallException& ex)
		{
			printf("stat_fs exception: %s\n", ex.to_string().c_str());
		}
	}

	return 0;
}

