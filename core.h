#ifndef CORE_H
#define CORE_H
#include "config.h"
#include "csapp.h"

typedef struct
{
    char content_type[MAXLINE]; 
    int content_length;         
} http_header_meta_t;

typedef struct
{
    char *protocol;
    char *ip;
    char *port;
    char *base_dir;
} server_config_t;

int init_server_config(server_config_t *server_config, int argc, char *argv[]);

void free_server_config(server_config_t *server_config);

int handle_http_request(int fd);

void serve_error_response(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

void serve_json_response(int fd, char *status_code, char *json);

void serve_file_response(int fd, char *filename, ssize_t filesize);

int parse_boundary_from_content_type(char *content_type, char *boundary);

int parse_filename_from_request_body(char *request_body, char *filename);

int read_request_headers(http_header_meta_t *meta_data);

int parse_static_filename_from_uri(char *uri, char *filename);

int gen_unique_str(char *dst);

int serve_static_file(int connfd, char *filename);

int serve_upload_file(int connfd);

void get_filetype(char *filename, char *filetype);

void usage(char *exec);

#endif // CORE_H