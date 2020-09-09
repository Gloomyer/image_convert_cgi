//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include "../common/utils.h"
#include <iostream>
#include <sys/stat.h>
#include <fcgi_stdio.h>

void controller_ret_file_handler(std::string &uri) {
    std::string path(FILE_PREFIX);
    path.append(uri);
    FILE *p_in_file = fopen(path.c_str(), "rb+");
    if (p_in_file == nullptr) {
        controller_error_handler(-3, "文件不存在");
    } else {
        struct stat info{};
        stat(path.c_str(), &info);
        long size = info.st_size;
        char buff[size];
        memset(buff, 0, size);
        fread(buff, 1, size, p_in_file);
        fclose(p_in_file);
        printf("Content-type: image/%s;\r\n\r\n", get_suffix(path));
        fwrite(buff, size, 1, stdout);
    }
}