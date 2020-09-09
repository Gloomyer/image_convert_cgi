#include "Apis.h"
#include "common/utils.h"
#include <iostream>
#include <fcgi_stdio.h>
#include <Magick++.h>

using namespace std;

int main(int argc, char **argv) {
    Magick::InitializeMagick(*argv);
    while (FCGI_Accept() >= 0) {
        try {
            std::string document_uri(getenv("DOCUMENT_URI"));
            std::string query_str(getenv("QUERY_STRING"));
            if (document_uri.find("fingo_test.test") < document_uri.length()) {
                controller_test_handler();
            } else if (query_str.length() == 0) {
                controller_ret_file_handler(document_uri);
            } else if (query_str.find("type=") < document_uri.length()) {
                controller_image_handler(document_uri, query_str);
            } else {
                controller_error_handler(-2, "错误的提交参数");
            }

//            else {
//                char *query_str = getenv("QUERY_STRING");
//                if (strlen(query_str) == 0) {
//                    return_origin_file(uri);
//                } else if (str_has(query_str, "type=basicInfo") > 0) {
//                    return_base_info(uri);
//                } else if (str_has(query_str, "type=detailInfo") > 0) {
//                    return_detail_info(uri);
//                } else if (str_has(query_str, "type") > 0) {
//                    return_image_handler(uri);
//                } else {
//                    //输出错误信息
//                    //return_error_info("不正确的请求参数.");
//                }
//            }

        } catch (...) {
            controller_error_handler(-1, "产生了崩溃");
        }
    }

    return 0;
}
