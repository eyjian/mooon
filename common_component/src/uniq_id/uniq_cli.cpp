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
#include <mooon/net/udp_socket.h>

static void usage();

// Usage1: uniq_cli agent_ip agent_port
// Usage2: uniq_cli agent_ip agent_port times
int main(int argc, char* argv[])
{
	if ((argc != 3) || (argc != 4))
	{
		usage();
		exit(1);
	}

	const char* agent_ip = argv[1];
	int agent_port = atoi(argv[2]);
	int times = 0;

	if (4 == argc)
		times = atoi(argv[3]);
	if (times < 0)
	{
		usage();
		exit(1);
	}

	for (int i=0; i<times; ++i)
	{

	}

	return 0;
}

void usage()
{
	fprintf(stderr, "Usage1: uniq_cli uniq_agent_ip uniq_agent_port\n");
	fprintf(stderr, "Usage2: uniq_cli uniq_agent_ip uniq_agent_port times\n");
}
