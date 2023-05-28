/* Server side - UDP Code				    */
/* By Hugh Smith	4/1/2017	*/

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
	double errorRate = 0;
	int fd;
	int pid;
	
	Pack newcomer;

	CommBund client;

	checkArgs(argc, argv, &portNumber, &errorRate);
	sendErr_init(errorRate, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

	//set up server
	client.otherAddrLen = sizeof(client.other);
	client.socket = udpServerSetup(portNumber);
	
	signal(SIGCHLD, handleZombies);

	while(1){
		//detect new client
		newcomer = receivePack(&client);

		//check for corrupted data
		if(!newcomer.empty){

			//fork a child
			if((pid = fork()) == 0){

				//set up communications with client
				if((fd = handshake(newcomer, portNumber, &client, &buffersize, &windowsize)) != -1){

					//process data from client
					processData(fd, client, windowsize);
					close(fd);
				}

				close(client.socket);
				exit(0);
			}
			else if(pid < 0){
				perror("fork failed\n");
				exit(-1);
			}
		}

	}

	return 0;
}


/*
handshake receives a flag7 packet, processes it and sends a return flag8 packet which is assumed to make it
*/
int handshake(Pack newcomer, int portNumber, CommBund* client, uint32_t* buffersize, uint32_t* windowsize){

	char filename[MAXBUF] = {0};

	int retval = -1;
	//check newcomer.data
	*buffersize = ((uint32_t*)newcomer.data)[0];
	*windowsize = ((uint32_t*)newcomer.data)[1];

	if(*windowsize < MAXWINDOW && *buffersize < MAXBUF){
		memcpy(filename, newcomer.data+2*sizeof(uint32_t), newcomer.datalen-2*sizeof(uint32_t));

		if((retval = open(filename, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) < 0){
			printf("Error: file %s not found", filename);
			exit(-1);
		} 
	}

	if(retval < 0){
		//send rejection
		sendPack(client, Pack((uint8_t*)"",0,0,CHECKFLAG));
	}
	else{
		//create new socket and send acceptance
		if (((*client).socket = socket(AF_INET6,SOCK_DGRAM,0)) < 0)
		{
			perror("socket() call error");
			exit(-1);
		}

		//send confirmation
		sendPack(client, Pack((uint8_t*)(&(*client).socket),sizeof(uint32_t),0,CHECKFLAG));
	}
	return retval;
}


void processData(int fd, CommBund client, uint32_t windowSize){

	int curseq = -1;
	uint32_t RR = 0;
	uint32_t untracked = 0;
	int32_t check;

	Window window(windowSize);
	Pack incpack;

	//add client to poll set
	addToPollSet(client.socket);

	//wait for data
	while(pollCall(LONGWAIT)>0){
	
		//receive packet
		incpack = receivePack(&client);

		//check for valid packet
		if(!incpack.empty && (signed)incpack.seq > curseq){

			//buffer incoming packet
			window[(incpack.seq-window.getLow())%window.getSize()] = incpack;
			untracked++;

			//write sequentially buffered data to file
			RR = 0;
			for(uint32_t flush = 0; flush<window.getSize() && !window[0].empty; flush++){

				//write current packet to file
				if((check = write(fd, window[0].data, window[0].datalen))<0){
					perror("write failed");
					exit(-1);
				}
				
				//update current sequence number and RR value
				curseq = window[0].seq;
				RR = curseq+1;

				if(flush == 0) untracked--;

				window.upshift(1);
				
				
				
			}

			//check if RR is ready
			if(RR != 0){
				//send RR
				controlpack(&client, curseq+1, RRFLAG);
			}

			//end SREJ's for missing data
			while(window.isopen() && untracked > 0){
				//check if current data is missing
				if(window[window.getCurr()-window.getLow()].empty){
					//send SREJ
					controlpack(&client, curseq+abs((int)(window.getCurr()-window.getLow()))+1, SREJFLAG);
				}
				else{
					//current data exists, but is out of order
					untracked--;
				}

				//move to next packet in buffer
				if(untracked != 0) window.incCurr();
			}
		}

		//old data was resent, an RR must have failed
		else if(!incpack.empty && (signed)incpack.seq <= curseq){
			//resend most recent RR
			controlpack(&client, curseq+1, RRFLAG);
		}
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
	Pack pack((uint8_t*)srejPDU, 4, num, flag);

	sendPack(client, pack);
}

void handleZombies(int sig){
	int stat = 0;
	while(waitpid(-1, &stat, WNOHANG)>0){}
}
