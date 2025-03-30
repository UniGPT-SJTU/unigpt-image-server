#ifndef CONFIG_H
#define CONFIG_H

#define LISTENQ 1024
#define MAXLINE 1024
#define SHORTLINE 64
#define MAXBUF 1024
#define MAXFILESIZE (5 << 20) // 5MB
#define FILE_TEMPLATE "upload_XXXXXX"
#define SERVER_PROTOCOL "http"
#define SERVER_IP "localhost"
#define SERVER_PORT "10339"
#define SERVER_BASE_DIR "./"
#endif // CONFIG_H