#ifndef WALL_H
#define WALL_H

#include <SDL.h>
#include "constant.h"


class Wall
{
    public:
     int x, y; // tọa độ của wall
    SDL_Rect rect; // đại diện cho texture trên màn hình
    bool active; // trạng thái có hiệu lực không?
    SDL_Texture* texture; // con trỏ trỏ tới hình ảnh của wall

    //hàm khởi tạo wall
    Wall(int startX, int startY, SDL_Texture* tex);
    void render(SDL_Renderer* renderer);
};

#endif // WALL_H
