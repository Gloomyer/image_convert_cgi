#include "methods.h"
#include "fcgi_stdio.h"

void return_error_info(const char *msg) {
    printf("Content-type: application/json;\r\n\r\n");
    printf(R"({"ret":{"errCode":-1,"errName":"%s","msg":"%s"},"data":null})",
           msg, msg);
}