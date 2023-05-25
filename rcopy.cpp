// Client side - UDP Code				    
// By Hugh Smith	4/1/2017		

#include "includes.h"
#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "window.hpp"

#define MAXBUF 80

void talkToServer(int socketNum, struct sockaddr_in6 * server);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[], int* portNumber, double* errorRate);


int main (int argc, char *argv[])
 {
	uint32_t polltime = 0;
	uint32_t pollcheck = 0;
	int socketNum = 0;				
	struct sockaddr_in6 server;		// Supports 4 and 6 but requires IPv6 struct
	int portNumber = 0;
	double errorRate = 0;

	Window window;
	
	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

	socketNum = setupUdpClientToServer(&server, argv[2], portNumber);
	
	//talkToServer(socketNum, &server);
	
	//poll(0)
	//read into pack
	//add pack to window
	//send pack

	while(/*file not empty&&*/pollcheck < 11){

		/*start blasting*/
		while(window.isopen()){
			//read into pack
			//add pack to window
			window[window.getCurr()] = outpack;

			//send pack
		}
		if(pollCall(polltime)){
			//handle RR or SREJ
			polltime = 0;
			pollcheck = 0;
		}
		else{
			
		}
	}




	close(socketNum);

	return 0;
}

void talkToServer(int socketNum, struct sockaddr_in6 * server)
{
	int serverAddrLen = sizeof(struct sockaddr_in6);
	char * ipString = NULL;
	int dataLen = 0; 
	char buffer[MAXBUF+1];
	char payload[MAXBUF+1];
	int seq = 0;
	
	buffer[0] = '\0';
	while (buffer[0] != '.')
	{
		dataLen = readFromStdin(payload);

		printf("Sending: %s with len: %d\n", payload,dataLen);
	
		dataLen = createPDU((uint8_t*)buffer, seq++, 0, (uint8_t*)payload, dataLen);

		//print pdu for verification
		printPDU((uint8_t*)buffer, dataLen);
		safeSendto(socketNum, buffer, dataLen, 0, (struct sockaddr *) server, serverAddrLen);
		
		/*
		dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) server, &serverAddrLen);

		// print out pdu received
		ipString = ipAddressToString(server);
		printf("Server with ip: %s and port %d said it received %s\n", ipString, ntohs(server->sin6_port), buffer);
	    printPDU((uint8_t*)buffer, dataLen);*/
	}
}

int readFromStdin(char * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[], int* portNumber, double* errorRate)
{
	
        /* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s error-rate host-name port-number \n", argv[0]);
		exit(1);
	}
	
	//TODO error checkings
	*portNumber = atoi(argv[3]);
	*errorRate = atof(argv[1]);
		
	return;
}





