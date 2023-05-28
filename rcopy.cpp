// Client side - UDP Code				    
// By Hugh Smith	4/1/2017		

#include "includes.h"
#include "commControl.hpp"
#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "window.hpp"

bool handshake(char* filename, uint32_t buffersize, uint32_t windowsize, CommBund* server);
void sendData(CommBund server, int fd);
void talkToServer(int socketNum, struct sockaddr_in6 * server);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[], int* portNumber, double* errorRate);


int main (int argc, char *argv[])
 {
	
	int portNumber = 0;
	int fd;
	double errorRate = 0;

	Pack badpack;
	badpack.flag = 5;

	char destfile[] = "rcopytestout";

	CommBund server;
	server.otherAddrLen = sizeof(struct sockaddr_in6);;
	
	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

	server.socket = setupUdpClientToServer(&(server.other), argv[2], portNumber);
		
	addToPollSet(server.socket);

	fd = open("rcopytestin", O_RDONLY);

	if(handshake(destfile,5,10,&server) > 0){
		sendData(server, fd);
		close(server.socket);
	}

	printf("\n--END--\n");

	return 0;
}

bool handshake(char* filename, uint32_t buffersize, uint32_t windowsize, CommBund* server){

	CommBund reset = *server;
	bool retval = true;
	Pack inp;
	Pack outp;
	uint8_t buffer[MAXBUF] = {0};
	uint32_t timeout = 0;
	bool done = false;

	((uint32_t*)buffer)[0] = buffersize;
	((uint32_t*)buffer)[1] = windowsize;
	memcpy(buffer+8, filename, strlen(filename));
	outp = Pack((uint8_t*)buffer,strlen(filename)+8,0,GREETFLAG);
	sendPack(server, outp);

	while(timeout++ < 100 && done == false){
		if(pollCall(1*100)>0){

			inp = receivePack(server);

			if(!inp.empty) done = true;
			else *server = reset;

			if(inp.datalen == 0) retval = false;

		}
		else{
			sendPack(server, outp);
		}
	}
	if(!done){
		retval = false;
	}

	return retval;
}

void sendData(CommBund server, int fd){

	Window window(10);
	Pack incpack;
	Pack outpack;

	uint8_t buffer[MAXBUF] = {0};

	uint32_t pollcheck = 0;
	uint32_t curseq = 0;
	uint32_t len = -1;
	bool done = false;

	//TODO wait for missing RR's
	while(!done && pollcheck < 100){
		printf("starting rcopying %d %d %d\n",window.getLow(), window.getCurr(), window.getUp());
		/*start blasting*/
		if(window.isopen() && len != 0){
			printf("read from file\n");
			len = read(fd,buffer,5);
			if(len<0){
				perror("read failed\n");
				exit(-1);
			}
			else if (len != 0){
				outpack = Pack(buffer, len, curseq++, DATAFLAG);
				window[window.getCurr()-window.getLow()] = outpack;
				window.incCurr();

				//send pack
				sendPack(&server, outpack);
			}
		}
		if(pollCall((!window.isopen())*10) > 0){
			//handle RR or SREJ
			incpack = receivePack(&server);

			if(!incpack.empty){
				switch(incpack.flag){
					case RRFLAG:
						printf("got RR %d, upshifting %d, from %d at %d\n", incpack.seq, incpack.seq-window[0].seq,window[0].seq,window.getLow());
						window.upshift(incpack.seq-window[0].seq);
						done = (incpack.seq == curseq)&&(len == 0);
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
		else if(!window.isopen()||len==0){
			pollcheck++;
			sendPack(&server, window[0]);
		}
		else{
			printf("unblocked poll\n");
		}
	}

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





