#include "Pack.hpp"

Pack::Pack(){
    datalen = 0;
    empty = true;
    seq = -1;
    flag = 0;

}

Pack::Pack(uint8_t* buff, uint32_t bufflen){
    empty = (in_cksum((unsigned short*)buff, bufflen) != 0);
    datalen = bufflen;
    if(!empty){

        seq = ntohl(((uint32_t*) buff)[0]);
        flag = buff[6];
        memcpy(data,buff+7, bufflen-7);
    }
    else{
        seq = -1;
    }
}