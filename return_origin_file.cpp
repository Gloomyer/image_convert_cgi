#include "methods.h"
#include <iostream>
#include <fcgi_stdio.h>
#include <sys/stat.h>

const char *get_suffix(char *uri) {
    std::string url(uri);
    int idx = url.find_last_of(".");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
    return url.substr(idx + 1).c_str();
#pragma clang diagnostic pop
}

void return_origin_file(char *uri) {
    std::string path(FILE_PREFIX);
    path.append(uri);
    FILE *p_in_file = fopen(path.c_str(), "rb+");
    if (p_in_file == nullptr) {
        printf("Content-type: application/json;\r\n\r\n");
        printf(R"({"error":"fail","path":"%s", "suffix":"%s"})", path.c_str(), get_suffix(uri));
    } else {
        struct stat info{};
        stat(path.c_str(), &info);
        long size = info.st_size;
        char buff[size];
        memset(buff, 0, size);
        int st = fread(buff, 1, size, p_in_file);
        fclose(p_in_file);

        printf("Content-type: image/%s;\r\n\r\n", get_suffix(uri));
        fwrite(buff, size, 1, stdout);
    }
}