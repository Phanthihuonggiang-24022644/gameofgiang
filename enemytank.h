#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include <SDL.h>
#include "constant.h"
#include "wall.h"
#include "Bullet.h"
#include <vector>
#include <algorithm>
#include "playertank.h"

class EnemyTank
{
    public:
    int x, y, dirX, dirY, moveDelay, shootDelay;
    // moveDelay : delay giữa mỗi lần thay đổi hướng di chuyển
    // shootDelay : delay giữa mỗi lần bắn đạn
    SDL_Rect rect;
    bool active;
    std::vector<Bullet> Bullets;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture; // danh sách đạn đã bắn ra

    EnemyTank(int startX, int startY, SDL_Texture* tex, SDL_Texture* bulletTex, const std::vector<Wall>& walls);
    void move(const std::vector<Wall>& walls);
    void shoot();
    void updateBullets();
    void render(SDL_Renderer* renderer);
};

#endif // ENEMYTANK_H
