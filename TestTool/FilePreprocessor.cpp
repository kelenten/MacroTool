// FilePreprocessor.cpp
#include "FilePreprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdio>

FilePreprocessor::FilePreprocessor() {}

std::string FilePreprocessor::preprocessFile(const std::string& path) {
    // 构造预处理命令
    std::string command = "clang++ -E -fexceptions " + path + " -o " + path + ".preprocessed";

    // 执行预处理命令
    int result = std::system(command.c_str());
    if (result != 0) {
        return "-1";
    }

    // 返回预处理文件的路径
    return path + ".preprocessed";
}
