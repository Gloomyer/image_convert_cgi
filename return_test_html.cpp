#include "methods.h"
#include <iostream>
#include <fcgi_stdio.h>

void return_test_info() {
    printf("Content-type: application/json;\r\n\r\n");
    printf(R"(
{"software":"%s",
"path":"%s",
"method":"%s",
"params":"%s",
"realPath":"%s",
"protocol":"%s",
"requestIP":"%s",
"requestPort":"%s"})",
           getenv("SERVER_SOFTWARE"),
           getenv("REQUEST_URI"),
           getenv("REQUEST_METHOD"),
           getenv("QUERY_STRING"),
           getenv("DOCUMENT_URI"),
           getenv("SERVER_PROTOCOL"),
           getenv("REMOTE_ADDR"),
           getenv("REMOTE_PORT"));
}