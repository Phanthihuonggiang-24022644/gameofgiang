#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <fstream>


using namespace std;
// các hằng số cấu hình game
const int SCREEN_WIDTH = 880; // chiều rộng màn hình
const int SCREEN_HEIGHT = 640; // chiều cao màn hình
const int TILE_SIZE = 40; // kích thước mỗi ô vuông
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE; // số ô theo chiều ngang
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // số ô theo chiều cao
int bestScore = 0;


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

// lớp wall trong game
class Wall {
public:
    int x, y; // tọa độ của wall
    SDL_Rect rect; // đại diện cho texture trên màn hình
    bool active; // trạng thái có hiệu lực không?
    SDL_Texture* texture; // con trỏ trỏ tới hình ảnh của wall

    //hàm khởi tạo wall
    Wall(int startX, int startY, SDL_Texture* tex) {
        x = startX;
        y = startY;
        active = true;
        rect = {x, y, TILE_SIZE , TILE_SIZE}; //tạo vùng vẽ cho SDL
        texture = tex; //lưu con trỏ texture để vẽ
    }

    // hàm vẽ tường ra màn hình : chỉ vẽ nếu wall còn hoạt động và texture hợp lệ
    void render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);// vẽ ảnh từ con trỏ renderer vào vị trí rect
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
        x = startX; // vị trí ban đầu
        y = startY;
        dx = dirX; // hướng bay
        dy = dirY;
        active = true;
        texture = tex;
        rect = {x, y, 20, 20}; // vùng hiển thị
    }

    // di chuyển của đạn
    void move() {
        x += dx; // cập nhật tọa độ
        y += dy;
        rect.x = x; // đồng bộ lại hình vẽ
        rect.y = y;
        // nếu đạn ra khỏi màn hình => tắt
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE ||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false;
        }
    }

    // vẽ đạn và xoay hướng
    void render(SDL_Renderer* renderer) {
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

};


class PlayerTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    vector<Bullet> bullets; // danh sách đạn đang bắn

    // hàm khởi tạo
    PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* tex, SDL_Texture* bulletTex) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
        texture = tex;
        bulletTexture = bulletTex;
    }

    // di chuyển xe tăng
    void move(int dx, int dy, const vector<Wall>& walls) {
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
    void shoot() {
       bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);

    }

    // duyệt qua từng viên đạn, cho đạn di chuyển
    //remove_if(...): tìm tất cả các viên đạn mà !b.active (không còn hoạt động xóa khỏi vector bullet

    void updateBullets() {
        for (auto& bullet : bullets) bullet.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
                                [](Bullet& b) { return !b.active; }), bullets.end());
    }
// vẽ xe tăng có xoay hướng
    void render(SDL_Renderer* renderer) {
    if (texture) {
        double angle = 0.0;
        if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
        else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
        else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
        else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }
// vẽ đạn
    for (auto& bullet : bullets) bullet.render(renderer);
}

};

class EnemyTank {
public:
    int x, y, dirX, dirY, moveDelay, shootDelay;
    // moveDelay : delay giữa mỗi lần thay đổi hướng di chuyển
    // shootDelay : delay giữa mỗi lần bắn đạn
    SDL_Rect rect;
    bool active;
    vector<Bullet> bullets;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture; // danh sách đạn đã bắn ra

    EnemyTank(int startX, int startY, SDL_Texture* tex, SDL_Texture* bulletTex, const vector<Wall>& walls) {
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


    void move(const vector<Wall>& walls) {
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


    void shoot() {
        if (--shootDelay > 0) return; // giảm biến shootdelay sau mỗi lần được gọi, Nếu chưa về 0 thì không bắn, thoát hàm sớm.
        shootDelay = 20;
        bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);
    }

    void updateBullets() {
        for (auto& bullet : bullets) bullet.move();
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
                                [](Bullet& b) { return !b.active; }), bullets.end());
    }
    // vẽ enemy
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

