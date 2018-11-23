/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     ChenYong     First version
 */

#ifndef SAL_H__
#define SAL_H__

#include <rtdevice.h>

#ifdef SAL_USING_POSIX
#include <dfs_file.h>
#endif
#include <sys/socket.h>
#ifdef __cplusplus
extern "C" {
#endif

//#if !defined(socklen_t) && !defined(SOCKLEN_T_DEFINED)
//typedef unsigned long socklen_t;
//#endif 

/* sal socket magic word */
#define SAL_SOCKET_MAGIC               0x5A10

/* The maximum number of sockets structure */
#ifndef SAL_SOCKETS_NUM
#define SAL_SOCKETS_NUM                DFS_FD_MAX
#endif

/* The maximum number of protocol families */
#ifndef SAL_PROTO_FAMILIES_NUM
#define SAL_PROTO_FAMILIES_NUM         4
#endif

/* sal socket offset */
#ifndef SAL_SOCKET_OFFSET
#define SAL_SOCKET_OFFSET              0
#endif

struct proto_ops
{
    int (*socket)     (int domain, int type, int protocol);
    int (*closesocket)(int s);
    int (*bind)       (int s, const struct sockaddr *name, socklen_t namelen);
    int (*listen)     (int s, int backlog);
    int (*connect)    (int s, const struct sockaddr *name, socklen_t namelen);
    int (*accept)     (int s, struct sockaddr *addr, socklen_t *addrlen);
    int (*sendto)     (int s, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
    int (*recvfrom)   (int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
    int (*getsockopt) (int s, int level, int optname, void *optval, socklen_t *optlen);
    int (*setsockopt) (int s, int level, int optname, const void *optval, socklen_t optlen);
    int (*shutdown)   (int s, int how);
    int (*getpeername)(int s, struct sockaddr *name, socklen_t *namelen);
    int (*getsockname)(int s, struct sockaddr *name, socklen_t *namelen);
    int (*ioctlsocket)(int s, long cmd, void *arg);
#ifdef SAL_USING_POSIX
    int (*poll)       (struct dfs_fd *file, struct rt_pollreq *req);
#endif
};

struct sal_socket
{
    uint32_t magic;                    /* sal socket magic word */

    int socket;                        /* sal socket descriptor */
    int domain;
    int type;
    int protocol;

    const struct proto_ops  *ops;      /* socket options */
    void *user_data;                   /* specific sal socket data */
};

struct proto_family
{
    char name[RT_NAME_MAX];
    int family;                        /* primary protocol families type */
    int sec_family;                    /* secondary protocol families type */
    int             (*create)(struct sal_socket *sal_socket, int type, int protocol);   /* register socket options */

    struct hostent* (*gethostbyname)  (const char *name);
    int             (*gethostbyname_r)(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop);
    void            (*freeaddrinfo)   (struct addrinfo *ai);
    int             (*getaddrinfo)    (const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
};

/* SAL(Socket Abstraction Layer) initialize */
int sal_init(void);

struct sal_socket *sal_get_socket(int sock);

/* protocol family register and unregister operate */
int sal_proto_family_register(const struct proto_family *pf);
int sal_proto_family_unregister(const struct proto_family *pf);
struct proto_family *sal_proto_family_find(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* SAL_H__ */