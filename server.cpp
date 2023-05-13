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

#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "cpe464.h"

#define MAXBUF 80

void processClient(int socketNum);
void checkArgs(int argc, char *argv[], int* portNumber, double* errorRate);

int main ( int argc, char *argv[]  )
{ 
	
	int socketNum = 0;				
	int portNumber = 0;
	double errorRate = 0;

	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);
	socketNum = udpServerSetup(portNumber);

	processClient(socketNum);

	close(socketNum);
	
	return 0;
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


