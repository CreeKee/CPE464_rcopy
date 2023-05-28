/* Server side - UDP Code				    */
/* By Hugh Smith	4/1/2017	*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "window.hpp"
#include "pollLib.h"
#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "cpe464.h"

int handshake(Pack newcomer, int portNumber, CommBund* client, uint32_t* buffersize, uint32_t* windowsize);
void processData(int fd, CommBund client, uint32_t windowSize);
void processClient(int socketNum);
void checkArgs(int argc, char *argv[], int* portNumber, double* errorRate);
void controlpack(CommBund* client, uint32_t num, uint8_t flag);
void handleZombies(int sig);

int main ( int argc, char *argv[]  )
{ 
	uint32_t buffersize = 0;
	uint32_t windowsize = 0;
	int portNumber = 0;
	int serverSocket;
	double errorRate = 0;
	int fd;
	int pid;
	
	Pack newcomer;

	uint8_t buffer[MAXBUF] = {0};

	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

	CommBund client;
	client.otherAddrLen = sizeof(client.other);
	client.socket = udpServerSetup(portNumber);
	
	signal(SIGCHLD, handleZombies);

	while(1){
		//detect client
		newcomer = receivePack(&client);

		if(!newcomer.empty){
			if((pid = fork()) == 0){
				if((fd = handshake(newcomer, portNumber, &client, &buffersize, &windowsize)) != -1){
					printf("\nready for data on %d\n", client.socket);

					processData(fd, client, windowsize);
					close(fd);
				}

				printf("\n---END---\n");
				close(client.socket);
				exit(0);
			}
			else if(pid < 0){
				perror("fork failed\n");
				exit(-1);
			}
		}

	}

	

	//fork

	//check filename
		//if failed, send failure

	//create new socket

	//send acceptance with new socket number

	



/*
	addToPollSet(client.socket);

	//fork??
	fd = handshake();
    
	
	return 0;*/
}

int handshake(Pack newcomer, int portNumber, CommBund* client, uint32_t* buffersize, uint32_t* windowsize){

	char filename[MAXBUF] = {0};

	int retval = -1;
	//check newcomer.data
	*buffersize = ((uint32_t*)newcomer.data)[0];
	*windowsize = ((uint32_t*)newcomer.data)[1];
	printf("doing handshake %d %d\n", *buffersize, *windowsize);

	if(*windowsize < MAXWINDOW && *buffersize < MAXBUF){
		memcpy(filename, newcomer.data+8, newcomer.datalen-8);

		printf("accessing file %s\n", filename);

		retval = open(filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		
	}

	if(retval < 0){
		//send reject
		printf("could not open file\n");
		sendPack(client, Pack((uint8_t*)"",0,0,CHECKFLAG));
	}
	else{
		//create new socket and send acceptance
		printf("creating new socket for client\n");
		if (((*client).socket = socket(AF_INET6,SOCK_DGRAM,0)) < 0)
		{
			perror("socket() call error");
			exit(-1);
		}

		sendPack(client, Pack((uint8_t*)(&(*client).socket),4,0,CHECKFLAG));
	}
	return retval;
}

void processData(int fd, CommBund client, uint32_t windowSize){

	int curseq = -1;
	uint32_t oldRR;
	uint32_t RR = 0;
	uint32_t untracked = 0;
	int32_t check;

	//TODO window sizing
	Window window(windowSize);
	Pack incpack;

	addToPollSet(client.socket);

		//TODO magic num
	while(pollCall(20*1000)>0){
		
		printf("awoke from poll\n");

		//receive packet
		incpack = receivePack(&client);

		if(!incpack.empty && (signed)incpack.seq > curseq){

			printf("pack validated against %d\n", curseq);

			window[(incpack.seq-window.getLow())%window.getSize()] = incpack;
			untracked++;

			printf("pack inserted in %d, low is %d\n", (incpack.seq-window.getLow())%window.getSize(),window.getLow());

			RR = 0;
			for(int i = 0; i<window.getSize() && !window[0].empty; i++){
				//write window[i] to file
				printf("time to write seq: %d, %s with length %d\n", window[0].seq, window[0].data, window[0].datalen);
				if((check = write(fd, window[0].data, window[0].datalen))<0){
					perror("write failed");
					exit(-1);
				}
				printf("wrote %d bytes\n", check);
				
				curseq = window[0].seq;
				RR = curseq+1;
				window.upshift(1);
				
				untracked--;
			}
			if(RR != 0){
				printf("ready to RR %d\n",curseq+1);
				controlpack(&client, curseq+1, RRFLAG);
			}

			while(window.isopen() && untracked > 0){
				if(window[window.getCurr()-window.getLow()].empty){
					//SREJ curr
					printf("SREJ %d from index %d holding data %s\n",curseq+window.getCurr(), window.getCurr(),window[window.getCurr()].data);
					controlpack(&client, curseq+window.getCurr(), SREJFLAG);
				}
				else{
					untracked--;
				}
				window.incCurr();
				printf("\n");
			}
			printf("\n");
		}
		else if(!incpack.empty && incpack.seq <= curseq){

			controlpack(&client, curseq+1, RRFLAG);
		}
		else{
			printf("invalid packet\n");
		}
		printf("curseq = %d\n\n",curseq);
	}
	//timeout
}

void processClient(int socketNum)
{
	int dataLen = 0; 
	char buffer[MAXBUF + 1];	  
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);	
	
	buffer[0] = '\0';
	while (buffer[0] != '.')
	{
		dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &client, &clientAddrLen);
	
		printf("Received message from client with ");
		printIPInfo(&client);
		printPDU((uint8_t*)buffer, dataLen);

		// just for fun send back to client number of bytes received
		safeSendto(socketNum, buffer, dataLen, 0, (struct sockaddr *) & client, clientAddrLen);

	}
}

void checkArgs(int argc, char *argv[], int* portNumber, double* errorRate)
{
	// Checks args and returns port number
	if (argc > 3 || argc<2)
	{
		fprintf(stderr, "Usage %s error-rate [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 3)
	{
		*portNumber = atoi(argv[2]);
	}

	*errorRate = atof(argv[1]);
	
	return;
}

void controlpack(CommBund* client, uint32_t num, uint8_t flag){


	
    uint32_t srejPDU[1] = {htonl(num)};
	printf("NUM IS %d --- %d\n",num,srejPDU[0]);
	Pack pack((uint8_t*)srejPDU, 4, num, flag);

	sendPack(client, pack);
}

void handleZombies(int sig){
	int stat = 0;
	while(waitpid(-1, &stat, WNOHANG)>0){}
}
