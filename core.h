#ifndef CORE_H
#define CORE_H

#include "csapp.h"

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);

#endif // CORE_H