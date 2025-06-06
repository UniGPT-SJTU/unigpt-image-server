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
server_config_t server_config;

int parse_endpoint_from_uri(char *uri, char *endpoint)
{
    if (sscanf(uri, "/%[^/]", endpoint) != 1)
    {
        return -1;
    }
    return 0;
}

int init_server_config(server_config_t *server_config, int argc, char *argv[])
{
    server_config->protocol = server_config->ip = server_config->port = server_config->base_dir = NULL;

    void *init_member = NULL;
    for(int i = 1;i < argc; ++i) {
        if(!init_member) {
            if(strcmp(argv[i], "--protocol") == 0) {
                init_member = &server_config->protocol;
            } else if(strcmp(argv[i], "--ip") == 0) {
                init_member = &server_config->ip;
            } else if(strcmp(argv[i], "--port") == 0) {
                init_member = &server_config->port;
            } else if(strcmp(argv[i], "--base-dir") == 0) {
                init_member = &server_config->base_dir;
                // FIXME: base_dir must ended with / 
            } else {
                LOG_ERROR("unknown option %s", argv[i]);
                return -1;
            }
        } else {
            *(char **) init_member = (char *) malloc(strlen(argv[i]) + 1);
            strcpy(*(char **) init_member, argv[i]);
            init_member = NULL;
        }
    }
    if(init_member) {
        LOG_ERROR("one config not initialized");
        return -1;
    }

    // default init
    if(!server_config->protocol) {
        server_config->protocol = (char *) malloc(strlen(SERVER_PROTOCOL) + 1);
        strcpy(server_config->protocol, SERVER_PROTOCOL);
    }
    if (!server_config->ip) {
        server_config->ip = (char *)malloc(strlen(SERVER_IP) + 1);
        strcpy(server_config->ip, SERVER_IP);
    }
    if (!server_config->port) {
        server_config->port = (char *)malloc(strlen(SERVER_PORT) + 1);
        strcpy(server_config->port, SERVER_PORT);
    }
    if (!server_config->base_dir) {
        server_config->base_dir = (char *)malloc(strlen(SERVER_BASE_DIR) + 1);
        strcpy(server_config->base_dir, SERVER_BASE_DIR);
    }

    return 0;
}

void free_server_config(server_config_t *server_config) {
    free(server_config->protocol);
    free(server_config->ip);
    free(server_config->port);
    free(server_config->base_dir);
}

int handle_http_request(int connfd)
{
    char method[SHORTLINE], uri[SHORTLINE], version[SHORTLINE];
    char endpoint[SHORTLINE] = {};
    char filename[SHORTLINE] = {};

    rio_readinitb(&rio, connfd);

    // read request line
    rio_readlineb(&rio, buf, MAXLINE);
    // LOG_DEBUG("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (parse_endpoint_from_uri(uri, endpoint) < 0)
    {
        LOG_ERROR("Parse endpoint from uri error");
        LOG_ERROR("uri: %s", uri);
        serve_error_response(connfd, uri, "400", "Bad Request", "Parse endpoint from uri error");
        return -1;
    }

    if (!strcasecmp(method, "GET") && !strcmp(endpoint, "file"))
    {
        // GET /file/<filename>
        if (parse_static_filename_from_uri(uri, filename) < 0)
        {
            LOG_ERROR("Parse file name from uri error");
            serve_error_response(connfd, uri, "400", "Bad Request", "Parse file name from uri error");
            return -1;
        }
        return serve_static_file(connfd, filename);
    }

    if (!strcasecmp(method, "POST") && !strcmp(endpoint, "upload"))
    {
        // POST /upload
        return serve_upload_file(connfd);
    }

    // request not implemented yet
    LOG_ERROR("Unable to handle the request");
    serve_error_response(connfd, uri, "400", "Bad Request", "Unable to handle the request");
    return -1;
}

