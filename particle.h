#ifndef PARTICLE_H
#define PARTICLE_H

#include <cstdlib>
#include <list>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "sprite.h"
#include "point.h"

struct Texture{
    int ref_count;
    Texture(SDL_Texture *tex){
        texture = tex;
        ref_count = 1;
    }
    SDL_Texture* get_tex(){
        return texture;
    }
private:
    SDL_Texture *texture;
};
class FadeParticle{
public:
    Vector2d velocity;
    FadeParticle(SDL_Surface* surf, SDL_Renderer* r,int pos_x, int pos_y, Vector2d velocity, int max_frame, int ms_per_frame){
        this->r = r;
        tex = new Texture(SDL_CreateTextureFromSurface(r, surf));
        SDL_SetTextureAlphaMod(tex->get_tex(),255);
        draw_rect.w = surf->w;
        draw_rect.h = surf->h;
        pos.x = pos_x;
        pos.y = pos_y;
        this->velocity = velocity;
        frame = 0;
        buffer = new char[80];
        frame_t = ms_per_frame;
        elapsed = 0;
        this->max_frame = max_frame;
    }
    FadeParticle(const FadeParticle &other){
        r = other.r;
        tex = other.tex;
        tex->ref_count++;
        draw_rect = other.draw_rect;
        velocity = other.velocity;
        frame = other.frame;
        max_frame = other.max_frame;
        elapsed = other.elapsed;
        frame_t = other.frame_t;
    }
    void render(){
        if(!is_dead()){
            SDL_RenderCopy(r, tex->get_tex(), NULL, &draw_rect); 
        }
    }
    bool is_dead(){
        if(frame > max_frame)   return true;
        return false;
    }
    void update(int delta_ms, Vector2d offset){
        if(is_dead()) return;
        double delta_s = delta_ms / 1000.0;
        pos = pos + velocity*delta_s;
        draw_rect.x = (int)pos.x - (int)offset.x;
        draw_rect.y = pos.y - offset.y;
        elapsed += delta_ms;
        if(elapsed > frame_t){
            elapsed = 0;
            frame++;
            int new_alpha = 255 - 255*frame/(double)max_frame;
            set_alpha(new_alpha);
        }
    }
    Point get_pos(){
        Point point(pos.x, pos.y);
        return point;
    }
    void set_alpha(Uint8 alpha){
//        SDL_SetTextureAlphaMod(tex, alpha);
    }
private:
    Texture* tex;
    char* buffer;
    Vector2d pos;
    SDL_Renderer* r;
    SDL_Rect draw_rect;
    int frame, max_frame, elapsed, frame_t;
};

class ParticleGenerator{
public:
    ParticleGenerator(){
        sprite_sheet = NULL;
    }
    ParticleGenerator(Sprite* sprite, Vector2d velocity, Vector2d direction, Point pos, int ms_per_frame, int max_particles, SDL_Renderer* r){
        sprite_sheet = sprite;
        this->pos = pos;
        this->max_particles = max_particles;
        this->ms_per_frame = ms_per_frame;
        this->r = r;
        buffer = new char[80];
    }
    void update(int delta_ms, Vector2d velocity, Vector2d direction, Point pos, Vector2d offset, bool generate){
        list<FadeParticle>::iterator it;
        sprintf(buffer, "%d , %d",pos.x, pos.y);
        if(particle_list.size() < max_particles and generate){
            generate_particles(pos, velocity);
        }
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
            if(!it->is_dead()){
                it->update(delta_ms, offset);
            }
            else{
                it = particle_list.erase(it);
            }
        }
    }
    void draw(){
        list<FadeParticle>::iterator it;
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
     //       if(!it->is_dead())
                it->render();
        }
        it = particle_list.begin();
        stringRGBA(r, 10,10, buffer, 255,255,255,255);
    }
private:
    Sprite* sprite_sheet;
    char* buffer;
    SDL_Renderer* r;
    Point pos;
    list<FadeParticle>  particle_list;
    int max_particles, ms_per_frame;

    void generate_particles(Point pos, Vector2d velocity){
        Vector2d new_velocity;
        int frame;
        for(int i = particle_list.size(); i < max_particles; i++){
                frame = rand() % (sprite_sheet->max_frame + 1);
                new_velocity = Vector2d(1,1);
                cout<<pos<<endl;
                particle_list.push_back(FadeParticle((*sprite_sheet)[frame], r, pos.x, pos.y, new_velocity, 10, 50));
        }
    }
};
#endif
