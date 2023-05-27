// Client side - UDP Code				    
// By Hugh Smith	4/1/2017		

#include "includes.h"
#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "window.hpp"

void talkToServer(int socketNum, struct sockaddr_in6 * server);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[], int* portNumber, double* errorRate);


int main (int argc, char *argv[])
 {
	int len = 0;
	uint8_t buffer[MAXBUF] = {0};
	uint32_t polltime = 0;
	uint32_t pollcheck = 0;
	uint32_t curseq = 0;
	int portNumber = 0;
	double errorRate = 0;

	Pack badpack;
	badpack.flag = 5;

	Pack incpack;
	Pack outpack;

	CommBund server;
	server.otherAddrLen = sizeof(struct sockaddr_in6);;

	Window window(10);
	
	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

	server.socket = setupUdpClientToServer(&(server.other), argv[2], portNumber);
	
	//talkToServer(socketNum, &server);
	
	addToPollSet(server.socket);

	//poll(0)
	//read into pack
	//add pack to window
	//send pack

	int x = 0;
	while(/*file not empty&&*/pollcheck < 11){
		printf("starting rcopying %d %d %d\n",window.getLow(), window.getCurr(), window.getUp());
		/*start blasting*/
		while(window.isopen()){
			printf("read from file\n");

			len = readFromStdin((char*)buffer);

			outpack = Pack(buffer, len, curseq++, DATAFLAG);
			//read into pack
			//add pack to window
			window[window.getCurr()-window.getLow()] = outpack;
			printf("curr %d, curseq %d should be %d\n",window.getCurr(),window[window.getCurr()].seq,outpack.seq);
			window.incCurr();

			if(len == 1){
				printf("sending bad pack\n");
				uint8_t PDU[MAXBUF+1] = {0};

				((uint32_t*)PDU)[0] = htonl(outpack.seq);
				PDU[6] = outpack.flag;

				memcpy(PDU+7, "was a bad packet", 17);

				((uint16_t*) PDU)[2] = 0;
				safeSendto(server.socket, PDU, 17+HEADERSIZE, 0, (struct sockaddr *) &server.other, server.otherAddrLen);
			}
			else{

			//send pack
			sendPack(&server, outpack);
			}
		}
		if(pollCall(polltime)){
			//handle RR or SREJ
			incpack = receivePack(&server);

			if(!incpack.empty){
				switch(incpack.flag){
					case RRFLAG:
						x = incpack.seq;
						printf("got RR %d, upshifting %d, from %d at %d\n", x, x-window[0].seq,window[0].seq,window.getLow());
						window.upshift(incpack.seq-window[0].seq);
						break;

					case SREJFLAG:
						printf("got SREJ %d\n", incpack.seq);
						sendPack(&server, window[incpack.seq-window[0].seq]);
						break;

					default:
						break;
				}

			}
			pollcheck = 0;
		}
		else{
			pollcheck++;
			sendPack(&server, window[0]);
		}
	}

	close(server.socket);

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





