#pragma once
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

const int SERV_PORT = 9999;
const int OPEN_MAX = 100;

typedef struct Head
{
    uint32_t pack_size;
    unsigned char src_adr[4];
    unsigned char dst_adr[4];
    uint16_t src_port;
    uint16_t dst_port;
}head_t;


void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
ssize_t my_read(int fd, char *ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);
