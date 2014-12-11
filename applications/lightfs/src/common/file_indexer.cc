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
#include "common/file_indexer.h"

#include <assert.h>
#include <dirent.h>
#include <algorithm>
#include <deque>
#include <iostream>
#include <sys/log.h>
#include "common/util.h"

namespace lightfs {
namespace common {

static uint32_t kLfsDepthMax = 12;
static uint32_t kLfsPathSizeMax = 1024;
static uint32_t kLfsLineSizeMax = kLfsPathSizeMax + 100;

INode::INode(const std::string& name)
    :m_name(name),
    m_parent(NULL)
{
}

INode::~INode()
{
}

void INode::set_name(const std::string& name)
{
    m_name = name;
}

const std::string& INode::get_name() const
{
    return m_name;
}

void INode::set_parent(INodeDirectory* parent)
{
    assert(parent != NULL);
    m_parent = parent;
}

INodeDirectory* INode::get_parent() const
{
    return m_parent;
}

void INode::get_path_names(const std::string& path,
    std::vector<std::string>* result)
{
    assert(result != NULL);
    if (path.length() == 0 || path[0] != '/') {
        return;
    }

    Util::split_string(path, '/', result);
}

struct INodeComparator
{
    bool operator() (const INode* lhs, const std::string& rhs) const
    {
        return (lhs->get_name().compare(rhs) < 0);
    }
};

INodeDirectory::INodeDirectory(const std::string& name) : INode(name)
{
}

INodeDirectory::~INodeDirectory()
{
    std::vector<INode*>::iterator it = m_children.begin();
    for (; it != m_children.end(); it++) {
        delete *it;
    }
    m_children.clear();
}

int INodeDirectory::binary_search(const std::string& name)
{
    std::vector<INode*>::iterator first =
        lower_bound(m_children.begin(), m_children.end(),
        name, INodeComparator());
    if (first != m_children.end() &&
        !(name.compare((*first)->get_name()) < 0)) {
        return first - m_children.begin();
    }
    return -(first - m_children.begin()) - 1;
}

bool INodeDirectory::is_directory() const
{
    return true;
}

INode* INodeDirectory::get_child(const std::string& name)
{
    int low = binary_search(name);
    if (low >= 0) {
        return m_children.at(low);
    }
    return NULL;
}

INode* INodeDirectory::add_sub_dir(const std::string& name)
{
    int low = binary_search(name);
    if (low >= 0) {
        return m_children.at(low);
    }

    INodeDirectory* sub_dir = new INodeDirectory();
    sub_dir->set_name(name);
    sub_dir->set_parent(this);
    m_children.insert(m_children.begin() + (-low - 1), sub_dir);
    return sub_dir;
}

INode* INodeDirectory::add_child(const std::string& name)
{
    int low = binary_search(name);
    if (low >= 0) {
        return m_children.at(low);
    }
    INode* node  = new INodeFile(name);
    node->set_parent(this);
    m_children.insert(m_children.begin() + (-low - 1), node);
    return node;
}

INode* INodeDirectory::remove_child(const std::string& name)
{
    int low = binary_search(name);
    if (low >= 0) {
        delete m_children.at(low);
        return *m_children.erase(m_children.begin() + low);
    }
    return NULL;
}

uint32_t INodeDirectory::get_child_count() const
{
    return m_children.size();
}

INode *INodeDirectory::get_child(size_t index) const
{
    if (index >= m_children.size())
        return NULL;

    return m_children.at(index);
}

INodeFile::INodeFile(const std::string& name) : INode(name)
{
    memset(&m_file_attribute, 0, sizeof(FileAttribute));
}

INodeFile::~INodeFile()
{
}

void INodeFile::set_file_attribute(const FileAttribute* attr)
{
    memcpy(&m_file_attribute, attr, sizeof(FileAttribute));
}

void INodeFile::get_file_attribute(FileAttribute* attr)
{
    memcpy(attr, &m_file_attribute, sizeof(FileAttribute));
}

bool INodeFile::is_directory() const
{
    return false;
}

FileIndexer::FileIndexer(uint32_t tree_index)
    :m_tree_index(tree_index)
{
    m_root = new INodeDirectory();
}

FileIndexer::~FileIndexer()
{
    delete m_root;
    m_root  = NULL;
}

bool FileIndexer::add_node(const std::string& path, const FileAttribute& attri)
{
    if (!check_path_valid(path)) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return false;
    }

