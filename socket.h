#ifndef SOCKET_H
#define SOCKET_H

/**
 * @brief 打开监听套接字
 * @details 打开一个监听套接字，绑定到指定端口，返回监听套接字的文件描述符
 *          如果失败，终止进程
 * 
 * @param port 
 * @return int 
 */
int open_listenfd_w(char *port);

#endif