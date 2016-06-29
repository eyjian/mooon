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
 * Author: JianYi, eyjian@qq.com
 *
 * 除标准库和系统调用外，要求本文件不依赖其它任何文件和库
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#if defined(PATH_MAX)
#undef PATH_MAX
#endif
#define PATH_MAX 1024

// 用来启动其它可执行程序，自动设置好运行环境

// 只带一个参数，即待运行的程序名
int main(int argc, char* argv[])
{
	// 参数检查
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s program_name\n", argv[0]);
		exit(1);
	}

	char* program_name = argv[1];

	// 得到运行目录
	char path[PATH_MAX];
	int retval = readlink("/proc/self/exe", path, sizeof(path)-1);
	if (-1 == retval)
	{
		fprintf(stderr, "readlink error for %s.\n", strerror(errno));
		exit(2);
	}

	// 加工得到路径
	path[retval] = '\0';
	if (!strcmp(path+retval-10," (deleted)"))
		path[retval-10] = '\0';
	char* splash = strrchr(path, '/');
	if (NULL == splash)
	{
		fprintf(stderr, "%s error, 2 slashes(/) needed.\n", argv[0]);
		exit(3);
	}

	*splash = '\0';
	splash = strrchr(path, '/');
	if (NULL == splash)
	{
		fprintf(stderr, "%s error, 2 slashes(/) needed.\n", argv[0]);
		exit(4);
	}	

	*splash = '\0';
	fprintf(stdout, "Program path is %s.\n", path);

	// 设置环境变量
	char* env;
	char* old = getenv("LD_LIBRARY_PATH");
	if (NULL == old)
	{
		env = new char[strlen(path)+sizeof("/lib")];
		sprintf(env, "%s/lib", path);
	}
	else
	{
		env = new char[strlen(path)+strlen(old)+sizeof("/lib:")];
		sprintf(env, "%s/lib:%s", path, old);
	}
	
	setenv("LD_LIBRARY_PATH", env, 1);

	pid_t pid = fork();
	if (-1 == pid)
	{
		fprintf(stderr, "fork error for %s.\n", strerror(errno));
		exit(5);
	}
	else if (0 == pid)
	{
		// 启动程序
		if (-1 == execl(program_name, program_name, NULL))
		{
			fprintf(stderr, "execl error for %s.\n", strerror(errno));
			exit(110);
		}
	}

	int wait_stat = 0;
	waitpid(pid, &wait_stat, 0);
	delete []env;
	return 0;
}
