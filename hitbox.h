#ifndef HITBOX_H
#define HITBOX_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <SDL2/SDL.h>

using namespace std;
class HitBox{
public:
    int num_points;
    HitBox(){
        base_point=NULL;
        rot_point=NULL;
    } 
    //Dist represents distance points are from center
    //If num_points == 0, then hitbox is a circle
    HitBox(int rotation, int num_points, SDL_Point draw_center, int dist){
        this->current_rotation = 0;
        this->num_points = num_points;
        this->dist = dist;
        this->draw_center=draw_center;
        base_point = NULL;
        rot_point = NULL;
        gen_points();
        do_rotation(rotation);
    }
    HitBox(int rotation, int num_points, SDL_Point draw_center, SDL_Point* point){
        this->current_rotation = 0;
        this->num_points = num_points;
        this->dist = dist;
        this->draw_center=draw_center;
        base_point = point;
        rot_point = NULL;
        do_rotation(rotation);
    }
    HitBox(const HitBox& other){
        current_rotation = other.current_rotation;
        num_points = other.num_points;
        dist = other.dist;
        draw_center = other.draw_center;
        if(other.base_point!=NULL){
            base_point = new SDL_Point[num_points];
            for(int i=0;i<num_points;i++) base_point[i] = other.base_point[i];
        }
        else base_point = NULL;

        if(other.rot_point!=NULL){
            rot_point = new SDL_Point[num_points];
            for(int i=0;i<num_points;i++) rot_point[i] = other.rot_point[i];
        }
        else rot_point = NULL;
    }

    HitBox& operator=(const HitBox& other){
        current_rotation = other.current_rotation;
        num_points = other.num_points;
        dist = other.dist;
        draw_center = other.draw_center;

        if(base_point!=NULL){
            delete[] base_point;  
            base_point = NULL;
        }
        if(rot_point!=NULL){
            delete[] rot_point;  
            rot_point = NULL;
        }

        if(other.base_point!=NULL){
            base_point = new SDL_Point[num_points];
            for(int i=0;i<num_points;i++) base_point[i] = other.base_point[i];
        }

        if(other.rot_point!=NULL){
            rot_point = new SDL_Point[num_points];
            for(int i=0;i<num_points;i++) rot_point[i] = other.rot_point[i];
        }

        return *this;
    }

    ~HitBox(){
        if(base_point!=NULL){
            delete[] base_point;
        }
        if(rot_point!=NULL){
            delete[] rot_point;
        }
    }
    SDL_Point* get_points(){
        if(num_points == 0) return NULL;         //Shouldn't be asking for the points on a circle

        SDL_Point* point = new SDL_Point[num_points];
        for(int i=0;i<num_points;i++){
            point[i].x = rot_point[i].x + draw_center.x;
            point[i].y = rot_point[i].y + draw_center.y;
        }
        return point;
    }
    bool is_in_box(SDL_Point p){
        if(num_points == 0){
            SDL_Point p2 = {draw_center.x - p.x, draw_center.y - p.y};
            double distance = sqrt(p2.x*p2.x + p2.y*p2.y);
            if(distance <= dist)
                return true;
        }
        SDL_Point* point=this->get_points();
       
        //The following code is based on black magic
        //It was found here: 
        //      www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
        bool c=false;
        for(int i=0, j=num_points-1; i < num_points; j=i++){
            if(((point[i].y>p.y) != (point[j].y>p.y)) and 
                    (p.x<(point[j].x-point[i].x)*(p.y-point[i].y)/
                     (point[j].y-point[i].y) + point[i].x))
                c = !c;
        }
        delete[] point;
        return c;
    }
    void update(int rotation, int x, int y){
        if(rotation != current_rotation)    do_rotation(rotation);

        draw_center.x = x;
        draw_center.y = y;
    }

    
private:
    SDL_Point *base_point, *rot_point, draw_center;
    int dist, current_rotation;

    void do_rotation(int rotation){
        if(num_points == 0) return;
        if(rot_point==NULL) rot_point = new SDL_Point[num_points];

        double rot_matrix[2][2];
        double rad = rotation*M_PI/180; 
        double sine = sin(rad), cosine = cos(rad);

        rot_matrix[0][0] = cosine;
        rot_matrix[0][1] = sine;
        rot_matrix[1][0] = -sine;
        rot_matrix[1][1] = cosine;

        for(int i=0;i<num_points;i++){
            rot_point[i].x = base_point[i].x*rot_matrix[0][0] + base_point[i].y*rot_matrix[1][0];
            rot_point[i].y = base_point[i].x*rot_matrix[0][1] + base_point[i].y*rot_matrix[1][1];
        }
        current_rotation = rotation;
    }

    void gen_points(){
        int angle_deg;
        double angle_rad;
        base_point = NULL;
        if(num_points>2){
            base_point = new SDL_Point[num_points];
            if(num_points%2==0)
                angle_deg = 45;
            else
                angle_deg = 90;

            for(int i=0;i<num_points;i++){
                angle_rad = (double)angle_deg*M_PI/180.0;;
                base_point[i].x = cos(angle_rad)*dist;
                base_point[i].y = sin(angle_rad)*dist;
                angle_deg = (angle_deg + 360/num_points)%360;
            }
        }
    }
};   

#endif
