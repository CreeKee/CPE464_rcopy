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

        printf("\nsequence num: %d\nflag%d\npayload length: %d\npayload: %s\n", seqNum, flag, pduLength, aPDU+7);
    }
    else{

        //TODO error message
        printf("ERROR invalid checksum: %d\n", check);
    }
}