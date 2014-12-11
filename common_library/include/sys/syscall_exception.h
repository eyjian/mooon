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
#ifndef MOOON_SYS_SYSCALL_EXCEPTION_H
#define MOOON_SYS_SYSCALL_EXCEPTION_H
#include "sys/config.h"
SYS_NAMESPACE_BEGIN

/** 系统调用出错异常，多数系统调用出错时，均以此异常型反馈给调用者 */
class CSyscallException
{
public:
    /** 构造系统调用异常
      * @errcode: 系统调用出错代码
      * @filename: 出错所发生的文件名
      * @linenumber: 出错发生的行号
      * @tips: 额外的增强信息，用以进一步区分是什么错误
      */
	CSyscallException(int errcode, const char* filename, int linenumber, const char* tips=NULL);    

    /** 得到调用出错发生的文件名 */
    const char* get_filename() const { return _filename; }

    /** 得到调用出错时发生的文件行号 */
    int get_linenumber() const { return _linenumber; }

    /** 得到调用出错时的系统出错码，在Linux上为errno值 */
    int get_errcode() const { return _errcode; }

    /** 得到出错信息 */
    std::string get_errmessage() const;

    /** 得到调用出错时的提示信息，提示信息用于辅助明确问题，为可选内容
      * 如果返回非NULL，则表示有提示信息，否则表示无提示信息
      */
    const char* get_tips() const { return _tips.empty()? NULL: _tips.c_str(); }

    /** 异常信息打包成字符串，内容包括文件名、行号、出错代码和出错信息 */
    std::string to_string() const;

private:
	int _errcode;
	int _linenumber;
	char _filename[FILENAME_MAX];
    std::string _tips;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SYSCALL_EXCEPTION_H
