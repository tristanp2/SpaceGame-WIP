#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "vector2d.h"
#include "point.h"
#include "enumeration.h"
#include "gameobject.h"
#include "particle.h"
#include "sprite.h"
#include "effect.h"
#include "background.h"

using namespace std;

static const int WINDOW_WIDTH=800;
static const int WINDOW_HEIGHT=600;

class GameCanvas{
public:
    static const int CANVAS_WIDTH=800;
    static const int CANVAS_HEIGHT=600;
    GameObject* player;
    Sprite *player_sprite, *back_tile, *bullet, *particles, *explosion, *asteroid;
    char *buffer, *score;
    list<GameObject> object_list;
    list<Effect> effect_list;
    Vector2d screen_offset;
    BackGround background;
    ParticleGenerator generator;
    GameCanvas(){
        srand(time(NULL));
        load_resources();
        object_list.push_back(GameObject(enum_player, player_sprite, 2, Vector2d(400,300), false, Vector2d(0,0), 270,0,100, true));
        player = &object_list.front();
    }
    ~GameCanvas(){
    }
            
    void load_resources(){
        player_sprite = new Sprite("./resources/spaceship.bmp",2,3,1);
        asteroid = new Sprite("./resources/asteroid.bmp", 0,1,1);
        back_tile = new Sprite("./resources/backtile.bmp",7,8,1);
        bullet = new Sprite("./resources/bullet.bmp",5,6,1);
        explosion = new Sprite("./resources/explosion.bmp", 6,7,1);
        particles = new Sprite("./resources/particles.bmp",5,6,1);
    }
    void init_game(){
    }
    GameState frame_loop(SDL_Renderer* r, SDL_Window* window){
        init_game();
        particles_active = false;
        background = BackGround(SDL_GetWindowPixelFormat(window), CANVAS_WIDTH, CANVAS_HEIGHT, r);
        generator = ParticleGenerator(particles,Vector2d(0,0),Vector2d(0,0),Point(400,300), 500, 50, r);
        unsigned int last_frame = SDL_GetTicks();
        unsigned int frame_t = 0;
        int delta_spawn = 0;
        buffer = new char[80];
        score = new char[40];
        refire = 0;
        firing = false;
        while(1){
            unsigned int current_frame = SDL_GetTicks();
            unsigned int delta_t = current_frame - last_frame;
            refire += delta_t;
            frame_t += delta_t;
            delta_spawn += delta_t;
            if(firing and refire>fire_rate) fire_bullet();

            update_objects(delta_t);
            screen_offset.x = player->pos.x - 400;    //using center as 0,0
            screen_offset.y = player->pos.y - 300;
            if(frame_t > 32){
                draw_objects(r,window);
                frame_t=0;
            }
            delete_objects();
            delta_spawn = spawn_objects(delta_spawn);
            check_collisions();
            SDL_Event e;
            while(SDL_PollEvent(&e)){
                switch(e.type){
                    case(SDL_QUIT):
                        return enum_quit;
                    case(SDL_KEYDOWN):
                        handle_key_down(e.key.keysym.sym);
                        break;
                    case(SDL_KEYUP):
                        handle_key_up(e.key.keysym.sym);
                        break;
                    default:
                        break;
                }
            }
            spawn_rate = (abs(screen_offset.x) + abs(screen_offset.y)) / 300;
            sprintf(score, "SpawnRate: %d", spawn_rate);
            sprintf(buffer, "Num_Objects: %d", (int)object_list.size());
            last_frame=current_frame;
        }
    }
    void draw_objects(SDL_Renderer *r, SDL_Window *w){
        SDL_RenderClear(r);
        background.draw();
        generator.draw();
        list<GameObject>::iterator it=object_list.begin();
        ++it; //skip past player. need to draw on top of bullets
        for(; it!=object_list.end(); ++it){
            (*it).draw(r, w);
        }
        object_list.front().draw(r, w);
        for(list<Effect>::iterator e_it=effect_list.begin(); e_it!=effect_list.end(); ++e_it){
            (*e_it).draw(r, w);
        }
        stringRGBA(r, 10,10, buffer, 255, 255, 255, 255);
        stringRGBA(r, 10,50, score, 255, 255, 255, 255);
        SDL_RenderPresent(r);
    }
    void update_objects(int delta_ms){
        for(list<GameObject>::iterator it=object_list.begin(); it!=object_list.end(); ++it){
            (*it).update(delta_ms, screen_offset);
        }
        for(list<Effect>::iterator it=effect_list.begin(); it!=effect_list.end(); ++it){
            (*it).update(delta_ms);
        }
        background.update(screen_offset.x, screen_offset.y);
        generator.update(delta_ms, player->velocity,player->direction, Point(player->pos.x, player->pos.y), screen_offset, particles_active);
    }
    int spawn_objects(int dt){
        if(dt>respawn){
            GameObject obj;
            int rv=rand()%121+20;
            int sidex=rand()%2;
            int x=-100*sidex + 900*abs(sidex-1); //x coord for asteroid spawn. either spawns at -100 or +900 (100 pixels outside of the screen)
            int y=rand()%600;   //y coord for asteroid spawn
            Vector2d pos(x,y);
            Vector2d end_point(300,rand()%400+100); //Get some random point from center x of screen so
                                                    //to set up vector for asteroid direction
            Vector2d dir = end_point - pos;
            dir=dir.unit_vector();
            obj=GameObject(enum_asteroid,asteroid,rand()%3 + 1,pos,true,rv*dir,0,rand()%180 - 360,0,false);
            object_list.push_back(obj);
            respawn = 500 + rand()%5000/spawn_rate; 
            return 0;
        }    
        else return dt;
        return 0;   //Only to stop compiler from complaining
    }
    //Delete the objects that have gone out of the screen
    void delete_objects(){
       Point pos;
       for(list<GameObject>::iterator it = object_list.begin(); it != object_list.end(); ++it){
           pos = it->get_draw_coords();
           if(pos.x < -CANVAS_WIDTH or pos.x > CANVAS_WIDTH*2 or pos.y < -CANVAS_HEIGHT or pos.y > CANVAS_HEIGHT*2){
               it = object_list.erase(it);
           }
       }
    }
    void check_collisions(){
        list<GameObject>::iterator it1,it2;
        for(it1=object_list.begin(); it1!=object_list.end(); ++it1){
            it2=it1;
            ++it2;
            for(;it2!=object_list.end(); ++it2){
                if((*it1).is_collided(*it2)){
                    handle_collision(*it1,*it2);
                    object_list.erase(it2);
                    it1=object_list.erase(it1);
                    break;
                }
            }
        }
    }
    void handle_collision(GameObject obj1, GameObject obj2){
        int new_scale, num_objects = 0;
        Vector2d old_pos;
        if(obj1.type==enum_asteroid and obj2.type==enum_asteroid){
            if(obj1.scale < obj2.scale){
                new_scale = obj2.scale - 1;   
                old_pos = obj2.pos;
            }
            else{
                new_scale = obj1.scale - 1;
                old_pos = obj1.pos;
            }
            num_objects = obj2.scale + obj1.scale - 1;
        }        
        else if(obj1.type==enum_bullet or obj2.type==enum_bullet){ //At the moment it should be impossible for two bullets to collide
            GameObject other;
            if(obj1.type==enum_bullet){
                other=obj2;
            }
            else{
                other=obj1;
            }
            new_scale = other.scale - 1;
            num_objects = new_scale + 1;
            old_pos = other.pos;
        }
        else if(obj1.type == enum_player or obj2.type == enum_player){
            if(obj1.type != enum_bullet and obj2.type!=enum_bullet){ //bullets spawn in the player's hitbox
                effect_list.push_front(Effect(explosion, 100, false, 3, player->pos));
                dead = true;
            }
            return;
        }            

        GameObject new_obj;
        Vector2d new_dir, new_pos;
        double angle_rad;
        int angle_deg;
        for(int i=0; i<num_objects; i++){
            angle_deg = (i*360/num_objects + rand()%45)%360;
            angle_rad = angle_deg*M_PI/180;
            new_dir = Vector2d(cos(angle_rad),sin(angle_rad));
            new_pos = new_scale*15*new_dir + old_pos;
            object_list.push_back(GameObject(enum_asteroid,asteroid,new_scale,new_pos,false,(rand()%121 + 20)*new_dir,angle_deg,rand()%180 -360,0,false));
        }
        effect_list.push_back(Effect(explosion, 100, false, new_scale + 1, old_pos));
    }
        
private:
    bool firing, particles_active, dead;
    int refire, spawn_rate, respawn;
    static const int fire_rate=200;    //ms/bullet
    void handle_key_down(SDL_Keycode key){
        if(key == SDLK_RIGHT){
            player->raccel = 360;
        }
        else if(key == SDLK_LEFT){
            player->raccel = -360;
        }
        else if(key == SDLK_SPACE){
            firing=true; 
        }
        else if(key == SDLK_LSHIFT){
            player->animated = true;
            particles_active = true;
            player->accel=60;
        }
    }
    void handle_key_up(SDL_Keycode key){
        if(key == SDLK_LEFT){
            player->raccel=0;
        }
        else if(key == SDLK_RIGHT){
            player->raccel=0;
        }
        else if(key == SDLK_SPACE){
            firing=false;
        }
        else if(key == SDLK_LSHIFT){
            player->accel=0;
            player->animated=false;
            particles_active = false;
            player->set_frame(0);
        }
    }
    //Move to a ship class at some point
    void fire_bullet(){
        GameObject obj(enum_bullet,bullet,2,player->pos + player->direction*17,
                true, 350*player->direction + player->velocity,player->rotation,0,100,false);
        object_list.push_back(obj); 
        refire=0;
    }
};

int main(){
    SDL_Window* window=SDL_CreateWindow("Space Game",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer=SDL_CreateRenderer(window,-1,0);

    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    GameCanvas canvas;
    canvas.frame_loop(renderer, window);
    return 0;
}
