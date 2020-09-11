#include <netinet/in.h>
#include <zconf.h>
#include <iostream>
#include <fcntl.h>
#include <sys/event.h>
//#include "Apis.h"

#define HEAD_LEN                8  //消息头长度固定为8个字节
#define BUFLEN                  4096
#define FCGI_VERSION_1           1  //版本号


// 消息类型
enum fcgi_request_type {
    FCGI_BEGIN_REQUEST = 1,
    FCGI_ABORT_REQUEST = 2,
    FCGI_END_REQUEST = 3,
    FCGI_PARAMS = 4,
    FCGI_STDIN = 5,
    FCGI_STDOUT = 6,
    FCGI_STDERR = 7,
    FCGI_DATA = 8,
    FCGI_GET_VALUES = 9,
    FCGI_GET_VALUES_RESULT = 10,
    FCGI_UNKOWN_TYPE = 11
};

// 服务器希望fastcgi程序充当的角色, 这里只讨论 FCGI_RESPONDER 响应器角色
enum fcgi_role {
    FCGI_RESPONDER = 1,
    FCGI_AUTHORIZER = 2,
    FCGI_FILTER = 3
};

//消息头
struct fcgi_header {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
};

//请求开始发送的消息体
struct FCGI_BeginRequestBody {
    unsigned char roleB1;
    unsigned char roleB0;
    unsigned char flags;
    unsigned char reserved[5];
};

//请求结束发送的消息体
struct FCGI_EndRequestBody {
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
};

// protocolStatus
enum protocolStatus {
    FCGI_REQUEST_COMPLETE = 0,
    FCGI_CANT_MPX_CONN = 1,
    FCGI_OVERLOADED = 2,
    FCGI_UNKNOWN_ROLE = 3
};


// 打印错误并退出
void halt_error(const char *type, int err_code) {
    fprintf(stderr, "%s: %s\n", type, strerror(err_code));
    exit(EXIT_FAILURE);
}

// 存储键值对的结构体
struct paramNameValue {
    char **pname;
    char **pvalue;
    int maxLen;
    int curLen;
};


// 初始化一个键值结构体
void init_paramNV(struct paramNameValue *nv) {
    nv->maxLen = 16;
    nv->curLen = 0;
    nv->pname = (char **) malloc(nv->maxLen * sizeof(char *));
    nv->pvalue = (char **) malloc(nv->maxLen * sizeof(char *));
}

// 扩充一个结键值构体的容量为之前的两倍
void extend_paramNV(struct paramNameValue *nv) {
    nv->maxLen *= 2;
    nv->pname = static_cast<char **>(realloc(nv->pname, nv->maxLen * sizeof(char *)));
    nv->pvalue = static_cast<char **>(realloc(nv->pvalue, nv->maxLen * sizeof(char *)));
}

// 释放一个键值结构体
void free_paramNV(struct paramNameValue *nv) {
    int i;

    for (i = 0; i < nv->curLen; i++) {
        free(nv->pname[i]);
        free(nv->pvalue[i]);
    }
    free(nv->pname);
    free(nv->pvalue);
}


// 获取指定 paramName 的值
char *getParamValue(struct paramNameValue *nv, char *paramName) {

    int i;

    for (i = 0; i < nv->curLen; i++) {
        if (strncmp(paramName, nv->pname[i], strlen(paramName)) == 0) {
            return nv->pvalue[i];
        }
    }
    return NULL;
}


