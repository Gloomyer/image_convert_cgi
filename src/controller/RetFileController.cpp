//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include "../common/utils.h"
#include <iostream>
#include <sys/stat.h>
#include <fcgi_stdio.h>
#include <zconf.h>
#include <Magick++/Image.h>


void controller_ret_file_handler(FCGX_Request &request, std::string &uri) {
    std::string path(FILE_PREFIX);
    path.append(uri);

    if (access(path.c_str(), F_OK) == 0) {
        Magick::Image image;
        auto start = std::chrono::system_clock::now();
        try{
            image.read(path);
        } catch (...) {
            controller_error_handler(request, -3, "文件打开失败");
            return;
        }
        Magick::Blob blob;
        image.write(&blob);

        auto end = std::chrono::system_clock::now();
        auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
        FCGX_FPrintF(request.out, "Time-Consuming: %lld;\r\n", duration);
        FCGX_FPrintF(request.out, CONTENT_TYPE_IMAGE, image.magick().c_str());
        FCGX_PutStr((char *) blob.data(), blob.length(), request.out);
    } else {
        controller_error_handler(request, -3, "文件不存在");
    }
}