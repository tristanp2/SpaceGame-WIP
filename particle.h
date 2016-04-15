#ifndef PARTICLE_H
#define PARTICLE_H

#include <cstdlib>
#include <list>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "sprite.h"
#include "point.h"

struct Texture{
    int refcount;
    Texture(SDL_Texture *tex){
        texture = tex;
        refcount = 1;
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
    FadeParticle(SDL_Surface* surf, SDL_Renderer* r,int pos_x, int pos_y, Vector2d velocity, int lifetime){
        this->r = r;
        tex = new Texture(SDL_CreateTextureFromSurface(r, surf));
        this->lifetime = lifetime;
        SDL_SetTextureAlphaMod(tex->get_tex(),255);
        draw_rect.w = surf->w;
        draw_rect.h = surf->h;
        pos.x = pos_x;
        pos.y = pos_y;
        rot_v = rand()%360;
        rotation = 0;
        this->velocity = velocity;
        elapsed = 0;
    }
    FadeParticle(const FadeParticle &other){
        r = other.r;
        tex = other.tex;
        tex->refcount++;
        draw_rect = other.draw_rect;
        velocity = other.velocity;
        pos = other.pos;
        lifetime = other.lifetime; 
        elapsed = other.elapsed;
    }
    ~FadeParticle(){
        if(tex->refcount <= 1){
            delete tex;
        }
        else tex->refcount--;
    }

    void render(){
        if(!is_dead()){
            SDL_RenderCopyEx(r, tex->get_tex(), NULL, &draw_rect, rotation, NULL, SDL_FLIP_NONE); 
        }
    }
    bool is_dead(){
        if(elapsed >= lifetime)   return true;
        return false;
    }
    void update(int delta_ms, Vector2d offset){
        if(is_dead()) return;
        double delta_s = delta_ms / 1000.0;
        rotation = (rotation + rot_v)%360;
        pos = pos + velocity*delta_s;
        draw_rect.x = pos.x - draw_rect.w/2 - offset.x;
        draw_rect.y = pos.y - draw_rect.h/2 - offset.y;
        if(elapsed < lifetime)  elapsed += delta_ms;
        set_alpha(255 - 255*((double)elapsed/lifetime));
    }

    Point get_pos(){
        Point point(pos.x, pos.y);
        return point;
    }
    void set_alpha(Uint8 alpha){
        SDL_SetTextureAlphaMod(tex->get_tex(), alpha);
    }
private:
    Texture* tex;
    int rot_v, rotation;
    Vector2d pos;
    SDL_Renderer* r;
    SDL_Rect draw_rect;
    int lifetime, elapsed;
};

class ParticleGenerator{
public:
    ParticleGenerator(){
        sprite_sheet = NULL;
    }
    ParticleGenerator(Sprite* sprite, Vector2d velocity, Vector2d direction, Point pos, int max_lifetime, int max_particles, SDL_Renderer* r){
        sprite_sheet = sprite;
        this->pos = pos;
        this->max_particles = max_particles;
        this->max_lifetime = max_lifetime;
        this->r = r;
        buffer = new char[80];
    }
    ~ParticleGenerator(){
        delete[] buffer;    //This should be fine because the copy constructor won't be used
    }
    void update(int delta_ms, Vector2d velocity, Vector2d direction, Point pos, Vector2d offset, bool generate){
        list<FadeParticle>::iterator it;
        sprintf(buffer, "%d , %d",pos.x, pos.y);
        if(particle_list.size() < max_particles and generate){
            generate_particles(pos, velocity, direction);
        }
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
            it->update(delta_ms, offset);
            if(it->is_dead()){
                it = particle_list.erase(it);
            }
        }
    }
    void draw(){
        list<FadeParticle>::iterator it;
        for(it = particle_list.begin(); it != particle_list.end(); ++it){
            if(!it->is_dead())
                it->render();
        }
        it = particle_list.begin();
        //stringRGBA(r, 10,10, buffer, 255,255,255,255);
    }
private:
    Sprite* sprite_sheet;
    char* buffer;
    SDL_Renderer* r;
    Point pos;
    list<FadeParticle>  particle_list;
    int max_particles, max_lifetime;

    void generate_particles(Point pos, Vector2d velocity, Vector2d direction){
        Vector2d new_velocity;
        pos.x -= direction.x*25;
        pos.y -= direction.y*25;
        int frame;
        int sign_x = rand()%2 - 1;
        int sign_y = rand()%2 - 1;
        Vector2d perp(((sign_x!=0)? sign_x: 1)*direction.y*(rand()%50 + 10),((sign_y!=0)? sign_y: 1)*direction.x*(rand()%50 + 10));
        for(int i = particle_list.size(); i < max_particles; i++){
                frame = rand() % (sprite_sheet->max_frame + 1);
                new_velocity = -200*direction + perp + velocity;
                particle_list.push_back(FadeParticle((*sprite_sheet)[frame], r, pos.x, pos.y, new_velocity, rand()%max_lifetime + 100));
        }
    }
};
#endif
