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
    StarField(SDL_Renderer* r, Uint32 format, int width, int height, bool transparent = true, int min_dist = -1, int max_dist = -1){
        pixels = NULL;
        this->format = format;
        make_starfield(r, width, height, transparent, min_dist, max_dist);
    }
    void make_starfield(SDL_Renderer* r, int width, int height, bool transparent = true, int min_dist = -1, int max_dist = -1){
        this->width = width;
        this->height = height;
        this->transparent = transparent;
        tex = SDL_CreateTexture(r, format, SDL_TEXTUREACCESS_STREAMING, width, height);
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
    Uint32 *pixels, format;
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
        Uint32 star_colour = 0xffffffff;    //pixel format can mostly be ignored here since rgba values are the same for each colour
        Uint32 space_colour = 0;
        int dist;
        max_dist -= min_dist;
        for(int i=0; i<pixel_count; i++){
            dist = rand()%max_dist + min_dist;
            do{
                pixels[i++] = space_colour;
                dist--;
            }while(dist>0 and i < pixel_count - 1);
            pixels[i] = star_colour;
        }
        int row_width = pitch/4;
        dist = rand()%(max_dist*4) + min_dist*4;
        for(int i = row_width*10; i < pixel_count - row_width*10; i++){ //will only draw larger stars 10 pixels away from boundaries
            if(dist < 0){
                if(i%row_width > 10 and row_width - i%row_width > 10){ 
                    pixels[i] = star_colour;
                    pixels[i-1] = star_colour;
                    pixels[i+1] = star_colour;
                    pixels[i - row_width] = star_colour; //pixel above i
                    pixels[i + row_width] = star_colour;
                }
                dist = rand()%(max_dist*4) + min_dist*4;
            }
            else dist--;
        }
    }
};
#endif
