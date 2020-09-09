#include "../Apis.h"
#include <fcgi_stdio.h>

void controller_test_handler(FCGX_Request &request) {
    FCGX_FPrintF(request.out, CONTENT_TYPE_JSON);
    FCGX_FPrintF(request.out, R"(
{"software":"%s",
"path":"%s",
"method":"%s",
"params":"%s",
"realPath":"%s",
"protocol":"%s",
"requestIP":"%s",
"requestPort":"%s"})",
                 FCGX_GetParam("SERVER_SOFTWARE", request.envp),
                 FCGX_GetParam("REQUEST_URI", request.envp),
                 FCGX_GetParam("REQUEST_METHOD", request.envp),
                 FCGX_GetParam("QUERY_STRING", request.envp),
                 FCGX_GetParam("DOCUMENT_URI", request.envp),
                 FCGX_GetParam("SERVER_PROTOCOL", request.envp),
                 FCGX_GetParam("REMOTE_ADDR", request.envp),
                 FCGX_GetParam("REMOTE_PORT", request.envp));
}