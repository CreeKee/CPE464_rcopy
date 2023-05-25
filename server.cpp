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

#include "window.hpp"
#include "pollLib.h"
#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "cpe464.h"

#define MAXBUF 80

void processClient(int socketNum);
void checkArgs(int argc, char *argv[], int* portNumber, double* errorRate);
inline Pack receivePack(int socketNum);

int main ( int argc, char *argv[]  )
{ 
	Pack incpack;
	int socketNum = 0;				
	int portNumber = 0;
	int curseq = 0;
	double errorRate = 0;
	uint32_t RR = 0;

	//TODO
	Window window(10);

	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);
	socketNum = udpServerSetup(portNumber);

	//processClient(socketNum);


	addToPollSet(socketNum);

	//TODO magic num
	while(pollCall(-1)){
		
		printf("awoke from poll\n");

		//receive packet
		incpack = receivePack(socketNum);

		if(!incpack.empty && incpack.seq >= curseq){

			printf("pack validated against %d\n", curseq);

			window[(incpack.seq-window.getLow())%window.getSize()] = incpack;

			printf("pack inserted in %d, low is %d\n", (incpack.seq-window.getLow())%window.getSize(),window.getLow());

			for(int i = 0, RR = 0; i<window.getSize() && !window[i].empty; i++){
				//write window[i] to file
				printf("time to write seq: %d, %s\n", window[i].seq, window[i].data);

				RR = i+1;
				curseq = window[i].seq;
				window.upshift(1);

			}
			if(RR != 0){
				printf("RR(%d)\n",RR);
			}

			while(window.isopen()&&window[window.getCurr()].seq != incpack.seq){
				if(window[window.getCurr()].empty){
					//SREJ curr
					printf("SREJ %d\n",curseq+window.getCurr());
				}
				window.incCurr();
			}
			printf("\n");
		}
	}
	//timeout

	close(socketNum);
	
	return 0;
}

inline Pack receivePack(int socketNum){
	 
	uint8_t buffer[MAXBUF + 1] = {0};	  
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);	
	int dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &client, &clientAddrLen);

	return Pack(buffer, dataLen);
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


