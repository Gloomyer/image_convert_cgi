//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include <iostream>
#include <fcgi_stdio.h>

void controller_error_handler(FCGX_Request &request, int code, const char *msg) {
    FCGI_printf("Status: 500;\r\n", request.out);
    FCGI_printf(CONTENT_TYPE_JSON, request.out);
    FCGI_printf(R"("code":%d,"message":"%s")", code, msg, request.out);
}