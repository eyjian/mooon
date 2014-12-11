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
#include "common/file_indexer_manager.h"

namespace lightfs {
namespace common {

SINGLETON_IMPLEMENT(FileIndexerManager)

FileIndexerManager::FileIndexerManager()
    : m_tree_count(0)
{
}

FileIndexerManager::~FileIndexerManager()
{
    uninit();
}

void FileIndexerManager::uninit()
{
    for (uint32_t i = 0; i < m_tree_count; ++i) {
        delete m_array_file_index[i];
    }
    m_array_file_index.clear();
}

void FileIndexerManager::init(uint32_t count)
{
    m_tree_count = count;
    for (uint32_t i = 0; i < count; ++i) {
        FileIndexer* file_indexer = new FileIndexer(i);
        m_array_file_index.push_back(file_indexer);
    }
}

FileIndexer* FileIndexerManager::get_file_index(uint32_t index)
{
    if (index >= m_tree_count) {
        return NULL;
    }
    return m_array_file_index[index];
}

INode* FileIndexerManager::find_node(const std::string& path, uint32_t* index)
{
    for (uint32_t i = 0; i < m_tree_count; i++) {
        FileIndexer* file_indexer = m_array_file_index[i];
        INode* node = file_indexer->find_node(path);
        if (node) {
            *index = i;
            return node;
        }
    }
    return NULL;
}

INodeFile* FileIndexerManager::find_file(const std::string& path, uint32_t* index)
{
    INode * node = find_node(path, index);
    if (node) {
        if (node->is_directory()) {
            return NULL;
        }
        return static_cast<INodeFile*>(node);
    }
    return NULL;
}

void FileIndexerManager::list_files(const std::string& path,
    std::vector<INode*>* list_nodes)
{
    for (size_t i = 0; i < m_tree_count; i++) {
        FileIndexer *file_indexer = m_array_file_index[i];
        file_indexer->list_files(path, list_nodes);
    }
}

} // namespace common
} // namespace lightfs
