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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#include "protocol_parser.h"
MOOON_NAMESPACE_BEGIN

CProtocolParser::CProtocolParser()
{
    reset();
}

CProtocolParser::~CProtocolParser()
{
    delete []_packet;
}

void CProtocolParser::reset()
{
    _current_offset = 0;
    _to_receive_packet = false;
    _first_four_bytes.zero();
}

util::handle_result_t CProtocolParser::parse(uint32_t data_size)
{
    if (!_to_receive_packet)
    {
        // Í·ËÄ×Ö½Ú
        if (_current_offset + data_size == sizeof(_first_four_bytes))
        {
            _current_offset = 0;
            _to_receive_packet = true;

            _first_four_bytes.ntoh();
            _packet = new char[_first_four_bytes.total_size];
        }
    }
    else
    {
        if (_current_offset + data_size == _first_four_bytes.total_size)
        {
            return util::handle_finish;
        }
    }

    return util::handle_continue;
}

void CProtocolParser::move_buffer_offset(uint32_t offset)
{
    _current_offset += offset;
}

uint32_t CProtocolParser::get_buffer_offset() const
{
    return _current_offset;
}

uint32_t CProtocolParser::get_buffer_size() const
{
    return _to_receive_packet
         ? _first_four_bytes.total_size - _current_offset
         : sizeof(_first_four_bytes) - _current_offset;
}

char* CProtocolParser::get_buffer()
{
    return static_cast<char*>(_to_receive_packet? _packet: &_first_four_bytes);
}

MOOON_NAMESPACE_END
