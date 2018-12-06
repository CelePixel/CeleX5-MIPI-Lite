/*
* Copyright (c) 2017-2018 CelePixel Technology Co. Ltd. All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "xbase.h"
#include <fstream>
#include <time.h>
#include <iostream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

XBase::XBase()
{

}

XBase::~XBase()
{

}

std::string XBase::getApplicationDirPath()
{
    //D:\\Work\\build-okBaseTest-Desktop_Qt_5_9_1_MinGW_32bit-Debug\\debug\\okBaseTest.exe
#ifdef _WIN32
    char path[1024];
    memset(path, 0, 1024);
    GetModuleFileNameA(NULL, path, 1024);
    std::string strPath = path;
    int len = strPath.find_last_of('\\');
    return strPath.substr(0, len);
#else
    char path[1024];
    int cnt = readlink("/proc/self/exe", path, 1024);
	cout << "XBase::getApplicationDirPath: readlink count = " << cnt << endl;
    if(cnt < 0|| cnt >= 1024)
    {
        return NULL;
    }
    for(int i = cnt; i >= 0; --i)
    {
        if(path[i]=='/')
        {
            path[i + 1]='\0';
            break;
        }
    }
    string strPath(path);
    return strPath;
#endif
}

bool XBase::isFileExists(std::string filePath)
{
    //"D:/Work/build-okBaseTest-Desktop_Qt_5_9_1_MinGW_32bit-Debug/debug/top.bit";
    printf("XBase::isFileExists: %s", filePath.c_str());
    fstream _file;
    _file.open(filePath.c_str(), ios::in);
    if (!_file)
    {
        printf("%s can't find!", filePath.c_str());
        return false;
    }
    _file.close();
    return true;
}
