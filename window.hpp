#include "includes.h"
#include "Pack.hpp"

#ifndef WINDOW_H
#define WINDOW_H

class Window{

    private:
        uint32_t lower;
        uint32_t upper;
        uint32_t current;
        uint32_t windowSize;

        Pack* buffer;

    public:
        Window(uint32_t size);

        uint32_t getSize(){return windowSize;}
        uint32_t getCurr(){return current;}
        uint32_t getLow(){return lower;}
        uint32_t getUp(){return upper;}
        void incCurr(){current = (current+1)%windowSize;}

        bool isopen(){return current != upper;}

        void upshift(uint32_t val);

        Pack &operator[](int index){return buffer[(index+lower)%windowSize];}        

};

#endif