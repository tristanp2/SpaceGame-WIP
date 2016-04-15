#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
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

static const int WINDOW_WIDTH=1200;
static const int WINDOW_HEIGHT=900;

class GameCanvas{
public:
    static const int CANVAS_WIDTH=1200;
    static const int CANVAS_HEIGHT=900;
    GameObject* player;
    Sprite *player_sprite, *back_tile, *bullet, *start, *quit, *particles, *explosion, *asteroid;
    char *buffer, *score;
    list<GameObject> object_list;
    list<Effect> effect_list;
    Vector2d screen_offset;
    Point ship_box[3], ast_box[7];
    BackGround background;
    ParticleGenerator generator;
    GameCanvas(){
        srand(time(NULL));
        load_resources();
        ship_box[0] = Point(7,0);
        ship_box[1] = Point(7,16);
        ship_box[2] = Point(26,8);
        ast_box[0] = Point(0,8);
        ast_box[1] = Point(3,18);
        ast_box[2] = Point(10,19);
        ast_box[3] = Point(20,12);
        ast_box[4] = Point(21,5);
        ast_box[5] = Point(16,0);
        ast_box[6] = Point(6,2);
    }
    ~GameCanvas(){
    }
    GameState show_menu(SDL_Renderer* r, SDL_Window* window){
        object_list.clear();
        menu = true;
        GameObject* button[2];
        object_list.push_back(GameObject(enum_misc, start, 2, Vector2d(CANVAS_WIDTH/2,CANVAS_HEIGHT/2), false, Vector2d(0,0), 0,0,0, false));
        button[0] = &(object_list.front());
        object_list.push_back(GameObject(enum_misc, quit, 2, Vector2d(CANVAS_WIDTH/2,CANVAS_HEIGHT/2 + 100), false, Vector2d(0,0), 0,0,0, false));
        button[1] = &(object_list.back());
        int index = 0;

        while(1){
            draw_objects(r,window);
            button[index]->set_frame(1);
            button[(index+1)%2]->set_frame(0);
            SDL_Event e;
            while(SDL_PollEvent(&e)){
                SDL_Keycode key = e.key.keysym.sym;
                switch(e.type){
                    case(SDL_QUIT):
                        return enum_quit;
                    case(SDL_KEYDOWN):
                        if(key == SDLK_RETURN){
                            if(index==0){
                                object_list.clear();
                                return enum_play;
                            }
                            else{
                                return enum_quit;
                            }
                        }
                        if(key == SDLK_DOWN){
                            index = abs(index + 1);
                        }
                        else if(key == SDLK_UP){
                            index = abs(index - 1);
                        }
                        index %= 2;
                        break;
                    case(SDL_KEYUP):
                        break;
                    default:
                        break;
        object_list.front().set_center(14,8);
        object_list.front().make_hitbox(ship_box,3);
                }
            }
        }
    }
            
