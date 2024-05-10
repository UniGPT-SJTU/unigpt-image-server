#include <strings.h>
#include <assert.h>
#include <sys/mman.h>

#include "config.h"
#include "csapp.h"
#include "core.h"

static rio_t rio;

void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char endpoint[MAXLINE] = {};
    char filename[MAXLINE] = {};

    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if(strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
        clienterror(fd, method, "405", "Method Not Allowed", 
            "Tiny does not implement this method");
        return;
    }

    if(!parse_uri(uri, endpoint, filename)) {
        clienterror(fd, uri, "400", "Bad Request", "Tiny couldn't parse the uri");
        return ;
    }


    if(!strcmp(endpoint, "file") && !strcasecmp(method, "GET") ) {
        read_requesthdrs(&rio);
        serve_static_file(fd, filename);
        return ;
    } else if(!strcmp(endpoint, "upload") && !strcasecmp(method, "POST")) {
        serve_upload_file(fd, filename);
        return ;
    } else {
        clienterror(fd, uri, "400", "Bad Request", "Tiny couldn't parse the uri");
    }
}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
}

void read_request_headers(struct http_header_meta_data *meta_data) {
    char buf[MAXLINE];
    rio_readlineb(&rio, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        if(strstr(buf, "boundary=")) {
            sscanf(buf, "Content-Type: multipart/form-data; boundary=%s", meta_data->boundary);
        }
        if(strstr(buf, "Content-Length:")) {
            sscanf(buf, "Content-Length: %d", &meta_data->content_length);
        }


        rio_readlineb(&rio, buf, MAXLINE);
    }
}

int parse_uri(char *uri, char *endpoint, char *filename) {
    if(sscanf(uri, "/%[^/]/%s", endpoint, filename) != 2) {
        // 解析uri失败
        return 0;
    }
    if(strcmp(endpoint, "upload") && strcmp(endpoint, "file")) {
        return 0;
    }
    return 1;
}
void serve_upload_file(int fd, char *filename) {
    char buf[MAXBUF];

    
    char request_body[MAXFILESIZE];

    struct http_header_meta_data meta_data;
    read_request_headers(&meta_data);

    rio_readnb(&rio, request_body, meta_data.content_length);

    // 去除request_body前4行
    char *rawdata = request_body;
    char *end_boundary = NULL;
    for(int i = 0; i < 4; i++) {
        rawdata = strchr(rawdata, '\n') + 1;
    }

    // 将文件内容写入文件
    int filefd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    write(filefd, rawdata, meta_data.content_length);
    close(filefd);

    char *json = "{\"status\": \"success\"}";
    int json_len = strlen(json);

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, json_len);
    sprintf(buf, "%sContent-type: application/json\r\n\r\n", buf);
    rio_writen(fd, buf, strlen(buf));

    printf("Response headers:\n");
    printf("%s", buf);


    rio_writen(fd, json, json_len);
}
void serve_static_file(int fd, char *filename) {
    struct stat sbuf;
    if(stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found", "No such file");
        return ;
    }

    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return ;
    }
    int filesize = sbuf.st_size;
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype) {
    if(strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    } else if(strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    } else if(strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    } else if(strstr(filename, ".jpg")) {
        strcpy(filetype, "image/jpeg");
    } else {
        strcpy(filetype, "text/plain");
    }
}