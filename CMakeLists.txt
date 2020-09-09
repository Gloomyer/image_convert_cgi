cmake_minimum_required(VERSION 3.17.3)
project(convert_cgi)
set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/Cellar/fcgi/2.4.2/include)
include_directories(/usr/local/Cellar/ffmpeg/4.3.1/include)
include_directories(/usr/local/opt/openssl@1.1/include)
include_directories(/usr/local/Cellar/imagemagick/7.0.10-28/include/ImageMagick-7)


MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")
IF (WIN32)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is APPLE")
    add_library(f_cgi SHARED IMPORTED)
    set_target_properties(f_cgi PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/fcgi/2.4.2/lib/libfcgi.dylib)
    add_library(f_cgi_pp SHARED IMPORTED)
    set_target_properties(f_cgi_pp PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/fcgi/2.4.2/lib/libfcgi++.dylib)

    add_library(avcodec SHARED IMPORTED)
    set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavcodec.58.dylib)
    add_library(avformat SHARED IMPORTED)
    set_target_properties(avformat PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavformat.58.dylib)
    add_library(avutil SHARED IMPORTED)
    set_target_properties(avutil PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libavutil.56.dylib)
    add_library(swr_e_sample SHARED IMPORTED)
    set_target_properties(swr_e_sample PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/ffmpeg/4.3.1/lib/libswresample.3.dylib)
    add_library(swscale SHARED IMPORTED)
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
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
ENDIF ()
MESSAGE(STATUS "====================================")


FILE(GLOB SRC_MAIN "./src/*.cpp" "./src/*.h")
FILE(GLOB SRC_SOURCE "./src/*/*.cpp" "./src/*/*.h")

add_executable(convert_cgi ${SRC_SOURCE} ${SRC_MAIN})

target_link_libraries(${PROJECT_NAME}
        f_cgi f_cgi_pp
        avcodec avformat avutil swr_e_sample swscale
        openssl-crypto openssl
        imagemagick-core imagemagick-wand imagemagick-pp)
