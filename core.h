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
} server_config_t;

void init_server_config(server_config_t *server_config_ptr, char *protocol, char *ip, char *port);

void handle_http_request(int fd);

/**
 * @brief 客户端错误响应
 *
 * @param fd 客户端连接的文件描述符
 * @param cause
 * @param errnum
 * @param shortmsg
 * @param longmsg
 */
void serve_error_response(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/**
 * @brief 向客户端返回JSON响应
 *
 * @param fd 客户端连接的文件描述符
 * @param status_code 状态码
 * @param json JSON字符串
 */
void serve_json_response(int fd, char *status_code, char *json);

/**
 * @brief 向客户端返回文件响应，默认200状态码
 *
 * @param fd 客户端连接的文件描述符
 * @param filename 映射的文件名
 * @param filesize 映射的文件大小
 */
void serve_file_response(int fd, char *filename, ssize_t filesize);

/**
 * @brief 从multipart/form-data内容类型中解析"boundary"字符串
 *
 * @param content_type
 * @param boundary 输出的boundary字符串
 * @return int 成功返回0，失败返回-1
 */
int parse_boundary_from_content_type(char *content_type, char *boundary);

/**
 * @brief 从request_body中解析文件名,写入filename
 *
 * @param request_body
 * @param filename
 * @return int 成功返回0，失败返回-1
 */
int parse_filename_from_request_body(char *request_body, char *filename);

/**
 * @brief 从request_body中解析文件数据,写入raw_data
 *
 * @param request_body
 * @param raw_data
 * @param raw_data_size
 * @return int
 */
int parse_raw_data_from_request_body(char *request_body, char *raw_data, int *raw_data_size);

/**
 * @brief 读取请求头，写入meta_data
 *
 * @param meta_data
 * @return int 成功返回0，失败返回-1
 */
int read_request_headers(http_header_meta_t *meta_data);

/**
 * @brief 解析uri中的端点和文件名
 *
 * @param uri
 * @param filename 输出的文件名
 * @return int 成功返回0，失败返回-1
 */
int parse_static_filename_from_uri(char *uri, char *filename);

/**
 * @brief 生成唯一的六位字符串
 *
 * @param dst 输出的字符串
 * @return int 成功返回0，失败返回-1
 */
int gen_unique_str(char *dst);

/**
 * @brief 处理查看文件请求
 *
 * @param fd 客户端连接的文件描述符
 * @param filename 文件名
 * @return int 成功返回0，失败返回-1
 */
int serve_static_file(int fd, char *filename);

/**
 * @brief 处理上传文件请求
 *
 * @param fd 客户端连接的文件描述符
 * @return int 成功返回0，失败返回-1
 */
int serve_upload_file(int fd);

/**
 * @brief 获取文件类型
 *
 * @param filename 文件名
 * @param filetype 输出的文件类型
 */
void get_filetype(char *filename, char *filetype);

#endif // CORE_H