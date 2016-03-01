#include <SDL2/SDL.h>

class Point{
public:
    int x,y;
    Point(){
        x = 0;
        y = 0;
    }
    Point(int x, int y){
        this->x = x;
        this->y = y;
    }
    SDL_Point get_sdl_point(){
        SDL_Point point = {x,y};
        return point;
    }
    int operator[] (int i){
        if(i==0)    return x;
        if(i==1)    return y;
        return 0;
    }
};
