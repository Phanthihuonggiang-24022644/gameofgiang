#include "Bullet.h"

Bullet::Bullet(int startX, int startY, int dirX, int dirY, SDL_Texture* tex) {
        x = startX; // vị trí ban đầu
        y = startY;
        dx = dirX; // hướng bay
        dy = dirY;
        speed = 4;
        active = true;
        texture = tex;
        rect = {x, y, 20, 20}; // vùng hiển thị
    }


void Bullet::move() {
        x += dx * speed; // cập nhật tọa độ
        y += dy * speed;
        rect.x = x; // đồng bộ lại hình vẽ
        rect.y = y;
        // nếu đạn ra khỏi màn hình => tắt
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE ||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false;
        }
    }
void Bullet::render(SDL_Renderer* renderer) {
    if (active && texture) {
        double angle = 0.0;
        // xác định góc xoay theo hướng di chuyển
        if (dx == 0 && dy < 0) angle = 0.0;         // UP
        else if (dx == 0 && dy > 0) angle = 180.0;  // DOWN
        else if (dx < 0 && dy == 0) angle = 270.0;  // LEFT
        else if (dx > 0 && dy == 0) angle = 90.0;   // RIGHT

        // renderer : bộ vẽ SDl, texture: ảnh, null : vẽ toàn bộ, &rect : vị trí
        // angle : góc xoay, null : tâm xoay là tâm texture, SDL_FLIP_NONE : không lật
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }
}
