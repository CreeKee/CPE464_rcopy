#include "includes.h"
#include "cpe464.h"
#include "Pack.hpp"
#include "safeUtil.h"


int createPDU(uint8_t* pduBuffer, uint32_t sequenceNumber, uint8_t flag, uint8_t* payload, int payloadLen);
void printPDU(uint8_t* aPDU, int pduLength);

Pack receivePack(CommBund *other);

void sendPack(CommBund *other, Pack pack);