#ifndef CORE_H
#define CORE_H
#include "config.h"
#include "csapp.h"


/**
 * @brief http请求头的部分元数据
 * 
 */
struct http_header_meta_data {
    char boundary[MAXLINE]; // multipart/form-data的分隔符
    int content_length; // 请求体的长度
};


void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
void read_request_headers(struct http_header_meta_data *meta_data);

/**
 * @brief 解析uri中的端点和文件名
 * 
 * @param uri 
 * @param endpoint 输出的端点
 * @param filename 输出的文件名
 * @return int 解析成功返回1，从参数返回endpoint, filename;
 *        解析失败返回0，endpoint, filename未定义
 */
int parse_uri(char *uri, char *endpoint, char *filename);

/**
 * @brief 处理查看文件请求
 * 
 * @param fd 客户端连接的文件描述符
 * @param filename 文件名
 */
void serve_static_file(int fd, char *filename);

/**
 * @brief 处理上传文件请求
 * 
 * @param fd 客户端连接的文件描述符
 * @param filename 上传的文件名
 */
void serve_upload_file(int fd, char *filename);

/**
 * @brief 获取文件类型
 * 
 * @param filename 文件名
 * @param filetype 输出的文件类型
 */
void get_filetype(char *filename, char *filetype);

#endif // CORE_H