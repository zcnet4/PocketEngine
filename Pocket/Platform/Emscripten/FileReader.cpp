//
//  WindowWeb.cpp
//  GUIEditor
//
//  Created by Jeppe Nielsen on 3/31/14.
//  Copyright (c) 2014 Jeppe Nielsen. All rights reserved.
//

#include "FileReader.hpp"
using namespace Nano;

std::string FileReader::GetFile(std::string localFile) {
    return "Assets/" + localFile;
}

unsigned char* FileReader::GetData(std::string localFile, size_t* length) {
    return 0;
}

std::string FileReader::GetWritablePath() {
    return "";
}
std::string FileReader::GetExecutablePath() {
    return "";
}
std::string FileReader::GetBundlePath() {
    return "";
}
std::string FileReader::GetBundleDir() {
    return "";
}
