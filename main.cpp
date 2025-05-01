#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <fstream>
#include "wall.h"
#include "constant.h"
#include "Bullet.h"
#include "playertank.h"
#include "enemytank.h"

using namespace std;

// hàm tải texture từ file ảnh:  định nghĩa hàm loadTexture trả về con trỏ kiểu SDL_Texture
// const string& path : đường dẫn đến file ảnh cần tải
// SDL_Renderer* renderer: Con trỏ tới đối tượng renderer => tạo texture từ file
//path.c_str() chuyển chuỗi string thành dạng const char*
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}
int bestScore = 0;
void loadBestScore(const string& filename) {
    ifstream inFile(filename);
    if (inFile.is_open()) {
        inFile >> bestScore;
        inFile.close();
    } else {
        bestScore = 0; // nếu chưa có file
    }
}
void saveBestScore(const string& filename, int score) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << score;
        outFile.close();
    }
}
class Game {
public:
    bool running;
    bool gameWon;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* backgroundTexture;
    SDL_Texture* wallTexture;
    SDL_Texture* bulletTexture;
    SDL_Texture* playerTexture;
    SDL_Texture* player2Texture; // Sửa thêm riêng texture player2
    SDL_Texture* enemyTexture;

    vector<Wall> walls;
    PlayerTank* player;
    PlayerTank* player2;

    int enemyNumber = 10;
    vector<EnemyTank> enemies;
    int score;
    TTF_Font* font;
    TTF_Font* titleFont;
    TTF_Font* menuFont;
    int highScore;
    enum GameState { MENU, PLAYING };
    GameState state;
    bool isTwoPlayer;

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

        if (TTF_Init() < 0) {
            cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
            running = false;
            return;
        }

        font = TTF_OpenFont("arial.ttf", 24);
        titleFont = TTF_OpenFont("arial.ttf", 100);
        menuFont = TTF_OpenFont("arial.ttf", 32);
        if (!font || !titleFont || !menuFont) {
            cerr << "Failed to load fonts: " << TTF_GetError() << endl;
            running = false;
            return;
        }

        backgroundTexture = loadTexture("background.png", renderer);
        wallTexture = loadTexture("wall.png", renderer);
        bulletTexture = loadTexture("bullet.png", renderer);
        playerTexture = loadTexture("player.png", renderer);
        player2Texture = loadTexture("player2.png", renderer); // load player2 riêng
        enemyTexture = loadTexture("enemy.png", renderer);

        if (!backgroundTexture || !wallTexture || !bulletTexture || !playerTexture || !player2Texture || !enemyTexture) {
            cerr << "Failed to load one or more textures." << endl;
            running = false;
            return;
        }

        score = 0;
        ifstream infile("highscore.txt");
        if (infile.is_open()) {
            infile >> highScore;
            infile.close();
        } else {
            highScore = 0;
        }