void listen_loop(int socket_server_fd) {
    int epoll_fd, i, ret;
    int timeout = 300;
    int kq = kqueue();
    struct kevent event{};
    EV_SET(&event, socket_server_fd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    assert(-1 != kevent(kq, &event, 1, NULL, 0, NULL));

    int junk = open("some.big.file", O_RDONLY);
    uint64_t bytes_written = 0;

    struct kevent evList[32];

    while (1) {
        // returns number of events
        int nev = kevent(kq, NULL, 0, evList, 32, NULL);
//        printf("kqueue got %d events\n", nev);

        for (int i = 0; i < nev; i++) {
            int fd = (int) evList[i].ident;
            std::cout << "fd: " << fd << std::endl;
            if (evList[i].flags & EV_EOF) {
                printf("Disconnect\n");
                close(fd);
                // Socket is automatically removed from the kq by the kernel.
            } else if (fd == socket_server_fd) {
                struct sockaddr_storage addr;
                socklen_t socklen = sizeof(addr);
                int connfd = accept(fd, (struct sockaddr *) &addr, &socklen);
                assert(connfd != -1);

                // Listen on the new socket
                EV_SET(&event, connfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                kevent(kq, &event, 1, NULL, 0, NULL);
                printf("Got connection!\n");

                int flags = fcntl(connfd, F_GETFL, 0);
                assert(flags >= 0);
                fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

                // schedule to send the file when we can write (first chunk should happen immediately)
                EV_SET(&event, connfd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                kevent(kq, &event, 1, NULL, 0, NULL);

            } else if (evList[i].filter == EVFILT_READ) {
                // Read from socket.
                char buf[1024];
                size_t bytes_read = recv(fd, buf, sizeof(buf), 0);
                printf("read %zu bytes, content:%s\n", bytes_read, buf);
            } else if (evList[i].filter == EVFILT_WRITE) {
//                printf("Ok to write more!\n");
                off_t offset = (off_t) evList[i].udata;
                off_t len = 0;//evList[i].data;
                if (sendfile(junk, fd, offset, &len, NULL, 0) != 0) {
//                    perror("sendfile");
//                    printf("err %d\n", errno);

                    if (errno == EAGAIN) {
                        // schedule to send the rest of the file
                        EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, (void *) (offset + len));
                        kevent(kq, &event, 1, NULL, 0, NULL);
                    }
                }
                bytes_written += len;
                printf("wrote %lld bytes, %lld total\n", len, bytes_written);
            }
        }
    }

}

int main(int argc, char **argv) {
    int socket_server_fd, connfd;
    int ret, i;
    struct sockaddr_in servaddr{}, cliaddr{};
    socklen_t slen, clen;

    struct fcgi_header header{}, headerBuf{};
    struct FCGI_BeginRequestBody brBody{};
    struct paramNameValue paramNV{};
    struct FCGI_EndRequestBody erBody{};

    ssize_t rdlen;
    int requestId, contentLen;
    unsigned char paddingLen;
    int paramNameLen, paramValueLen;

    char buf[BUFLEN];

    unsigned char c;
    unsigned char lenbuf[3];
    char *paramName, *paramValue;

    char *htmlHead, *htmlBody;


    /*socket bind listen*/
    socket_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_fd == -1) {
        halt_error("socket", errno);
    }
    slen = clen = sizeof(struct sockaddr_in);
    bzero(&servaddr, slen);

    int on = 1;
    setsockopt(socket_server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    //这里让 fastcgi程序监听 127.0.0.1:9000  和 php-fpm 监听的地址相同， 方便我们用 nginx 来测试
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(socket_server_fd, (struct sockaddr *) &servaddr, slen);

    if (ret == -1) {
        halt_error("bind", errno);
    }

    ret = listen(socket_server_fd, 16);

    if (ret == -1) {
        halt_error("listen", errno);
    }

    listen_loop(socket_server_fd);

    close(socket_server_fd);
    return 0;
    while (true) {
        bzero(&cliaddr, clen);
        connfd = accept(socket_server_fd, (struct sockaddr *) &cliaddr, &clen);
        std::cout << "accept :" << connfd << std::endl;
        if (connfd == -1) {
            halt_error("accept", errno);
            break;
        }

        fcntl(connfd, F_SETFL, O_NONBLOCK); // 设置socket为非阻塞

        init_paramNV(&paramNV);

        while (1) {

            //读取消息头
            bzero(&header, HEAD_LEN);
            rdlen = read(connfd, &header, HEAD_LEN);

            if (rdlen == -1) {
                // 无数据可读
                if (errno == EAGAIN) {
                    break;
                } else {
                    halt_error("read", errno);
                }
            }

            if (rdlen == 0) {
                break; //消息读取结束
            }

            headerBuf = header;

            requestId = (header.requestIdB1 << 8) + header.requestIdB0;
            contentLen = (header.contentLengthB1 << 8) + header.contentLengthB0;
            paddingLen = header.paddingLength;


            printf("version = %d, type = %d, requestId = %d, contentLen = %d, paddingLength = %d\n",
                   header.version, header.type, requestId, contentLen, paddingLen);

            printf("%lx\n", header);


            switch (header.type) {

                case FCGI_BEGIN_REQUEST:
                    printf("******************************* begin request *******************************\n");

                    //读取开始请求的请求体
                    bzero(&brBody, sizeof(brBody));
                    read(connfd, &brBody, sizeof(brBody));

                    printf("role = %d, flags = %d\n", (brBody.roleB1 << 8) + brBody.roleB0, brBody.flags);

                    break;

                case FCGI_PARAMS:
                    printf("begin read params...\n");

                    // 消息头中的contentLen = 0 表明此类消息已发送完毕
                    if (contentLen == 0) {
                        printf("read params end...\n");
                    }

                    //循环读取键值对
                    while (contentLen > 0) {
                        /*
                        FCGI_PARAMS 以键值对的方式传送，键和值之间没有'=',每个键值对之前会分别用1或4个字节来标识键和值的长度 例如：
                        \x0B\x02SERVER_PORT80\x0B\x0ESERVER_ADDR199.170.183.42
                         上面的长度是用十六进制表示的  \x0B = 11  正好为SERVER_PORT的长度， \x02 = 2 为80的长度
                        */

                        // 获取paramName的长度
                        rdlen = read(connfd, &c, 1);  //先读取一个字节，这个字节标识 paramName 的长度
                        contentLen -= rdlen;

                        if ((c & 0x80) != 0)  //如果 c 的值大于 128，则该 paramName 的长度用四个字节表示
                        {
                            rdlen = read(connfd, lenbuf, 3);
                            contentLen -= rdlen;
                            paramNameLen = ((c & 0x7f) << 24) + (lenbuf[0] << 16) + (lenbuf[1] << 8) + lenbuf[2];
                        } else {
                            paramNameLen = c;
                        }

                        // 同样的方式获取paramValue的长度
                        rdlen = read(connfd, &c, 1);
                        contentLen -= rdlen;
                        if ((c & 0x80) != 0) {
                            rdlen = read(connfd, lenbuf, 3);
                            contentLen -= rdlen;
                            paramValueLen = ((c & 0x7f) << 24) + (lenbuf[0] << 16) + (lenbuf[1] << 8) + lenbuf[2];
                        } else {
                            paramValueLen = c;
                        }

                        //读取paramName
                        paramName = (char *) calloc(paramNameLen + 1, sizeof(char));
                        rdlen = read(connfd, paramName, paramNameLen);
                        contentLen -= rdlen;

                        //读取paramValue
                        paramValue = (char *) calloc(paramValueLen + 1, sizeof(char));
                        rdlen = read(connfd, paramValue, paramValueLen);
                        contentLen -= rdlen;

                        printf("read param: %s=%s\n", paramName, paramValue);

                        if (paramNV.curLen == paramNV.maxLen) {
                            // 如果键值结构体已满则把容量扩充一倍
                            extend_paramNV(&paramNV);
                        }

                        paramNV.pname[paramNV.curLen] = paramName;
                        paramNV.pvalue[paramNV.curLen] = paramValue;
                        paramNV.curLen++;

                    }

                    if (paddingLen > 0) {
                        rdlen = read(connfd, buf, paddingLen);
                        contentLen -= rdlen;
                    }

                    break;

                case FCGI_STDIN:
                    printf("begin read post...\n");

                    if (contentLen == 0) {
                        printf("read post end....\n");
                    }

                    if (contentLen > 0) {
                        while (contentLen > 0) {
                            if (contentLen > BUFLEN) {
                                rdlen = read(connfd, buf, BUFLEN);
                            } else {
                                rdlen = read(connfd, buf, contentLen);
                            }

                            contentLen -= rdlen;
                            fwrite(buf, sizeof(char), rdlen, stdout);
                        }
                        printf("\n");
                    }

                    if (paddingLen > 0) {
                        rdlen = read(connfd, buf, paddingLen);
                        contentLen -= rdlen;
                    }

                    break;

                case FCGI_DATA:
                    printf("begin read data....\n");

                    if (contentLen > 0) {
                        while (contentLen > 0) {
                            if (contentLen > BUFLEN) {
                                rdlen = read(connfd, buf, BUFLEN);
                            } else {
                                rdlen = read(connfd, buf, contentLen);
                            }

                            contentLen -= rdlen;
                            fwrite(buf, sizeof(char), rdlen, stdout);
                        }
                        printf("\n");
                    }

                    if (paddingLen > 0) {
                        rdlen = read(connfd, buf, paddingLen);
                        contentLen -= rdlen;
                    }

                    break;

            }
        }


        /* 以上是从web服务器读取数据，下面向web服务器返回数据 */


        headerBuf.version = FCGI_VERSION_1;
        headerBuf.type = FCGI_STDOUT;

        htmlHead = "Content-type: text/html\r\n\r\n";  //响应头
        htmlBody = getParamValue(&paramNV, "SCRIPT_FILENAME");  // 把请求文件路径作为响应体返回

        printf("html: %s%s\n", htmlHead, htmlBody);

        contentLen = strlen(htmlHead) + strlen(htmlBody);

        headerBuf.contentLengthB1 = (contentLen >> 8) & 0xff;
        headerBuf.contentLengthB0 = contentLen & 0xff;
        headerBuf.paddingLength = (contentLen % 8) > 0 ? 8 - (contentLen % 8) : 0;  // 让数据 8 字节对齐


        write(connfd, &headerBuf, HEAD_LEN);
        write(connfd, htmlHead, strlen(htmlHead));
        write(connfd, htmlBody, strlen(htmlBody));

        if (headerBuf.paddingLength > 0) {
            write(connfd, buf, headerBuf.paddingLength);  //填充数据随便写什么，数据会被服务器忽略
        }

        free_paramNV(&paramNV);

        //回写一个空的 FCGI_STDOUT 表明 该类型消息已发送结束
        headerBuf.type = FCGI_STDOUT;
        headerBuf.contentLengthB1 = 0;
        headerBuf.contentLengthB0 = 0;
        headerBuf.paddingLength = 0;
        write(connfd, &headerBuf, HEAD_LEN);


        // 发送结束请求消息头
        headerBuf.type = FCGI_END_REQUEST;
        headerBuf.contentLengthB1 = 0;
        headerBuf.contentLengthB0 = 8;
        headerBuf.paddingLength = 0;

        bzero(&erBody, sizeof(erBody));
        erBody.protocolStatus = FCGI_REQUEST_COMPLETE;

        write(connfd, &headerBuf, HEAD_LEN);
        write(connfd, &erBody, sizeof(erBody));

        close(connfd);

        printf("******************************* end request *******************************\n");
    }

    close(socket_server_fd);

    return 0;
}