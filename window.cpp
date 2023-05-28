#include "window.hpp"

Window::Window(uint32_t size){
    windowSize = size;
    buffer = new Pack[size];

    lower = 0;
    upper = lower+windowSize-1;
    current = 0;

    return;
}

void Window::upshift(uint32_t val){

    //remove old data
    for(uint32_t cull = 0; cull<val; cull++){

        //lazy delete old data
        buffer[(cull+lower)%windowSize].empty = true;

        //move current if needed
        if(lower == current) current = (lower+1)%windowSize;

        //move lower
        lower = (lower+1)%windowSize; 
    }

    //update upper
    upper = (lower+windowSize-1)%windowSize;

}