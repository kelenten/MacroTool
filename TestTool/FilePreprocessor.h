// FilePreprocessor.h
#ifndef FILEPREPROCESSOR_H
#define FILEPREPROCESSOR_H

#include <string>

class FilePreprocessor {
public:
    FilePreprocessor();
    std::string preprocessFile(const std::string& path);
};

#endif
#pragma once
