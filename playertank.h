#ifndef PLAYERTANK_H
#define PLAYERTANK_H


#include <SDL.h>
#include "constant.h"
#include "wall.h"
#include "Bullet.h"
#include <vector>
#include <algorithm>

class PlayerTank
{
    public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    std::vector<Bullet> Bullets; // danh sách đạn đang bắn

    // hàm khởi tạo
    PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* tex, SDL_Texture* bulletTex);
    void move(int dx, int dy, const std::vector<Wall>& walls);
    void shoot();
    void updateBullets();
    void render(SDL_Renderer* renderer);
};

#endif // PLAYERTANK_H
