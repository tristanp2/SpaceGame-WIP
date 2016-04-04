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
    StarField(SDL_Renderer* r, SDL_PixelFormat* pixel_format, int width, int height, bool transparent = true, int min_dist = -1, int max_dist = -1){
        pixels = NULL;
        make_starfield(r, pixel_format, width, height, transparent, min_dist, max_dist);
    }
    void make_starfield(SDL_Renderer* r, SDL_PixelFormat* pixel_format, int width, int height, bool transparent = true, int min_dist = -1, int max_dist = -1){
        this->width = width;
        this->height = height;
        this->pixel_format = pixel_format;
        this->transparent = transparent;
        tex = SDL_CreateTexture(r, pixel_format->format, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
        if(transparent) SDL_SetTextureAlphaMod(tex,200);
        if(max_dist < 0)    this->max_dist = width;
        else this->max_dist = max_dist;
        if(min_dist < 0)    this->min_dist = this->max_dist/2;
        else this->min_dist = min_dist;
        if(!lock_texture()){    //  was getting some odd situations where the created texture
            unlock_texture();   //was already locked, resulting in segfault during starfield
            lock_texture();     //generation. this seems to fix it
        }
        pixel_count = pitch/4 * height;
        generate_texture();
        unlock_texture();
        
    }
    SDL_Texture* get_texture(){
        return tex;
    }
private:
    int width, height, pitch, pixel_count, max_dist, min_dist;
    bool transparent;
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
        Uint32 space_colour;
        if(transparent) space_colour = SDL_MapRGBA(pixel_format, 0,0,0,0);
        else space_colour = SDL_MapRGB(pixel_format, 0,0,0);
        int dist;
        max_dist -= min_dist;
        cout<<hex<<"star: "<<star_colour<<endl<<"space: "<<space_colour<<dec<<endl;
        for(int i=0; i<pixel_count; i++){
            dist = rand()%max_dist + min_dist;
            do{
                pixels[i++] = space_colour;
                dist--;
            }while(dist>0 and i < pixel_count - 1);
            pixels[i] = star_colour;
        }
        cout<<space_colour<<endl;

    }
};
#endif
