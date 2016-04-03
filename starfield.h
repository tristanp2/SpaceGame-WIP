#ifndef STARFIELD_H
#define STARFIELD_H

#include <SDL2/SDL.h>
#include <cstdlib>

class StarField{
public:
    StarField(){
        tex = NULL;
        width = 0;
        height = 0;
    }
    StarField(SDL_Renderer* r, SDL_PixelFormat* pixel_format, int width, int height){
        make_starfield(r, pixel_format, width, height);
    }
    void make_starfield(SDL_Renderer* r, SDL_PixelFormat* pixel_format, int width, int height){
        this->width = width;
        this->height = height;
        this->pixel_format = pixel_format;
        tex = SDL_CreateTexture(r, pixel_format->format, SDL_TEXTUREACCESS_STREAMING, width, height);
        lock_texture();
        pixel_count = pitch/4 * height;
        generate_texture();
        unlock_texture();
    }
    SDL_Texture* get_texture(){
        return tex;
    }
private:
    int width, height, pitch, pixel_count;
    Uint32* pixels;
    SDL_PixelFormat* pixel_format;
    SDL_Texture* tex;

    bool lock_texture(){
        void* temp_pixels;
        if(pixels != NULL){
            cout<<"already locked\n";
            return false;
        }
        else{
            if(SDL_LockTexture(tex, NULL, &temp_pixels, &pitch) != 0){
                cout<<"unable to lock texture\t"<<SDL_GetError()<<endl;
                return false;
            }
            pixels = (Uint32*)temp_pixels;
        }
        return true;
    }
    bool unlock_texture(){
        if(pixels == NULL){
            cout<<"already unlocked\n";
            return false;
        }
        else{
            SDL_UnlockTexture(tex);
            pixels = NULL;
            pitch = 0;
        }
        return true;
    }
    void generate_texture(){
        Uint32 star_colour = SDL_MapRGB(pixel_format, 0xff, 0xff, 0xff);
        Uint32 space_colour = SDL_MapRGB(pixel_format, 0, 0, 0);
        int max_dist = width/4;
        int min_dist = 100;
        int dist;
        max_dist -= min_dist;
        for(int i=0; i<pixel_count; i++){
            dist = rand()%max_dist + min_dist;
            while(dist>=0 and i<pixel_count - 1){
                pixels[i++] = space_colour;
                dist--;
            }
            pixels[i] = star_colour;
        }
        cout<<space_colour<<endl;
        for(int i=0;i<pixel_count; i++){
           // cout<<hex<<pixels[i]<<endl;
        }

    }
};
#endif
