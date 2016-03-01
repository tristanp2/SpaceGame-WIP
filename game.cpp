#include <iostream>
#include <cmath>
#include <cstdlib>
#include <list>
#include <SDL2/SDL.h>

#include "vector2d.h"
#include "point.h"
#include "enumeration.h"
#include "gameobject.h"
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
    Sprite *player_sprite, *back_tile, *bullet;
    list<GameObject> object_list;
    list<Effect> effect_list;
    Point screen_offset;
    BackGround background;
    GameCanvas(){
        srand(time(NULL));
        load_resources();
        object_list.push_back(GameObject(enum_player, player_sprite, 2,Vector2d(400,300),false,Vector2d(0,0),180,0,100,true));
        player = &object_list.front();
        background = BackGround(back_tile,true,CANVAS_WIDTH,CANVAS_HEIGHT);
    }
    ~GameCanvas(){
    }
            
    void load_resources(){
        player_sprite = new Sprite("./resources/spaceship.bmp",2,3,1);
        back_tile = new Sprite("./resources/backtile.bmp",7,8,1);
        bullet = new Sprite("./resources/bullet.bmp",5,6,1);
    }

    void init_game(){
    }

    GameState frame_loop(SDL_Renderer* r, SDL_Window* window){
        init_game();
        unsigned int last_frame = SDL_GetTicks();
        unsigned int frame_t = 0;
        refire = 0;
        firing = false;
        while(1){
            unsigned int current_frame=SDL_GetTicks();
            unsigned int delta_t=current_frame - last_frame;
            refire+=delta_t;
            frame_t+=delta_t;
            if(firing and refire>fire_rate) fire_bullet();

            update_objects(delta_t);
            screen_offset.x = player->pos.x - 400;    //using center as 0,0
            screen_offset.y = player->pos.y - 300;
            if(frame_t > 32){
                draw_objects(r,window);
                frame_t=0;
            }
            delete_objects();
            //check_collisions();
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
            last_frame=current_frame;
        }
    }
    void draw_objects(SDL_Renderer *r, SDL_Window *w){
        SDL_RenderClear(r);
        background.draw(r);
        list<GameObject>::iterator it=object_list.begin();
        ++it; //skip past player. need to draw on top of bullets
        for(; it!=object_list.end(); ++it){
            (*it).draw(r, w);
        }
        object_list.front().draw(r, w);
        for(list<Effect>::iterator e_it=effect_list.begin(); e_it!=effect_list.end(); ++e_it){
            (*e_it).draw(r, w);
        }
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
    }
    //Delete the objects that have gone out of the screen
    void delete_objects(){
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
        int new_scale, num_objects;
        Vector2d old_pos;
    }
        
private:
    bool firing;
    int refire;
    static const int fire_rate=200;    //ms/bullet
    void handle_key_down(SDL_Keycode key){
        if(key == SDLK_RIGHT){
            player->raccel=360;
        }
        else if(key == SDLK_LEFT){
            player->raccel=-360;
        }
        else if(key == SDLK_SPACE){
            firing=true; 
        }
        else if(key == SDLK_LSHIFT){
            player->animated=true;
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
            player->set_frame(0);
        }
    }
    //Move to a ship class at some point
    void fire_bullet(){
        GameObject obj(enum_bullet,bullet,2,player->pos + player->direction*17,
                true,350*player->direction,player->rotation,0,100,false);
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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    
    GameCanvas canvas;
    canvas.frame_loop(renderer, window);
    return 0;
}
