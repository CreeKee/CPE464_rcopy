#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAXWINDOW 1073741824
#define MAXBUF BUFFMAX+HEADERSIZE
#define BUFFMAX 1400
#define HEADERSIZE 7
#define MAXFILENAME 1400-9

#define WINDMIN 2
#define BUFFMIN 1

#define CHECKFLAG 8
#define GREETFLAG 7
#define SREJFLAG 6
#define RRFLAG 5
#define DATAFLAG 3

#define SHORTWAIT 100
#define LONGWAIT 10000
#define TIMEOUTCNT 10

#define FFIND 1
#define TFIND 2
#define WSIND 3
#define BSIND 4
#define ERIND 5
#define HNIND 6
#define PNIND 7



#ifndef BUND
#define BUND
struct CommBund{

    int socket;
    struct sockaddr_in6 other;
    int otherAddrLen;

};

#endif