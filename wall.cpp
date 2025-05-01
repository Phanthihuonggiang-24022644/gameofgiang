#include "wall.h"

Wall::Wall(int startX, int startY, SDL_Texture* tex){
    x = startX;
        y = startY;
        active = true;
        rect = {x, y, TILE_SIZE , TILE_SIZE}; //tạo vùng vẽ cho SDL
        texture = tex; //lưu con trỏ texture để vẽ
}
void Wall::render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);// vẽ ảnh từ con trỏ renderer vào vị trí rect
        }
    }
