#ifndef STARBACK_H
#define STARBACK_H

#include <SDL2/SDL.h>
#include <cstdlib>
#include "starfield.h"

class BackGround{
public:
    BackGround(){
        scale_surf = NULL;
    }
    BackGround(SDL_PixelFormat* pixel_format, unsigned int width, unsigned int height, SDL_Renderer *r){
        this->pixel_format = pixel_format;
        draw_rect.h = height;
        draw_rect.w = width;
        draw_rect.x = -draw_rect.w;
        draw_rect.y = -draw_rect.h;
        offset_x = 0;
        offset_y = 0;
        last_x = 0;
        last_y = 0;
        this->r = r;
        tile_x = 3;
        tile_y = 3;
        
        scale_surf=SDL_CreateRGBSurface(0,draw_rect.w, draw_rect.h,32,0,0,0,0);
        scale_surf->refcount++;
        SDL_SetColorKey(scale_surf, SDL_TRUE, SDL_MapRGB(scale_surf->format,0,0xff,0xa1)); 
        SDL_SetSurfaceBlendMode(scale_surf, SDL_BLENDMODE_NONE);

        generate_background();
    }
    void update(int x, int y){
        offset_x = x;
        offset_y = y;
        delta.x = offset_x - last_x;
        delta.y = offset_y - last_y;
        draw_rect.x = -draw_rect.w - delta.x;
        draw_rect.y = -draw_rect.h - delta.y;
        if(draw_rect.x > 0 or draw_rect.x < -2*draw_rect.w){
            draw_rect.x = -draw_rect.w + draw_rect.x % draw_rect.w;
        }
        if(draw_rect.y > 0 or draw_rect.y < -2*draw_rect.h){
            draw_rect.y = -draw_rect.h + draw_rect.y % draw_rect.h;
        }
    }
    void draw(){
        int start_y, start_x;
        start_x = draw_rect.x;
        start_y = draw_rect.y;
        for(int i=0; i<tile_x; draw_rect.x+=draw_rect.w, i++){
            draw_rect.y = start_y;
            for(int j=0; j<tile_y; draw_rect.y+=draw_rect.h, j++){
                SDL_RenderCopyEx(r, starfield[i].get_texture(), NULL, &draw_rect, 0, NULL, SDL_FLIP_NONE);
            }
        }
        draw_rect.x = start_x;
        draw_rect.y = start_y;
    } 
private:
    SDL_PixelFormat* pixel_format;
    Point delta;
    StarField* starfield;
    SDL_Surface *scale_surf;
    SDL_Renderer* r;
    SDL_Rect draw_rect;
    int offset_x, offset_y,last_x,last_y;
    int tile_x, tile_y;

    void generate_background(){
        starfield = new StarField[tile_x*tile_y];
        for(int i=0; i<tile_x; i++){
            for(int j=0; j<tile_y; j++){
                starfield[i + j*tile_x].make_starfield(r, pixel_format, draw_rect.w, draw_rect.h);
            }
        }
    }
};
#endif
