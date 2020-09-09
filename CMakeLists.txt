cmake_minimum_required(VERSION 3.17)
project(convert_cgi)
set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/Cellar/fcgi/2.4.2/include)
include_directories(/usr/local/Cellar/ffmpeg/4.3.1/include)
include_directories(/usr/local/opt/openssl@1.1/include)
include_directories(/usr/local/Cellar/imagemagick/7.0.10-28/include/ImageMagick-7)

add_library(libfcgi SHARED IMPORTED)
set_target_properties(libfcgi PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/fcgi/2.4.2/lib/libfcgi.dylib)
add_library(libfcgi_pp SHARED IMPORTED)
set_target_properties(libfcgi_pp PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/fcgi/2.4.2/lib/libfcgi++.dylib)


add_library(avcodec SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavcodec.58.dylib)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavformat.58.dylib)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavutil.56.dylib)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libswresample.3.dylib)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libswscale.5.dylib)

add_library(openssl-crypto SHARED IMPORTED)
add_library(openssl SHARED IMPORTED)
set_target_properties(openssl-crypto PROPERTIES IMPORTED_LOCATION /usr/local/opt/openssl@1.1/lib/libcrypto.dylib)
set_target_properties(openssl PROPERTIES IMPORTED_LOCATION /usr/local/opt/openssl@1.1/lib/libssl.dylib)

add_library(imagemagick-core SHARED IMPORTED)
set_target_properties(imagemagick-core PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/imagemagick/7.0.10-28/lib/libMagickCore-7.Q16HDRI.dylib)
add_library(imagemagick-wand SHARED IMPORTED)
set_target_properties(imagemagick-wand PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/imagemagick/7.0.10-28/lib/libMagickWand-7.Q16HDRI.dylib)
add_library(imagemagick-pp SHARED IMPORTED)
set_target_properties(imagemagick-pp PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/imagemagick/7.0.10-28/lib/libMagick++-7.Q16HDRI.dylib)

FILE(GLOB SRC_MAIN "./src/*.cpp" "./src/*.h")
FILE(GLOB SRC_SOURCE "./src/*/*.cpp" "./src/*/*.h")

add_executable(convert_cgi ${SRC_SOURCE} ${SRC_MAIN})


target_link_libraries(${PROJECT_NAME}
        libfcgi libfcgi_pp
        avcodec avformat avutil swresample swscale
        openssl-crypto openssl
        imagemagick-core imagemagick-wand imagemagick-pp)