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
#include "protocol.h"
#include <mooon/net/udp_socket.h>
#include <mooon/net/utils.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>

int main(int argc, char* argv[])
{
    if ((argc != 3) && (argc != 4))
    {
        fprintf(stderr, "Usage1: master_cli master_ip master_port\n");
        fprintf(stderr, "Usage2: master_cli master_ip master_port label\n");
        exit(1);
    }

    const char* label = NULL;
    const char* master_ip = argv[1];
    const char* master_port = argv[2];
    if (4 == argc)
        label = argv[3];

    try
    {
        struct sockaddr_in from_addr;
        mooon::net::CUdpSocket udp_socket;
        mooon::MessageHead response;
        mooon::MessageHead request;
        request.len = sizeof(request);
        request.type = mooon::REQUEST_LABEL;
        request.echo = 2016;
        request.value1 = (NULL == label)? 0: atoi(label);
        request.value2 = 0;

        udp_socket.send_to(&request, sizeof(request), master_ip, atoi(master_port));
        udp_socket.timed_receive_from(&response, sizeof(response), &from_addr, 2000);
        fprintf(stdout, "%s\n", response.str().c_str());

        if (mooon::RESPONSE_ERROR == response.type)
            fprintf(stderr, "ERROR: %u\n", response.value1.to_int());
        else if (mooon::RESPONSE_LABEL == response.type)
            fprintf(stdout, "label: %u(%s)\n", response.value1.to_int(), mooon::label2string(response.value1.to_int()).c_str());
        else
            fprintf(stderr, "UNKNOWN\n");
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
    catch (mooon::utils::CException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }

    return 0;
}
