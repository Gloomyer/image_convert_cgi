#include "methods.h"
#include <iostream>
#include <fstream>
#include <fcgi_stdio.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <Magick++/Image.h>


const char *get_file_md5(const char *in_file_path) {
    std::ifstream file(in_file_path, std::ifstream::binary);
    if (file) {
        MD5_CTX md5_ctx;
        MD5_Init(&md5_ctx);

        char buff[1024 * 4];
        while (file.good()) {
            file.read(buff, sizeof(buff));
            MD5_Update(&md5_ctx, buff, file.gcount());
        }

        unsigned char md5_result[MD5_DIGEST_LENGTH];
        MD5_Final(md5_result, &md5_ctx);

        char *hex = new char[35];
        memset(hex, 0, 35);
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            sprintf(hex + i * 2, "%02x", md5_result[i]);
        }

        hex[32] = '\0';
        file.close();
        return hex;
    }
    return nullptr;
}

void return_base_info(const char *uri) {
    std::string file_path(FILE_PREFIX);
    file_path.append(uri);
    Magick::Image image;
    try {
        image.read(file_path);
    } catch (...) {
        return;
    }
    printf("Content-type: application/json;\r\n\r\n");
    printf(R"({"ret":{"errCode":0,"errName":"success","msg":"success"},"data":
{
"fileSize": %ld,
"width": %d,
"height": %d,
"imageType": "%s"
}})",
           image.fileSize(), image.rows(), image.columns(), image.magick().c_str());
}

void return_detail_info(const char *uri) {
    std::string file_path(FILE_PREFIX);
    file_path.append(uri);
    Magick::Image image;
    const char *md5;
    try {
        image.read(file_path);
        md5 = get_file_md5(file_path.c_str());
    } catch (...) {
        return;
    }
    printf("Content-type: application/json;\r\n\r\n");
    printf(R"({"ret":{"errCode":0,"errName":"success","msg":"success"},"data":
{
"fileSize": %ld,
"width": %d,
"height": %d,
"light": %d,
"imageType": "%s",
"md5": "%s"
}})",
           image.fileSize(), image.rows(), image.columns(), 0, image.magick().c_str(),md5);
    delete md5;
}