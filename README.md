## 简介
这是一个利用 fcgi 搭配nginx使用的组件

利用 nginx 转发请求给fcgi 提供给当前项目 来实现读取图片的信息和转码/裁图功能

所以运行当前项目需要机器安装了

- ffmpeg
- openssl

## 已经实现的功能点

- 纯文件转发
- 图片文件信息读取(基础:宽 高 格式 大小)
- 图片文件详细信息读取(在基础信息的基础上增加了 详细:图片亮度(0-255),图片md5值)

## 准备实现的功能点
- 图片转码
- 图片缩放
- 图片裁切

## 返回样例

基础信息请求样例:http://???.???/111.png?type=basicInfo
```json
{
    ret: {
        errCode: 0,
        errName: "success",
        msg: "success"
    },
    data: {
        fileSize: 27566,
        width: 1610,
        height: 816,
        mineType: "png"
    }
}
```

详细信息请求样例:http://???.???/111.png?type=detialInfo
```json
{
    ret: {
        errCode: 0,
        errName: "success",
        msg: "success"
    },
    data: {
        fileSize: 27566,
        width: 1610,
        height: 816,
        light: 255,
        mineType: "png",
        md5: "9000ce9c9930f78e4fb4ff7c11a10f5f"
    }
}
```