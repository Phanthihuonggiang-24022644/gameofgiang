#include "level.h"
#include <fstream>
#include <sstream>
#include <iostream>

LevelData getLevelData(int level) {
    LevelData data;

    // Giới hạn level chỉ từ 1 đến 3
    if (level < 1 || level > 3) {
        return data; // Trả về dữ liệu trống
    }

    std::ifstream file("level" + std::to_string(level) + ".txt");

    if (!file.is_open()) {
        std::cerr << "Failed to open level file!" << std::endl;
        return data;
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line)) {
        for (int col = 0; col < line.length(); ++col) {
            if (line[col] == '1') {
                data.wallPositions.push_back({col, row});
            }
        }
        ++row;
    }

    file.close();

    data.enemyCount = 5 + (level - 1) * 2;

    return data;
}
