//
// Created by Gloomy Pan on 2020/9/9.
//

#include "../Apis.h"
#include <iostream>
#include <fcgi_stdio.h>

void controller_error_handler(int code, const char *msg) {
    printf("Status: 500;\r\n");
    printf(CONTENT_TYPE_JSON);
    printf(R"("code":%d,"message":"%s")", code, msg);
}