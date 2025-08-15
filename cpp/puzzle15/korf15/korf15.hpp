#pragma once
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../puzzle15.hpp"

namespace korf15 {

// Korfの15パズル問題集を読み込む
inline std::vector<puzzle15::Puzzle> load_korf_problems(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open " + filename); // 例外を投げる
    }

    std::vector<puzzle15::Puzzle> problems;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        int idx;
        iss >> idx; // 問題のインデックスを読み飛ばす
        std::array<uint8_t, 16> tiles{};
        for (int i = 0; i < 16; ++i) {
            int v;
            iss >> v;
            tiles[i] = static_cast<uint8_t>(v); // 8ビット整数に変換
        }
        puzzle15::Puzzle p;
        p.packed = 0;
        for (int i = 0; i < 16; ++i) {
            puzzle15::Puzzle::set_nibble(p.packed, i, tiles[i]); // 4ビットごとに詰める
            if (tiles[i] == 0) {
                p.zero_pos = static_cast<uint8_t>(i); // ゼロタイルの位置を記録
            }
        }
        problems.push_back(p);
    }
    return problems;
}

} // namespace korf15
