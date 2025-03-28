#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include "socket.h"
#include "config.h"
#include "core.h"
#include "logger.h"

#define MAX_THREAD 8

extern server_config_t server_config;

void *thread_work(void *param) {
    int *connfd = (int *) param;
    handle_http_request(*connfd);
    close(*connfd);

    // free applied heap mem in thread function
    free(connfd);
    return NULL;
}

int main(int argc, char **argv) {
    int listenfd;
    int *connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    init_server_config(&server_config, SERVER_PROTOCOL, SERVER_IP, argv[1]);
    listenfd = open_listenfd(server_config.port);

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = (int *) malloc(sizeof(int));
        *connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if(*connfd < 0) {
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
        pthread_create(&tid, &attr, thread_work, connfd);
    }
    
}