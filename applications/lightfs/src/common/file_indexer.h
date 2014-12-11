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
#ifndef LIGHTFS_COMMON_FILE_INDEXER_H
#define LIGHTFS_COMMON_FILE_INDEXER_H

#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include <sys/lock.h>

namespace lightfs {
namespace common {

class INodeDirectory;

class INode
{
public:
    explicit INode(const std::string& name = "");
    virtual ~INode();

public:
    void set_name(const std::string& name);
    const std::string& get_name() const;
    void set_parent(INodeDirectory* parent);
    INodeDirectory* get_parent() const;

public:
    static void get_path_names(const std::string& path,
        std::vector<std::string>* result);

public:
    virtual bool is_directory() const = 0;
protected:
    std::string m_name;
    INodeDirectory* m_parent;
};

class INodeDirectory : public INode
{
public:
    explicit INodeDirectory(const std::string& name = "");
    ~INodeDirectory();

    INode* add_sub_dir(const std::string& name);
    INode* add_child(const std::string& name);
    INode* remove_child(const std::string& name);

    INode* get_child(const std::string& name);
    INode* get_child(size_t index) const;
    uint32_t get_child_count() const;

    virtual bool is_directory() const;
private:
    int binary_search(const std::string& name);

private:
    std::vector<INode*>m_children;

protected:
    static const size_t DEFAULT_FILES_PER_DIRECTORY = 5;
};

struct FileAttribute
{
    size_t size;
    uint32_t crc;
};

class INodeFile : public INode
{
public:
    explicit INodeFile(const std::string& name);
    ~INodeFile();

    void set_file_attribute(const FileAttribute* attr);
    void get_file_attribute(FileAttribute* attr);

    virtual bool is_directory() const;
private:
    FileAttribute m_file_attribute;
};

class FileIndexer
{
public:
    explicit FileIndexer(uint32_t tree_index);
    ~FileIndexer();

    bool add_node(const std::string& path, const FileAttribute& attri);
    bool del_node(const std::string& path);
    INode* find_node(const std::string& path);
    bool rename_node(const std::string& old_path,
        const std::string& new_path);
    void list_files(const std::string& path,
        std::vector<INode*>* list_nodes);

    bool rebuild_tree_from_disk(const std::vector<std::string>& exclude_paths);
    bool rebuild_tree_from_file(const std::string& path);
    bool serialize_to_file(const std::string& path);
private:
    bool check_path_valid(const std::string& path);
    INodeDirectory* make_parent_dirs(const std::vector<std::string>& paths);
    INodeDirectory* get_parent_node(const std::vector<std::string>& paths) const;
    INodeDirectory* get_last_node(const std::vector<std::string>& paths) const;
    void recurve_resolver_tree(FILE* fp, INodeDirectory *dir_node,
        const std::string& parent_path);
private:
    INodeDirectory *m_root;
    sys::CLock m_lock;
    uint32_t m_tree_index;
};

} // namespace common
} // namespace lightfs

#endif // LIGHTFS_COMMON_FILE_INDEXER_H
