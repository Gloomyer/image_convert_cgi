#include "../Apis.h"
#include "../common/utils.h"
#include <iostream>
#include <fcgi_stdio.h>
#include <Magick++/Image.h>
#include <map>

/**
 * 图片缩放
 * @param image image
 * @param dst_w  目标w
 * @param dst_h  目标y
 * @param src_w  原宽
 * @param src_h  原高
 */
int shrink_image(Magick::Image &image, int dst_w, int dst_h) {
    int src_w = image.columns();
    int src_h = image.rows();

    if (dst_w > src_w || dst_h > src_h) {
        //只缩，不放大
        return 0;
    }

    int scale_w = src_w, scale_h = src_h;
    if (dst_w > 0) { //根据宽度计算缩放之后的宽高
        scale_w = dst_w;
        double scale = dst_w * 1.0 / src_w;
        scale_h = (int) (src_h * scale);
    }

    if (dst_h > 0) { //根据高度计算缩放之后的宽高
        scale_h = dst_h;
        double scale = dst_h * 1.0 / src_h;
        scale_w = (int) (src_w * scale);
    }

    image.scale(Magick::Geometry(scale_w, scale_h));
    return 0;
}

/**
 * 裁切图片
 * @param image image
 * @param dst_w  目标w
 * @param dst_h  目标y
 * @param src_w  原宽
 * @param src_h  原高
 */
int crop_image(Magick::Image &image, int dst_w, int dst_h) {
    if (dst_w == -1 || dst_h == -1) return -7;

    int src_w = image.columns();
    int src_h = image.rows();

    //如果要裁的的尺寸比原图大,重新计算要裁切的尺寸
    if (dst_w > src_w) {//宽比原图大
        dst_w = src_w;
        double scale = src_w * 1.0 / dst_w;
        dst_h = (int) (scale * dst_h);
    }

    if (dst_h > src_h) {//高比原图大
        dst_h = src_h;
        double scale = src_h * 1.0 / dst_h;
        dst_w = (int) (scale * dst_w);
    }

    image.crop(Magick::Geometry(dst_w, dst_h,
                                src_w / 2 - dst_w / 2,
                                src_h / 2 - dst_h / 2));
    return 0;
}

int quality_image(Magick::Image &image, int q) {
    if (q < 100 && q > 0) {
        image.quality(q);
    }
    return 0;
}

void controller_image_handler(FCGX_Request &request, std::string &uri, std::string &query_str) {
    auto start = std::chrono::system_clock::now();

    std::string path(FILE_PREFIX);
    path.append(uri);

    std::vector<std::string> entities = str_spilt(query_str, "&");
    std::vector<std::string> methods;
    int target_w = -1, target_h = -1, q = 100;
    for (auto &entity : entities) {
        std::vector<std::string> map = str_spilt(entity, "=");
        //如果不是2 就是无效参数
        if (map.size() >= 2) {
            if (map[0] == "type") {
                methods = str_spilt(map[1], ",");
            } else if ("w" == map[0]) {
                target_w = std::stoi(map[1]);
            } else if ("h" == map[0]) {
                target_h = std::stoi(map[1]);
            } else if ("q" == map[0]) {
                q = std::stoi(map[1]);
            }
        }
    }

    if ((target_w == -1 && target_h == -1) || methods.empty()) {
        controller_error_handler(request, -4, "缺少处理参数");
        return;
    }

    Magick::Image image;
    int origin_w, origin_h;
    try {
        image.read(path);
        origin_w = image.columns();
        origin_h = image.rows();
    } catch (...) {
        controller_error_handler(request, -5, "文件不存在");
        return;
    }

    int ret = -6;
    for (auto &method :methods) {
        if (method == "shrink") {
            ret = shrink_image(image, target_w, target_h);
        } else if (method == "crop") {
            ret = crop_image(image, target_w, target_h);
        } else if (method == "quality") {
            ret = quality_image(image, q);
        }

        if (ret != 0)break;
    }

    if (ret != 0) {
        controller_error_handler(request, ret, "缺少处理参数");
        return;
    }

    Magick::Blob blob;
    image.write(&blob);
    auto end = std::chrono::system_clock::now();
    auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
    FCGX_FPrintF(request.out, "Image-Size: %dx%d;%dx%d;\r\n",
                 origin_w, origin_h,
                 image.columns(), image.rows());
    FCGX_FPrintF(request.out, "Time-Consuming: %lld;\r\n", duration);
    FCGX_FPrintF(request.out, CONTENT_TYPE_IMAGE, image.magick().c_str());
    FCGX_PutStr((char *) blob.data(), blob.length(), request.out);
}