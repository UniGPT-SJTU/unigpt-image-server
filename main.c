#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

#include "socket.h"
#include "config.h"
#include "core.h"
#include "logger.h"

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    init_server_config(&server_config, SERVER_PROTOCOL, SERVER_IP, argv[1]);
    listenfd = open_listenfd_w(server_config.port);

    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if(connfd < 0) {
            LOG_ERROR("Error accepting connection");
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
        LOG_INFO("Accepted connection from (%s, %s)", hostname, port);
        handle_http_request(connfd);
        close(connfd);
    }
    
}