class Game {
public:
    bool running; // trạng thái game đang chạy
    bool gameWon;
    SDL_Window* window; // cửa số game
    SDL_Renderer* renderer; // bộ vẽ đồ họa
    SDL_Texture* backgroundTexture; //background
    SDL_Texture* wallTexture; // wall
    SDL_Texture* bulletTexture; // đạn
    SDL_Texture* playerTexture; // player
    SDL_Texture* enemyTexture; // enemy

    vector<Wall> walls; // danh sách wall
    PlayerTank* player; // xe tăng người chơi
    int enemyNumber = 10; // số lượng xe tăng địch
    vector<EnemyTank> enemies; // danh sách xe tăng địch
    int score;
    TTF_Font* font;
    enum GameState { MENU, PLAYING }; // định nghĩa trạng thái
    TTF_Font* titleFont;
    TTF_Font* menuFont;
    int highScore; // hoặc bạn có thể đọc từ file
    GameState state;
    bool isTwoPlayer;


    // hàm khởi tạo game
    Game() {
        running = true; // báo game chạy
        // khởi tạo SDL : SDL_Init() với SDL_INIT_VIDEO để khởi tạo hệ thống đồ họa video
         if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        // tạo cửa sổ game : tiêu đề battle city, đặt giữa màn hình, nễu lỗi kết thúc.
        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        // tạo renderer là bộ vẽ đồ họa dùng để hiển thị texture lên cửa sổ
        // SDL_RENDERER_ACCELERATED: Yêu cầu dùng hardware (GPU) để tăng tốc vẽ
        // -1 card đồ họa
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }
        // khởi tạo SDL_ttf
        if (TTF_Init() < 0) {
                cout << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
                running = false;
                return;
            }
            // mở font chữ
            font = TTF_OpenFont("arial.ttf", 24);

            if (!font) {
            cout << "Failed to load font: " << TTF_GetError() << endl;
            running = false;
            return;
            }
            titleFont = TTF_OpenFont("arial.ttf", 100);
            if (!titleFont) {
                cout << "Failed to load title font: " << TTF_GetError() << endl;
            running = false;
            return;
            }
            menuFont = TTF_OpenFont("arial.ttf", 32);
            if (!menuFont) {
                    cout << "Failed to load menu font: " << TTF_GetError() << endl;
                    running = false;
                    return;
}

        score = 0;
        // Đọc highScore từ file
        ifstream infile("highscore.txt");
        if (infile.is_open()) {
                infile >> highScore;
        infile.close();
        } else {
            highScore = 0; // nếu không có file thì mặc định là 0
            }
        // chọn texture cho các hàm
        backgroundTexture = loadTexture("background.png", renderer);
        wallTexture = loadTexture("wall.png", renderer);
        bulletTexture = loadTexture("bullet.png", renderer);
        playerTexture = loadTexture("player.png", renderer);
        enemyTexture = loadTexture("enemy.png", renderer);

