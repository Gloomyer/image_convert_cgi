//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include <fcgi_stdio.h>

void controller_error_handler(FCGX_Request &request, int code, const char *msg) {
    FCGX_FPrintF(request.out, "Status: 500;\r\n");
    FCGX_FPrintF(request.out, CONTENT_TYPE_JSON);
    FCGX_FPrintF(request.out, R"("code":%d,"message":"%s")", code, msg);
}