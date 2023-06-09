#include "Pack.hpp"

Pack::Pack(){
    datalen = 0;
    empty = true;
    seq = -1;
    flag = 0;
}

Pack::Pack(uint8_t* buff, uint32_t bufflen){

    //verify data
    empty = (in_cksum((unsigned short*)buff, bufflen) != 0);

    //read in data
    datalen = bufflen-HEADERSIZE;
    if(!empty){
        seq = ntohl(((uint32_t*)buff)[0]);
        flag = buff[6];
        memcpy(data,buff+HEADERSIZE, bufflen-HEADERSIZE);
    }
    else{
        seq = -1;
    }
}

Pack::Pack(uint8_t* buff, uint32_t bufflen, uint32_t inseq, uint8_t inflag){
    empty = false;
    flag = inflag;
    seq = inseq;
    datalen = bufflen;
    memcpy(data,buff, bufflen);
}

Pack& Pack::operator=(const Pack& p){

    datalen = p.datalen;
    seq = p.seq;
    flag = p.flag;
    empty = p.empty;
    memcpy(data, p.data, datalen);

    return *this;
}