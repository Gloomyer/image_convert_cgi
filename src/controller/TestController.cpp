#include "../Apis.h"
#include <fcgi_stdio.h>
#include <cstdlib>

void controller_test_handler() {
    printf(CONTENT_TYPE_JSON);
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