        // xây tường
        generateWalls();
        // tạo xe tăng người chơi : x = (MAP_WIDTH - 1)/2 * TILE_SIZE: cột giữa map
        // y = (MAP_HEIGHT - 2) * TILE_SIZE: gần dưới đáy
        // truyền tham số vào
        player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, playerTexture, bulletTexture);
        // sinh ra xe tăng địch
        spawnEnemies();
        state = MENU; // trạng thái ban đầu là menu
        isTwoPlayer = false;
    }
    // hàm giải phóng game
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
        TTF_CloseFont(font);
        TTF_Quit();
    }

    // tạo các bức tường : Cứ mỗi 3 ô thì bạn đặt 1 bức tường
    void generateWalls() {
        for (int i = 3; i < MAP_HEIGHT - 2; i += 3) {
            for (int j = 3; j < MAP_WIDTH - 2; j += 3) {
                walls.emplace_back(j * TILE_SIZE , i * TILE_SIZE , wallTexture); //j * TILE_SIZE và i * TILE_SIZE: chuyển từ tọa độ bản đồ sang tọa độ pixel
            }
        }
    }

    void spawnEnemies() {
    enemies.clear(); // xóa danh sách địch cũ
    // vòng lặp sinh xe tăng địch
    for (int i = 0; i < enemyNumber; ++i) {
        int ex, ey;
        bool valid = false;
        while (!valid) {
            // tạo ví trí ngẫu nhiên: chọn ngẫu nhiên cột từ 1 đến MAP_WIDTH - 2 (tránh mép bản đồ)
            ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
            ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
            SDL_Rect tempRect = {ex, ey, TILE_SIZE, TILE_SIZE};

            valid = true;
            // kiểm tra va chạm với tường
            for (const auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&tempRect, &wall.rect)) {
                    valid = false;
                    break;
                }
            }

            // kiểm tra va chạm với người chơi
            if (valid && SDL_HasIntersection(&tempRect, &player->rect)) {
                valid = false;
            }

            // kiểm tra va chạm enemy khác
            if (valid) {
                for (const auto& enemy : enemies) {
                    if (SDL_HasIntersection(&tempRect, &enemy.rect)) {
                        valid = false;
                        break;
                    }
                }
            }
        }

        // nếu hợp lệ : thêm vào danh sách
        enemies.emplace_back(ex, ey, enemyTexture, bulletTexture, walls);
    }
}



    void handleEvents() {
        SDL_Event event; // event kiểu SDL_Event, dùng để chứa thông tin về sự kiện mà SDL sẽ gửi
        while (SDL_PollEvent(&event)) {
            // SDL_PollEvent() sẽ kiểm tra xem có sự kiện nào cần xử lý hay không
            // Nếu có, nó sẽ đẩy sự kiện vào event và trả về true
            if (event.type == SDL_QUIT) running = false; //Kiểm tra nếu sự kiện là yêu cầu thoát game => kết thúc
            // nếu sự kiện là người dùng nhấn một phím
            else if (event.type == SDL_KEYDOWN) {
                // kiểm tra phím mà người chơi đã nhấn và thực hiện hành động thích hợp
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

    // cập nhật logic game
    void update() {
    player->updateBullets();  // Cập nhật tất cả đạn đã bắn

    // Cập nhật các kẻ thù
    for (auto& enemy : enemies) {
        enemy.move(walls);  // Kẻ thù di chuyển
        enemy.updateBullets();  // Kẻ thù cập nhật đạn
        if (rand() % 100 < 2) enemy.shoot(); // 2% cơ hội để kẻ thù bắn
    }

    // Kiểm tra va chạm giữa đạn và tường
    for (auto& bullet : player->bullets) {
        for (auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                wall.active = false;  // Tường bị phá hủy
                bullet.active = false;  // Đạn biến mất
                score += 100;  // Cộng điểm
                break;  // Dừng vòng lặp để tránh cộng điểm nhiều lần cho cùng một viên đạn
            }
        }

        // Kiểm tra va chạm giữa đạn và kẻ thù
        for (auto& enemy : enemies) {
            if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                enemy.active = false;  // Kẻ thù bị tiêu diệt
                bullet.active = false;  // Đạn bị vô hiệu hóa
                score += 200;  // Cộng điểm cho việc tiêu diệt kẻ thù
                break;  // Dừng vòng lặp để tránh cộng điểm nhiều lần cho cùng một viên đạn
            }
        }
    }

    // Xóa các kẻ thù không còn hoạt động
    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active; }), enemies.end());

    // Kiểm tra thắng
    if (enemies.empty()) {
        running = false;  // Kết thúc game nếu không còn kẻ thù
        gameWon = true;  // Đánh dấu game thắng
    }

    // Kiểm tra xem kẻ thù có bắn trúng người chơi không
    for (auto& enemy : enemies) {
        for (auto& bullet : enemy.bullets) {
            if (SDL_HasIntersection(&bullet.rect, &player->rect)) {
                running = false;  // Kết thúc game nếu người chơi bị bắn
                gameWon = false;  // Đánh dấu game thua
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

    // vẽ game
    void render() {
    if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    for (auto& wall : walls) wall.render(renderer);
    player->render(renderer);
    for (auto& enemy : enemies) enemy.render(renderer);

    // Hiển thị điểm số
    SDL_Color color = {255, 255, 255}; // màu trắng
    string scoreText = "Score: " + to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {10, 10, surface->w, surface->h}; // vị trí góc trái
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer); // Hiển thị lên màn hình thật
}
// hiển thị màn hình cuối cùng của game
void showEndScreen() {
    bool waiting = true; // là cờ để lặp vòng đợi cho đến khi người chơi nhấn phím hoặc đủ thời gian
    SDL_Event e; // dùng để lấy sự kiện từ người dùng
    Uint32 startTime = SDL_GetTicks(); //startTime lưu thời điểm bắt đầu hiển thị màn hình kết thúc

    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
                waiting = false; // người chơi thoát hoặc nhấn phím => thoát vòng lặp
            }
        }

        // Vẽ lại toàn bộ khung cảnh cuối
        if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        for (auto& wall : walls) if (wall.active) wall.render(renderer);
        player->render(renderer);
        for (auto& enemy : enemies) if (enemy.active) enemy.render(renderer);

        // Vẽ điểm số
        SDL_Color color = {255, 255, 255}; //white
        string scoreText = "Score: " + to_string(score);
        SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
        //TTF_RenderText_Solid để tạo surface, rồi chuyển sang texture để vẽ bằng SDL
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        // // Tạo một hình chữ nhật textRect định vị trí và kích thước để vẽ chữ lên màn hình
        SDL_Rect textRect = {10, 10, surface->w, surface->h};
        // viết chữ vào vị trí textrect
        SDL_RenderCopy(renderer, texture, NULL, &textRect);
        SDL_FreeSurface(surface);
        // giải phóng bộ nhớ sau khi vẽ
        SDL_DestroyTexture(texture);

        // Vẽ thông báo kết thúc
        string endMessage;
        if (gameWon) {
                endMessage = "You Win! Score: " + to_string(score);
        } else {
            endMessage = "You Lose! Score: " + to_string(score);
            }
        //Vẽ chuỗi endMessage thành một SDL_Surface dùng font đã tải và màu đã chọn
         SDL_Surface* endSurface = TTF_RenderText_Solid(font, endMessage.c_str(), color);
         //Chuyển surface sang texture để có thể render lên màn hình
        SDL_Texture* endTexture = SDL_CreateTextureFromSurface(renderer, endSurface);
        // Tính toán hình chữ nhật để vẽ chữ ở giữa màn hình
        SDL_Rect endRect = {
            SCREEN_WIDTH / 2 - endSurface->w / 2,
            SCREEN_HEIGHT / 2 - endSurface->h / 2,
            endSurface->w,
            endSurface->h
        };
        // Vẽ texture của dòng chữ kết thúc vào đúng vị trí đã tính
        SDL_RenderCopy(renderer, endTexture, NULL, &endRect);
        SDL_FreeSurface(endSurface);
        SDL_DestroyTexture(endTexture);

        SDL_RenderPresent(renderer);

        // Đợi 5 giây hoặc đến khi người dùng thoát
        if (SDL_GetTicks() - startTime > 5000) {
            waiting = false;
        }

        SDL_Delay(100);
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
                        resetGame();
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
                    resetGame();
                    return;
                } else if (x >= twoPlayerRect.x && x <= twoPlayerRect.x + twoPlayerRect.w &&
                           y >= twoPlayerRect.y && y <= twoPlayerRect.y + twoPlayerRect.h) {
                    isTwoPlayer = true;
                    resetGame();
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

void resetGame() {
    score = 0;
    walls.clear();
    generateWalls();
    delete player;
    player = new PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE, renderer, playerTexture, bulletTexture);
    spawnEnemies();
    state = PLAYING;
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