void serve_error_response(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char body[MAXBUF];

    snprintf(body, MAXBUF,
             "<html><title>Tiny Error</title>"
             "<body bgcolor=\"ffffff\">\r\n"
             "%s: %s\r\n"
             "<p>%s: %s\r\n"
             "<hr><em>The Tiny Web server</em>\r\n",
             errnum, shortmsg, longmsg, cause);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

void serve_json_response(int fd, char *status_code, char *json)
{
    int json_len = strlen(json);

    snprintf(buf, MAXBUF,
             "HTTP/1.0 %s OK\r\n"
             "Server: Tiny Web Server\r\n"
             "Connection: close\r\n"
             "Content-length: %d\r\n"
             "Content-type: application/json\r\n\r\n",
             status_code, json_len);
    rio_writen(fd, buf, strlen(buf));

    printf("Response headers:\n");
    printf("%s", buf);

    rio_writen(fd, json, json_len);
}

void serve_file_response(int fd, char *filename, ssize_t filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    snprintf(buf, MAXBUF,
             "HTTP/1.0 200 OK\r\n"
             "Server: Tiny Web Server\r\n"
             "Connection: close\r\n"
             "Content-length: %ld\r\n"
             "Content-type: %s\r\n\r\n",
             filesize, filetype);

    // LOG_DEBUG(buf);

    rio_writen(fd, buf, strlen(buf));

    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

int parse_boundary_from_content_type(char *content_type, char *boundary)
{
    if (sscanf(content_type, "multipart/form-data; boundary=%s", boundary) != 1)
    {
        return -1;
    }
    return 0;
}

int parse_filename_from_request_body(char *request_body, char *filename)
{
    char *start = strstr(request_body, "filename=\"");
    if (start == NULL)
    {
        return -1;
    }
    start += strlen("filename=\"");
    char *end = strstr(start, "\"");
    if (end == NULL)
    {
        return -1;
    }
    strncpy(filename, start, end - start);
    filename[end - start] = '\0';
    return 0;
}

int read_request_headers(http_header_meta_t *meta_data)
{
    while (1)
    {
        rio_readlineb(&rio, buf, MAXLINE);
        // LOG_DEBUG("%s", buf);
        if (!strcmp(buf, "\r\n"))
        {
            // empty line
            break;
        }
        if (strstr(buf, "Content-Type:"))
        {
            // read Content-Type
            if (sscanf(buf, "Content-Type: %s", meta_data->content_type) != 1)
            {
                return -1;
            }
        }
        if (strstr(buf, "Content-Length:"))
        {
            // read Content-Length
            if (sscanf(buf, "Content-Length: %d", &meta_data->content_length) != 1)
            {
                return -1;
            }
        }
    }
    return 0;
}

int parse_static_filename_from_uri(char *uri, char *filename)
{
    strcpy(filename, server_config.base_dir);
    if (sscanf(uri, "/file/%s", filename + strlen(server_config.base_dir)) != 1)
    {
        return -1;
    }
    return 0;
}

int gen_unique_str(char *dst)
{
    char temp_template[] = "/tmp/" FILE_TEMPLATE;
    int tempfd = mkstemp(temp_template);
    if (tempfd < 0)
    {
        return -1;
    }
    close(tempfd);
    unlink(temp_template);
    strncpy(dst, temp_template + 5, strlen(temp_template + 5) + 1);
    return 0;
}

int serve_upload_file(int connfd)
{
    char request_body[MAXFILESIZE];

    http_header_meta_t meta_data;
    if (read_request_headers(&meta_data) < 0)
    {
        LOG_ERROR("Read request error");
        serve_error_response(connfd, "", "400", "Bad Request", "Read request header error");
        return -1;
    }

    rio_readnb(&rio, request_body, meta_data.content_length);

    // printf("Request body:\n");
    // printf("%s", request_body);

    char upload_file_name[MAXLINE];

    if (parse_filename_from_request_body(request_body, upload_file_name) < 0)
    {
        serve_error_response(connfd, "", "400", "Bad Request", "Tiny couldn't parse the file name");
        return -1;
    }
    // 找到rawdata的起始位置
    char *rawdata = strstr(request_body, "\r\n\r\n") + 4;

    // TODO: 未去除rawdata之后的数据

    // 将raw data写入文件
    char new_file_name[MAXLINE];
    if (gen_unique_str(new_file_name) < 0)
    {
        LOG_ERROR("Tiny couldn't generate unique file name");
        serve_error_response(connfd, "", "500", "Internal Server Error", "Tiny couldn't generate unique file name");
        return -1;
    }
    char temp_file_name[MAXLINE];
    strncpy(temp_file_name, new_file_name, sizeof(temp_file_name) - 1);
    temp_file_name[sizeof(temp_file_name) - 1] = '\0';
    snprintf(new_file_name, MAXLINE, "%s_%s", temp_file_name, upload_file_name);

    int outputfd = open(new_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (write(outputfd, rawdata, meta_data.content_length) < 0)
    {
        LOG_ERROR("save to server disk failed");
        serve_error_response(connfd, "", "400", "Bad Request", "save to server disk failed");
        close(outputfd);
        return -1;
    }
    close(outputfd);

    // construct json response
    char json_resp[MAXLINE];
    sprintf(
        json_resp,
        "{\"status\": \"success\", \"url\": \"%s://%s:%s/file/%s\"}",
        server_config.protocol,
        server_config.ip,
        server_config.port,
        new_file_name);

    serve_json_response(connfd, "200", json_resp);

    return 0;
}

int serve_static_file(int connfd, char *filename)
{
    http_header_meta_t meta_data;
    if (read_request_headers(&meta_data) < 0)
    {
        LOG_ERROR("Read request header error");
        serve_error_response(connfd, "", "400", "Bad Request", "Read request header error");
        return -1;
    }

    struct stat sbuf;
    if (stat(filename, &sbuf) < 0)
    {
        LOG_ERROR("No such file: %s", filename);
        serve_error_response(connfd, filename, "404", "Not found", "No such file");
        return -1;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
        LOG_ERROR("Forbidden to read the file: %s", filename);
        serve_error_response(connfd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return -1;
    }
    serve_file_response(connfd, filename, sbuf.st_size);
    return 0;
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
    {
        strcpy(filetype, "text/html");
    }
    else if (strstr(filename, ".gif"))
    {
        strcpy(filetype, "image/gif");
    }
    else if (strstr(filename, ".png"))
    {
        strcpy(filetype, "image/png");
    }
    else if (strstr(filename, ".jpg"))
    {
        strcpy(filetype, "image/jpeg");
    }
    else
    {
        strcpy(filetype, "text/plain");
    }
}

void usage(char *exec)
{
    printf("Usage: %s --protocol <PROTOCOL> --ip <IP> --port <PORT> --base-dir <BASE_DIR> \n", exec);
}
