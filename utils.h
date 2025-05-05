
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <SDL.h>

// Hàm load texture
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);

// Hàm xử lý điểm cao
void loadBestScore(const std::string& filename, int& bestScore);
void saveBestScore(const std::string& filename, int score);

#endif
