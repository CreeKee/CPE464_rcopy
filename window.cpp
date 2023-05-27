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

    for(uint32_t cull = 0; cull<val; cull++){
        buffer[(cull+lower)%windowSize].empty = true;
        if(lower == current) current = (lower+1)%windowSize;
        lower = (lower+1)%windowSize; 
    }
    upper = (lower+windowSize-1)%windowSize;

}