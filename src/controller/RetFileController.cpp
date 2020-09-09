//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include "../common/utils.h"
#include <iostream>
#include <sys/stat.h>
#include <fcgi_stdio.h>

void controller_ret_file_handler(FCGX_Request &request, std::string &uri) {
    std::string path(FILE_PREFIX);
    path.append(uri);
    FILE *p_in_file = FCGI_fopen(path.c_str(), "rb+");
    if (p_in_file == nullptr) {
        controller_error_handler(request, -3, "文件不存在");
    } else {
        struct stat info{};
        stat(path.c_str(), &info);
        int size = info.st_size;
        char buff[size];
        memset(buff, 0, size);
        FCGI_fread(buff, 1, size, p_in_file);
        FCGI_fclose(p_in_file);

        FCGX_FPrintF(request.out, "Content-Length: %ld;\r\n", info.st_size);
        FCGX_PutStr(buff, size, request.out);
    }
}