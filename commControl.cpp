#include "commControl.hpp"

int createPDU(uint8_t* pduBuffer, uint32_t sequenceNumber, uint8_t flag, uint8_t* payload, int payloadLen){
        
    //TODO magic numbs
    memset(pduBuffer+4, 0, payloadLen+7+payloadLen%2);
    
    ((uint32_t*) pduBuffer)[0] = htonl(sequenceNumber);

    pduBuffer[6] = flag;

    memcpy(pduBuffer+7, payload, payloadLen);

    ((uint16_t*) pduBuffer)[2] = in_cksum((unsigned short*)pduBuffer, payloadLen+7);

    return payloadLen + 7;
}

void printPDU(uint8_t* aPDU, int pduLength){

    uint16_t check = in_cksum((unsigned short*)aPDU, pduLength);

    int seqNum;
    int flag;

    if(check == 0){

        seqNum = ntohl(((uint32_t*) aPDU)[0]);
        flag = aPDU[6];

        printf("\nsequence num: %d\nflag: %d\npayload length: %d\npayload: %s\n\n", seqNum, flag, pduLength, aPDU+7);
    }
    else{
        printf("ERROR invalid checksum: %d\n", check);
    }
}

void sendPack(CommBund *other, Pack pack){

    uint8_t PDU[MAXBUF+1] = {0};

    ((uint32_t*)PDU)[0] = htonl(pack.seq);
    PDU[6] = pack.flag;

    memcpy(PDU+7, pack.data, pack.datalen);

    ((uint16_t*) PDU)[2] = in_cksum((unsigned short*)PDU, pack.datalen+HEADERSIZE);
	safeSendto(other->socket, PDU, pack.datalen+HEADERSIZE, 0, (struct sockaddr *) &other->other, other->otherAddrLen);
}

Pack receivePack(CommBund *other){	
    uint8_t buffer[MAXBUF + 1] = {0};	  
	int dataLen = safeRecvfrom(other->socket, buffer, MAXBUF, 0, (sockaddr*)&(other->other), &(other->otherAddrLen));

	return Pack(buffer, dataLen);
}