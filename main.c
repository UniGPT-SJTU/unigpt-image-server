#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

#include "socket.h"
#include "config.h"
#include "core.h"

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    assert(argc == 2);

    listenfd = open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        getnameinfo(
            (struct sockaddr *)&clientaddr, 
            clientlen, 
            hostname, 
            MAXLINE, 
            port, 
            MAXLINE, 
            0
        );
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }
    
}