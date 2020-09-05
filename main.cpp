#include "methods.h"
#include "utils.hpp"
#include <iostream>
#include <fcgi_stdio.h>
#include <Magick++.h>

using namespace std;

int main(int argc,char **argv) {
    Magick::InitializeMagick(*argv);

    while (FCGI_Accept() >= 0) {
        try {
            char *uri = getenv("DOCUMENT_URI");
            if (str_has(uri, "fingo_test.test") > 0) {
                return_test_info();
            } else {
                char *query_str = getenv("QUERY_STRING");
                if (strlen(query_str) == 0) {
                    return_origin_file(uri);
                } else if (str_has(query_str, "type=basicInfo") > 0) {
                    return_base_info(uri);
                } else if (str_has(query_str, "type=detailInfo") > 0) {
                    return_detail_info(uri);
                } else if (str_has(query_str, "type=scale") > 0) {
                    return_image_scale(uri);
                } else {
                    //输出错误信息
                    //return_error_info("不正确的请求参数.");
                }
            }
        } catch (...) {
            //return_error_info("产生了崩溃");
        }
    }

    return 0;
}