    void load_resources(){
        player_sprite = new Sprite("./resources/spaceship.bmp",2,3,1);
        asteroid = new Sprite("./resources/asteroid.bmp", 0,1,1);
        bullet = new Sprite("./resources/bullet.bmp",5,6,1);
        start = new Sprite("./resources/startbutton.bmp", 1,2,1);
        quit = new Sprite("./resources/quitbutton.bmp", 1,2,1);
        explosion = new Sprite("./resources/explosion.bmp", 6,7,1);
        particles = new Sprite("./resources/particles.bmp",5,6,1);
    }
    void init_game(){
        object_list.push_back(GameObject(enum_player, player_sprite, 2, Vector2d(CANVAS_WIDTH/2,CANVAS_HEIGHT/2), false, Vector2d(0,0), 270,0,100, true));
        player = &object_list.front();
        object_list.front().make_hitbox(ship_box,3);
    }
    GameState frame_loop(SDL_Renderer* r, SDL_Window* window){
        init_game();
        menu = false;
        particles_active = false;
        background = BackGround(SDL_GetWindowPixelFormat(window), CANVAS_WIDTH, CANVAS_HEIGHT, r);
        generator = ParticleGenerator(particles,Vector2d(0,0),Vector2d(0,0),Point(CANVAS_WIDTH/2,CANVAS_HEIGHT/2), 500, 50, r);
        unsigned int last_frame = SDL_GetTicks();
        unsigned int frame_t = 0;
        int delta_spawn = 0;
        int death_timer = 3000;
        spawn_rate = 1;
        screen_offset = Vector2d(0,0);
        buffer = new char[80];
        refire = 0;
        firing = false;
        while(1){
            unsigned int current_frame = SDL_GetTicks();
            unsigned int delta_t = current_frame - last_frame;
            refire += delta_t;
            frame_t += delta_t;
            delta_spawn += delta_t;
            if(firing and refire>fire_rate) fire_bullet();
            if(dead and death_timer>2000)
                death_timer = 2000;
            if(dead and death_timer >= 0 and death_timer <= 3000)   death_timer -= delta_t;
            if(dead and death_timer<0){
                sprintf(buffer, "Your score was %d. Noice", spawn_rate*100);
                stringRGBA(r, 400,400, buffer, 255, 255, 255, 255);
                SDL_RenderPresent(r);
                sleep(3);
                object_list.clear();
                effect_list.clear();
                dead = false;
                return enum_dead;
            }
            if(!dead){
                screen_offset.x = player->pos.x - CANVAS_WIDTH/2;    //using center as 0,0
                screen_offset.y = player->pos.y - CANVAS_HEIGHT/2;
            }
            if(frame_t > 17){
                draw_objects(r,window);
                frame_t=0;
            }
            update_objects(delta_t);
            check_collisions();
            delete_objects();
            if(!dead)   delta_spawn = spawn_objects(delta_spawn);
            SDL_Event e;
            while(SDL_PollEvent(&e)){
                switch(e.type){
                    case(SDL_QUIT):
                        return enum_quit;
                    case(SDL_KEYDOWN):
                        if(dead) break;
                        handle_key_down(e.key.keysym.sym);
                        break;
                    case(SDL_KEYUP):
                        if(dead) break;
                        handle_key_up(e.key.keysym.sym);
                        break;
                    default:
                        break;
                }
            }
            spawn_rate = (abs(screen_offset.x) + abs(screen_offset.y)) / 300 + 1;
            sprintf(buffer, "Score: %d", spawn_rate*100);
            last_frame=current_frame;
        }
    }
    void draw_objects(SDL_Renderer *r, SDL_Window *w){
        SDL_RenderClear(r);
        if(!menu){
            background.draw();
            if(!dead)   generator.draw();
        }
        list<GameObject>::iterator it=object_list.begin();
        ++it; //skip past player. need to draw on top of bullets
        for(; it!=object_list.end(); ++it){
            (*it).draw(r, w);
        }
        if(!dead) object_list.front().draw(r, w);
        for(list<Effect>::iterator e_it=effect_list.begin(); e_it!=effect_list.end(); ++e_it){
            (*e_it).draw(r, w);
        }
        if(!menu)    stringRGBA(r, 10,10, buffer, 255, 255, 255, 255);
        SDL_RenderPresent(r);
    }
    void update_objects(int delta_ms){
        for(list<GameObject>::iterator it=object_list.begin(); it!=object_list.end(); ++it){
            (*it).update(delta_ms, screen_offset);
        }
        for(list<Effect>::iterator it=effect_list.begin(); it!=effect_list.end(); ++it){
            (*it).update(delta_ms, screen_offset);
        }
        background.update(screen_offset.x, screen_offset.y);
        generator.update(delta_ms, player->velocity,player->direction, Point(player->pos.x, player->pos.y), screen_offset, particles_active);
    }
    int spawn_objects(int dt){
        if(dt>respawn){
            GameObject obj;
            int rv=rand()%121+20;
            int x, y, side = rand()%4;
            Vector2d direction;
            if(player->velocity.length() <= 200){
                switch(side){
                    case 0: //top
                        x = CANVAS_WIDTH/2  - (rand()%CANVAS_WIDTH/4 - CANVAS_WIDTH/8);
                        y = -CANVAS_HEIGHT/2 - (rand()%CANVAS_HEIGHT/4);
                        break;
                    case 1: //right
                        x = CANVAS_WIDTH + CANVAS_WIDTH/2  + (rand()%CANVAS_WIDTH/4);
                        y = CANVAS_HEIGHT/2 - (rand()%CANVAS_HEIGHT/4 - CANVAS_WIDTH/8);
                        break;
                    case 2: //bottom
                        x = CANVAS_WIDTH/2  - (rand()%CANVAS_WIDTH/4 - CANVAS_WIDTH/8);
                        y = CANVAS_HEIGHT + CANVAS_HEIGHT/2 - (rand()%CANVAS_HEIGHT/4);
                        break;
                    case 3: //left
                        x = -CANVAS_WIDTH/2  - (rand()%CANVAS_WIDTH/4);
                        y = -CANVAS_HEIGHT/2 - (rand()%CANVAS_HEIGHT/4 - CANVAS_HEIGHT/8);
                        break;
                }
            }
            else{
                direction = player->velocity.unit_vector();
                int dist = CANVAS_HEIGHT/3 + CANVAS_WIDTH/3;
                x = direction.x*dist + (rand()%CANVAS_WIDTH/2 - CANVAS_WIDTH/4)*direction.y + CANVAS_WIDTH/2;
                y = direction.y*dist + (rand()%CANVAS_HEIGHT/2 - CANVAS_HEIGHT/4)*direction.x + CANVAS_HEIGHT/2;
            }
            Vector2d pos(x,y);
            Vector2d end_point(CANVAS_WIDTH/2 + rand()%200 - 100,CANVAS_HEIGHT/2 + rand()%200 - 100); //Get some random point near center of screen 
                                                    //to set up vector for asteroid direction
            Vector2d dir = end_point - pos;
            dir = rv*dir.unit_vector() + Vector2d(rand()%(rv/4) - rv/2, rand()%(rv/4) - rv/2);
            pos += screen_offset;
            obj=GameObject(enum_asteroid,asteroid,rand()%3 + 1,pos,true,dir,0,rand()%360 - 180,0,false);
            obj.make_hitbox(ast_box, 7);
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
        for(list<Effect>::iterator it=effect_list.begin(); it!=effect_list.end(); ++it){
            if((*it).done)
                it = effect_list.erase(it);
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
            new_pos = new_scale*50*new_dir + old_pos;
            object_list.push_back(GameObject(enum_asteroid,asteroid,new_scale,new_pos,false,(rand()%121 + 20)*new_dir,angle_deg,rand()%180 -360,0,false));
            object_list.back().make_hitbox(ast_box,7);
        }
        effect_list.push_back(Effect(explosion, 100, false, new_scale + 1, old_pos));
    }
        
private:
    bool firing, particles_active, dead, menu;
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
        GameObject obj(enum_bullet,bullet,2,player->pos + player->direction*27,
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
    GameState state=enum_menu;
    while(1){
        switch(state){
            case enum_quit:
                return 0;
            case enum_play:
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                state=canvas.frame_loop(renderer, window);
                break;
            case enum_dead:
            default:
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                state=canvas.show_menu(renderer,window);
                break;
        }
    }
    return 0;
}