        generateWalls();
        player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, playerTexture, bulletTexture);
        player2 = new PlayerTank(((MAP_WIDTH - 1) / 2 + 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, player2Texture, bulletTexture);

        spawnEnemies();
        state = MENU;
        isTwoPlayer = false;
    }

    ~Game() {
        delete player;
        delete player2;
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(wallTexture);
        SDL_DestroyTexture(bulletTexture);
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(player2Texture);
        SDL_DestroyTexture(enemyTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_CloseFont(titleFont);
        TTF_CloseFont(menuFont);
        TTF_Quit();
        SDL_Quit();
    }

    void generateWalls() {
        for (int i = 3; i < MAP_HEIGHT - 2; i += 3) {
            for (int j = 3; j < MAP_WIDTH - 2; j += 3) {
                walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE, wallTexture);
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

                if (valid && SDL_HasIntersection(&tempRect, &player->rect)) valid = false;
                if (valid && SDL_HasIntersection(&tempRect, &player2->rect)) valid = false;

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

        else if (event.type == SDL_MOUSEBUTTONDOWN && state == MENU) {
            int x = event.button.x;
            int y = event.button.y;

            // Kiểm tra nếu click vào vùng nút "1 PLAYER"
            if (x >= 300 && x <= 500 && y >= 250 && y <= 290) {
                resetGame(false); // 1 người chơi
            }
            // Nút "2 PLAYER"
            else if (x >= 300 && x <= 500 && y >= 300 && y <= 340) {
                resetGame(true); // 2 người chơi
            }
        }

        else if (event.type == SDL_KEYDOWN && state == PLAYING) {
            switch (event.key.keysym.sym) {
                case SDLK_UP: player->move(0, -5, walls); break;
                case SDLK_DOWN: player->move(0, 5, walls); break;
                case SDLK_LEFT: player->move(-5, 0, walls); break;
                case SDLK_RIGHT: player->move(5, 0, walls); break;
                case SDLK_SPACE: player->shoot(); break;

                case SDLK_w: if (isTwoPlayer) player2->move(0, -5, walls); break;
                case SDLK_s: if (isTwoPlayer) player2->move(0, 5, walls); break;
                case SDLK_a: if (isTwoPlayer) player2->move(-5, 0, walls); break;
                case SDLK_d: if (isTwoPlayer) player2->move(5, 0, walls); break;
                case SDLK_LSHIFT: if (isTwoPlayer) player2->shoot(); break;
            }
        }
    }
}


    void update() {
        player->updateBullets();
        if (player2) player2->updateBullets();

        for (auto& enemy : enemies) {
            enemy.move(walls);
            enemy.updateBullets();
            if (rand() % 100 < 2) enemy.shoot();
        }

        for (auto& bullet : player->Bullets) {
            for (auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    wall.active = false;
                    bullet.active = false;
                    score += 100;
                    break;
                }
            }
            for (auto& enemy : enemies) {
                if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                    enemy.active = false;
                    bullet.active = false;
                    score += 200;
                    break;
                }
            }
        }

        if (isTwoPlayer) {
        for (auto& bullet : player2->Bullets) {
            for (auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    wall.active = false;
                    bullet.active = false;
                    score += 100;
                    break;
                }
            }
            for (auto& enemy : enemies) {
                if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                    enemy.active = false;
                    bullet.active = false;
                    score += 200;
                    break;
                }
            }
        }
        }

        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active; }), enemies.end());

        if (enemies.empty()) {
            running = false;
            gameWon = true;
        }

        for (auto& enemy : enemies) {
    for (auto& bullet : enemy.Bullets) {
        if (SDL_HasIntersection(&bullet.rect, &player->rect) || SDL_HasIntersection(&bullet.rect, &player2->rect)) {
            running = false;
            gameWon = false;
            return;
        }
    }
}


        if (score > highScore) {
            highScore = score;
            ofstream outfile("highscore.txt");
            if (outfile.is_open()) {
                outfile << highScore;
                outfile.close();
            }
        }
    }

    void render() {
        if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        for (auto& wall : walls) wall.render(renderer);
        player->render(renderer);
        if(isTwoPlayer){player2->render(renderer);}
        for (auto& enemy : enemies) enemy.render(renderer);

        SDL_Color color = {255, 255, 255};
        string scoreText = "Score: " + to_string(score);
        SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = {10, 10, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        SDL_RenderPresent(renderer);
    }

    void showEndScreen() {
        bool waiting = true;
        SDL_Event e;
        Uint32 startTime = SDL_GetTicks();

        while (waiting) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
                    waiting = false;
                }
            }

            if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
            for (auto& wall : walls) if (wall.active) wall.render(renderer);
            player->render(renderer);
            player2->render(renderer);
            for (auto& enemy : enemies) if (enemy.active) enemy.render(renderer);

            SDL_Color color = {255, 255, 255};
            string endMessage = gameWon ? "You Win! Score: " + to_string(score) : "You Lose! Score: " + to_string(score);
            SDL_Surface* endSurface = TTF_RenderText_Solid(font, endMessage.c_str(), color);
            SDL_Texture* endTexture = SDL_CreateTextureFromSurface(renderer, endSurface);
            SDL_Rect endRect = {
                SCREEN_WIDTH / 2 - endSurface->w / 2,
                SCREEN_HEIGHT / 2 - endSurface->h / 2,
                endSurface->w,
                endSurface->h
            };
            SDL_RenderCopy(renderer, endTexture, NULL, &endRect);
            SDL_FreeSurface(endSurface);
            SDL_DestroyTexture(endTexture);

            SDL_RenderPresent(renderer);

            if (SDL_GetTicks() - startTime > 5000) { // Tự thoát sau 5s
                waiting = false;
            }
        }
    }
void renderText(const string &message, TTF_Font* font, SDL_Color color, int x, int y, bool center = false) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest;
    dest.w = surface->w;
    dest.h = surface->h;
    dest.x = center ? x - dest.w / 2 : x;
    dest.y = center ? y - dest.h / 2 : y;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

