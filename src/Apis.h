//
// Created by Gloomy Pan on 2020/9/4.
//
#include <iostream>
#include <fcgi_stdio.h>

#ifndef CONVERT_CGI_APIS_H
#define CONVERT_CGI_APIS_H

#define CONTENT_TYPE_JSON "Content-type: application/json;\r\n\r\n"
#define CONTENT_TYPE_IMAGE "Content-type: image/%s;\r\n\r\n"

static const char *FILE_PREFIX = "/Users/gloomypan/www";

int main(int, char **);

void controller_test_handler(FCGX_Request &);

void controller_ret_file_handler(FCGX_Request &, std::string &);

void controller_image_handler(FCGX_Request &,std::string &, std::string &);

void controller_error_handler(FCGX_Request &, int, const char *);


#endif //CONVERT_CGI_APIS_H
