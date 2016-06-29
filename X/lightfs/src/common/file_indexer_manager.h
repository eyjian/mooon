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
 * Author: caobiao@gmail.com
 */
#ifndef LIGHTFS_COMMON_FILE_INDEXER_MANAGER_H
#define LIGHTFS_COMMON_FILE_INDEXER_MANAGER_H

#include <string>
#include <vector>
#include "common/file_indexer.h"

namespace lightfs {
namespace common {

class FileIndexerManager
{
    SINGLETON_DECLARE(FileIndexerManager)

public:
    FileIndexerManager();
    ~FileIndexerManager();

    void init(uint32_t count);
    void uninit();

    FileIndexer* get_file_index(uint32_t index);

    INode* find_node(const std::string& path, uint32_t* index);

    INodeFile* find_file(const std::string& path, uint32_t* index);

    void  list_files(const std::string& path,
        std::vector<INode*>* list_nodes);
private:
    uint32_t m_tree_count;
    std::vector<FileIndexer *> m_array_file_index;
};

} // namespace common
} // namespace lightfs

#endif // LIGHTFS_COMMON_FILE_INDEXER_MANAGER_H
