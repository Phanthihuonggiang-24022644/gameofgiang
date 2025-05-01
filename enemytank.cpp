#include "enemytank.h"
#include <algorithm>

EnemyTank::EnemyTank(int startX, int startY, SDL_Texture* tex, SDL_Texture* bulletTex, const std::vector<Wall>& walls) {
    x = startX;
    y = startY;
    dirX = 0; // gán vị trí ban đầu cho xe tăng địch
    dirY = 1; // mặc định hướng ban đầu là hướng xuống
    moveDelay = 15;
    shootDelay = 5;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    active = true;
    texture = tex;
    bulletTexture = bulletTex;
    // kiểm tra va chạm với tường khi sinh ra
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&rect, &wall.rect)) {
            active = false;
            break;
        }
    }
}

 void EnemyTank::move (const std::vector<Wall>& walls) {
    if (--moveDelay > 0) return; // Chỉ khi giảm về 0 thì enemy mới chọn hướng mới và di chuyển
    moveDelay = 15; //chờ 15 frame tiếp theo mới được đổi hướng tiếp

    int r = rand() % 4;
    if (r == 0) { dirX = 0; dirY = -5; } //up
    else if (r == 1) { dirX = 0; dirY = 5; } //down
    else if (r == 2) { dirX = -5; dirY = 0; } // left
    else if (r == 3) { dirX = 5; dirY = 0; } //right

    int newX = x + dirX;
    int newY = y + dirY; //Tính toán vị trí mới nếu di chuyển theo hướng vừa chọn
    SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE }; //Tạo SDL_Rect mới để kiểm tra va chạm
    //kiểm tra va chạm với tường
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) return;
    }
    // kiểm tra tránh đi ra ngoài màn hình
    if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
        newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }
}
void EnemyTank::shoot() {
        if (--shootDelay > 0) return; // giảm biến shootdelay sau mỗi lần được gọi, Nếu chưa về 0 thì không bắn, thoát hàm sớm.
        shootDelay = 20;
        Bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);
    }
void EnemyTank::updateBullets() {
        for (auto& bullet : Bullets) bullet.move();
        Bullets.erase(remove_if(Bullets.begin(), Bullets.end(),
                                [](Bullet& b) { return !b.active; }), Bullets.end());
    }
void EnemyTank::render(SDL_Renderer* renderer) {
    if (texture) {
        double angle = 0.0;
        if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
        else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
        else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
        else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }

    for (auto& bullet : Bullets) bullet.render(renderer);
}
