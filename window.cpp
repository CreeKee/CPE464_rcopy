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

    for(uint32_t cull = lower; cull<lower+val; cull++){
        buffer[cull%windowSize].empty = true;
    }

    if(lower+val > current) current = val%windowSize;
    lower = (lower+val)%windowSize;
    upper = (lower+windowSize-1)%windowSize;

}