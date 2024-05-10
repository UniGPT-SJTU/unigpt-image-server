#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "socket.h"

int open_listenfd(char *port) {

    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for(p = listp; p; p = p->ai_next) {
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            // Socket 连接失败，尝试下一个
            continue;
        }

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int));

        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            // 绑定成功
            break;
        }
        close(listenfd);    // 绑定失败，关闭连接
    }

    freeaddrinfo(listp);
    if(!p) {
        return -1;
    }

    if(listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
        return -1;
    }

    return listenfd;
}