    std::vector<std::string> names;
    INode::get_path_names(path, &names);

    if (names.size() > kLfsDepthMax || names.size() == 0) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return false;
    }

    sys::LockHelper<sys::CLock> lh(m_lock);
    INodeDirectory *parent_node = make_parent_dirs(names);
    if (!parent_node) {
        MYLOG_ERROR("Make [%s] parent dirs failed", path.c_str());
        return false;
    }

    INodeFile *node = static_cast<INodeFile*>(
        parent_node->add_child(names[names.size() - 1]));
    node->set_file_attribute(&attri);

    return true;
}

bool FileIndexer::del_node(const std::string& path)
{
    if (!check_path_valid(path)) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return false;
    }

    std::vector<std::string> names;
    INode::get_path_names(path, &names);

    if (names.size() > kLfsDepthMax || names.size() == 0) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return false;
    }

    sys::LockHelper<sys::CLock> lh(m_lock);
    INodeDirectory *parent_node  = get_parent_node(names);
    if (!parent_node) {
        return false;
    }

    if (!parent_node->remove_child(names[names.size() - 1])) {
        return false;
    }
    return true;
}

INode* FileIndexer::find_node(const std::string& path)
{
    if (!check_path_valid(path)) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return false;
    }

    std::vector<std::string> names;
    INode::get_path_names(path, &names);
    if (names.size() > kLfsDepthMax) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return NULL;
    }

    if (names.size() == 0) {
        return m_root;
    }

    sys::LockHelper<sys::CLock> lh(m_lock);
    INodeDirectory *parent_node  = get_parent_node(names);
    if (!parent_node) {
        return NULL;
    }
    return parent_node->get_child(names[names.size() - 1]);
}

bool FileIndexer::rename_node(const std::string& old_path, const std::string& new_path)
{
    sys::LockHelper<sys::CLock> lh(m_lock);

    INode *old_node = find_node(old_path);
    if (!old_node) {
        MYLOG_ERROR("Rename node failed. cant 't find old path.old path [%s] new path [%s]",
            old_path.c_str(), new_path.c_str());
        return false;
    }
    if (old_node->is_directory()) {
        MYLOG_ERROR("Rename node failed. old path is a dierectory.old path [%s] new path [%s]",
            old_path.c_str(), new_path.c_str());
        return false;
    }

    FileAttribute attri;
    static_cast<INodeFile*>(old_node)->get_file_attribute(&attri);
    if (!del_node(old_path)) {
        MYLOG_ERROR("Rename node failed. old path [%s] new path [%s]",
            old_path.c_str(), new_path.c_str());
        return false;
    }

    if (!add_node(new_path, attri)) {
        MYLOG_ERROR("Rename node failed. old path [%s] new path [%s]",
            old_path.c_str(), new_path.c_str());
        return false;
    }
    return true;
}

void  FileIndexer::list_files(const std::string& path,
    std::vector<INode*>* list_nodes)
{
    if (!check_path_valid(path)) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return;
    }

    std::vector<std::string> names;
    INode::get_path_names(path, &names);
    if (names.size() > kLfsDepthMax) {
        MYLOG_ERROR("Directory path [%s] is invalid", path.c_str());
        return;
    }
    sys::LockHelper<sys::CLock> lh(m_lock);

    INodeDirectory *last_node  = get_last_node(names);
    if (!last_node) {
        MYLOG_ERROR("Can't get last node [%s].", path.c_str());
        return;
    }

    for (uint32_t i = 0; i < last_node->get_child_count(); ++i) {
        INode *node = last_node->get_child(i);
        list_nodes->push_back(node);
    }
}

bool FileIndexer::rebuild_tree_from_disk(const std::vector<std::string>& exclude_paths)
{
    // to do
    return true;
}

