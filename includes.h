#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXBUF 1400+HEADERSIZE
#define HEADERSIZE 7

#define SREJFLAG 6
#define RRFLAG 5
#define DATAFLAG 4

#ifndef BUND
#define BUND
struct CommBund{

    int socket;
    struct sockaddr_in6 other;
    int otherAddrLen;

};

#endif