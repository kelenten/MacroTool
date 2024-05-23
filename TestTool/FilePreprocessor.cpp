// FilePreprocessor.cpp
#include "FilePreprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdio>

FilePreprocessor::FilePreprocessor() {}

std::string FilePreprocessor::preprocessFile(const std::string& path) {
    // ����Ԥ��������
    std::string command = "clang++ -E -fexceptions " + path + " -o " + path + ".preprocessed";

    // ִ��Ԥ��������
    int result = std::system(command.c_str());
    if (result != 0) {
        return "-1";
    }

    // ����Ԥ�����ļ���·��
    return path + ".preprocessed";
}
