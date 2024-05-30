#include <strings.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "config.h"
#include "csapp.h"
#include "core.h"
#include "logger.h"

static rio_t rio;
static char buf[MAXLINE];
struct server_config server_config;


void init_server_config(struct server_config *server_config, const char *protocol, const char *ip, const char *port) {
    server_config->protocol = protocol;
    server_config->ip = ip;
    server_config->port = port;
}

void handle_http_request(int fd) {
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char endpoint[MAXLINE] = {};
    char filename[MAXLINE] = {};

    rio_readinitb(&rio, fd);
    // 读取请求行(request line)
    rio_readlineb(&rio, buf, MAXLINE);
    LOG_DEBUG("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if(parse_endpoint_from_uri(uri, endpoint) < 0) {
        LOG_ERROR("Parse endpoint from uri error");
        LOG_ERROR("uri: %s", uri);
        serve_error_response(fd, uri, "400", "Bad Request", "Parse endpoint from uri error");
        return ;
    }

    if(!strcasecmp(method, "GET") && !strcmp(endpoint, "file")) {
        // GET /file/<filename>
         if(parse_static_filename_from_uri(uri, filename) < 0) {
            LOG_ERROR("Parse file name from uri error");
            serve_error_response(fd, uri, "400", "Bad Request", "Parse file name from uri error");
            return;
        }
        serve_static_file(fd, filename);
        return ;
    }

    if(!strcasecmp(method, "POST") && !strcmp(endpoint, "upload")) {
        // POST /upload
        serve_upload_file(fd);
        return ;
    }

    // 无法处理的请求
    LOG_ERROR("Unable to handle the request");
    serve_error_response(fd, uri, "400", "Bad Request", "Unable to handle the request");
}


void serve_error_response(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char body[MAXBUF];

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

void serve_json_response(int fd, char *status_code, char *json) {
    int json_len = strlen(json);

    sprintf(buf, "HTTP/1.0 %s OK\r\n", status_code);
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, json_len);
    sprintf(buf, "%sContent-type: application/json\r\n\r\n", buf);
    rio_writen(fd, buf, strlen(buf));

    printf("Response headers:\n");
    printf("%s", buf);
    
    rio_writen(fd, json, json_len);
}

void serve_file_response(int fd, const char *filename, const char *filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    
    LOG_DEBUG(buf);

    rio_writen(fd, buf, strlen(buf));

    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

int parse_boundary_from_content_type(const char *content_type, char *boundary) {
    if(sscanf(content_type, "multipart/form-data; boundary=%s", boundary) != 1) {
        return -1;
    }
    return 0;
}

int parse_filename_from_request_body(const char *request_body, char *filename) {
    char *start = strstr(request_body, "filename=\"");
    if(start == NULL) {
        return -1;
    }
    start += strlen("filename=\"");
    char *end = strstr(start, "\"");
    if(end == NULL) {
        return -1;
    }
    strncpy(filename, start, end - start);
    filename[end - start] = '\0';
    return 0;
}


int parse_raw_data_from_request_body(const char *request_body, char *raw_data, int *raw_data_size) {
    raw_data = strstr(request_body, "\r\n\r\n") + 4;
    if(raw_data) {
        return -1;
    }

    return 0;
    
}
int read_request_headers(struct http_header_meta_data *meta_data) {
    while(1) {
        rio_readlineb(&rio, buf, MAXLINE);
        LOG_DEBUG("%s", buf);
        if(!strcmp(buf, "\r\n")) {
            // 读取到空行
            break;
        }
        if(strstr(buf, "Content-Type:")) {
            // 解析Content-Type
            if(sscanf(buf, "Content-Type: %s", meta_data->content_type) != 1) {
                return -1;
            }
        }
        if(strstr(buf, "Content-Length:")) {
            // 解析Content-Length
            if(sscanf(buf, "Content-Length: %d", &meta_data->content_length) != 1) {
                return -1;
            }
        }
    }
    return 0;
}


int parse_static_filename_from_uri(char *uri, char *filename) {
    if(sscanf(uri, "/file/%s", filename) != 1) {
        // 解析uri失败
        return -1;
    }
    return 0;
}

int parse_endpoint_from_uri(char *uri, char *endpoint) {
    if(sscanf(uri, "/%[^/]", endpoint) != 1) {
        return -1;
    }
    return 0;
}

int gen_unique_str(char *dst) {
    strcpy(dst, FILE_TEMPLATE);
    int tempfd = mkstemp(dst);
    if(tempfd < 0) {
        return -1;
    }
    close(tempfd);
    unlink(dst);
    return 0;
}



int serve_upload_file(int fd) {
    char request_body[MAXFILESIZE];

    struct http_header_meta_data meta_data;
    read_request_headers(&meta_data);

    rio_readnb(&rio, request_body, meta_data.content_length);

    printf("Request body:\n");
    printf("%s", request_body);

    char upload_file_name[MAXLINE];
    
    if(parse_filename_from_request_body(request_body, upload_file_name) < 0) {
        serve_error_response(fd, "", "400", "Bad Request", "Tiny couldn't parse the file name");
        return -1;
    }
    // 找到rawdata的起始位置
    char *rawdata = strstr(request_body, "\r\n\r\n") + 4;

    // TODO: 未去除rawdata之后的数据

    // 将raw data写入文件
    char new_file_name[MAXLINE];
    if(gen_unique_str(new_file_name) < 0) {
        LOG_ERROR("Tiny couldn't generate unique file name");
        serve_error_response(fd, "", "500", "Internal Server Error", "Tiny couldn't generate unique file name");
        return -1;
    }
    sprintf(new_file_name, "%s_%s", new_file_name, upload_file_name);

    int outputfd = open(new_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    write(outputfd, rawdata, meta_data.content_length);
    close(outputfd);

    // 构造json响应
    char response[MAXLINE];
    sprintf(
        response, 
        "{\"status\": \"success\", \"url\": \"%s://%s:%s/file/%s\"}",
        server_config.protocol,
        server_config.ip,
        server_config.port,
        new_file_name
    );

    serve_json_response(fd, "200", response);
    
    return 0;
}

int serve_static_file(int fd, char *filename) {
    struct http_header_meta_data meta_data;
    read_request_headers(&meta_data);

    struct stat sbuf;
    if(stat(filename, &sbuf) < 0) {
        LOG_ERROR("No such file: %s", filename);
        serve_error_response(fd, filename, "404", "Not found", "No such file");
        return -1;
    }

    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        LOG_ERROR("Forbidden to read the file: %s", filename);
        serve_error_response(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return -1;
    }
    serve_file_response(fd, filename, sbuf.st_size);
    return 0;
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