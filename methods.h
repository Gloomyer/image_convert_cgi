//
// Created by Gloomy Pan on 2020/9/4.
//

#ifndef CONVERT_CGI_METHODS_H
#define CONVERT_CGI_METHODS_H


static const char* FILE_PREFIX = "/Users/gloomypan/www";
int main();
void return_test_info();
void return_origin_file(const char *uri);
void return_base_info(const char* uri);
void return_detail_info(const char* uri);
void return_error_info(const char *msg);

#endif //CONVERT_CGI_METHODS_H
