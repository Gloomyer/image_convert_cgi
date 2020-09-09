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
void shrink_image(Magick::Image &image, int dst_w, int dst_h) {
    int src_w = image.columns();
    int src_h = image.rows();

    if (dst_w > src_w || dst_h > src_h) {
        //只缩，不放大
        return;
    }

    //优先使用 w 缩放图片,
    if (dst_w > 0) {
        int scale_w = dst_w;
        double scale = dst_w * 1.0 / src_w;
        int scale_h = (int) (src_h * scale);
        image.scale(Magick::Geometry(scale_w, scale_h));
    } else {
        int scale_h = dst_h;
        double scale = dst_h * 1.0 / src_h;
        int scale_w = (int) (src_w * scale);
        image.scale(Magick::Geometry(scale_w, scale_h));
    }
}

/**
 * 裁切图片
 * @param image image
 * @param dst_w  目标w
 * @param dst_h  目标y
 * @param src_w  原宽
 * @param src_h  原高
 */
void crop_image(Magick::Image &image, int dst_w, int dst_h) {
    int src_w = image.columns();
    int src_h = image.rows();
    if (dst_w > src_w || dst_h > src_h) {
        //如果要裁的的尺寸比原图大,重新计算要裁切的尺寸
        if (dst_w > src_w) {
            dst_w = src_w;
            double scale = src_w * 1.0 / dst_w;
            dst_h = (int) (scale * dst_h);
            image.crop(Magick::Geometry(dst_w, dst_h,
                                        src_w / 2 - dst_w / 2,
                                        src_w / 2 - dst_w / 2));
        } else {

        }
    }
}

void controller_image_handler(std::string &uri, std::string &query_str) {
    auto start = std::chrono::system_clock::now();

    std::string path(FILE_PREFIX);
    path.append(uri);

    std::vector<std::string> entities = str_spilt(query_str, "&");
    std::vector<std::string> methods;
    int target_w = -1, target_h = -1;
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
            }
        }
    }

    if ((target_w == -1 && target_h == -1) || methods.empty()) {
        controller_error_handler(-4, "缺少处理参数");
        return;
    }

    Magick::Image image;
    int origin_w, origin_h;
    try {
        image.read(path);
        origin_w = image.columns();
        origin_h = image.rows();
    } catch (...) {
        controller_error_handler(-5, "文件不存在");
        return;
    }


    for (auto &method :methods) {
        if (method == "shrink") {
            shrink_image(image, target_w, target_h);
        } else if (method == "crop") {
            crop_image(image, target_w, target_h);
        }
    }

    Magick::Blob blob;
    image.write(&blob);
    auto end = std::chrono::system_clock::now();
    auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
    printf("Image-Size: %dx%d;%dx%d;\r\n", origin_w, origin_h,
           image.columns(), image.rows());
    printf("Time-Consuming: %lld;\r\n", duration);
    printf(CONTENT_TYPE_IMAGE, get_suffix(path));
    fwrite((void *) blob.data(), blob.length(), 1, stdout);
}