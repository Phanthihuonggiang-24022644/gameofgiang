#include "playertank.h"
#include <algorithm>

PlayerTank::PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* tex, SDL_Texture* bulletTex) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
        texture = tex;
        bulletTexture = bulletTex;
    }


void PlayerTank::move(int dx, int dy, const std::vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;
        dirX = dx;
        dirY = dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        // kiểm tra đụng tường : duyệt qua tất cả các wall nếu vị trí mới và vị trí tường trùng thì return
        for (const auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
                return;
            }
        }

        // không đi quá bên trái, bên phải, đỉnh, đáy (trừ ra TILE_SIZE * 2 để tính cả chiều rộng xe tăng)
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }
// bắn đạn tương đương với Bullet newBullet()...
void PlayerTank::shoot() {
       Bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);

    }
// duyệt qua từng viên đạn, cho đạn di chuyển
    //remove_if(...): tìm tất cả các viên đạn mà !b.active (không còn hoạt động xóa khỏi vector bullet

void PlayerTank::updateBullets() {
        for (auto& bullet : Bullets) bullet.move();
        Bullets.erase(remove_if(Bullets.begin(), Bullets.end(),
                                [](Bullet& b) { return !b.active; }), Bullets.end());
    }
// vẽ xe tăng có xoay hướng
    void PlayerTank::render(SDL_Renderer* renderer) {
    if (texture) {
        double angle = 0.0;
        if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
        else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
        else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
        else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }
// vẽ đạn
    for (auto& bullet : Bullets) bullet.render(renderer);
}
