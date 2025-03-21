#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

class Wall {
public :
    int x, y;
    SDL_Rect rect;
    bool active;
    Wall(int startX, int startY){
    x= startX;
    y = startY;
    active = true;
    rect = {x,y,TILE_SIZE,TILE_SIZE};
    }

    void render(SDL_Renderer* renderer){
    if(active){
        SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
    }
};

class Bullet {
public:
    int x, y;
    int dx, dy;
    SDL_Rect rect;
    bool active;

    Bullet(int startX, int startY, int dirX, int dirY) {
        x = startX;
        y = startY;
        dx = dirX;
        dy = dirY;
        active = true;
        rect = {x, y, 10, 10};
    }
    void move() {
        x += dx;
        y += dy;;
        rect.x = x;
        rect.y = y;
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
                active = false;
            }
    }
    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer,0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }


};


class PlayerTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    vector<Bullet> bullets;

    PlayerTank (int startX, int startY, SDL_Renderer* renderer) {
    x= startX;
    y = startY;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = -1;
    texture = IMG_LoadTexture(renderer, "player.png");
    }

    ~PlayerTank() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

   SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;
        this ->dirX = dx;
        this ->dirY = dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for(int i = 0; i < walls.size(); i++){
            if(walls[i].active && SDL_HasIntersection(&newRect, &walls[i].rect)){
                return;
            }
        }

        if(newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
           newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2){
               x = newX;
               y = newY;
               rect.x = x;
               rect.y = y;
           }
    }
    void shoot() {
        bullets.push_back(Bullet(x + TILE_SIZE/2 - 5, y + TILE_SIZE/2 - 5,
                                 this -> dirX, this-> dirY));
    }
    void updateBullets() {
        for(auto &bullet : bullets){
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                     [](Bullet &b) { return !b.active; }), bullets.end());
    }
    void render(SDL_Renderer* renderer){
       if (texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
        for(auto &bullet : bullets) {
            bullet.render(renderer);
        }
    }

};


class Game {
public:
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    vector<Wall> walls;
    PlayerTank* player;

    Game() {
        running = true;
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
            running = false;
            return;
        }


        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }
        player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer);

        generateWalls();
    }
    ~Game() {
        delete player;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 1; i < MAP_HEIGHT - 1; ++i) {
            for (int j = 1; j < MAP_WIDTH - 1; ++j) {
                SDL_Rect tile = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &tile);
            }
        }
        for(int i = 0; i < walls.size();i++){
                walls[i].render(renderer);
        }
        player->render(renderer);
        SDL_RenderPresent(renderer);
    }

    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);
        }
    }
    void generateWalls() {
    for(int i = 3; i < MAP_HEIGHT - 3; i+=2){
        for(int j = 3; j < MAP_WIDTH - 3; j+=2){
            Wall w = Wall(j *TILE_SIZE, i * TILE_SIZE);
            walls.push_back(w);
        }
    }
    }
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_UP: player->move(0, -5, walls); break;
            case SDLK_DOWN: player->move(0, 5, walls); break;
            case SDLK_LEFT: player->move(-5, 0, walls); break;
            case SDLK_RIGHT: player->move(5, 0, walls); break;
            case SDLK_SPACE: player->shoot(); break;
            }
            }
        }
    }

    void update() {
        player->updateBullets();

        for (auto& bullet : player->bullets) {
            for(auto& wall : walls) {
                if(wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    wall.active = false;
                    bullet.active = false;
                    break;
                }
            }
        }
    }

};


int main(int argc, char* argv[]) {
    Game game;
    if (game.running) {
        game.run();
    }
    return 0;
}
