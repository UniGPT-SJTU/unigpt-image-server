#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

#include "socket.h"
#include "config.h"
#include "core.h"
#include "logger.h"

extern server_config_t server_config;
int listenfd;

void cleanup_and_exit(int signo) {
    LOG_INFO("Received signal %d, cleaning up...", signo);
    close(listenfd);
    free_server_config(&server_config);
    exit(0);
}

int main(int argc, char **argv) {
    int connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    signal(SIGINT, cleanup_and_exit);

    if(init_server_config(&server_config, argc, argv) < 0) {
        usage(argv[0]);
        free_server_config(&server_config);
        return 1;
    }
    listenfd = open_listenfd(server_config.port);
    LOG_INFO("Tiny server launched");

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

    // CTRL NEVER REACHED HERE!
    
}