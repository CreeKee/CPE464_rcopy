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
void sendData(CommBund server, int fd, uint32_t buffersize, uint32_t windowsize);
void talkToServer(int socketNum, struct sockaddr_in6 * server);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[]);


int main (int argc, char *argv[])
 {
	int fd;
	CommBund server;
	server.otherAddrLen = sizeof(struct sockaddr_in6);;
	
	checkArgs(argc, argv);
	sendErr_init(atof(argv[ERIND]), DROP_ON, FLIP_ON, DEBUG_OFF, RSEED_ON);

	server.socket = setupUdpClientToServer(&(server.other), argv[HNIND], atoi(argv[PNIND]));
		
	addToPollSet(server.socket);

	//open file
	if((fd = open(argv[FFIND], O_RDONLY)) >0){

		//connect to server
		if(handshake(argv[TFIND],atof(argv[BSIND]),atof(argv[WSIND]),&server) > 0){

			//send data
			sendData(server, fd, atof(argv[BSIND]), atof(argv[WSIND]));

			//close connection
			close(server.socket);
		}

		//close file
		close(fd);
	}
	else{
		printf("Error: file %s not found\n", argv[FFIND]);
		exit(-1);
	}

	return 0;
}

/*
handshake sends a flag7 packet and receives then processes a flag8 packet from the server
*/
bool handshake(char* filename, uint32_t buffersize, uint32_t windowsize, CommBund* server){

	CommBund reset = *server;
	bool retval = true;
	Pack inp;
	Pack outp;
	uint8_t buffer[MAXBUF] = {0};
	uint32_t timeout = 0;
	bool done = false;

	//create flag7 packet with buffer size, window size, and null terminated filename
	((uint32_t*)buffer)[0] = buffersize;
	((uint32_t*)buffer)[1] = windowsize;
	memcpy(buffer+2*sizeof(uint32_t), filename, strlen(filename)+1);
	outp = Pack((uint8_t*)buffer,strlen(filename)+1+2*sizeof(uint32_t),0,GREETFLAG);

	//send flag7 packet
	sendPack(server, outp);

	//wait for confirmation from server
	while(timeout++ < TIMEOUTCNT && done == false){

		//poll server
		if(pollCall(SHORTWAIT)>0){

			//receive packet from server and update server socket
			inp = receivePack(server);

			//check for valid packet
			if(!inp.empty){ 
				done = true;
				removeFromPollSet(reset.socket);
				addToPollSet(server->socket);
			}

			//reset communication socket in case of corrupted packet (create new child and let previous child die)
			else *server = reset;

			//check confirmation/deny
			if(inp.datalen == 0) retval = false;

		}
		else{
			//resend packet in case of corruption of previous attempt
			sendPack(server, outp);
		}
	}

	//check for timeout
	if(!done){
		retval = false;
	}

	return retval;
}

/*
sendData sequentially reads data and sends it to the server, using sliding window for error detection
*/
void sendData(CommBund server, int fd, uint32_t buffersize, uint32_t windowsize){

	Window window(windowsize);
	Pack incpack;
	Pack outpack;

	int32_t pollres = 0;

	uint8_t buffer[MAXBUF] = {0};

	uint32_t pollcheck = 0;
	uint32_t curseq = 0;
	uint32_t len = -1;
	bool done = false;

	//send data until file is empty or timeout occurs

	/*start blasting*/
	while(!done && pollcheck < TIMEOUTCNT){

		//check if window is open, and there is data which needs sending
		if(window.isopen() && len != 0){

			//read data from file
			len = read(fd,buffer,buffersize);
			if(len<0){
				perror("read failed\n");
				exit(-1);
			}

			//check for EOF
			else if (len != 0){

				//create packed with current data from the file
				outpack = Pack(buffer, len, curseq++, DATAFLAG);

				//add current packet to the window
				window[window.getCurr()-window.getLow()] = outpack;
				if(window.isopen()) window.incCurr();

				//send current packet
				sendPack(&server, outpack);
			}
		}

		//check with server for any responses
		while((pollres = pollCall((!window.isopen())*SHORTWAIT)) > 0){

			//receive packet from server
			incpack = receivePack(&server);

			//check for corrupted data
			if(!incpack.empty){

				//handle SREJ or RR
				switch(incpack.flag){
					case RRFLAG:
						//slide window according to the new RR value
						window.upshift(incpack.seq-window[0].seq);

						//check to see if that was the last RR expected
						done = (incpack.seq == curseq)&&(len == 0);
						break;

					case SREJFLAG:
						//resend missing packet
						sendPack(&server, window[incpack.seq-window[0].seq]);
						break;

					default:
						break;
				}

			}
			//reset timeout counter
			pollcheck = 0;
		}

		//server failed to respond, increase timeout counter and resend lowest packet
		if(pollres < 0 && (!window.isopen()||len == 0)){
			pollcheck++;
			sendPack(&server, window[0]);
		}
	}
	sendPack(&server, Pack(buffer, 0, curseq, DATAFLAG));
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

void checkArgs(int argc, char * argv[])
{
	
        /* check command line arguments  */
	if (argc != 8)
	{
		printf("usage: %s from-file to-file window-size buffer-size error-rate host-name port-number \n", argv[0]);
		exit(1);
	}

	if(strlen(argv[TFIND]) > MAXFILENAME || strlen(argv[FFIND])>MAXFILENAME){
		printf("filename too long\n");
		exit(1);
	}

	if(atof(argv[WSIND])<WINDMIN){
		printf("window size too small\n");
		exit(1);
	}

	if(atof(argv[WSIND])>MAXWINDOW){
		printf("window size too big\n");
		exit(1);
	}
	
	if(atof(argv[BSIND])<BUFFMIN){
		printf("buffer size too small\n");
		exit(1);
	}

	if(atof(argv[BSIND])>BUFFMAX){
		printf("buffer size too big\n");
		exit(1);
	}

	return;
}





