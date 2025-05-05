#include "utils.h"
#include <SDL_image.h>
#include <iostream>
#include <fstream>

// hàm tải texture từ file ảnh:  định nghĩa hàm loadTexture trả về con trỏ kiểu SDL_Texture
// const string& path : đường dẫn đến file ảnh cần tải
// SDL_Renderer* renderer: Con trỏ tới đối tượng renderer => tạo texture từ file
//path.c_str() chuyển chuỗi string thành dạng const char*

SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        std::cerr << "Failed to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    return newTexture;
}

void loadBestScore(const std::string& filename, int& bestScore) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        inFile >> bestScore;
        inFile.close();
    } else {
        bestScore = 0; // nếu chưa có file
    }
}

void saveBestScore(const std::string& filename, int score) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << score;
        outFile.close();
    }
}
