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
#include <mooon/uniq_id/uniq_id.h>

static void usage();

// Usage1: uniq_cli agent_nodes
// Usage2: uniq_cli agent_nodes times
int main(int argc, char* argv[])
{
	if ((argc != 2) && (argc != 3))
	{
		usage();
		exit(1);
	}

	int times = 1;
	const char* agent_nodes = argv[1];
	if (3 == argc)
		times = atoi(argv[2]);
	if (times < 1)
	    times = 1;

	for (int i=0; i<times; ++i)
	{
	    try
	    {
	        mooon::CUniqId uniq_id(agent_nodes);
	        uint64_t uid = uniq_id.get_uniq_id();
	        union mooon::UniqID uid_struct;
	        uid_struct.value = uid;
	        fprintf(stdout, "uid: %"PRIu64" => %s\n", uid, uid_struct.id.str().c_str());
	        fprintf(stdout, "label: %s, sequence: %u\n", mooon::label2string(uid_struct.id.label).c_str(), (uint32_t)uid_struct.id.seq);
	    }
	    catch (mooon::sys::CSyscallException& ex)
	    {
	        fprintf(stderr, "%s\n", ex.str().c_str());
	        break;
	    }
	    catch (mooon::utils::CException& ex)
	    {
	        fprintf(stderr, "%s\n", ex.str().c_str());
	        break;
	    }
	}

	return 0;
}

void usage()
{
	fprintf(stderr, "Usage1: uniq_cli uniq_agent_nodes\n");
	fprintf(stderr, "Usage2: uniq_cli uniq_agent_nodes times\n");
}
