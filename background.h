#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <cstdlib>
#include "starfield.h"

class BackGround{
public:
    BackGround(){
        scale_surf = NULL;
    }
    BackGround(Uint32 pixel_format, unsigned int width, unsigned int height, SDL_Renderer *r){
        this->pixel_format = pixel_format;
        front_rect.h = height;
        front_rect.w = width;
        front_rect.x = -front_rect.w;
        front_rect.y = -front_rect.h;
        para_rect = front_rect;
        offset_x = 0;
        offset_y = 0;
        last_x = 0;
        last_y = 0;
        last_px = 0;
        last_py = 0;
        this->r = r;
        tile_x = 3;
        tile_y = 3;
        
        scale_surf=SDL_CreateRGBSurface(0,front_rect.w, front_rect.h,32,0,0,0,0);
        scale_surf->refcount++;
        SDL_SetColorKey(scale_surf, SDL_TRUE, SDL_MapRGB(scale_surf->format,0,0xff,0xa1)); 
        SDL_SetSurfaceBlendMode(scale_surf, SDL_BLENDMODE_NONE);

        generate_background();
    }
    void update(int x, int y){
        offset_x = x;
        offset_y = y;

        delta_front.x = offset_x - last_x;
        delta_front.y = offset_y - last_y;
        delta_back.x = offset_x/2 - last_px;
        delta_back.y = offset_y/2 - last_py;

        front_rect.x = -front_rect.w - delta_front.x;
        front_rect.y = -front_rect.h - delta_front.y;
        para_rect.x = -para_rect.w - delta_back.x;
        para_rect.y = -para_rect.h - delta_back.y;

        if(front_rect.x > 0 or front_rect.x < -2*front_rect.w){
            front_rect.x = -front_rect.w + front_rect.x % front_rect.w;
            last_x = offset_x;
        }
        if(front_rect.y > 0 or front_rect.y < -2*front_rect.h){
            front_rect.y = -front_rect.h + front_rect.y % front_rect.h;
            last_y = offset_y;
        }
        if(para_rect.x > 0 or para_rect.x < -2*para_rect.w){
            para_rect.x = -para_rect.w + para_rect.x % para_rect.w;
            last_px = offset_x/2;
        }
        if(para_rect.y > 0 or para_rect.y < -2*para_rect.h){
            para_rect.y = -para_rect.h + para_rect.y % para_rect.h;
            last_py = offset_y/2;
        }
    }
    void draw(){
        int start_y, start_x, para_x, para_y;
        start_x = front_rect.x;
        start_y = front_rect.y;
        para_x = para_rect.x;
        para_y = para_rect.y;
        for(int i=0; i<tile_x; front_rect.x+=front_rect.w, para_rect.x += para_rect.w, i++){
            front_rect.y = start_y;
            para_rect.y = para_y;
            for(int j=0; j<tile_y; front_rect.y+=front_rect.h, para_rect.y += para_rect.h, j++){
                SDL_RenderCopyEx(r, starfield->get_texture(), NULL, &front_rect, 0, NULL, SDL_FLIP_NONE);
                SDL_RenderCopyEx(r, parallax->get_texture(), NULL, &para_rect, 0, NULL, SDL_FLIP_NONE);
            }
        }
        front_rect.x = start_x;
        front_rect.y = start_y;
        para_rect.x = para_x;
        para_rect.y = para_y;
    } 
private:
    Point delta_front, delta_back;
    Uint32 pixel_format;
    StarField *starfield, *parallax;
    SDL_Surface *scale_surf;
    SDL_Renderer* r;
    SDL_Rect front_rect, para_rect;
    int offset_x, offset_y,last_x,last_y, last_px, last_py;
    int tile_x, tile_y;

    void generate_background(){
        starfield = new StarField(r, pixel_format, front_rect.w, front_rect.h);
        parallax = new StarField(r, pixel_format, para_rect.w, para_rect.h, true, 200, 600);
    }
};
#endif
