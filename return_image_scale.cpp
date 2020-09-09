#include "src/Apis.h"
#include "src/common/utils.h"
#include <iostream>
#include <fcgi_stdio.h>
#include <Magick++/Image.h>
#include <map>

void return_image_handler(const char *uri) {
    auto start = std::chrono::system_clock::now();

    std::string path(FILE_PREFIX);
    path.append(uri);


    Magick::Image image;
    try {
        image.read(path);
    } catch (...) {
        //图片不存在
        return;
    }
    std::string url(uri);
    std::vector<std::string> *elements = str_spilt(uri, "?");
    if (elements->size() > 1) {
        std::string params = elements->at(1);

        int target_w = 0, target_h = 0;

        image.scale(Magick::Geometry(384, 216));
        Magick::Blob blob;
        image.write(&blob);

        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        printf("params: %s;\r\n", params.c_str());
        printf("target_w: %d;\r\n", target_w);
        printf("target_h: %d;\r\n", target_h);
        printf("Time-Consuming: %lld;\r\n", duration.count());
        printf("Image-Width: %d;\r\n", image.columns());
        printf("Image-Height: %d;\r\n", image.rows());
        printf("Content-type: image/%s;\r\n\r\n", get_suffix(path));
        fwrite((void *) blob.data(), blob.length(), 1, stdout);
    }
    delete elements;

//    elements = str_spilt(params, "&");
//    std::map<std::string, std::string> paramsMap;
//    for (int i = 0; i < elements.size(); ++i) {
//        elements = str_spilt(elements[i], "=");
//        if (elements.size() >= 2)paramsMap[elements[0]] = elements[1];
//    }


//    try {
//        std::string target_w_str = paramsMap["w"];
//        std::string target_h_str = paramsMap["h"];
//        if (target_w_str.length() > 0) {
//            target_w = std::stoi(target_w_str);
//        }
//
//        if (target_w_str.length() > 0) {
//            target_h = std::stoi(target_h_str);
//        }
//    } catch (...) {
//    }


}