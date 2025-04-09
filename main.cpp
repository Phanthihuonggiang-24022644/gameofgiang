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

SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Wall(int startX, int startY, SDL_Texture* tex) {
        x = startX;
        y = startY;
        active = true;
        rect = {x, y, TILE_SIZE *2 , TILE_SIZE * 2};
        texture = tex;
    }

    void render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class Bullet {
public:
    int x, y;
    int dx, dy;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Texture* tex) {
        x = startX;
        y = startY;
        dx = dirX;
        dy = dirY;
        active = true;
        texture = tex;
        rect = {x, y, 20, 20};
    }

    void move() {
        x += dx;
        y += dy;
        rect.x = x;
        rect.y = y;
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE ||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
    if (active && texture) {
        double angle = 0.0;
        if (dx == 0 && dy < 0) angle = 0.0;         // UP
        else if (dx == 0 && dy > 0) angle = 180.0;  // DOWN
        else if (dx < 0 && dy == 0) angle = 270.0;  // LEFT
        else if (dx > 0 && dy == 0) angle = 90.0;   // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }
}

};

class PlayerTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    vector<Bullet> bullets;

    PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* tex, SDL_Texture* bulletTex) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
        texture = tex;
        bulletTexture = bulletTex;
    }

    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;
        dirX = dx;
        dirY = dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for (const auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
                return;
            }
        }

        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void shoot() {
       bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);

    }

    void updateBullets() {
        for (auto& bullet : bullets) bullet.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
                                [](Bullet& b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
    if (texture) {
        double angle = 0.0;
        if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
        else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
        else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
        else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }

    for (auto& bullet : bullets) bullet.render(renderer);
}

};

class EnemyTank {
public:
    int x, y, dirX, dirY, moveDelay, shootDelay;
    SDL_Rect rect;
    bool active;
    vector<Bullet> bullets;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;

    EnemyTank(int startX, int startY, SDL_Texture* tex, SDL_Texture* bulletTex, const vector<Wall>& walls) {
    x = startX;
    y = startY;
    dirX = 0;
    dirY = 1;
    moveDelay = 15;
    shootDelay = 5;
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    active = true;
    texture = tex;
    bulletTexture = bulletTex;
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&rect, &wall.rect)) {
            active = false;
            break;
        }
    }
}


    void move(const vector<Wall>& walls) {
    if (--moveDelay > 0) return;
    moveDelay = 15;

    int r = rand() % 4;
    if (r == 0) { dirX = 0; dirY = -5; }
    else if (r == 1) { dirX = 0; dirY = 5; }
    else if (r == 2) { dirX = -5; dirY = 0; }
    else if (r == 3) { dirX = 5; dirY = 0; }

    int newX = x + dirX;
    int newY = y + dirY;
    SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE };
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) return;
    }

    if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
        newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }
}


    void shoot() {
        if (--shootDelay > 0) return;
        shootDelay = 20;
        bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);
    }

    void updateBullets() {
        for (auto& bullet : bullets) bullet.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
                                [](Bullet& b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
        if (active && texture) SDL_RenderCopy(renderer, texture, NULL, &rect);
        for (auto& bullet : bullets) bullet.render(renderer);
    }
};

class Game {
public:
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* backgroundTexture;
    SDL_Texture* wallTexture;
    SDL_Texture* bulletTexture;
    SDL_Texture* playerTexture;
    SDL_Texture* enemyTexture;

    vector<Wall> walls;
    PlayerTank* player;
    int enemyNumber = 3;
    vector<EnemyTank> enemies;

    Game() {
        running = true;
         if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
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
        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        backgroundTexture = loadTexture("background.png", renderer);
        wallTexture = loadTexture("wall.png", renderer);
        bulletTexture = loadTexture("bullet.png", renderer);
        playerTexture = loadTexture("player.png", renderer);
        enemyTexture = loadTexture("enemy.png", renderer);

        generateWalls();
        player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, playerTexture, bulletTexture);
        spawnEnemies();
    }

    ~Game() {
        delete player;
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(wallTexture);
        SDL_DestroyTexture(bulletTexture);
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(enemyTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    void generateWalls() {
        for (int i = 3; i < MAP_HEIGHT - 3; i += 3) {
            for (int j = 3; j < MAP_WIDTH - 3; j += 3) {
                walls.emplace_back(j * TILE_SIZE , i * TILE_SIZE , wallTexture);
            }
        }
    }

    void spawnEnemies() {
    enemies.clear();
    for (int i = 0; i < enemyNumber; ++i) {
        int ex, ey;
        bool valid = false;
        while (!valid) {
            ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
            ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
            SDL_Rect tempRect = {ex, ey, TILE_SIZE, TILE_SIZE};

            valid = true;
            for (const auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&tempRect, &wall.rect)) {
                    valid = false;
                    break;
                }
            }

            if (valid && SDL_HasIntersection(&tempRect, &player->rect)) {
                valid = false;
            }

            if (valid) {
                for (const auto& enemy : enemies) {
                    if (SDL_HasIntersection(&tempRect, &enemy.rect)) {
                        valid = false;
                        break;
                    }
                }
            }
        }

        enemies.emplace_back(ex, ey, enemyTexture, bulletTexture, walls);
    }
}



    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_KEYDOWN) {
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
        for (auto& enemy : enemies) {
            enemy.move(walls);
            enemy.updateBullets();
            if (rand() % 100 < 2) enemy.shoot();
        }

        for (auto& bullet : player->bullets) {
            for (auto& wall : walls)
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    wall.active = false;
                    bullet.active = false;
                    break;
                }
            for (auto& enemy : enemies)
                if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                    enemy.active = false;
                    bullet.active = false;
                    break;
                }
        }

        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active; }), enemies.end());
        if (enemies.empty()) running = false;

        for (auto& enemy : enemies)
            for (auto& bullet : enemy.bullets)
                if (SDL_HasIntersection(&bullet.rect, &player->rect)) {
                    running = false;
                    return;
                }
    }

    void render() {
        if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        for (auto& wall : walls) wall.render(renderer);
        player->render(renderer);
        for (auto& enemy : enemies) enemy.render(renderer);
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
};

int main(int argc, char* argv[]) {
    Game game;
    if (game.running) game.run();
    return 0;
}
