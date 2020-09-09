//
// Created by Gloomy Pan on 2020/9/4.
//
#include <iostream>

#ifndef CONVERT_CGI_APIS_H
#define CONVERT_CGI_APIS_H

#define CONTENT_TYPE_JSON "Content-type: application/json;\r\n\r\n"
#define CONTENT_TYPE_IMAGE "Content-type: image/%s;\r\n\r\n"

static const char *FILE_PREFIX = "/Users/gloomypan/www";

int main(int, char **);

void controller_test_handler();

void controller_ret_file_handler(std::string &);

void controller_image_handler(std::string &, std::string &);

void controller_error_handler(int, const char *);


#endif //CONVERT_CGI_APIS_H
