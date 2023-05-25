#include "includes.h"
#include "cpe464.h"
#include "safeUtil.h"

#ifndef PACKET_H
#define PACKET_H

class Pack{

    private:

    public:
        uint32_t datalen;
        uint32_t seq;
        uint16_t check;
        uint8_t flag;

        uint8_t data[MAXBUF+1] = {0};

        bool empty = true;

        Pack();
        Pack(uint8_t* buff, uint32_t bufflen);



        //Pack& operator=(const Pack& p);

};

#endif