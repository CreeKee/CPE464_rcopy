#include "includes.h"
#include "cpe464.h"

int createPDU(uint8_t* pduBuffer, uint32_t sequenceNumber, uint8_t flag, uint8_t* payload, int payloadLen);
void printPDU(uint8_t* aPDU, int pduLength);