## 简介
这是一个利用 fcgi 搭配nginx使用的组件

利用 nginx 转发请求给fcgi 提供给当前项目 来实现读取图片的信息和转码/裁图功能

所以运行当前项目需要机器安装了

- ffmpeg
- imagemagick
- openssl

## 项目配置
打开CMakeLists.txt

找到
```cmake
MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")
IF (WIN32)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is APPLE")
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
ENDIF ()
MESSAGE(STATUS "====================================")
```

APPLE 已经实现，参照 内容 修改WIN32或者UNIX 提供动态库

## 已经实现的功能点

- 纯文件转发
- 图片文件信息读取(基础:宽 高 格式 大小)
- 图片文件详细信息读取(在基础信息的基础上增加了 详细:图片亮度(0-255),图片md5值)

## 准备实现的功能点
- 图片转码
- 图片缩放
- 图片裁切

## 请求demo

基础信息请求样例:http://url/111.png?type=basicInfo
```json5
{
    fileSize: 27566,
    width: 1610,
    height: 816,
    mineType: "png"
}
```

详细信息请求样例:http://url/111.png?type=detialInfo
```json5
{
    fileSize: 27566,
    width: 1610,
    height: 816,
    light: 255,
    mineType: "png",
    md5: "9000ce9c9930f78e4fb4ff7c11a10f5f"
}
```

图片缩放请求样例:http://url/111.png?type=shrink&w=整数&h=整数
> 备注: w/h只给1个即可 如果都给了 会计算两次 先宽 再高 然后缩放

图片缩放请求样例:http://url/111.png?type=crop&w=整数&h=整数
> 备注: w/h都必须得给 如果大于原图将 按照真实图片比例缩放 w/h

图片降低质量样例:http://url/111.png?type=quality&q=0-100
> 备注: q必须指定 取值1=100 不然报错

还可以组合请求 如
图片缩放请求样例:http://url/111.png?type=shrink,crop,quality&w=整数&h=整数&q=75

处理逻辑按顺序处理


