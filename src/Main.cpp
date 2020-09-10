#include "Apis.h"
#include "common/utils.h"
#include <fcgi_stdio.h>
#include <Magick++.h>
#include <thread>
#include <zconf.h>

using namespace std;

FCGX_Request request_list[1000];

static void *handler_thread(void *args) {
    long thread_serial_number = (long) args;
    int rc;
    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    for (;;) {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
            break;

        try {
            FCGX_FPrintF(request.out, "Thread-Serial-Number: %d \r\n", thread_serial_number);
            FCGX_FPrintF(request.out, "Process-Id: %ld \r\n", getpid());
            std::string document_uri(FCGX_GetParam("DOCUMENT_URI", request.envp));
            std::string query_str(FCGX_GetParam("QUERY_STRING", request.envp));
            if (document_uri.find("fingo_test.test") < document_uri.length()) {
                controller_test_handler(request);
            } else if (query_str.length() == 0) {
                controller_ret_file_handler(request, document_uri);
            } else if (query_str.find("type=basicInfo") < document_uri.length()) {
                controller_ret_image_basic_info(request, document_uri);
            } else if (query_str.find("type=detailInfo") < document_uri.length()) {
                controller_ret_image_detail_info(request, document_uri);
            } else if (query_str.find("type=") < document_uri.length()) {
                controller_image_handler(request, document_uri, query_str);
            } else {
                controller_error_handler(request, -2, "错误的提交参数");
            }
        } catch (...) {
            try {
                controller_error_handler(request, -1, "产生了崩溃");
            } catch (...) {
            }
        }

        try{
            FCGX_Finish_r(&request);
        } catch (...) {
        }
    }
    return nullptr;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

int main(int argc, char **argv) {
    int hardware_thread_count = thread::hardware_concurrency();

    Magick::InitializeMagick(*argv);

    pthread_t thread_id_array[hardware_thread_count];
    FCGX_Init();

    for (long i = 1; i < hardware_thread_count; i++)
        pthread_create(&thread_id_array[i], nullptr, handler_thread, (void *) (i + 1));

    handler_thread((void *) 1);

    return 0;
}

#pragma clang diagnostic pop
