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
    init_server_config(&server_config, SERVER_PROTOCOL, SERVER_IP, argv[1]);
    listenfd = open_listenfd(server_config.port);
    if(listenfd < 0) {
        printf("Port %s is already in use\n", server_config.port);
        return 1;
    }

    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if(connfd < 0) {
            printf("Error accepting connection\n");
            continue;
        }
        
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