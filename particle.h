#ifndef PARTICLE_H
#define PARTICLE_H

#include <cstdlib>
#include <list>
#include <SDL2/SDL.h>
#include "sprite.h"
#include "point.h"

class FadeParticle{
public:
    FadeParticle(SDL_Surface* surf, SDL_Renderer* r,int pos_x, int pos_y, Vector2d velocity, int max_frame, int ms_per_frame){
        this->r = r;
        tex = SDL_CreateTextureFromSurface(r, surf);
        draw_rect.w = surf->w;
        draw_rect.h = surf->h;
        draw_rect.x = pos_x;
        draw_rect.y = pos_y;
        this->velocity = velocity;
        frame = 0;
        frame_t = ms_per_frame;
        elapsed = 0;
        this->max_frame = max_frame;
    }
    FadeParticle(const FadeParticle &other){
        r = other.r;
        Uint32 format;
        int access, w, h;
        SDL_QueryTexture(other.tex, &format, &access, &w, &h);
        tex = SDL_CreateTexture(r, format, access, w, h);
        draw_rect = other.draw_rect;
        velocity = other.velocity;
        frame = other.frame;
        max_frame = other.max_frame;
        elapsed = other.elapsed;
        frame_t = other.frame_t;
    }

    void render(){
        if(!is_dead())  SDL_RenderCopy(r, tex, NULL, &draw_rect); 
    }
    bool is_dead(){
        if(frame > max_frame)   return true;
        return false;
    }
    void update(int delta_ms){
        double delta_s = delta_ms / 1000.0;
        draw_rect.x += velocity.x * delta_s;
        draw_rect.y += velocity.y * delta_s;
        elapsed += delta_ms;
        if(elapsed > frame_t){
            elapsed = 0;
            frame++;
            set_alpha(255.0 - 255.0*frame/max_frame);
        }
    }
    void set_alpha(Uint8 alpha){
        SDL_SetTextureAlphaMod(tex, alpha);
    }
private:
    SDL_Texture* tex;
    SDL_Renderer* r;
    SDL_Rect draw_rect;
    Vector2d velocity;
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
        int frame;
        Vector2d new_velocity;
        for(int i=0; i < max_particles; i++){
            frame = rand() % (sprite->max_frame + 1);
            new_velocity = -1*direction*(rand()%(int)(velocity.length()/4 + 1)) + velocity;
            particle_list.push_back(FadeParticle((*sprite_sheet)[frame], r, pos.x, pos.y, new_velocity, 10, 100));
        }
    }
    void update(int delta_ms, Vector2d velocity, Vector2d direction, Point pos){
        list<FadeParticle>::iterator it;
        pos = pos;
        Vector2d new_velocity;
        int frame;
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
            if(!it->is_dead())
                it->update(delta_ms);
            else{
                it = particle_list.erase(it);
                frame = rand() % (sprite_sheet->max_frame + 1);
                new_velocity = -1*direction*(rand() % (int)(velocity.length()/4 + 1)) + velocity;
                particle_list.push_front(FadeParticle((*sprite_sheet)[frame], r, pos.x, pos.y, new_velocity, 10, 100));
            }
        }
    }
    void draw(){
        list<FadeParticle>::iterator it;
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
            it->render();
        }
    }
private:
    Sprite* sprite_sheet;
    SDL_Renderer* r;
    Point pos;
    list<FadeParticle>  particle_list;
    int max_particles, ms_per_frame;
};
#endif