bool FileIndexer::rebuild_tree_from_file(const std::string& path)
{
    if (path == "") {
        return false;
    }
    FILE* fp = fopen(path.c_str(), "r");
    if (!fp) {
        MYLOG_ERROR("Open failed. path [%s]", path.c_str());
        return false;
    }
    char line_str[kLfsLineSizeMax];
    char file_path[kLfsPathSizeMax];
    FileAttribute attri;
    bool ret = true;

    do {
        int32_t read_size = Util::read_line(fp, line_str, sizeof(line_str));
        if (read_size == 0) {
            break;
        } else if (read_size < 0) {
            MYLOG_ERROR("ReadLine failed. path [%s]", path.c_str());
            ret = false;
            break;
        } else {
            int count = sscanf(line_str, "%1124[^,],%lu,%u", file_path, &attri.size, &attri.crc);
            if (count != 3) {
                ret = false;
                break;
            }
            if (!add_node(file_path, attri)) {
                ret = false;
                std::cout << "add node failed" << std::endl;
                break;
            }
        }
    } while (true);
    fclose(fp);

    return ret;
}

bool FileIndexer::serialize_to_file(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "w");
    if (!fp) {
        MYLOG_ERROR("Open failed. path [%s]", path.c_str());
        return false;
    }
    sys::LockHelper<sys::CLock> lh(m_lock);
    recurve_resolver_tree(fp, m_root, "");
    fclose(fp);
    return true;
}

void FileIndexer::recurve_resolver_tree(FILE* fp, INodeDirectory* dir_node,
    const std::string& parent_path)
{
    if (dir_node == NULL) {
        return;
    }

    for (uint32_t i = 0; i < dir_node->get_child_count(); i++) {
        INode *child_node = dir_node->get_child(i);
        if (!child_node->is_directory()) {
            INodeFile *child_node_file = static_cast<INodeFile*>(child_node);
            FileAttribute attri;
            child_node_file->get_file_attribute(&attri);

            char full_path[kLfsLineSizeMax];
            snprintf(full_path, sizeof(full_path), "%s/%s, %lu, %u\n", parent_path.c_str(),
                child_node_file->get_name().c_str(),  attri.size,
                attri.crc);
            size_t ret_size = fwrite(full_path, 1, strlen(full_path), fp);
            if (ret_size != strlen(full_path)) {
                MYLOG_ERROR("Write failed. path [%s]", full_path);
            }
        } else {
            INodeDirectory *child_node_dir = static_cast<INodeDirectory*>(child_node);

            char full_path[kLfsLineSizeMax];
            snprintf(full_path, sizeof(full_path), "%s/%s",
                parent_path.c_str(), child_node->get_name().c_str());

            recurve_resolver_tree(fp, child_node_dir, full_path);
        }
    }
}

bool FileIndexer::check_path_valid(const std::string& path)
{
    if (path.length() == 0  || path.size() > kLfsPathSizeMax) {
        return false;
    }
    if (path[0] != '/') {
        return false;
    }
    return true;
}

INodeDirectory* FileIndexer::make_parent_dirs(const std::vector<std::string>& paths)
{
    if (paths.size() == 0) {
        return NULL;
    }

    INode *cur_node = m_root;
    for (size_t  i = 0; i < paths.size() - 1; ++i) {
        INode* node = static_cast<INodeDirectory*>(cur_node)->get_child(paths[i]);
        if (node == NULL) {
            cur_node =  static_cast<INodeDirectory*>(cur_node)->add_sub_dir(paths[i]);
            if (!cur_node) {
                return NULL;
            }
        } else {
            if (!cur_node->is_directory()) {
                return NULL;
            }
            cur_node = node;
        }
    }
    return static_cast<INodeDirectory*>(cur_node);
}

INodeDirectory* FileIndexer::get_parent_node(const std::vector<std::string>& paths) const
{
    if (paths.size() == 0) {
        return NULL;
    }

    INode* parent_node = m_root;
    for (size_t i = 0; i < paths.size() - 1; ++i) {
        parent_node = static_cast<INodeDirectory*>(parent_node)->get_child(paths[i]);
        if (!parent_node || !parent_node->is_directory()) {
            return NULL;
        }
    }
    return static_cast<INodeDirectory*>(parent_node);
}

INodeDirectory* FileIndexer::get_last_node(const std::vector<std::string>& paths) const
{
    INode* parent_node = m_root;
    for (size_t i = 0; i < paths.size(); ++i) {
        parent_node = static_cast<INodeDirectory*>(parent_node)->get_child(paths[i]);
        if (!parent_node || !parent_node->is_directory()) {
            return NULL;
        }
    }
    return static_cast<INodeDirectory*>(parent_node);
}

} // namespace common
} // namespace lightfs
