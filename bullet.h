#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>
#include "constant.h"
#include "wall.h"
class Bullet
{
    public:
    int x, y;
    int dx, dy;
    int speed;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Texture* tex);
    void move();
    void render(SDL_Renderer* renderer);

};

#endif // BULLET_H
