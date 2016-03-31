#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <cstdlib>

class BackGround{
public:
    BackGround(){
        tiles = NULL;
        scale_surf = NULL;
    }
    BackGround(Sprite* tiles, bool randomize, unsigned int width, unsigned int height){
        this->tiles = tiles;
        tile_rect.h = tiles->get_frame_height();
        tile_rect.w = tiles->get_frame_width();
        tile_rect.x = -tile_rect.w;
        tile_rect.y = -tile_rect.h;
        offset_x = 0;
        offset_y = 0;
        last_x = 0;
        last_y = 0;
        full_width = width;
        full_height = height;
        tiles_x = full_width / tile_rect.w + 2;
        tiles_y = full_height / tile_rect.h + 2;
        this->randomize = randomize;
        
        scale_surf=SDL_CreateRGBSurface(0,tile_rect.w, tile_rect.h,32,0,0,0,0);
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
        tile_rect.x = -tile_rect.w - delta.x;
        tile_rect.y = -tile_rect.h - delta.y;
        if(tile_rect.x > 0 or tile_rect.x < -2*tile_rect.w){
            load_column();
            tile_rect.x = -tile_rect.w + tile_rect.x % tile_rect.w;
            cout<<"new x: "<<tile_rect.x<<endl;
        }
        if(tile_rect.y > 0 or tile_rect.y < -2*tile_rect.h){
            load_row();
            tile_rect.y = -tile_rect.h + tile_rect.y % tile_rect.h;
        }
    }
    void draw(SDL_Renderer* r){
        SDL_Texture* texture;
        int start_y, start_x;
        start_x = tile_rect.x;
        start_y = tile_rect.y;
        for(unsigned int i=0; i<tiles_x; tile_rect.x+=tile_rect.w, i++){
            tile_rect.y = start_y;
            for(unsigned int j=0; j<tiles_y; tile_rect.y+=tile_rect.h, j++){
                SDL_BlitScaled((*tiles)[tile_info[i][j].index], NULL, scale_surf, NULL);
                texture = SDL_CreateTextureFromSurface(r, scale_surf);
                SDL_RenderCopyEx(r, texture, NULL, &tile_rect, 0, NULL, tile_info[i][j].flip);
                SDL_DestroyTexture(texture);
            }
        }
        tile_rect.x = start_x;
        tile_rect.y = start_y;
    } 
    void tile_info_print(){
        cout<<"index,flip"<<endl;
        for(int i=0;i<tiles_x;i++){
            for(int j=0;j<tiles_y;j++){
                cout<<tile_info[i][j].index<<","<<tile_info[i][j].flip<<"\t";
            }
            cout<<endl;
        }
        cout<<"x,y"<<endl;
        cout<<tile_rect.x<<","<<tile_rect.y<<endl;
    }
private:
    bool randomize;
    Sprite* tiles;
    Point delta;
    SDL_Surface *scale_surf;
    SDL_Rect tile_rect;
    unsigned int full_width, full_height, tiles_x, tiles_y;
    int offset_x, offset_y,last_x,last_y;

    struct TileInfo{
        int index;
        SDL_RendererFlip flip;
    };
    TileInfo** tile_info; 

    void load_column(){
        last_x = offset_x;
        int n = abs(delta.x)/tile_rect.w;
        if(delta.x > 0){
            //Need to shift tiles to the left and get new column on right
            for(unsigned int i=1; i<tiles_x; i++){
                for(unsigned int j=0; j<tiles_y; j++){
                    tile_info[i-1][j] = tile_info[i][j];
                }
            }
            int i = tiles_x - 1;
            for(int j=0; j<tiles_y; j++){
                tile_info[i][j].index = rand() % (tiles->max_frame + 1);
                tile_info[i][j].flip = get_flip(rand()%4);
            }
        }
        else{
            for(int i=tiles_x-2; i>=0; i--){
                for(unsigned int j=0; j<tiles_y; j++){
                    tile_info[i+1][j] = tile_info[i][j];
                }
            }
            int i = 0;
            for(int j=0; j<tiles_y; j++){
                tile_info[i][j].index = rand() % (tiles->max_frame + 1);
                tile_info[i][j].flip = get_flip(rand()%4);
            }
        }
    }       
    void load_row(){
        last_y = offset_y;
        int n = abs(delta.y)/tile_rect.h - 1; //Need to determine how many rows need to be generated 
        if(delta.y > 0){
            cout<<"moving down\n";
            //Need to shift tiles up and get new row on bottom
            for(unsigned int i=0; i<tiles_x; i++){
                for(unsigned int j=1; j<tiles_y; j++){
                    tile_info[i][j-1] = tile_info[i][j];
                }
            }
            int j = tiles_y-1;
            for(int i=0; i<tiles_x; i++){
                tile_info[i][j].index = rand() % (tiles->max_frame + 1);
                tile_info[i][j].flip = get_flip(rand()%4);
            }
        }
        else{
            cout<<"moving up\n";
            for(unsigned int i=0; i<tiles_x; i++){
                for(int j=tiles_y - 1; j>=0; j--){
                    tile_info[i][j+1] = tile_info[i][j];
                }
            }
            int j=0;
            for(int i=0; i<tiles_x; i++){
                tile_info[i][j].index = rand() % (tiles->max_frame + 1);
                tile_info[i][j].flip = get_flip(rand()%4);
            }

        }
    }
    void generate_background(){
        tile_info = new TileInfo*[tiles_x];
        unsigned int index=-1, flip;
        for(unsigned int i=0; i<tiles_x; i++){
            tile_info[i] = new TileInfo[tiles_y];
            for(unsigned int j=0; j<tiles_y; j++){
                if(randomize){
                    index = rand();
                    flip = rand()%4;
                }
                else{
                    index++;
                    flip = 0;
                }
                index %= tiles->max_frame + 1;
                tile_info[i][j].index = index;
                tile_info[i][j].flip = get_flip(flip);
            }
        }
    }
    SDL_RendererFlip get_flip(int in){
        switch(in){
            case 0:
                return SDL_FLIP_NONE;
            case 1:
                return SDL_FLIP_HORIZONTAL;
            case 2:
                return SDL_FLIP_VERTICAL;
            case 3:
                return (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
            default:
                return SDL_FLIP_NONE;
        }
    }
};
#endif
