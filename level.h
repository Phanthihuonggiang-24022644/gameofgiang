#ifndef LEVEL_H
#define LEVEL_H


#include <vector>

struct LevelData {
    std::vector<std::pair<int, int>> wallPositions; // Vị trí của các bức tường
    int enemyCount; // Số lượng kẻ địch
};

LevelData getLevelData(int level); // Lấy dữ liệu màn chơi từ số màn

#endif
