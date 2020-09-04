cmake_minimum_required(VERSION 3.17)
project(convert_cgi)
set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/Cellar/fcgi/2.4.2/include)
include_directories(/usr/local/Cellar/ffmpeg/4.3.1/include)

add_library(libfcgi SHARED IMPORTED)
set_target_properties(libfcgi PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/fcgi/2.4.2/lib/libfcgi.0.dylib)

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

add_executable(convert_cgi utils.hpp methods.h return_test_html.cpp return_error_info.cpp
        return_origin_file.cpp return_image_info.cpp
        main.cpp)

target_link_libraries(${PROJECT_NAME} libfcgi avcodec avformat avutil swresample swscale)