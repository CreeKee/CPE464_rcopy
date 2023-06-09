# Makefile for CPE464 tcp and udp test code
# updated by Hugh Smith - April 2023

# all target makes UDP test code
# tcpAll target makes the TCP test code


CC= g++ -std=c++11
CFLAGS= -g -Wall
LIBS = 

OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o commControl.o window.o Pack.o

#uncomment next two lines if your using sendtoErr() library
LIBS += libcpe464.2.21.a -lstdc++ -ldl
CFLAGS += -D__LIBCPE464_

.PHONY: $(OBJS) rcopy server

all: udpAll

udpAll: rcopy server
tcpAll: myClient myServer

rcopy: rcopy.cpp $(OBJS) 
	$(CC) $(CFLAGS) -o rcopy rcopy.cpp $(OBJS) $(LIBS)

server: server.cpp $(OBJS) 
	$(CC) $(CFLAGS) -o server server.cpp  $(OBJS) $(LIBS)

myClient: myClient.c $(OBJS)
	$(CC) $(CFLAGS) -o myClient myClient.c  $(OBJS) $(LIBS)

myServer: myServer.c $(OBJS)
	$(CC) $(CFLAGS) -o myServer myServer.c $(OBJS) $(LIBS)

safeUtil.o:
	$(CC) $(CFLAGS) -c safeUtil.c

networks.o:
	$(CC) $(CFLAGS) -c networks.c

window.o:
	$(CC) $(CFLAGS) -c window.cpp

Pack.o:
	$(CC) $(CFLAGS) -c Pack.cpp

gethostbyname.o:
	$(CC) $(CFLAGS) -c gethostbyname.c

pollLib.o:
	$(CC) $(CFLAGS) -c pollLib.c

commControl.o: checksum.o
	$(CC) $(CFLAGS) -c commControl.cpp

checksum.o:
	$(CC) $(CFLAGS) -c libcpe464/checksum.c

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f myServer myClient rcopy server *.o

unixupdate:
	git reset --hard
	git pull
	rm libcpe464.2.21.a
	make -f build464Lib.mk
	make all