void resetGame(bool isTwoPlayer) {
    state = PLAYING;
    score = 0;
    walls.clear();
    generateWalls();

    // Tạo player 1
    player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, playerTexture, bulletTexture);

    // Nếu 2 người chơi thì tạo player2
    if (isTwoPlayer == true) {
        player2 = new PlayerTank(((MAP_WIDTH - 1) / 2 + 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, player2Texture, bulletTexture);
    }

    spawnEnemies();

}


void showMenuScreen() {
    SDL_Event e;
    SDL_Color titleColor = {180, 200, 255};
    SDL_Color scoreColor = {130, 150, 200};
    SDL_Color buttonColor = {255, 255, 120};        // Màu nút thường
    SDL_Color selectedColor = {255, 240, 100};      // Màu nút được chọn
    SDL_Color buttonTextColor = {30, 50, 80};
    SDL_Color selectedTextColor = {10, 30, 60};     // Màu chữ khi chọn

    SDL_Rect onePlayerRect = {SCREEN_WIDTH / 2 - 150, 260, 300, 70};
    SDL_Rect twoPlayerRect = {SCREEN_WIDTH / 2 - 150, 350, 300, 70};

    int selectedOption = 0;

    while (state == MENU && running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                return;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        selectedOption = 1 - selectedOption; // Chuyển qua lại 0 <-> 1
                        break;
                    case SDLK_RETURN:
                        isTwoPlayer = (selectedOption == 1);
                        resetGame(isTwoPlayer);
                        return;
                    case SDLK_ESCAPE:
                        running = false;
                        return;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x = e.button.x;
                int y = e.button.y;
                if (x >= onePlayerRect.x && x <= onePlayerRect.x + onePlayerRect.w &&
                    y >= onePlayerRect.y && y <= onePlayerRect.y + onePlayerRect.h) {
                    isTwoPlayer = false;
                    resetGame(false);
                    return;
                }else if (x >= twoPlayerRect.x && x <= twoPlayerRect.x + twoPlayerRect.w &&
                           y >= twoPlayerRect.y && y <= twoPlayerRect.y + twoPlayerRect.h) {
                    isTwoPlayer = true;
                    resetGame(true);
                    return;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 10, 30, 70, 255);
        SDL_RenderClear(renderer);

        renderText("BATTLE CITY", titleFont, titleColor, SCREEN_WIDTH / 2, 160, true);

        // --- Vẽ nút 1 PLAYER ---
        if (selectedOption == 0) {
            SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, 255);
            SDL_RenderFillRect(renderer, &onePlayerRect);
            renderText("1 PLAYER", menuFont, selectedTextColor, SCREEN_WIDTH / 2, 295, true);
        } else {
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, 255);
            SDL_RenderFillRect(renderer, &onePlayerRect);
            renderText("1 PLAYER", menuFont, buttonTextColor, SCREEN_WIDTH / 2, 295, true);
        }

        // --- Vẽ nút 2 PLAYER ---
        if (selectedOption == 1) {
            SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, 255);
            SDL_RenderFillRect(renderer, &twoPlayerRect);
            renderText("2 PLAYER", menuFont, selectedTextColor, SCREEN_WIDTH / 2, 385, true);
        } else {
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, 255);
            SDL_RenderFillRect(renderer, &twoPlayerRect);
            renderText("2 PLAYER", menuFont, buttonTextColor, SCREEN_WIDTH / 2, 385, true);
        }

        renderText("BEST SCORE", menuFont, scoreColor, SCREEN_WIDTH / 2, 470, true);
        renderText(to_string(highScore), menuFont, scoreColor, SCREEN_WIDTH / 2, 510, true);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}



  void run() {
    while (running) {
        if (state == MENU) {
            showMenuScreen();
        } else if (state == PLAYING) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);
        }
    }

    showEndScreen(); // khi thoát khỏi vòng lặp, hiện màn hình kết thúc

}

};
int main(int argc, char* argv[]) {
    // Khởi tạo SDL (bao gồm âm thanh)
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    // tải và phát nhạc nền
    Mix_Music* music = Mix_LoadMUS("music.wav");
    if (!music) {
        SDL_Log("Fail to open music file", Mix_GetError());
    } else {
        Mix_PlayMusic(music, -1);
    }

    // Khởi động game
    Game game;
    if (game.running) game.run();

    // Giải phóng nhạc và tắt SDL sau khi game kết thúc
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}

