#include "common/file_indexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "common/util.h"
using namespace lightfs::common;

int main(int argc, char *argv[])
{
    FileIndexer file_indexer(0);
    FileAttribute attri;
    const std::string& prefix_path = "/test/logstore/yewumingzi/201306150930/";
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 100; j++) {
            std::stringstream stream;
            stream << prefix_path << i << "/" << j;
            bool ret = file_indexer.add_node(stream.str(), attri);
            if (!ret) {
                std::cout << "add node failed, path:" << stream.str() << std::endl;
            }
        }
    }

    INode* node = file_indexer.find_node(prefix_path + "5/10");
    if (!node) {
        std::cout << "find node failed" << std::endl;
        return 0;
    } else {
        std::cout << "find node success" << std::endl;
    }
    uint64_t begin_time = Util::milli_seconds();
    bool ret = file_indexer.serialize_to_file("0.image");
    if (!ret) {
        std::cout << "serialize to file failed" << std::endl;
        return 0;
    }
    uint64_t elapsed_time = Util::milli_seconds() - begin_time;
    std::cout << "serial const time:" << elapsed_time << std::endl;

    begin_time = Util::milli_seconds();
    ret = file_indexer.rebuild_tree_from_file("./0.image");
    if (!ret) {
        std::cout << "rebuild tree from file failed" << std::endl;
    }
    elapsed_time = Util::milli_seconds() - begin_time;
    std::cout << "rebuild tree from file const time:" << elapsed_time << std::endl;
    return 0;
}
