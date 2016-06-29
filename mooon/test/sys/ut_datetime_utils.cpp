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
#include "mooon/sys/datetime_utils.h"
SYS_NAMESPACE_USE

int main()
{
    struct tm datetime_struct;
    const char* str = "2010-09-18 17:28:30";

    if (!CDatetimeUtils::datetime_struct_from_string(str, &datetime_struct))
        printf("ERROR datetime_struct_from_string: %s\n", str);
    else
        printf("%s ==> \n"
               "\tyear=%04d\n"
               "\tmonth=%02d\n"
               "\tday=%02d\n"
               "\thour=%02d\n"
               "\tminitue=%02d\n"
               "\tsecond=%02d\n"
              ,str
              ,datetime_struct.tm_year+1900
              ,datetime_struct.tm_mon+1
              ,datetime_struct.tm_mday
              ,datetime_struct.tm_hour
              ,datetime_struct.tm_min
              ,datetime_struct.tm_sec);

    time_t dt = mktime(&datetime_struct);
    if (-1 == dt)
        printf("Error mktime\n");
    else
    {
        struct tm result;
        localtime_r(&dt, &result);

        printf("%ld ==> \n"
               "\tyear=%04d\n"
               "\tmonth=%02d\n"
               "\tday=%02d\n"
               "\thour=%02d\n"
               "\tminitue=%02d\n"
               "\tsecond=%02d\n"
              ,dt
              ,result.tm_year+1900
              ,result.tm_mon+1
              ,result.tm_mday
              ,result.tm_hour
              ,result.tm_min
              ,result.tm_sec);
    }

    return 0;
}
